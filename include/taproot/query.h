#pragma once
#include "db.h"

class Query {
    protected:
        DB& database;
        inline static const std::unordered_map<std::string, std::string> tips = {
            {"put", "put <KEY> <VALUE>"},
            {"del", "del <KEY>"},
            {"get", "get <KEY>"},
            {"mget", "mget <KEY1> <KEY2> ... <KEYN>"},
            {"mdel", "mdel <KEY1> <KEY2> ... <KEYN>"},
        };

    public:
        Query(DB& database);
        virtual ~Query();

        void parseCommand(const std::string& command);
};