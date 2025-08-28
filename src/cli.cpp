#include "taproot/db.h"
#include "taproot/query.h"
#include "taproot/utils.h"
#include "taproot/config.h"
#include <iostream>
#include <cctype>
#include <asio.hpp>

void cli() {
    /* Command-line display loop for interacting with Taproot. */

    const std::string WELCOME = R"(
            taprootdb
    ----------------------------------
    ----------------------------------
   
    use     → switch keyspace
    put     → store value by key
    del     → delete key-value pair
    get     → retrieve value by key
    mget    → retrieve multiple values by key
    mdel    → delete multiple values by key
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
    std::cout << WELCOME << std::endl;
    DB db(keyspaceName);
    Query query(db);
    while (true) {
        std::string input;
        std::cout << "\ntap> ";
        std::getline(std::cin, input);
        std::vector<std::string> tokens = tokenize(input);
        if (input == "quit" || input == "exit") {
            break;
        }
        else if (input == "help") {
            std::cout << WELCOME << std::endl;
        }
        else if (input == "show") {
            db.display();
        }
        else if (tokens[0] == "use" && tokens.size() == 2) {
            std::cout << "\nCommand not yet available." << std::endl;
        }
        else {
            query.parseCommand(input);
        }
    }
    std::cout << "\nGoodbye\n" << std::endl;

}

int main(int argc, char** argv) {
    /* Main entrypoint for the command-line client. */
    const std::string configFilename = "config.cfg";
    Config cfg = parseConfig(configFilename);

    // Overwrite default config values with args passed in from command-line
    if (argc > 1) {
        validateHost(cfg, argv[1]);
    }
    if (argc > 2) {
        validatePort(cfg, argv[2]);
    }

    // Set up networking
    asio::error_code err;
    asio::io_context context;
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address(cfg.host, err), cfg.port);
    asio::ip::tcp::socket socket(context);

    // Client connection retry loop
    const int maxRetries = 5;
    for (int i = 0; i < maxRetries; i++) {
        socket.connect(endpoint, err);
        if (!err) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::cout << "Retrying..." << std::endl;
    }

    if (!err) {
        std::cout << std::format("Successfully connected to address '{}:{}'", cfg.host, cfg.port) << std::endl;
    }
    else {
        std::cout << std::format("Failed to connect to address '{}:{}': {}", cfg.host, cfg.port, err.message()) << std::endl;
        return 1;
    }

    // Run command-line interface
    cli();

    socket.close();

    return 0;
}