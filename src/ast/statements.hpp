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
    struct StructProperty {
        wstring Property;
        ast::Type* Type;
        ast::Expr* AssignedValue;
        bool isStatic;
        StructProperty(const wstring& p, ast::Type* t, ast::Expr* v, bool s) : Property(p), Type(t), AssignedValue(v), isStatic(s) {}
        ~StructProperty() {
            delete Type;
            delete AssignedValue;
        }
        void Dump(int indent = 0) const {
            wcout << wstring(indent * 2, L' ') << L"StructProperty: " << Property << (isStatic ? L" (static)" : L"") << endl;
            if (Type) {
                Type->Dump(indent + 1);
            }
            if (AssignedValue) {
                AssignedValue->Dump(indent + 1);
            }
        }
    };
    struct StructMethod {  // TODO: implement (add fn type)
        bool isStatic;
        wstring MethodName;
        void Dump(int indent = 0) const {
            wcout << wstring(indent * 2, L' ') << L"StructMethod: " << MethodName << (isStatic ? L" (static)" : L"") << endl;
        }
    };
    struct StructDeclStmt : public Stmt {
        wstring StructName;
        unordered_map<wstring, StructProperty*> Properties;
        unordered_map<wstring, StructMethod*> Methods;
        StructDeclStmt(const wstring& n, const unordered_map<wstring, StructProperty*>& p, const unordered_map<wstring, StructMethod*>& m) : StructName(n), Properties(p), Methods(m) {}
        ~StructDeclStmt() {
            for (auto& p : Properties) delete p.second;
            for (auto& m : Methods) delete m.second;
        }
        void stmt() override {}
        void Dump(int indent = 0) const override {
            wcout << wstring(indent * 2, L' ') << L"StructDeclStmt: " << StructName << endl;
            for (const auto& p : Properties) {
                p.second->Dump(indent + 1);
            }
            for (const auto& m : Methods) {
                m.second->Dump(indent + 1);
            }
        }
    };
}