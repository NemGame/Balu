#pragma once

namespace ast {
    // Literal expressions
    struct NumberExpr {
        long double value;
        void expr() {}
    };
    struct StringExpr {
        string value;
        void expr() {}
    };
    struct BooleanExpr {
        bool value;
        void expr() {}
    };
    struct SymbolExpr {
        string value;
        void expr() {}
    };
    // Complex expressions

    // 10 + 5 * 2
    struct BinaryExpr {
        Expr* left;
        Expr* right;
        lexer::Token op;
        void expr() {}
    };
}