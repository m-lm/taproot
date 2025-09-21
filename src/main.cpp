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

    // Run server loop
    //std::thread serverThread([&server]() {server.run();});
    server.run();
    //server.stop();
    //serverThread.join();

    return 0;
}