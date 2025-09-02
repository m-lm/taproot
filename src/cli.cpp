#include "taproot/db.h"
#include "taproot/utils.h"
#include "taproot/config.h"
#include <iostream>
#include <cctype>
#include <asio.hpp>

void welcome() {
    /* Startup display for command-line. */
    const std::string WELCOME = R"(
            taprootdb
    ----------------------------------
    ----------------------------------
   
    put     → store value by key
    del     → delete key-value pair
    get     → retrieve value by key
    mput    → store multiple values by key
    mdel    → delete multiple values by key
    mget    → retrieve multiple values by key

    show    → display all key-value pairs
    keys    → display all keys
    values  → display all values

    use     → switch keyspace
    stats   → display stats
    help    → display commands
    quit    → close the program
    )";
    std::cout << WELCOME << std::endl;
}

void cli() {
    /* Command-line display loop for interacting with Taproot. */
    std::string keyspaceName;
    std::cout << "\nEnter keyspace to use\n> ";
    std::getline(std::cin, keyspaceName);
    welcome();
    std::unique_ptr<DB> db = std::make_unique<DB>(keyspaceName);
    while (true) {
        std::string input;
        std::cout << "\ntap> ";
        std::getline(std::cin, input);
        std::vector<std::string> tokens = tokenize(input);
        if (input == "quit" || input == "exit") break;
        else if (input == "help") {
            welcome();
        }
        else if (input == "stats") {
            db->displayStats();
        }
        else if (tokens[0] == "use" && tokens.size() == 2) {
            try {
                db->shutdown();
                db = std::make_unique<DB>(tokens[1]);
                std::cout << "\nSwitched keyspace to " << tokens[1] << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "\nFailed to switch keyspace to " << tokens[1] << std::endl;
            }
            welcome();
            continue;
        }
        else if (input == "show") {
            db->display();
        }
        else if (input == "keys") {
            db->displayKeys();
        }
        else if (input == "values") {
            db->displayValues();
        }
        else {
            db->parseCommand(input);
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
        std::cerr << "Retrying..." << std::endl;
    }

    if (!err) {
        std::cout << std::format("Successfully connected to address '{}:{}'", cfg.host, cfg.port) << std::endl;
    }
    else {
        std::cerr << std::format("Failed to connect to address '{}:{}': {}", cfg.host, cfg.port, err.message()) << std::endl;
        return 1;
    }

    // Run command-line interface
    cli();

    socket.close();

    return 0;
}