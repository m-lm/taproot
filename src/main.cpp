#include "taproot/cli.h"
#include "taproot/utils.h"
#include <iostream>
#include <asio.hpp>

int main() {
    // Configuration
    const std::string configFilename = "config.cfg";
    Config cfg = parseConfig(configFilename);

    // Networking interface
    asio::error_code err;
    asio::io_context context;

    // Set up address to be able to connect (IPv4)
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address(cfg.host, err), cfg.port);

    // Set up server listener for client 
    asio::ip::tcp::acceptor acceptor(context);
    acceptor.open(asio::ip::tcp::v4());
    acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen();
    std::cout << "Listening..." << std::endl;

    // Set up communication line
    asio::ip::tcp::socket socket(context);
    acceptor.accept(socket);
    std::cout << "Client connected..." << std::endl;

    return 0;
}