#include "log.h"
#include <fstream>
#include <iostream>
#include <string>

Log::Log(const std::string& filename) : filename(filename) {
    // Logs are owned by DB (store) objects.
    this->logfile.open(filename);
}

Log::~Log() {
    this->logfile.close();
}

void Log::appendPut(const std::string& key, const std::string& value) {
    // Appends the "put" query to the log file.
    if (this->logfile.is_open()) {
        this->logfile << std::format("put {} {}\n", key, value);
    }
    else {
        std::cout << std::format("\nError: Logfile {} has not been opened.\n", this->filename) << std::endl;
    }
}

void Log::appendDelete(const std::string& key) {
    // Appends the "del" query to the log file.
    if (this->logfile.is_open()) {
        this->logfile << std::format("del {}\n", key);
    }
    else {
        std::cout << std::format("\nError: Logfile {} has not been opened.\n", this->filename) << std::endl;
    }
}