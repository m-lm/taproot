#include "log.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <utility>

Log::Log(const std::string& keyspaceName) : keyspaceName(keyspaceName) {
    // Logs (full changelogs) are owned by DB (store) objects
    this->logfile.open("logs/" + keyspaceName + ".log", std::ios::app);
}

Log::~Log() {
}

void Log::appendPut(const std::string& key, const std::string& value) {
    // Append the "put" query to the log file
    if (this->logfile.is_open()) {
        this->logfile << std::format("put {} {}\n", key, value);
    }
    else {
        std::cout << std::format("\nError: Logfile {} has not been opened.\n", this->keyspaceName) << std::endl;
    }
}

void Log::appendDelete(const std::string& key) {
    // Append the "del" query to the log file
    if (this->logfile.is_open()) {
        this->logfile << std::format("del {}\n", key);
    }
    else {
        std::cout << std::format("\nError: Logfile {} has not been opened.\n", this->keyspaceName) << std::endl;
    }
}

void Log::writeBinarySnapshot(const std::unordered_map<std::string, std::string>& state) {
    // Takes the current final state (equivalent to compacted human-readable snapshot) and serializes to binary for performance
    // The keyspace state is passed by reference from owner DB instance
    // Binary format: [Keyspace size][Key1 size][Key1 data][Value1 size][Value1 data]...
    std::string binaryFilename = std::format("logs/{}.db", this->keyspaceName);
    std::ofstream writer(binaryFilename, std::ios::binary);
    if (writer.is_open()) {
        const char* MAGIC = "TDB";
        size_t keyspaceSize = state.size();

        writer.write(MAGIC, 3);
        writer.write(reinterpret_cast<const char*>(&keyspaceSize), sizeof(keyspaceSize));

        // Key-values
        for (const auto& [key, value] : state) {
            size_t keySize = key.size();
            size_t valSize = value.size();

            writer.write(reinterpret_cast<const char*>(&keySize), sizeof(keySize));
            writer.write(key.data(), keySize);
            writer.write(reinterpret_cast<const char*>(&valSize), sizeof(valSize));
            writer.write(value.data(), valSize);
        }
        writer.close();
    }
}

std::string Log::getLatestSnapshot(const std::string& keyspace) {
    // Searches the logs directory to find the latest .dat snapshot containing the keyspace name
    std::string latestSnapshot;
    for (const auto& entry : std::filesystem::directory_iterator("logs")) {
        std::string filename = entry.path().filename().string();
        if (filename.starts_with(keyspace + "_") && filename.ends_with(".dat")) {
            if (filename > latestSnapshot) {
                latestSnapshot = filename;
            }
        }
    }
    return "logs/" + latestSnapshot;
}

void Log::compactLog() {
    // Compacts the Append-Only Log to reduce old or redundant queries. Used before compression
    // Also assumes no invalid commands were permitted into the log
    std::string changelogFilename = std::format("logs/{}.log", this->keyspaceName);
    std::string snapshotFilename = std::format("logs/{}_{}.dat", this->keyspaceName, getTimestamp());
    if (this->logfile.is_open()) {
        throw std::runtime_error("Cannot compact logfile that is still open.");
    }

    std::unordered_map<std::string, std::string> parseMap;
    std::string line;
    std::ifstream reader(changelogFilename);
    if (reader.is_open()) {
        while (getline(reader, line)) {
            std::vector<std::string> tokens = tokenize(line);
            if (tokens[0] == "put") {
                parseMap[tokens[1]] = tokens[2];
            }
            else if (tokens[0] == "del") {
                parseMap.erase(tokens[1]);
            }
            else {
                throw std::runtime_error("Invalid operator keyword detected in logfile.");
            }
        }
        reader.close();
    }

    // Write to timestamped snapshot (replayability)
    if (!parseMap.empty()) {
        std::ofstream writer(snapshotFilename);
        if (writer.is_open()) {
            for (const std::pair<const std::string, std::string>& pair : parseMap) {
                writer << std::format("put {} {}\n", pair.first, pair.second);
            }
            writer.close();
        }
    }

    // Write to binary snapshot as well
    this->writeBinarySnapshot(parseMap);
}

void Log::closeLog() {
    // Close the logfile
    if (this->logfile.is_open()) {
        this->logfile.close();
    }
}