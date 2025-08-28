#include "taproot/db.h"
#include "taproot/log.h"
#include "taproot/query.h"
#include "taproot/utils.h"
#include <string>
#include <iostream>
#include <unordered_map>
#include <format>
#include <utility>
#include <filesystem>
#include <chrono>

DB::DB(const std::string& name) : name(name), logger(name), dirty(0) {
    /* Load data from AOF. */
    this->query = std::make_unique<Query>(*this);
    this->loadFromLog();
}

DB::DB(const std::string& name, const std::unordered_map<std::string, std::string>& store) : store(store), name(name), logger(name) {
    this->query = std::make_unique<Query>(*this);
}

DB::~DB() {
    this->shutdown();
}

void DB::put(const std::string& key, const std::string& value) {
    /* Add or update the key-value pair. */
    this->store[key] = value;
    if (!this->replaying) {
        this->dirty++;
    }
}

bool DB::del(const std::string& key) {
    /* Delete the key-value pair from the store. */
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
    auto iter = this->store.find(key);
    if (iter != this->store.end()) {
        return iter->second;
    }
    return std::nullopt;
}

std::vector<std::optional<std::string>> DB::mget(const std::vector<std::string>& keys) const {
    /* Get the values of multiple keys in a list. */
    std::vector<std::optional<std::string>> results;
    for (const auto& key : keys) {
        results.push_back(this->get(key));
    }
    return results;
}

void DB::mdel(const std::vector<std::string>& keys) {
    /* Get the values of multiple keys in a list. */
    std::vector<std::optional<std::string>> results;
    for (const auto& key : keys) {
        this->del(key);
    }
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
            this->query->parseCommand(line);
            this->replaying = false;
        }
        loader.close();
    }
    else {
        std::cout << "Note: Log not loaded." << std::endl;
    }
}

void DB::display() {
    /* Display the key-value store in a readable format. */
    std::cout << "\n============" << std::endl;
    std::cout << std::format("| Key-values for keyspace: '{}'\n", this->name) << std::endl;
    for (const auto& item : this->store) {
        std::cout << item.first + ": " + item.second << std::endl;
    }
    std::cout << "============" << std::endl;
}

void DB::display(const std::vector<std::string>& keys) {
    /* Display the key-value store in a readable format given a subset of keys. */
    std::cout << "\n============" << std::endl;
    for (const auto& item : this->store) {
        if (contains(keys, item.first)) {
            std::cout << item.first + ": " + item.second << std::endl;
        }
    }
    std::cout << "============" << std::endl;
}

void DB::shutdown() {
    /* Shutdown access to the AOF to allow for compaction and snapshots. */
    this->logger.closeLog();
    this->logger.compactLog(this->store, this->dirty);
    this->logger.writeBinarySnapshot(this->store);
}