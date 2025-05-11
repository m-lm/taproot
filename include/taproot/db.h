#pragma once
#include "log.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <memory>

class Query;

class DB {
    protected:
        std::unordered_map<std::string, std::string> store;
        std::string name;
        Log logger;
        std::unique_ptr<Query> query;
        bool replaying;
        size_t dirty;

    public:
        // Constructors and deconstructors
        DB(const std::string& name);
        DB(const std::string& name, const std::unordered_map<std::string, std::string>& store);
        virtual ~DB();

        // Base functionality
        void put(const std::string& key, const std::string& value);
        bool del(const std::string& key);
        std::optional<std::string> get(const std::string& key) const;
        std::vector<std::optional<std::string>> mget(const std::vector<std::string>& keys) const;

        // Group data getters
        /*
        std::vector<const std::string&> getKeys() const;
        std::vector<const std::string&> getValues() const;
        std::vector<const std::pair<std::string, std::string>> getItems() const;
        */
        bool isReplaying();
        Log& getLogger();
        void loadFromLog();

        // Console print
        void display();
        void display(const std::vector<std::string>& keys);

        // Shutdown key-value store
        void shutdown();
};