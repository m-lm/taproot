#pragma once
#include "taproot/config.h"
#include <asio.hpp>
#include <iostream>
#include <cctype>
#include <unordered_map>

class Client {
    protected:
        std::string host;
        unsigned short port;
        asio::io_context& ctx;
        asio::ip::tcp::socket socket;
        const Config& config;

    public:
        Client(asio::io_context& ctx, const Config& cfg);
        virtual ~Client();

        std::string send(const std::string& data);
};