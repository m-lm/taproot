#include "taproot/db.h"
#include "taproot/utils.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstring>
#include <cmath>
#include <utility>
#include <random>
#include <chrono>

std::pair<std::string, std::string> generateSample(const size_t length = 96, const bool random = true) {
    /* Generate dummy data for key-value pairs. */
    std::string key(16, '\0');
    std::string val(length, '\0');

    if (random) {
        static std::mt19937_64 rng(12345);
        std::uniform_int_distribution<int> dist(33, 126);
        for (auto& c : key) {
            c = static_cast<char>(dist(rng));
        }

        for (auto& c : val) {
            c = static_cast<char>(dist(rng));
        }
    }
    return {key, val};
}

int main() {
    const size_t TOTAL_WRITES = 1000000;

    auto start = std::chrono::high_resolution_clock::now();
    DB test_db("_TEST");
    for (size_t i = 0; i < TOTAL_WRITES; i++) {
        //std::pair<std::string, std::string> pair = generateSample();
        //std::string command = "put " + pair.first + " " + pair.second + "\n";
        std::string command = "put " + std::to_string(i) + " " + std::to_string(i*2) + "\n";
        test_db.parseCommand(command);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Write test @ " << duration << std::endl;

    return 0;
}