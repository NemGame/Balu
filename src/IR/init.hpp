#pragma once

#include "nodes.hpp"

namespace IR {
    struct _IRGlobals {
        unsigned long long UniqueIDCounter = 0;
        unsigned long long GetUniqueID() {
            return UniqueIDCounter++;
        }
    };
    _IRGlobals Globals;
}

#include "convert.hpp"
#include "IR.hpp"