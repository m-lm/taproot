#pragma once
#include "db.h"

class Query {
    protected:
        DB& database;

    public:
        Query(DB& database);
        virtual ~Query();

        void parseCommand(const std::string& command, const bool replay=false);
};