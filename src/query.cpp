#include "taproot/query.h"
#include "taproot/db.h"
#include "taproot/utils.h"
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
    // Parses user input query to execute the appropriate command
    // TODO: switch to enum for commands
    if (command.length() == 0 || isAllSpace(command)) {
        return;
    }

    const std::vector<std::string> tokens = tokenize(command);
    const std::string op = tokens[0];
    if (op == "put" && tokens.size() == 3) {
        this->database.put(tokens[1], tokens[2]);
        if (!this->database.isReplaying()) { // If replay is off, it will append commands to regular log file. Not advised for loadFromLog() on startup; use replay=true for loading.
            this->database.getLogger().appendCommand(Log::Command::PUT, tokens[1], tokens[2]);
        }
    }
    else if (op == "del" && tokens.size() == 2) {
        this->database.del(tokens[1]);
        this->database.getLogger().appendCommand(Log::Command::DEL, tokens[1]);
    }
    else if (op == "get" && tokens.size() == 2) {
        std::optional<std::string> value = this->database.get(tokens[1]);
        if (!this->database.isReplaying() && value) {
            std::cout << *value << std::endl;
        }
    }
    else if (op == "mget" && tokens.size() >= 3) {
        std::vector<std::string> keys;
        for (size_t i = 1; i < tokens.size(); i++) {
            keys.push_back(tokens[i]);
        }
        std::vector<std::optional<std::string>> values = this->database.mget(keys);
        if (!this->database.isReplaying() && !values.empty()) {
            this->database.display(keys);
        }
    }
    else {
        std::unordered_map<std::string, std::string> tips = {
            {"put", "put <KEY> <VALUE>"},
            {"del", "del <KEY>"},
            {"get", "get <KEY>"},
            {"mget", "mget <KEY1> <KEY2> ... <KEYN>"},
        };
        if (tips.count(op) > 0) {
            std::cout << std::format("\nInvalid operator usage: '{}' ({})", op, tips[op]) << std::endl;
        }
        else {
            std::cout << "\nPlease use 'put', 'get', and 'del' operators as first keyword, or use 'help' for more." << std::endl;
        }
        return;
    }
}