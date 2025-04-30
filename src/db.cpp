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

std::vector<std::optional<std::string>> DB::mget(const std::vector<std::string>& keys) const {
    // Get the values of multiple keys in a list
    std::vector<std::optional<std::string>> results;
    for (const auto& key : keys) {
        results.push_back(this->get(key));
    }
    return results;
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
    if (!std::filesystem::exists(snapshotFilename) || !std::filesystem::is_regular_file(snapshotFilename)) {
        std::cout << "Note: Replay snapshot not found." << std::endl;
        std::string logFilename = std::format("logs/{}.log", this->name);
        if (!std::filesystem::exists(snapshotFilename)) {
            std::cout << "Note: Changelog not found." << std::endl;
            return;
        }
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
    }
    else {
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
            std::cout << "Note: Either log loader isn't open, or the store is nonempty." << std::endl;
        }
    }
}

void DB::display() {
    // Display the key-value store in a readable format
    std::cout << "\n============" << std::endl;
    std::cout << std::format("| Key-values for keyspace: '{}'\n", this->name) << std::endl;
    for (const auto& item : this->store) {
        std::cout << item.first + ": " + item.second << std::endl;
    }
    std::cout << "============" << std::endl;
}

void DB::display(const std::vector<std::string>& keys) {
    // Display the key-value store in a readable format according to mget keys
    std::cout << "\n============" << std::endl;
    for (const auto& item : this->store) {
        if (contains(keys, item.first)) {
            std::cout << item.first + ": " + item.second << std::endl;
        }
    }
    std::cout << "============" << std::endl;
}

void DB::shutdown() {
    // Shutdown log file access to allow for compaction. Typically, use on DB close
    this->logger.closeLog();
    this->logger.compactLog(this->store, this->hasBeenAltered);
    this->logger.rotateLogs();
}