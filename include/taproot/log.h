#pragma once
#include <fstream>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>

class Log {
    protected:
        // Config variables
        const std::string& keyspaceName;
        std::string logFilepath;
        std::string dbFilepath;
        std::ofstream logfile;

        // Concurrency variables
        std::string logBuffer;
        std::mutex logMutex;
        std::thread flushThread;
        std::atomic<bool> stopFlushThread = false;
        int fileDescriptor = -1;
        std::mutex conditionVariableMutex;
        std::condition_variable conditionVariable;

    public:
        static constexpr size_t COMPRESSION_THRESHOLD = 10 * 1024 * 1024; // To compress binary .db snapshot, 10mb
        static constexpr size_t COMPACTION_THRESHOLD = 100 * 1024 * 1024; // To compact main .aof file, 100mb

        Log(const std::string& filename);
        virtual ~Log();

        enum class Command { PUT, DEL };

        // Getters
        std::string getDbFilepath();
        std::string getLogFilepath();

        // Concurrency
        void startAppendFlusher();

        // Storage mechanism
        void appendCommand(Command cmd, const std::string& key, const std::string& value = "");
        void writeBinarySnapshot(const std::unordered_map<std::string, std::string>& state);
        void compactLog(const std::unordered_map<std::string, std::string>& state, const size_t dirty);
        std::vector<uint8_t> compress(const std::vector<uint8_t>& input);

        void closeLog();
};