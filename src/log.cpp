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

Log::Log(const std::string& keyspaceName) : keyspaceName(keyspaceName) {
    // Logs (full changelogs) are owned by DB (store) objects
    this->dbFilepath = std::format("logs/{}.db", this->keyspaceName);
    this->logFilepath = std::format("logs/{}.aof", this->keyspaceName);

    this->fileDescriptor = ::open(this->logFilepath.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (this->fileDescriptor == -1) {
        throw std::runtime_error("Failed to open log file descripter.");
    }

    this->logfile.open(this->logFilepath, std::ios::app);
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

void Log::compactLog(const std::unordered_map<std::string, std::string>& state, const size_t dirty) {
    // Compacts the Append-Only Log to reduce old or redundant queries
    if (dirty <= 0) {
        // Skip writes if no changes have been made, unless no snapshots are saved, in which case generate one
        return;
    }
    std::string tempAof = std::format("logs/{}_temp.aof", this->keyspaceName);
    std::string buffer;
    buffer.reserve(10000000);

    // Write to temporary timestamped .aof for future replacement
    if (!state.empty()) {
        std::ofstream writer(tempAof);
        if (writer.is_open()) {
            for (const std::pair<const std::string, std::string>& pair : state) {
                buffer += "put " + pair.first + " " + pair.second + "\n";
            }
            writer << buffer;
            writer.close();
        }
    }
    std::filesystem::rename(tempAof, this->logFilepath);
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