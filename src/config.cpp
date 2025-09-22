#include "taproot/config.h"
#include "taproot/utils.h"
#include "asio.hpp"
#include <iostream>
#include <vector>
#include <sstream>
#include <chrono>
#include <fstream>
#include <filesystem>

Config parseConfig(const std::string& filename) {
    /* Read and parse config file to get system data like addresses, ports, etc. */
    std::ifstream file(filename);
    Config cfg;
    if (!file.is_open()) {
        std::cerr << std::format("\nError: Config file {} has not been opened.\n", filename) << std::endl;
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
        std::cerr << std::format("Invalid host address: using {} instead.", cfg.host) << std::endl;
    }
    if (map.count("port")) {
        validatePort(cfg, map["port"]);
    }
    else {
        std::cerr << std::format("Invalid port: using {} instead.", cfg.port) << std::endl;
    }
    if (map.count("keyspace")) {
        cfg.keyspace = map["keyspace"];
    }
    file.close();
    return cfg;
}

bool validateHost(Config& cfg, std::string host) {
    /* Check to see if host setting is valid from the config map; if so, set it and return true. Otherwise, return false. */
    if (!host.empty() && !isAllSpace(host)) {
        asio::error_code err;
        asio::ip::make_address(host, err);
        if (err) {
            std::cerr << std::format("{}\nInvalid host address: using {} instead.", err.message(), cfg.host) << std::endl;
            return false;
        }
        else {
            cfg.host = host;
        }
    }
    else {
        std::cerr << std::format("Invalid host address: using {} instead.", cfg.host) << std::endl;
        return false;
    }
    return true;
}

bool validatePort(Config& cfg, std::string port) {
    /* Check to see if port setting is valid from the config map; if so, set it and return true. Otherwise, return false. */
    try {
        int portNumber = std::stoi(port);
        if (portNumber < 0 || portNumber > 65535) {
            std::cerr << std::format("Port number out of range: using {} instead.", cfg.port) << std::endl;
            return false;
        }
        else {
            cfg.port = static_cast<unsigned short>(portNumber);
        }
    }
    catch (const std::exception& e) {
        std::cerr << std::format("{}\nInvalid port: using {} instead.", e.what(), cfg.port) << std::endl;
        return false;
    }
    return true;
}

void spawnConfig(const std::string& filename) {
    /* Create a default config file if one does not exist yet. */
    if (!std::filesystem::exists(filename)) {
        std::ofstream file(filename);
        file << "host=" << Defaults::HOST_DEFAULT << "\n";
        file << "port=" << Defaults::PORT_DEFAULT << "\n";
        file << "keyspace=" << Defaults::KEYSPACE_DEFAULT << "\n";
        file.close();
    }
}