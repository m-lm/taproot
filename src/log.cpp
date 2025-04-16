#include "log.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <utility>

Log::Log(const std::string& filename) : filename(filename) {
    // Logs are owned by DB (store) objects
    this->logfile.open("logs/" + filename + ".log", std::ios::app);
}

Log::~Log() {
}

void Log::appendPut(const std::string& key, const std::string& value) {
    // Append the "put" query to the log file
    if (this->logfile.is_open()) {
        this->logfile << std::format("put {} {}\n", key, value);
    }
    else {
        std::cout << std::format("\nError: Logfile {} has not been opened.\n", this->filename) << std::endl;
    }
}

void Log::appendDelete(const std::string& key) {
    // Append the "del" query to the log file
    if (this->logfile.is_open()) {
        this->logfile << std::format("del {}\n", key);
    }
    else {
        std::cout << std::format("\nError: Logfile {} has not been opened.\n", this->filename) << std::endl;
    }
}

void Log::compactLog() {
    // Compacts the Append-Only Log to reduce old or redundant queries. Used before compression
    // Also assumes no invalid commands were permitted into the log
    if (this->logfile.is_open()) {
        throw std::runtime_error("Cannot compact logfile that is still open.");
    }

    std::unordered_map<std::string, std::string> parseMap;
    std::string line;
    std::ifstream reader("logs/" + this->filename + ".log");
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
    if (!parseMap.empty()) {
        std::ofstream writer("logs/" + this->filename + "_compacted.log");
        if (writer.is_open()) {
            for (const std::pair<const std::string, std::string>& pair : parseMap) {
                writer << std::format("put {} {}\n", pair.first, pair.second);
            }
            writer.close();
        }
    }
}

void Log::closeLog() {
    // Close the logfile
    if (this->logfile.is_open()) {
        this->logfile.close();
    }
}