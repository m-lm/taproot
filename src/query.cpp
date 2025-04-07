#include "query.h"
#include <iostream>
#include <cctype>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>

Query::Query(DB& database) : database(database) {
}

Query::~Query() {
}

std::vector<std::string> Query::tokenize(const std::string& input) {
    // Tokenizes input string into separate values.
    std::vector<std::string> tokens = {};
    std::istringstream iss(input);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

void Query::parseCommand(const std::string& command) {
    // Parses user input query to execute the appropriate command.
    if (command.length() == 0) {
        return;
    }

    std::unordered_map<std::string, std::string> tips = {
        {"put", "put <KEY> <VALUE>"},
        {"get", "get <KEY>"},
        {"del", "del <KEY>"},
    };
    const std::vector<std::string> tokens = tokenize(command);
    const std::string op = tokens[0];

    if (op == "put" && tokens.size() == 3) {
        this->database.put(tokens[1], tokens[2]);
    }
    else if (op == "get" && tokens.size() == 2) {
        this->database.get(tokens[1]);
    }
    else if (op == "del" && tokens.size() == 2) {
        this->database.del(tokens[1]);
    }
    else {
        if (tips.count(op) > 0) {
            std::cout << std::format("\nInvalid operator usage: '{}' ({})", op, tips[op]) << std::endl;
        }
        else {
            std::cout << "\nPlease use 'put', 'get', and 'del' operators as first keyword." << std::endl;
        }
        return;
    }
}