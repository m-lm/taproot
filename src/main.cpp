#include "taproot/db.h"
#include "taproot/query.h"
#include <iostream>
#include <cctype>

void test() {
    DB db("Taproot");
    Query query(db);
    const std::string welcome = R"(
            TAPROOT 
    ----------------------------------
        Key-value. Document. RDF.
    ----------------------------------
   
    put     → store values by key
    get     → retrieve values by key
    del     → delete key-values
    show    → display all key-values
    help    → display commands
    quit    → close the program
    
   )";

    std::cout << welcome << std::endl;
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
    test();
    return 0;
}