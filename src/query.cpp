#include "query.h"
#include "db.h"
#include "utils.h"
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

void Query::parseCommand(const std::string& command, const bool replay) {
    // Parses user input query to execute the appropriate command.
    if (command.length() == 0 || isAllSpace(command)) {
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
        if (!replay) { // If replay is off, it will append commands to regular log file. Not advised for loadFromLog() on startup; use replay=true for loading.
            this->database.getLogger().appendPut(tokens[1], tokens[2]);
        }
    }
    else if (op == "get" && tokens.size() == 2) {
        this->database.get(tokens[1]);
    }
    else if (op == "del" && tokens.size() == 2) {
        this->database.del(tokens[1]);
        this->database.getLogger().appendDelete(tokens[1]);
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