#include "utils.h"
#include <vector>
#include <sstream>

std::vector<std::string> tokenize(const std::string& input) {
    // Tokenizes input string into separate values.
    std::vector<std::string> tokens = {};
    std::istringstream iss(input);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}