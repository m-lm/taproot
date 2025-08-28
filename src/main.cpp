#include "taproot/cli.h"
#include "taproot/utils.h"
#include "taproot/config.h"
#include <iostream>
#include <asio.hpp>

int main() {
    /* Entrypoint for setting up server process. */    

    // Configuration
    const std::string configFilename = "config.cfg";
    spawnConfig(configFilename);
    Config cfg = parseConfig(configFilename);

    // Networking interface
    asio::error_code err;
    asio::io_context context;

    // Set up address to be able to connect (IPv4)
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address(cfg.host, err), cfg.port);

    // Set up server listener for client 
    asio::ip::tcp::acceptor acceptor(context);
    acceptor.open(endpoint.protocol());
    acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen();
    std::cout << "Listening..." << std::endl;

    // Set up communication line
    asio::ip::tcp::socket socket(context);
    acceptor.accept(socket);

    if (!err) {
        std::cout << std::format("Client successfully connected...", cfg.host, cfg.port) << std::endl;
    }
    else {
        std::cout << std::format("Client failed to connect: {}...", err.message()) << std::endl;
    }

    acceptor.close();

    return 0;
}