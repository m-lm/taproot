#include <iostream>
#include "db.h"

void test() {
    DB db("Taproot");
    db.display();

    db.put("Bob", "Blue");
    db.put("Sally", "Red");
    db.put("James", "Purple");
    db.display();
}

int main() {
    test();
    return 0;
}