#include "taproot/db.h"
#include "taproot/log.h"
#include "taproot/query.h"
#include <string>
#include <iostream>
#include <unordered_map>
#include <format>
#include <utility>
#include <filesystem>
#include <chrono>

DB::DB(const std::string& name) : name(name), logger(name), hasBeenAltered(false) {
    this->query = std::make_unique<Query>(*this);
    this->loadFromLog();
}

DB::DB(const std::string& name, const std::unordered_map<std::string, std::string>& store) : store(store), name(name), logger(name) {
    this->query = std::make_unique<Query>(*this);
}

DB::~DB() {
    this->shutdown();
}

std::string& DB::operator[](const std::string& key) {
    // Overloaded [] operator for direct use on DB object. Returns a reference to string so it can be modified
    return this->store[key];
}

void DB::put(const std::string& key, const std::string& value) {
    // Add or update key-value pair
    this->store[key] = value;
    if (!this->replaying) {
        this->hasBeenAltered = true;
    }
}

bool DB::del(const std::string& key) {
    // Delete the key-value pair from the store
    int status = this->store.erase(key);
    if (status >= 1) {
        if (!this->replaying) {
            this->hasBeenAltered = true;
        }
        return true;
    }
    return false;
}

std::optional<std::string> DB::get(const std::string& key) const {
    // Get the value of a specified key
    auto iter = this->store.find(key);
    if (iter != this->store.end()) {
        return iter->second;
    }
    return std::nullopt;
}

bool DB::isReplaying() {
    // Get replay status
    return this->replaying;
}

Log& DB::getLogger() {
    // Return logger private member variable
    return this->logger;
}

void DB::loadFromLog() {
    // Replay the commands from the compacted log to fill the store with data
    // Note: this loads from the snapshot, not the full changelog
    std::string snapshotFilename = this->logger.getLatestSnapshot();
    if (!std::filesystem::exists(snapshotFilename)) {
        std::cout << "NOTE: replay snapshot not found." << std::endl;
        return;
    }
    std::ifstream loader(snapshotFilename);
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
        std::cout << "NOTE: either log loader isn't open, or the store is nonempty." << std::endl;
    }
}

void DB::display() {
    // Display the key value store in a readable format
    std::cout << "\n============" << std::endl;
    std::cout << std::format("| Key-values for keyspace: '{}'\n", this->name) << std::endl;
    for (const auto& item : this->store) {
        std::cout << std::format("{}: {}", item.first, item.second) << std::endl;
    }
    std::cout << "============" << std::endl;
}

void DB::shutdown() {
    // Shutdown log file access to allow for compaction. Typically, use on DB close
    this->logger.closeLog();
    auto start = std::chrono::high_resolution_clock::now();
    this->logger.compactLog(this->store, this->hasBeenAltered);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Compaction took " << duration << std::endl;
    this->logger.rotateLogs();
}