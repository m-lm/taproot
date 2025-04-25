#include "taproot/db.h"
#include "taproot/query.h"
#include "taproot/utils.h"
#include <random>

int main() {
    DB test_db("_TEST");
    Query test_query(test_db);
    for (int i = 0; i < 1000000; i++) {
        test_query.parseCommand(std::format("put {} {}", i, i*2));
    }
    return 0;
}