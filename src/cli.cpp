#include "taproot/client.h"
#include "taproot/utils.h"
#include "taproot/config.h"
#include "taproot/cli.h"
#include <asio.hpp>
#include <iostream>
#include <cctype>
#include <unordered_map>

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
    clear   → delete all data from keyspace (!)
    stats   → display stats
    help    → display commands
    quit    → close the program
    )";
    std::cout << WELCOME << std::endl;
}

void cli(Client& client, std::string& keyspace) {
    /* Command-line display loop for interacting with Taproot. */
    welcome();
    while (true) {
        std::string input;
        std::cout << "\ntap> ";
        std::getline(std::cin, input);
        std::string res;
        if (input == "help") {
            welcome();
        }
        else if (input == "use") {
            std::cout << "\nEnter keyspace to use\n> ";
            std::getline(std::cin, keyspace);
        }
        else {
            res = client.send(input);
            std::cout << res << std::endl;
            if (res == "BYE") break;
        }
    }

    std::cout << "\nGoodbye\n" << std::endl;
}

int main(int argc, char** argv) {
    /* Main entrypoint for the command-line client. */
    const std::string configFilename = "config.cfg";
    Config config = parseConfig(configFilename);

    // Overwrite default config values with args passed in from command-line
    if (argc > 1) {
        validateHost(config, argv[1]);
    }
    if (argc > 2) {
        validatePort(config, argv[2]);
    }
    if (argc > 3) {
        config.keyspace = argv[3];
    }

    // Set up networking
    asio::io_context context;
    Client client(context, config);

    // Run command-line interface
    cli(client, config.keyspace);

    return 0;
}