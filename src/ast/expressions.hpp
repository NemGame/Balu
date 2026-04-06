#pragma once

namespace ast {
    // Literal expressions
    struct NumberExpr : public Expr {
        long double value;
        NumberExpr(long double v) : value(v) {}
        void expr() override {}
        void Dump(int indent = 0) const override {
            wcout << wstring(indent * 2, L' ') << L"NumberExpr: " << value << endl;
        }
    };
    struct StringExpr : public Expr {
        wstring value;
        StringExpr(const wstring& v) : value(v) {}
        void expr() override {}
        void Dump(int indent = 0) const override {
            wcout << wstring(indent * 2, L' ') << L"StringExpr: \"" << value << L"\"" << endl;
        }
    };
    struct BooleanExpr : public Expr {
        bool value;
        BooleanExpr(bool v) : value(v) {}
        void expr() override {}
        void Dump(int indent = 0) const override {
            wcout << wstring(indent * 2, L' ') << L"BooleanExpr: " << (value ? L"true" : L"false") << endl;
        }
    };
    struct SymbolExpr : public Expr {
        wstring value;
        SymbolExpr(const wstring& v) : value(v) {}
        void expr() override {}
        void Dump(int indent = 0) const override {
            wcout << wstring(indent * 2, L' ') << L"SymbolExpr: " << value << endl;
        }
    };
    using IdentifierExpr = SymbolExpr;
    // Complex expressions

    // 10 + 5 * 2
    struct BinaryExpr : public Expr {
        Expr* left;
        Expr* right;
        lexer::Token op;
        BinaryExpr(Expr* l, Expr* r, lexer::Token o) : left(l), right(r), op(o) {}
        void expr() override {}
        void Dump(int indent = 0) const override {
            wcout << wstring(indent * 2, L' ') << L"BinaryExpr (" << lexer::TokenKindString(op.kind) << L")" << endl;
            if (left) left->Dump(indent + 1);
            if (right) right->Dump(indent + 1);
        }
    };
    struct PrefixExpr : public Expr {
        lexer::Token Operator;
        Expr* RightExpr;
        PrefixExpr(Expr* rightExpr, lexer::Token op) : RightExpr(rightExpr), Operator(op) {}
        void expr() override {}
        void Dump(int indent = 0) const override {
            wcout << wstring(indent * 2, L' ') << L"PrefixExpr (" << lexer::TokenKindString(Operator.kind) << L")" << endl;
            if (RightExpr) RightExpr->Dump(indent + 1);
        }
    };
    // a = a + 5;
    // a += 5;
    // foo.bar += 10;
    struct AssignmentExpr : public Expr {
        Expr* Assignee;
        lexer::Token Operator;
        Expr* Value;  // Right hand side value
        AssignmentExpr(Expr* assigne, lexer::Token op, Expr* rhsValue) : Assignee(assigne), Operator(op), Value(rhsValue) {}
        void expr() override {}
        void Dump(int indent = 0) const override {
            wcout << wstring(indent * 2, L' ') << L"AssignmentExpr (" << lexer::TokenKindString(Operator.kind) << L")" << endl;
            if (Assignee) Assignee->Dump(indent + 1);
            if (Value) Value->Dump(indent + 1);
        }
    };
}