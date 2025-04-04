#ifndef DB_H
#define DB_H
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

class DB {
    protected:
        std::unordered_map<std::string, std::string> store;
        std::string name;
    public:
        // Constructors and deconstructors
        DB(const std::string& name);
        DB(const std::string& name, const std::unordered_map<std::string, std::string>& store);
        virtual ~DB();

        // Operator overloading
        /*
        DB& operator+(const DB& other);
        DB& operator-=(const std::pair<std::string, std::string>& item);
        DB& operator+=(const DB& other);
        DB& operator+=(const std::pair<std::string, std::string>& item);
        bool operator==(const DB& other) const;
        bool operator!=(const DB& other) const;
        std::string& operator[](const std::string& key) const;
        */

        // Base functionality
        std::pair<std::string, std::string> put(const std::string& key, const std::string& value);
        std::optional<std::string> get(const std::string& key) const;
        std::optional<std::string> del(const std::string& key);

        // Group data getters
        /*
        std::vector<const std::string&> getKeys() const;
        std::vector<const std::string&> getValues() const;
        std::vector<const std::pair<std::string, std::string>> getItems() const;
        */

        // Console print
        void display() const;
};

#endif