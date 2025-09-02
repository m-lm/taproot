#include "taproot/utils.h"
#include "asio.hpp"
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <string>

bool isAllSpace(const std::string& input) {
    /* Check if input string is all whitespace. */
    for (const char& ch : input) {
        if (!std::isspace(ch)) {
            return false;
        }
    }
    return true;
}

std::vector<std::string> tokenize(const std::string& input) {
    /* Tokenize input string by whitespace, while also accounting for quotation-delimited multiword tokens. */
    std::vector<std::string> tokens = {};
    std::istringstream iss(input);
    std::string token;
    while (iss >> token) {
        const char DELIMITER = '"';
        if (!token.empty() && token.front() == DELIMITER) {
            std::ostringstream buffer;
            buffer << token;
            while (token.back() != DELIMITER && iss >> token) {
                buffer << " " << token;
            }
            std::string multiwordToken = buffer.str();

            if (multiwordToken.find(' ') == std::string::npos) {
                // Space does not exist, so even though the token has quotations it only contains one word in actuality. So, remove quotes from beginning and end.
                multiwordToken = multiwordToken.substr(1, multiwordToken.size() - 2);
            }

            tokens.push_back(multiwordToken);
        }
        else {
            tokens.push_back(token);
        }
    }
    return tokens;
}

std::string toLower(std::string input) {
    /* Convert input string to all lowercase. */
    for (char &c : input) {
        c = std::tolower(static_cast<unsigned char>(c));
    }
    return input;
}

std::string getTimestamp() {
    /* Get current timestamp - especially for data versioning. */
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm time = *std::localtime(&t);
    std::ostringstream stream;
    stream << std::put_time(&time, "%y%m%dT%H%M%S");
    return stream.str();
}
