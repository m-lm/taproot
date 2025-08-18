#include "taproot/cli.h"
#include <iostream>
#include <asio.hpp>

int main() {
    const unsigned portNumber = 6709;
    const std::string address = "93.184.216.34";
    asio::error_code err;

    // Networking interface
    asio::io_context context;

    // Set up address to be able to connect (IPv4)
    //asio::ip::tcp::endpoint endpoint(asio::ip::make_address(address, err), portNumber);
    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), portNumber);

    // Set up server listener for client 
    asio::ip::tcp::acceptor acceptor(context, endpoint);
    std::cout << "Listening..." << std::endl;

    // Set up communication line
    asio::ip::tcp::socket socket(context);
    acceptor.accept(socket);
    std::cout << "Client connected..." << std::endl;

    return 0;
}