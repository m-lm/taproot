#include <iostream>
#include "db.h"

void test() {
    DB db("Taproot");
    db.display();

    db.put("Bob", "Blue");
    db.put("Sally", "Red");
    db.put("James", "Purple");
    db.display();

    auto value = db.get("Sally");
    if (value) {
        std::cout << *value << std::endl;
    }

    db.del("Sally");
    value = db.get("Sally");
    if (value) {
        std::cout << *value << std::endl;
    }
    db.display();
}

int main() {
    test();
    return 0;
}