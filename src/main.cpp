#include "taproot/server.h"
#include "taproot/utils.h"
#include "taproot/config.h"
#include <asio.hpp>
#include <iostream>
#include <thread>

int main() {
    /* Entrypoint for setting up server process. */    

    // Configuration
    const std::string configFilename = "config.cfg";
    spawnConfig(configFilename);
    Config config = parseConfig(configFilename);

    // Networking interface
    asio::io_context context;
    Server server(context, config);
    Server::instance = &server;

    std::signal(SIGINT, Server::signalHandler);
    std::signal(SIGTERM, Server::signalHandler);

    // Run server loop
    server.run();

    return 0;
}