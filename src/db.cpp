#include "db.h"
#include "log.h"
#include <string>
#include <iostream>
#include <unordered_map>
#include <format>
#include <utility>

DB::DB(const std::string& name) : name(name), logger(name) {
}

DB::DB(const std::string& name, const std::unordered_map<std::string, std::string>& store) : store(store), name(name), logger(name) {
}

DB::~DB() {
    this->shutdown();
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

void DB::display() const {
    std::cout << "\n============" << std::endl;
    std::cout << std::format("| Key-values for {}\n", this->name) << std::endl;
    for (auto item : this->store) {
        std::cout << std::format("{}: {}", item.first, item.second) << std::endl;
    }
    std::cout << "============" << std::endl;
}

void DB::shutdown() {
    this->logger.closeLog();
    this->logger.compactLog();
}