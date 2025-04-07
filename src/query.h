#ifndef QUERY_H
#define QUERY_H
#include "db.h"

class Query {
    protected:
        DB& database;
        std::unordered_map<std::string, std::string> tips;
    public:
        Query(DB& database);
        virtual ~Query();

        void parseCommand(const std::string& command);
};

#endif