#pragma once
#include "log.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <memory>

class DB {
    protected:
        std::unordered_map<std::string, std::string> store;
        std::string name;
        std::unique_ptr<Log> logger;
        bool replaying;
        size_t dirty;

    public:
        // Constructors and deconstructors
        DB(const std::string& name);
        DB(const std::string& name, const std::unordered_map<std::string, std::string>& store);
        virtual ~DB();

        // Base functionality (API)
        void put(const std::string& key, const std::string& value);
        bool del(const std::string& key);
        std::optional<std::string> get(const std::string& key) const;
        void mput(const std::vector<std::string>& items);
        bool mdel(const std::vector<std::string>& keys);
        std::vector<std::optional<std::string>> mget(const std::vector<std::string>& keys) const;
        void clearData();

        // Log functionality
        bool isReplaying();
        void loadFromLog();

        // Console print
        std::string display() const;
        std::string display(const std::string& key) const;
        std::string display(const std::vector<std::string>& keys) const;
        std::string displayKeys() const;
        std::string displayValues() const;
        std::string displayStats() const;

        // General getters
        std::vector<std::string> getKeys() const;
        std::vector<std::string> getValues() const;
        std::vector<std::pair<std::string, std::string>> getItems() const;
        std::vector<std::pair<std::string, std::optional<std::string>>> getItems(const std::vector<std::string>& keys) const;

        // Incoming command parser
        std::vector<std::string> parseCommand(const std::string& command);

        // Shutdown key-value store
        void shutdown();
};