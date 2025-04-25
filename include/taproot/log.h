#pragma once
#include <fstream>
#include <vector>

class Log {
    protected:
        const std::string& keyspaceName;
        std::ofstream logfile;
    public:
        Log(const std::string& filename);
        virtual ~Log();

        void appendPut(const std::string& key, const std::string& value);
        void appendDelete(const std::string& key);
        void writeBinarySnapshot(const std::unordered_map<std::string, std::string>& state);
        std::string getLatestSnapshot(const std::string& keyspace);
        void compactLog();

        void closeLog();
};