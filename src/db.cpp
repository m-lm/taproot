#include "db.h"
#include "log.h"
#include "query.h"
#include <string>
#include <iostream>
#include <unordered_map>
#include <format>
#include <utility>

DB::DB(const std::string& name) : name(name), logger(name) {
    this->query = new Query(*this);
    this->loadFromLog();
}

DB::DB(const std::string& name, const std::unordered_map<std::string, std::string>& store) : store(store), name(name), logger(name) {
    this->query = new Query(*this);
}

DB::~DB() {
    this->shutdown();
    delete this->query;
}

std::string& DB::operator[](const std::string& key) {
    // Overloaded [] operator for direct use on DB object. Returns a reference to string so it can be modified.
    return this->store[key];
}

void DB::put(const std::string& key, const std::string& value) {
    // Add or update key-value pair
    this->store[key] = value;
}

std::optional<std::string> DB::get(const std::string& key) const {
    // Get the value of a specified key
    auto iter = this->store.find(key);
    if (iter != this->store.end()) {
        return iter->second;
    }
    return std::nullopt;
}

bool DB::del(const std::string& key) {
    // Delete the key-value pair from the store
    int status = this->store.erase(key);
    if (status >= 1) {
        return true;
    }
    return false;
}

Log& DB::getLogger() {
    // Return logger private member variable
    return this->logger;
}

void DB::loadFromLog() {
    // Replay the commands from the compacted log to fill the store with data
    std::ifstream loader("logs/" + this->name + "_compacted.log");
    std::string line;
    if (loader.is_open() && this->store.empty()) {
        while(getline(loader, line)) {
            this->query->parseCommand(line);
        }
        loader.close();
    }
    else {
        std::cout << "Note: either log loader isn't open, or the store is nonempty." << std::endl;
    }
}

void DB::display() const {
    // Display the key value store in a readable format
    std::cout << "\n============" << std::endl;
    std::cout << std::format("| Key-values for {}\n", this->name) << std::endl;
    for (auto item : this->store) {
        std::cout << std::format("{}: {}", item.first, item.second) << std::endl;
    }
    std::cout << "============" << std::endl;
}

void DB::shutdown() {
    // Shutdown log file access to allow for compaction. Typically, use on DB close
    this->logger.closeLog();
    this->logger.compactLog();
}