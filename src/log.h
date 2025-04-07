#ifndef LOG_H
#define LOG_H
#include <fstream>

class Log {
    protected:
        const std::string& filename;
        std::ofstream logfile;
    public:
        Log(const std::string& filename);
        virtual ~Log();

        void appendPut(const std::string& key, const std::string& value);
        void appendDelete(const std::string& key);
};

#endif