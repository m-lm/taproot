#pragma once
#include <iostream>
#include <vector>

struct Config {
    // Default values for configuration options
    std::string host = "127.0.0.1";
    unsigned short port = 6379;
};

Config parseConfig(const std::string& filename);
bool validateHost(Config& cfg, std::string host);
bool validatePort(Config& cfg, std::string port);