#include "taproot/db.h"
#include "taproot/query.h"
#include "taproot/utils.h"
#include <iostream>
#include <random>
#include <chrono>

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    DB test_db("_TEST");
    Query test_query(test_db);
    for (int i = 0; i < 1000000; i++) {
        test_query.parseCommand(std::format("put {} {}", i, i*2));
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Write test @ " << duration << std::endl;
    return 0;
}