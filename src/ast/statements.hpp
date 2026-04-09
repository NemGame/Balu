#pragma once

namespace ast {
    // {...Stmt[]}
    struct BlockStmt : public Stmt {
        vector<Stmt*> statements;
        void stmt() override {}
        BlockStmt(vector<Stmt*> statements) : statements(statements) {}
        BlockStmt(BlockStmt&& other) noexcept : statements(std::move(other.statements)) {
            other.statements.clear();
        }
        BlockStmt& operator=(BlockStmt&& other) noexcept {
            if (this != &other) {
                for (auto s : statements) delete s;
                statements = std::move(other.statements);
                other.statements.clear();
            }
            return *this;
        }
        BlockStmt(const BlockStmt&) = delete;
        BlockStmt& operator=(const BlockStmt&) = delete;
        ~BlockStmt() {
            for (auto s : statements) delete s;
        }
        void Dump(int indent = 0, wostream& wcout_ = _wcout) const override {
            wcout_ << wstring(indent * 2, L' ') << L"BlockStmt" << endl;
            for (auto s : statements) {
                if (s) s->Dump(indent + 1, wcout_);
            }
        }
        static BlockStmt Null() { return BlockStmt({}); }
        static BlockStmt* NullPtr() { return new BlockStmt({}); }
    };
    struct ExpressionStmt : public Stmt {
        Expr* expression;
        void stmt() override {}
        ExpressionStmt(Expr* e) : expression(e) {}
        ~ExpressionStmt() {
            delete expression;
        }
        void Dump(int indent = 0, wostream& wcout_ = _wcout) const override {
            wcout_ << wstring(indent * 2, L' ') << L"ExpressionStmt" << endl;
            if (expression) {
                expression->Dump(indent + 1, wcout_);
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
        void Dump(int indent = 0, wostream& wcout_ = _wcout) const override {
            wcout_ << wstring(indent * 2, L' ') << L"VarDeclStmt: " << VariableName << (isConstant ? L" (const)" : L"") << endl;
            if (ExplicitType) {
                ExplicitType->Dump(indent + 1, wcout_);
            }
            if (AssignedValue) {
                AssignedValue->Dump(indent + 1, wcout_);
            }
        }
    };
    struct MethodParameter {
        wstring Name;
        Type* ParamType;
        ast::Expr* DefaultValue;
        bool isConstant;
        MethodParameter(const wstring& n, Type* t, ast::Expr* d, bool c) : Name(n), ParamType(t), DefaultValue(d), isConstant(c) {}
        ~MethodParameter() {
            delete ParamType;
            delete DefaultValue;
        }
        void Dump(int indent = 0, wostream& wcout_ = _wcout) const {
            wcout_ << wstring(indent * 2, L' ') << L"MethodParameter: " << Name << (isConstant ? L" (const)" : L"") << endl;
            if (ParamType) {
                ParamType->Dump(indent + 1, wcout_);
            }
            if (DefaultValue) {
                DefaultValue->Dump(indent + 1, wcout_);
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
        void Dump(int indent = 0, wostream& wcout_ = _wcout) const {
            wcout_ << wstring(indent * 2, L' ') << L"StructProperty: " << Property << (isStatic ? L" (static)" : L"") << endl;
            if (Type) {
                Type->Dump(indent + 1, wcout_);
            }
            if (AssignedValue) {
                AssignedValue->Dump(indent + 1, wcout_);
            }
        }
    };
    struct StructMethod {  // TODO: implement (add fn type)
        bool isStatic;
        wstring MethodName;
        Type* ReturnType;
        Stmt* Body;
        unordered_map<wstring, MethodParameter*> Parameters;
        StructMethod(const wstring& n, Type* t, Stmt* b, bool s, unordered_map<wstring, MethodParameter*> params) : MethodName(n), ReturnType(t), Body(b), isStatic(s), Parameters(params) {}
        ~StructMethod() {
            delete ReturnType;
            delete Body;
            for (auto& p : Parameters) delete p.second;
        }
        void Dump(int indent = 0, wostream& wcout_ = _wcout) const {
            wcout_ << wstring(indent * 2, L' ') << L"StructMethod: " << MethodName << (isStatic ? L" (static)" : L"") << endl;
            if (ReturnType) {
                ReturnType->Dump(indent + 1, wcout_);
            }
            for (const auto& param : Parameters) {
                wcout_ << wstring((indent + 1) * 2, L' ') << L"Parameter: " << param.first << endl;
                if (param.second) param.second->Dump(indent + 2, wcout_);
            }
            if (Body) {
                Body->Dump(indent + 1, wcout_);
            }
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
        void Dump(int indent = 0, wostream& wcout_ = _wcout) const override {
            wcout_ << wstring(indent * 2, L' ') << L"StructDeclStmt: " << StructName << endl;
            for (const auto& p : Properties) {
                p.second->Dump(indent + 1, wcout_);
            }
            for (const auto& m : Methods) {
                m.second->Dump(indent + 1, wcout_);
            }
        }
    };
    struct AliasDeclStmt : public Stmt {
        wstring AliasName;
        ast::Expr* AliasedValue;
        void stmt() override {}
        AliasDeclStmt(const wstring& n, ast::Expr* v) : AliasName(n), AliasedValue(v) {}
        ~AliasDeclStmt() {
            delete AliasedValue;
        }
        void Dump(int indent = 0, wostream& wcout_ = _wcout) const override {
            wcout_ << wstring(indent * 2, L' ') << L"AliasDeclStmt: " << AliasName << endl;
            if (AliasedValue) {
                AliasedValue->Dump(indent + 1, wcout_);
            }
        }
    };
    struct IfStmt : public Stmt {
        ast::Expr* Condition;
        ast::Stmt* ThenBranch;
        ast::Stmt* ElseBranch;  // can be nullptr
        void stmt() override {}
        IfStmt(ast::Expr* condition, ast::Stmt* thenBranch, ast::Stmt* elseBranch) : Condition(condition), ThenBranch(thenBranch), ElseBranch(elseBranch) {}
        ~IfStmt() {
            delete Condition;
            delete ThenBranch;
            delete ElseBranch;
        }
        void Dump(int indent = 0, wostream& wcout_ = _wcout) const override {
            wcout_ << wstring(indent * 2, L' ') << L"IfStmt" << endl;
            if (Condition) {
                wcout_ << wstring((indent + 1) * 2, L' ') << L"Condition:" << endl;
                Condition->Dump(indent + 2, wcout_);
            }
            if (ThenBranch) {
                wcout_ << wstring((indent + 1) * 2, L' ') << L"ThenBranch:" << endl;
                ThenBranch->Dump(indent + 2, wcout_);
            }
            if (ElseBranch) {
                wcout_ << wstring((indent + 1) * 2, L' ') << L"ElseBranch:" << endl;
                ElseBranch->Dump(indent + 2, wcout_);
            }
        }
    };
    struct WhileStmt : public Stmt {
        ast::Expr* Condition;
        ast::Stmt* Body;
        ast::Stmt* ElseBranch;  // can be nullptr
        void stmt() override {}
        WhileStmt(ast::Expr* condition, ast::Stmt* body, ast::Stmt* elseBranch = nullptr) : Condition(condition), Body(body), ElseBranch(elseBranch) {}
        ~WhileStmt() {
            delete Condition;
            delete Body;
            delete ElseBranch;
        }
        void Dump(int indent = 0, wostream& wcout_ = _wcout) const override {
            wcout_ << wstring(indent * 2, L' ') << L"WhileStmt" << endl;
            if (Condition) {
                wcout_ << wstring((indent + 1) * 2, L' ') << L"Condition:" << endl;
                Condition->Dump(indent + 2, wcout_);
            }
            if (Body) {
                wcout_ << wstring((indent + 1) * 2, L' ') << L"Body:" << endl;
                Body->Dump(indent + 2, wcout_);
            }
            if (ElseBranch) {
                wcout_ << wstring((indent + 1) * 2, L' ') << L"ElseBranch:" << endl;
                ElseBranch->Dump(indent + 2, wcout_);
            }
        }
    };
}