#include "taproot/db.h"
#include "taproot/query.h"
#include "taproot/utils.h"
#include <iostream>
#include <cctype>
#include <asio.hpp>

int main(int argc, char** argv) {
    std::string host = "127.0.0.1";
    unsigned port = 6709;

    if (argc > 1) {
        host = argv[1];
    }
    if (argc > 2) {
        port = std::stoi(argv[2]);
    }
    asio::error_code err;
    asio::io_context context;
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address(host, err), port);
    asio::ip::tcp::socket socket(context);
    socket.connect(endpoint, err);

    if (!err) {
        std::cout << std::format("Successfully connected to address '{}'", host) << std::endl;
    }
    else {
        std::cout << std::format("Failed to connect to address '{}': {}", host, err.message()) << std::endl;
    }

    const std::string welcome = R"(
            taprootdb
    ----------------------------------
    ----------------------------------
   
    use     → switch keyspace
    put     → store value by key
    del     → delete key-value pair
    get     → retrieve value by key
    mget    → retrieve multiple values by key
    show    → display all key-value pairs
    help    → display commands
    quit    → close the program
    
    )";

    std::string keyspaceName;
    while (true) {
        std::cout << "\nEnter keyspace to use\n> ";
        std::getline(std::cin, keyspaceName);
        break;
    }
    std::cout << welcome << std::endl;
    DB db(keyspaceName);
    Query query(db);
    while (true) {
        std::string input;
        std::cout << "\ntap> ";
        std::getline(std::cin, input);
        if (input == "quit" || input == "exit") {
            break;
        }
        else if (input == "help") {
            std::cout << welcome << std::endl;
        }
        else if (input == "show") {
            db.display();
        }
        else {
            query.parseCommand(input);
        }
    }
    std::cout << "\nGoodbye\n" << std::endl;
    return 0;
}