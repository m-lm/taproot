#pragma once
#include "utils.h"
#include <unordered_map>

// Central file for operations and other keywords for commands
namespace Operation {
    enum class Ops { PUT, DEL, GET, MGET };

    static const std::unordered_map<std::string, Ops> strToOp = {
        {"put", Ops::PUT},
        {"del", Ops::DEL},
        {"get", Ops::GET},
        {"mget", Ops::MGET},
    };

    static const std::unordered_map<Ops, std::string> opToStr = {
        {Ops::PUT, "put"},
        {Ops::DEL, "del"},
        {Ops::GET, "get"},
        {Ops::MGET, "mget"},
    };

    Ops convertStr(std::string str);
}