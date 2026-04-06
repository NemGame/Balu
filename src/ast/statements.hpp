#pragma once

namespace ast {
    // {...Stmt[]}
    struct BlockStmt {
        vector<Stmt*> statements;
        void stmt() {}
    };
    struct ExpressionStmt {
        Expr* expression;
        void stmt() {}
    };
}