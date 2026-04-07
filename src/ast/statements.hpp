#pragma once

namespace ast {
    // {...Stmt[]}
    struct BlockStmt : public Stmt {
        vector<Stmt*> statements;
        void stmt() override {}
        BlockStmt(vector<Stmt*> statements) : statements(statements) {}
        ~BlockStmt() {
            for (auto s : statements) delete s;
        }
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
        ~ExpressionStmt() {
            delete expression;
        }
        void Dump(int indent = 0) const override {
            wcout << wstring(indent * 2, L' ') << L"ExpressionStmt" << endl;
            if (expression) {
                expression->Dump(indent + 1);
            }
        }
    };
    struct VarDeclStmt : public Stmt {
        wstring VariableName;
        bool isConstant;
        Expr* AssignedValue;
        Type* ExplicitType;
        void stmt() override {}
        VarDeclStmt(const wstring& n, bool c, Expr* init, Type* type) : VariableName(n), isConstant(c), AssignedValue(init), ExplicitType(type) {}
        ~VarDeclStmt() {
            delete AssignedValue;
            delete ExplicitType;
        }
        void Dump(int indent = 0) const override {
            wcout << wstring(indent * 2, L' ') << L"VarDeclStmt: " << VariableName << (isConstant ? L" (const)" : L"") << endl;
            if (ExplicitType) {
                ExplicitType->Dump(indent + 1);
            }
            if (AssignedValue) {
                AssignedValue->Dump(indent + 1);
            }
        }
    };
}