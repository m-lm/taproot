#pragma once
#include "taproot/db.h"
#include "taproot/config.h"
#include <asio.hpp>
#include <iostream>
#include <cctype>
#include <unordered_map>

class Server {
    protected:
        std::string host;
        unsigned short port;
        asio::io_context& ctx;
        asio::ip::tcp::acceptor acceptor;
        asio::ip::tcp::socket socket;
        const Config& config;

        std::atomic<bool> running{false};

        // Pointer to data store
        std::unique_ptr<DB> db;

    public:
        Server(asio::io_context& ctx, const Config& cfg);
        virtual ~Server();

        void run();
        void stop();
        std::string dispatcher(const std::string& input);
        void handleClient(asio::ip::tcp::socket clientSocket);
        static void signalHandler(int);

        static Server* instance;
};