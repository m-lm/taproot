#include "taproot/utils.h"
#include "asio.hpp"
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <string>

bool isAllSpace(const std::string& input) {
    // Check if input string is all whitespace
    for (const char& ch : input) {
        if (!std::isspace(ch)) {
            return false;
        }
    }
    return true;
}

std::vector<std::string> tokenize(const std::string& input) {
    // Tokenize input string into separate values
    std::vector<std::string> tokens = {};
    std::istringstream iss(input);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string getTimestamp() {
    // Get current timestamp, especially useful for data versioning
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm time = *std::localtime(&t);
    std::ostringstream stream;
    stream << std::put_time(&time, "%y%m%dT%H%M%S");
    return stream.str();
}

Config parseConfig(const std::string& filename) {
    // Read and parse config file to get system data like addresses, ports, etc.
    std::ifstream file(filename);
    Config cfg;
    if (!file.is_open()) {
        std::cout << std::format("\nError: Config file {} has not been opened.\n", filename) << std::endl;
        return cfg;
    }

    // Get content from .cfg file
    std::unordered_map<std::string, std::string> map;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        std::string key;
        std::string value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            map[key] = value;
        }
    }

    // Create and populate config object to return (and do validation)
    if (map.count("host")) {
        validateHost(cfg, map["host"]);
    }
    else {
        std::cout << std::format("Invalid host address: using {} instead.", cfg.host) << std::endl;
    }
    if (map.count("port")) {
        validatePort(cfg, map["port"]);
    }
    else {
        std::cout << std::format("Invalid port: using {} instead.", cfg.port) << std::endl;
    }
    file.close();
    return cfg;
}

bool validateHost(Config& cfg, std::string host) {
    // Check to see if host setting is valid from the config map; if it is, set it and return true. Otherwise, return false.
    if (!host.empty() && !isAllSpace(host)) {
        asio::error_code err;
        asio::ip::make_address(host, err);
        if (err) {
            std::cout << std::format("{}\nInvalid host address: using {} instead.", err.message(), cfg.host) << std::endl;
            return false;
        }
        else {
            cfg.host = host;
        }
    }
    else {
        std::cout << std::format("Invalid host address: using {} instead.", cfg.host) << std::endl;
        return false;
    }
    return true;
}

bool validatePort(Config& cfg, std::string port) {
    // Check to see if port setting is valid from the config map; if it is, set it and return true. Otherwise, return false.
    try {
        int portNumber = std::stoi(port);
        if (portNumber < 0 || portNumber > 65535) {
            std::cout << std::format("Port number out of range: using {} instead.", cfg.port) << std::endl;
            return false;
        }
        else {
            cfg.port = static_cast<unsigned short>(portNumber);
        }
    }
    catch (const std::exception& e) {
        std::cout << std::format("{}\nInvalid port: using {} instead.", e.what(), cfg.port) << std::endl;
        return false;
    }
    return true;
}

