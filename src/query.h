#ifndef QUERY_H
#define QUERY_H
#include "db.h"

class Query {
    protected:
        DB& database;
    public:
        Query(DB& database);
        virtual ~Query();

        void parseCommand(const std::string& command, const bool replay=false);
};

#endif