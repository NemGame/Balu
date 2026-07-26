#pragma once

#include "nodes.hpp"

namespace IR {
    Node* ConvertASTToIR(const ast::Stmt* stmt);
    Node* ConvertASTToIR(const ast::Expr* expr);
    struct _IRGlobals {
    private:
        unordered_map<string, size_t> idmap;
        size_t currentScope = 0;
    public:
        size_t GetUniqueID(string name = "") {
            if (idmap.find(name) == idmap.end()) {
                idmap[name] = 0;
            }
            return idmap[name]++;
        }
        size_t GetNewScope() {
            return ++currentScope;
        }
        size_t GetCurrentScope() {
            return currentScope;
        }
        size_t RemoveScope() {
            if (currentScope > 0) {
                return --currentScope;
            }
            return 0;
        }
    };
    _IRGlobals Globals;
}

#include "optimizer/init.hpp"
#include "statements.hpp"
#include "expressions.hpp"
#include "convert.hpp"
#include "IR.hpp"