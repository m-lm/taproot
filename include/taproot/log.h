#pragma once
#include <fstream>
#include <vector>

class Log {
    protected:
        static constexpr size_t MAX_AOF_FILES = 5;
        static constexpr size_t COMPRESSION_THRESHOLD = 10 * 1024 * 1024;
        static constexpr size_t ROTATION_THRESHOLD = 100 * 1024 * 1024;
        const std::string& keyspaceName;
        std::string logFilepath;
        std::string dbFilepath;
        size_t aofCount;
        std::ofstream logfile;

    public:
        Log(const std::string& filename);
        virtual ~Log();

        void appendPut(const std::string& key, const std::string& value);
        void appendDelete(const std::string& key);
        void writeBinarySnapshot(const std::unordered_map<std::string, std::string>& state);
        void updateAofCount();
        std::string getLatestSnapshot();
        std::string getEarliestSnapshot();
        void rotateLogs();
        void compactLog(const std::unordered_map<std::string, std::string>& state);
        std::vector<uint16_t> compress(const std::vector<uint8_t>& input);

        void closeLog();
};