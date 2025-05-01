#pragma once
#include <fstream>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>

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

        std::string logBuffer;
        std::mutex logMutex;
        std::thread flushThread;
        std::atomic<bool> stopFlushThread = false;
        int fileDescriptor = -1;
        std::mutex condivarMutex;
        std::condition_variable condivar;

    public:
        Log(const std::string& filename);
        virtual ~Log();

        enum class Command { PUT, DEL };

        void startFlushThread();
        void catchupLog();
        void appendCommand(Command cmd, const std::string& key, const std::string& value = "");
        void writeBinarySnapshot(const std::unordered_map<std::string, std::string>& state);
        void updateAofCount();
        std::string getLatestSnapshot();
        std::string getEarliestSnapshot();
        void rotateLogs();
        void compactLog(const std::unordered_map<std::string, std::string>& state, const bool hasBeenAltered);
        std::vector<uint8_t> compress(const std::vector<uint8_t>& input);

        void closeLog();
};