#ifndef QUERY_H
#define QUERY_H
#include "db.h"

class Query {
    protected:
        DB& database;
    public:
        Query(DB& database);
        virtual ~Query();

        std::vector<std::string> tokenize(const std::string& input);
        void parseCommand(const std::string& command);
};

#endif