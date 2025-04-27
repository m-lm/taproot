#include "taproot/db.h"
#include "taproot/query.h"
#include "taproot/utils.h"
#include <iostream>
#include <cctype>



void cli() {
    const std::string welcome = R"(
            TAPROOT 
    ----------------------------------
        Key-value. Document. RDF.
    ----------------------------------
   
    use     → switch keyspace
    put     → store value by key
    get     → retrieve value by key
    del     → delete key-value pair
    show    → display all key-values
    help    → display commands
    quit    → close the program
    
   )";

    std::cout << welcome << std::endl;
    std::string keyspaceName;
    while (true) {
        std::cout << "Enter keyspace to use\n> ";
        std::getline(std::cin, keyspaceName);
        break;
    }
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

int main() {
    cli();
    return 0;
}