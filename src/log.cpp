#include "taproot/log.h"
#include "taproot/utils.h"
#include "lz4.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <utility>
#include <cstdint>
#include <sstream>
#include <chrono>
#include <filesystem>
#include <mutex>
#include <atomic>
#include <thread>
#include <fcntl.h>
#include <unistd.h> 

Log::Log(const std::string& keyspaceName) : keyspaceName(keyspaceName), aofCount(0) {
    // Logs (full changelogs) are owned by DB (store) objects
    this->dbFilepath = std::format("logs/{}.db", this->keyspaceName);
    this->logFilepath = std::format("logs/{}.log", this->keyspaceName);

    this->fileDescriptor = ::open(this->logFilepath.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (this->fileDescriptor == -1) {
        throw std::runtime_error("Failed to open log file descripter.");
    }

    bool logExists = std::filesystem::exists(this->logFilepath);
    this->logfile.open(this->logFilepath, std::ios::app);
    if (!logExists) {
        this->catchupLog();
    }
    this->startFlushThread();
}

Log::~Log() {
    if (!this->stopFlushThread) {
        this->closeLog();
    }
}

void Log::startFlushThread() {
    // Start background thread for the append buffer when writing commands to changelog for everysec
    this->flushThread = std::thread([this]() {
        std::unique_lock<std::mutex> condivarLock(this->condivarMutex);
        while (!this->stopFlushThread) {
            this->condivar.wait_for(condivarLock, std::chrono::seconds(1));

            std::string localBuffer;
            {
                // Introduce curly-brace block here for scope; once out-of-scope, mutex deconstructs and unlocks
                std::lock_guard<std::mutex> lock(this->logMutex);
                if (this->logBuffer.empty()) {
                    continue;
                }
                localBuffer.swap(this->logBuffer);
            }

            this->logfile.write(localBuffer.data(), localBuffer.size());
            this->logfile.flush();
            ::fsync(this->fileDescriptor);
        }
    });
}

void Log::catchupLog() {
    // Rebuild start of log if it is missing based off of latest compacted AOF snapshot
    std::unordered_map<std::string, std::string> parseMap;
    std::string line;
    std::string snapshotFilename = this->getLatestSnapshot();
    if (!std::filesystem::is_regular_file(snapshotFilename)) {
        // Snapshot does not exist
        return;
    }
    std::ifstream reader(snapshotFilename);
    if (reader.is_open()) {
        while (getline(reader, line)) {
            std::vector<std::string> tokens = tokenize(line);
            parseMap[tokens[1]] = tokens[2]; // Assume "put" commands since compacted
        }
        reader.close();
    }
    for (const std::pair<const std::string, std::string>& pair : parseMap) {
        this->logfile << "put " + pair.first + " " + pair.second + "\n";
    }
}

void Log::appendCommand(Command cmd, const std::string& key, const std::string& value) {
    // Append and flush the "put"/"del" query to the log file
    if (!this->logfile.is_open()) {
        std::cout << std::format("\nError: Logfile {} has not been opened.\n", this->keyspaceName) << std::endl;
        return;
    }

    std::string line;
    line.reserve(256);
    switch (cmd) {
        case Command::PUT:
            line = "put " + key + " " + value + "\n";
            break;
        case Command::DEL:
            line = "del " + key + "\n";
            break;
        default:
            throw std::runtime_error("Cannot recognize command to be appended.");
    }
    std::lock_guard<std::mutex> lock(this->logMutex);
    this->logBuffer += line;
}

void Log::writeBinarySnapshot(const std::unordered_map<std::string, std::string>& state) {
    // Takes the current final state (equivalent to compacted human-readable snapshot) and serializes to binary for performance
    // The keyspace state is passed by reference from owner DB instance
    // Binary format: [MAGIC][isCompressed bit][Keyspace size][Key1 size][Key1 data][Value1 size][Value1 data]...
    std::ostringstream datastream;
    size_t keyspaceSize = state.size();
    const char* MAGIC = "TDB";
    datastream.write(MAGIC, 3);
    datastream.write(reinterpret_cast<const char*>(&keyspaceSize), sizeof(keyspaceSize));

    // Write to string stream
    for (const auto& [key, value] : state) {
        size_t keySize = key.size();
        size_t valSize = value.size();

        datastream.write(reinterpret_cast<const char*>(&keySize), sizeof(keySize));
        datastream.write(key.data(), keySize);
        datastream.write(reinterpret_cast<const char*>(&valSize), sizeof(valSize));
        datastream.write(value.data(), valSize);
    }

    std::ofstream writer(this->dbFilepath, std::ios::binary);

    // Write stream contents to file, using compression if needed
    if (writer.is_open()) {
        std::string serializedDatastream = datastream.str();
        size_t serialSize = serializedDatastream.size();

        if (serialSize > this->COMPRESSION_THRESHOLD) {
            // Serialized final state passes threshold, so attempt to compress
            std::vector<uint8_t> input(serializedDatastream.begin(), serializedDatastream.end());
            std::vector<uint8_t> compressedDatastream = this->compress(input);
            size_t compressedSize = compressedDatastream.size();
            size_t uncompressedSize = input.size();

            uint8_t flag = (compressedSize < uncompressedSize) ? 1 : 0; // 1 if compression works; write to beginning of file
            writer.write(reinterpret_cast<const char*>(&flag), 1);

            if (flag) {
                // Compression was effective enough
                writer.write(reinterpret_cast<const char*>(&uncompressedSize), sizeof(uncompressedSize)); // Original size for future decompression
                writer.write(reinterpret_cast<const char*>(compressedDatastream.data()), static_cast<std::streamsize>(compressedSize));
            }
            else {
                // Compression did not do anything
                writer.write(reinterpret_cast<const char*>(input.data()), static_cast<std::streamsize>(uncompressedSize));
            }
        }
        else {
            // If not state not big enough to compress, just serialize and write to .db
            writer.write(serializedDatastream.data(), static_cast<std::streamsize>(serialSize));
        }
        writer.close();
    }
}

void Log::updateAofCount() {
    // Update count of .aof files (compacted human-readable snapshots) for log management purposes
    size_t count = 0;
    for (const auto& entry : std::filesystem::directory_iterator("logs")) {
        std::string filename = entry.path().filename().string();
        if (filename.starts_with(this->keyspaceName + "_") && filename.ends_with(".aof")) {
            count++;
        }
    }
    this->aofCount = count;
}

std::string Log::getLatestSnapshot() {
    // Searches the logs directory to find the latest .aof snapshot containing the keyspace name
    std::string latestSnapshot;
    for (const auto& entry : std::filesystem::directory_iterator("logs")) {
        std::string filename = entry.path().filename().string();
        if (filename.starts_with(this->keyspaceName + "_") && filename.ends_with(".aof")) {
            if (filename > latestSnapshot) {
                latestSnapshot = filename;
            }
        }
    }
    return "logs/" + latestSnapshot;
}

std::string Log::getEarliestSnapshot() {
    // Searches the logs directory to find the earliest .aof snapshot containing the keyspace name to overwrite/archive
    // Note that sorting would be O(n log n) and linearly checking would simply be O(n), unless already sorted
    std::string earliestSnapshot = "";
    for (const auto& entry : std::filesystem::directory_iterator("logs")) {
        std::string filename = entry.path().filename().string();
        if (filename.starts_with(this->keyspaceName + "_") && filename.ends_with(".aof")) {
            if (earliestSnapshot.empty() || filename < earliestSnapshot) {
                earliestSnapshot = filename;
            }
        }
    }
    return "logs/" + earliestSnapshot;
}

void Log::rotateLogs() {
    // Manage DB Append-Only File (.aof) logs by rotating, archiving, deleting, or overwriting old logs. Limit max .aof files to 5 per keyspace
    this->updateAofCount();
    if (this->aofCount > MAX_AOF_FILES) {
        std::string earliestSnapshotFilename = this->getEarliestSnapshot();
        if (!std::filesystem::remove(earliestSnapshotFilename)) {
            std::perror("Cannot delete .aof file.");
            exit(1);
        }
    }

    // Rewrite whole log to latest compacted state when threshold is reached. Optionally archive old log
    if (std::filesystem::is_regular_file(this->logFilepath)) {
        size_t logFilesize = std::filesystem::file_size(this->logFilepath);
        if (logFilesize > ROTATION_THRESHOLD) {
            // TODO: Archive old .log file and compress using LZ4 on human-readable
            std::string latestSnapshotFilename = this->getLatestSnapshot();
            size_t latestSnapshotFilesize = std::filesystem::file_size(latestSnapshotFilename);
            if (std::filesystem::is_regular_file(latestSnapshotFilename) && logFilesize > latestSnapshotFilesize) {
                std::filesystem::copy(latestSnapshotFilename, this->logFilepath, std::filesystem::copy_options::overwrite_existing); // Duplicate and replace/fill, not rename
            }
        }
    }
}

void Log::compactLog(const std::unordered_map<std::string, std::string>& state, const bool hasBeenAltered) {
    // Compacts the Append-Only Log to reduce old or redundant queries. Used before compression
    // Also assumes no invalid commands were permitted into the log
    std::string latestSnapshotFilename = this->getLatestSnapshot();
    if (!hasBeenAltered && std::filesystem::is_regular_file(latestSnapshotFilename)) {
        // Skip writes if no changes have been made, unless no snapshots are saved, in which case generate one
        return;
    }
    std::string snapshotFilename = std::format("logs/{}_{}.aof", this->keyspaceName, getTimestamp());
    std::string buffer;
    buffer.reserve(10000000);
    if (this->logfile.is_open()) {
        throw std::runtime_error("Cannot compact logfile that is still open.");
    }

    // Write to timestamped snapshot (replayability)
    if (!state.empty()) {
        std::ofstream writer(snapshotFilename);
        if (writer.is_open()) {
            for (const std::pair<const std::string, std::string>& pair : state) {
                buffer += "put " + pair.first + " " + pair.second + "\n";
            }
            writer << buffer;
            writer.close();
        }
    }

    // Write to binary snapshot as well
    this->writeBinarySnapshot(state);
}

std::vector<uint8_t> Log::compress(const std::vector<uint8_t>& input) {
    // Wrapper to compress the binary-serialized compacted snapshot further using LZ4 compression
    if (input.empty()) {
        return {};
    }

    int maxCompressedSize = LZ4_compressBound(input.size());
    std::vector<uint8_t> compressed(maxCompressedSize);
    int compressedSize = LZ4_compress_default(reinterpret_cast<const char*>(input.data()), // Consider LZ4_compress_destSize here as well
        reinterpret_cast<char*>(compressed.data()),
        static_cast<int>(input.size()),
        maxCompressedSize);

    if (compressedSize <= 0) {
        throw std::runtime_error("Cannot compress data.");
    }

    compressed.resize(compressedSize);
    return compressed;
}

void Log::closeLog() {
    // Close the logfile including file descriptor and threads
    this->stopFlushThread = true;
    this->condivar.notify_one(); // If thread in between 1s interval, wake to close immediately
    if (flushThread.joinable()) {
        flushThread.join();
    }

    {
        std::lock_guard<std::mutex> lock(this->logMutex);
        if (!this->logBuffer.empty()) {
            // Check if a changelog append was added to buffer one final time
            this->logfile.write(this->logBuffer.data(), this->logBuffer.size());
            this->logfile.flush();
            ::fsync(this->fileDescriptor);
            this->logBuffer.clear();
        }
    }

    if (this->logfile.is_open()) {
        this->logfile.close();
    }
    if (this->fileDescriptor != -1) {
        ::close(this->fileDescriptor);
        this->fileDescriptor = -1;
    }
}