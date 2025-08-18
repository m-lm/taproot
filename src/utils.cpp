#include "taproot/utils.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <algorithm>

bool isAllSpace(const std::string& input) {
    // Check if input string is all whitespace
    for (const char& ch : input) {
        if (!std::isspace(ch)) {
            return false;
        }
    }
    return true;
}

std::vector<std::string> tokenize(const std::string& input) {
    // Tokenize input string into separate values
    std::vector<std::string> tokens = {};
    std::istringstream iss(input);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string getTimestamp() {
    // Get current timestamp, especially useful for data versioning
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm time = *std::localtime(&t);
    std::ostringstream stream;
    stream << std::put_time(&time, "%y%m%dT%H%M%S");
    return stream.str();
}

