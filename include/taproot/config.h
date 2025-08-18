#pragma once
#include <iostream>
#include <vector>

struct Defaults {
    inline static const std::string HOST_DEFAULT = "127.0.0.1";
    static constexpr unsigned short PORT_DEFAULT = 6379;
};

struct Config {
    // Default values for configuration options
    std::string host = Defaults::HOST_DEFAULT;
    unsigned short port = Defaults::PORT_DEFAULT;
};

Config parseConfig(const std::string& filename);
bool validateHost(Config& cfg, std::string host);
bool validatePort(Config& cfg, std::string port);
void spawnConfig(const std::string& filename);