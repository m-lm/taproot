#include "taproot/db.h"
#include "taproot/log.h"
#include "taproot/utils.h"
#include <string>
#include <iostream>
#include <unordered_map>
#include <format>
#include <utility>
#include <filesystem>
#include <chrono>
#include <ranges>

DB::DB(const std::string& name) : name(name), logger(name), dirty(0) {
    /* Load data from AOF. */
    this->loadFromLog();
}

DB::DB(const std::string& name, const std::unordered_map<std::string, std::string>& store) : store(store), name(name), logger(name) {
}

DB::~DB() {
    this->shutdown();
}

void DB::put(const std::string& key, const std::string& value) {
    /* Add or update the key-value pair. */
    /* USED IN API */
    this->store[key] = value;
    if (!this->replaying) {
        this->dirty++;
    }
}

bool DB::del(const std::string& key) {
    /* Delete the key-value pair from the store. Return true if successfully deleted key. */
    /* USED IN API */
    int status = this->store.erase(key);
    if (status >= 1) {
        if (!this->replaying) {
            this->dirty++;
        }
        return true;
    }
    return false;
}

std::optional<std::string> DB::get(const std::string& key) const {
    /* Get the value of a specified key. */
    /* USED IN API */
    auto iter = this->store.find(key);
    if (iter != this->store.end()) {
        return iter->second;
    }
    return std::nullopt;
}

std::vector<std::optional<std::string>> DB::mget(const std::vector<std::string>& keys) const {
    /* Get the values of multiple keys in a list. */
    /* USED IN API */
    std::vector<std::optional<std::string>> results;
    for (const auto& key : keys) {
        results.push_back(this->get(key));
    }
    return results;
}

bool DB::mdel(const std::vector<std::string>& keys) {
    /* Get the values of multiple keys in a list. Return false if at least one item was not deleted. */
    /* USED IN API */
    bool status = true;
    std::vector<std::optional<std::string>> results;
    for (const auto& key : keys) {
        if (!this->del(key)) {
            status = false;
        }
    }
    return status;
}

bool DB::isReplaying() {
    /* Get replay status. */
    return this->replaying;
}

Log& DB::getLogger() {
    /* Return logger private member variable. */
    return this->logger;
}

void DB::loadFromLog() {
    /* Replay the commands from the AOF to fill the store with data. */
    std::string logFilename = std::format("logs/{}.aof", this->name);
    std::ifstream loader(logFilename);
    std::string line;
    if (loader.is_open() && this->store.empty()) {
        while(getline(loader, line)) {
            this->replaying = true;
            this->parseCommand(line);
            this->replaying = false;
        }
        loader.close();
    }
    else {
        std::cerr << "Note: Log not loaded." << std::endl;
    }
}

void DB::display() const {
    /* Display the key-value store in a readable format. */
    std::cout << "\n============" << std::endl;
    std::cout << std::format("| Key-values for keyspace: '{}'\n", this->name) << std::endl;
    for (const auto& item : this->store) {
        std::cout << item.first + ": " + item.second << std::endl;
    }
    std::cout << "============" << std::endl;
}

void DB::display(const std::string& key) const {
    /* Display the key-value store in a readable format given a single key. Used in GET display. */
    std::cout << "\n============" << std::endl;
    std::cout << *this->get(key) << std::endl;
    std::cout << "============" << std::endl;
}

void DB::display(const std::vector<std::string>& keys) const {
    /* Display the key-value store in a readable format given a subset of keys. Used in MGET display. */
    std::cout << "\n============" << std::endl;
    for (const auto& item : this->store) {
        if (contains(keys, item.first)) {
            std::cout << item.first + ": " + item.second << std::endl;
        }
    }
    std::cout << "============" << std::endl;
}

void DB::displayKeys() const {
    /* Display the keys of the key-value store in a readable format. */
    std::cout << "\n============" << std::endl;
    std::cout << std::format("| Keys for keyspace: '{}'\n", this->name) << std::endl;
    for (const auto& key : this->getKeys()) {
        std::cout << key << std::endl;
    }
    std::cout << "============" << std::endl;
}

