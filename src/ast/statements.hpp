#pragma once

namespace ast {
    // {...Stmt[]}
    struct BlockStmt : public Stmt {
        vector<Stmt*> statements;
        void stmt() override {}
        BlockStmt(vector<Stmt*> statements) : statements(statements) {}
        void Dump(int indent = 0) const override {
            wcout << wstring(indent * 2, L' ') << L"BlockStmt" << endl;
            for (auto s : statements) {
                if (s) s->Dump(indent + 1);
            }
        }
    };
    struct ExpressionStmt : public Stmt {
        Expr* expression;
        void stmt() override {}
        ExpressionStmt(Expr* e) : expression(e) {}
        void Dump(int indent = 0) const override {
            wcout << wstring(indent * 2, L' ') << L"ExpressionStmt" << endl;
            if (expression) {
                expression->Dump(indent + 1);
            }
        }
    };
}