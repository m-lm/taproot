#include "db.h"
#include "query.h"
#include <iostream>
#include <cctype>

void test() {
    DB db("Taproot");
    Query query(db);
    std::cout << std::endl;
    std::cout << "Taproot" << std::endl;
    std::cout << "Display all key-values by typing 'show'" << std::endl;
    std::cout << "Exit the program by typing 'quit' or 'exit'" << std::endl;
    while (true) {
        std::string input;
        std::cout << "\nEnter command:\n> ";
        std::getline(std::cin, input);
        if (input == "quit" || input == "exit") {
            break;
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