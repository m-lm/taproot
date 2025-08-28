#include "taproot/operations.h"

namespace Operation {
    Ops convertStr(std::string str) {
        // Convert from raw string to equivalent enum operation
        str = toLower(str);
        auto iter = strToOp.find(str);
        if (iter != strToOp.end()) {
            return iter->second;
        }
        else {
            throw std::invalid_argument("Unknown operation: " + str);
        }
    }
}