#include "query.h"
#include <iostream>
#include <cctype>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>

Query::Query(DB& database) : database(database) {
    this->tips = {
        {"put", "put <KEY> <VALUE>"},
        {"get", "get <KEY>"},
        {"del", "del <KEY>"},
    };
}

Query::~Query() {
}

void Query::parseCommand(const std::string& command) {
    if (command.length() == 0) {
        return;
    }
    std::vector<std::string> tokens = {};
    std::istringstream iss(command);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }

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
            std::cout << std::format("Invalid operator usage: '{}' ({})", op, this->tips[op]) << std::endl;
        }
        else {
            std::cout << "Please use 'put', 'get', and 'del' operators as first keyword." << std::endl;
        }
        return;
    }
    this->database.display();
}