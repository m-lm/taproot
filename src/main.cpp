#include <iostream>
#include "db.h"
#include "query.h"
#include <cctype>

void test() {
    DB DB("Taproot");
    Query Query(DB);
    std::cout << std::endl;
    std::cout << "Taproot" << std::endl;
    std::cout << "Exit the program by typing 'quit' or 'exit'" << std::endl;
    while (true) {
        std::string input;
        std::cout << "\nEnter command:\n> ";
        std::getline(std::cin, input);
        if (input == "quit" || input == "exit") {
            break;
        }
        Query.parseCommand(input);
    }
    std::cout << "\nGoodbye\n" << std::endl;
}

int main() {
    test();
    return 0;
}