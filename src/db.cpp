#include "db.h"
#include <string>
#include <iostream>
#include <unordered_map>
#include <format>
#include <utility>

DB::DB(const std::string& name) : name(name) {
}

DB::DB(const std::string& name, const std::unordered_map<std::string, std::string>& store) : store(store), name(name) {
}

DB::~DB() {
}

std::pair<std::string, std::string> DB::put(const std::string& key, const std::string& value) {
    // Add or update key-value pair
    this->store[key] = value;
    return {key, value};
}

std::optional<std::string> DB::get(const std::string& key) const {
    auto iter = this->store.find(key);
    if (iter != this->store.end()) {
        return iter->second;
    }
    return std::nullopt;
}

std::optional<std::string> DB::del(const std::string& key) {
    // Delete the key-value pair from the store
    int status = this->store.erase(key);
    if (status) {
        return key;
    }
    return std::nullopt;
}

void DB::display() const {
    std::cout << "============" << std::endl;
    std::cout << std::format("| Key-values for {}\n", this->name) << std::endl;
    for (auto item : this->store) {
        std::cout << std::format("{}: {}", item.first, item.second) << std::endl;
    }
    std::cout << "============" << std::endl;
}