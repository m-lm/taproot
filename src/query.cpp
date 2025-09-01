#include "taproot/query.h"
#include "taproot/db.h"
#include "taproot/utils.h"
#include "taproot/operations.h"
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

void Query::parseCommand(const std::string& command) {
    /* Parses whole input query to execute the appropriate command like a dispatcher. */
    if (command.length() == 0 || isAllSpace(command)) {
        return;
    }

    const std::vector<std::string> tokens = tokenize(command);
    const std::string op = tokens[0];

    if (op == "put" && tokens.size() == 3) {
        this->database.put(tokens[1], tokens[2]);
        if (!this->database.isReplaying()) { // If replay is off, it will append commands to the AOF. If it is on, it will load data. Not advised for loadFromLog() on startup; use replay=true for loading.
            this->database.getLogger().appendCommand(Operation::convertStr(op), tokens[1], tokens[2]);
        }
    }
    else if (op == "del" && tokens.size() == 2) {
        this->database.del(tokens[1]);
        this->database.getLogger().appendCommand(Operation::convertStr(op), tokens[1]);
    }
    else if (op == "get" && tokens.size() == 2) {
        this->database.display(tokens[1]);
    }
    else if (op == "mget" && tokens.size() >= 3) {
        std::vector<std::string> keys;
        for (size_t i = 1; i < tokens.size(); i++) {
            keys.push_back(tokens[i]);
        }
        std::vector<std::optional<std::string>> values = this->database.mget(keys);
        this->database.display(keys);
    }
    else if (op == "mdel" && tokens.size() >= 3) {
        std::vector<std::string> keys;
        for (size_t i = 1; i < tokens.size(); i++) {
            keys.push_back(tokens[i]);
        }
        this->database.mdel(keys);
    }
    else {
        if (tips.count(op) > 0) {
            std::cout << std::format("\nInvalid operator usage: '{}' ({})", op, this->tips.at(op)) << std::endl;
        }
        else {
            std::cout << "\nPlease use 'put', 'get', and 'del' operators as first keyword, or use 'help' for more." << std::endl;
        }
        return;
    }
}