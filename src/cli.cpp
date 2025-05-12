#include "taproot/db.h"
#include "taproot/query.h"
#include "taproot/utils.h"
#include <iostream>
#include <cctype>

void cli() {
    const std::string welcome = R"(
            taprootDB 
    ----------------------------------
        Key-value. Document.
    ----------------------------------
   
    use     → switch keyspace
    put     → store value by key
    del     → delete key-value pair
    get     → retrieve value by key
    mget    → retrieve multiple values by keys
    show    → display all key-values
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
}