void DB::displayValues() const {
    /* Display the values of the key-value store in a readable format. */
    std::cout << "\n============" << std::endl;
    std::cout << std::format("| Values for keyspace: '{}'\n", this->name) << std::endl;
    for (const auto& value : this->getValues()) {
        std::cout << value << std::endl;
    }
    std::cout << "============" << std::endl;
}

void DB::displayStats() const {
    /* Display useful statistics of the key-value store. */
    std::cout << "\n============" << std::endl;
    std::cout << std::format("| Stats for keyspace: '{}'\n", this->name) << std::endl;
    std::cout << "Total keys: " << this->store.size() << std::endl;

    size_t totalBytes = 0;
    for (const auto& [key, value] : this->store) {
        totalBytes += key.size() + value.size();
    }
    std::cout << "Total bytes: " << totalBytes << std::endl;
    std::cout << "============" << std::endl;
}

std::vector<std::string> DB::getKeys() const {
    /* Get currently stored keys. */
    auto keys = std::views::keys(this->store);
    return {keys.begin(), keys.end()};
}

std::vector<std::string> DB::getValues() const {
    /* Get currently stored values. */
    auto values = std::views::values(this->store);
    return {values.begin(), values.end()};
}

std::vector<std::pair<std::string, std::string>> DB::getItems() const {
    /* Get all currently stored copies of key-value pairs. */
    /* USED IN API */
    return {this->store.begin(), this->store.end()};
}

std::vector<std::pair<std::string, std::optional<std::string>>> DB::getItems(const std::vector<std::string>& keys) const {
    /* Get currently stored copies of key-value pairs given a subset of keys. Similar to MGET, except return both keys and values. Also similar to overloaded display, except returns a vector. */
    /* USED IN API */
    std::vector<std::pair<std::string, std::optional<std::string>>> results;
    results.reserve(keys.size());
    for (const auto& key : keys) {
        results.emplace_back(key, this->get(key));
    }
    return results;
}

void DB::parseCommand(const std::string& command) {
    /* Parses whole input query to execute the appropriate command like a dispatcher. */
    if (command.length() == 0 || isAllSpace(command)) {
        return;
    }

    const std::vector<std::string> tokens = tokenize(command);
    const std::string op = tokens[0];

    if (op == "put" && tokens.size() == 3) {
        this->put(tokens[1], tokens[2]);
        if (!this->isReplaying()) { // If replay is off, it will append commands to the AOF. If it is on, it will load data. Not advised for loadFromLog() on startup; use replay=true for loading.
            this->getLogger().appendCommand(Operation::convertStr(op), tokens[1], tokens[2]);
        }
    }
    else if (op == "del" && tokens.size() == 2) {
        this->del(tokens[1]);
        this->getLogger().appendCommand(Operation::convertStr(op), tokens[1]);
    }
    else if (op == "get" && tokens.size() == 2) {
        this->display(tokens[1]);
    }
    else if (op == "mget" && tokens.size() >= 3) {
        std::vector<std::string> keys;
        for (size_t i = 1; i < tokens.size(); i++) {
            keys.push_back(tokens[i]);
        }
        std::vector<std::optional<std::string>> values = this->mget(keys);
        this->display(keys);
    }
    else if (op == "mdel" && tokens.size() >= 3) {
        std::vector<std::string> keys;
        for (size_t i = 1; i < tokens.size(); i++) {
            keys.push_back(tokens[i]);
        }
        this->mdel(keys);
    }
    else {
        const std::unordered_map<std::string, std::string> tips = {
            {"put", "put <KEY> <VALUE>"},
            {"del", "del <KEY>"},
            {"get", "get <KEY>"},
            {"mget", "mget <KEY1> <KEY2> ... <KEYN>"},
            {"mdel", "mdel <KEY1> <KEY2> ... <KEYN>"},
        };
        if (tips.count(op) > 0) {
            std::cerr << std::format("\nInvalid operator usage: '{}' ({})", op, tips.at(op)) << std::endl;
        }
        else {
            std::cerr << "\nPlease use 'put', 'get', and 'del' operators as first keyword, or use 'help' for more." << std::endl;
        }
    }
}

void DB::shutdown() {
    /* Shutdown access to the AOF to allow for compaction and snapshots. */
    this->logger.closeLog();
    this->logger.compactLog(this->store, this->dirty);
    this->logger.writeBinarySnapshot(this->store);
}