#include "utils.h"
#include <vector>
#include <sstream>

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