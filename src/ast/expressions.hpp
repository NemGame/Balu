#pragma once

namespace ast {
    // Literal expressions
    struct NumberExpr : public Expr {
        long double value;
        NumberExpr(long double v) : value(v) {}
        void expr() override {}
        void Dump(int indent = 0, wostream& wcout_ = wcout) const override {
            wcout_ << GetName(indent) << endl;
        }
        wstring GetName(int indent = 0) const override {
            return wstring(indent * 2, L' ') + L"NumberExpr: " + to_wstring(value);
        }
    };
    struct ByteExpr : public Expr {
        unsigned char value;
        ByteExpr(unsigned char v) : value(v) {}
        void expr() override {}
        void Dump(int indent = 0, wostream& wcout_ = wcout) const override {
            wcout_ << GetName(indent) << endl;
        }
        wstring GetName(int indent = 0) const override {
            return wstring(indent * 2, L' ') + L"ByteExpr: " + to_wstring(value);
        }
    };
    struct StringExpr : public Expr {
        wstring value;
        StringExpr(const wstring& v) : value(v) {}
        void expr() override {}
        void Dump(int indent = 0, wostream& wcout_ = wcout) const override {
            wcout_ << GetName(indent) << endl;
        }
        wstring GetName(int indent = 0) const override {
            return wstring(indent * 2, L' ') + L"StringExpr: \"" + value + L"\"";
        }
    };
    struct CharExpr : public Expr {
        wchar_t value;
        CharExpr(wchar_t v) : value(v) {}
        void expr() override {}
        void Dump(int indent = 0, wostream& wcout_ = wcout) const override {
            wcout_ << wstring(indent * 2, L' ') << L"CharExpr: '" << value << L"'" << endl;
        }
        wstring GetName(int indent = 0) const override {
            return wstring(indent * 2, L' ') + L"CharExpr: '" + value + L"'";
        }
    };
    struct BooleanExpr : public Expr {
        bool value;
        BooleanExpr(bool v) : value(v) {}
        void expr() override {}
        void Dump(int indent = 0, wostream& wcout_ = wcout) const override {
            wcout_ << GetName(indent) << endl;
        }
        wstring GetName(int indent = 0) const override {
            return wstring(indent * 2, L' ') + L"BooleanExpr: " + (value ? L"true" : L"false");
        }
    };
    struct NullExpr : public Expr {
        void expr() override {}
        void Dump(int indent = 0, wostream& wcout_ = wcout) const override {
            wcout_ << GetName(indent) << endl;
        }
        wstring GetName(int indent = 0) const override {
            return wstring(indent * 2, L' ') + L"NullExpr: null";
        }
    };
    struct SymbolExpr : public Expr {
        wstring value;
        SymbolExpr(const wstring& v) : value(v) {}
        void expr() override {}
        void Dump(int indent = 0, wostream& wcout_ = wcout) const override {
            wcout_ << GetName(indent) << endl;
        }
        wstring GetName(int indent = 0) const {
            return wstring(indent * 2, L' ') + L"SymbolExpr: " + value;
        }
    };
    using IdentifierExpr = SymbolExpr;
    struct RuleExpr : public Expr {
        vector<wstring> value;
        RuleExpr(const vector<wstring>& v) : value(v) {}
        RuleExpr(const wstring& v) : value(wstringToVector(v, L' ')) {}
        void expr() override {}
        void Dump(int indent = 0, wostream& wcout_ = wcout) const override {
            wcout_ << GetName(indent) << endl;
        }
        wstring GetName(int indent = 0) const {
            wstring result = wstring(indent * 2, L' ') + L"RuleExpr: [";
            for (const auto& v : value) {
                result += v + L", ";
            }
            if (!value.empty()) {
                result = result.substr(0, result.size() - 2); // Remove the last ", "
            }
            result += L"]";
            return result;
        }
    };
    // Complex expressions

    // 10 + 5 * 2
    struct BinaryExpr : public Expr {
        Expr* left;
        Expr* right;
        lexer::Token op;
        BinaryExpr(Expr* l, Expr* r, lexer::Token o) : left(l), right(r), op(o) {}
        ~BinaryExpr() {
            delete left;
            delete right;
        }
        void expr() override {}
        void Dump(int indent = 0, wostream& wcout_ = wcout) const override {
            wcout_ << GetName(indent) << endl;
        }
        wstring GetName(int indent = 0) const override {
            wstring str = wstring(indent * 2, L' ') + L"BinaryExpr (" + lexer::TokenKindString(op.kind) + L")";
            if (left) str += L"\n" + left->GetName(indent + 1);
            if (right) str += L"\n" + right->GetName(indent + 1);
            return str;
        }
    };
    struct PrefixExpr : public Expr {
        lexer::Token Operator;
        Expr* RightExpr;
        PrefixExpr(Expr* rightExpr, lexer::Token op) : RightExpr(rightExpr), Operator(op) {}
        ~PrefixExpr() {
            delete RightExpr;
        }
        void expr() override {}
        void Dump(int indent = 0, wostream& wcout_ = wcout) const override {
            wcout_ << GetName(indent) << endl;
        }
        wstring GetName(int indent = 0) const override {
            wstring str = wstring(indent * 2, L' ') + L"PrefixExpr (" + lexer::TokenKindString(Operator.kind) + L")";
            if (RightExpr) str += L"\n" + RightExpr->GetName(indent + 1);
            return str;
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
        ~AssignmentExpr() {
            delete Assignee;
            delete Value;
        }
        void expr() override {}
        void Dump(int indent = 0, wostream& wcout_ = wcout) const override {
            wcout_ << GetName(indent) << endl;
        }
        wstring GetName(int indent = 0) const override {
            wstring str = wstring(indent * 2, L' ') + L"AssignmentExpr (" + lexer::TokenKindString(Operator.kind) + L")";
            if (Assignee) str += L"\n" + Assignee->GetName(indent + 1);
            if (Value) str += L"\n" + Value->GetName(indent + 1);
            return str;
        }
    };
    struct StructInstantiationExpr : public Expr {
        wstring StructName;
        unordered_map<wstring, Expr*> Properties;
        StructInstantiationExpr(const wstring& structName, const unordered_map<wstring, Expr*>& properties) : StructName(structName), Properties(properties) {}
        ~StructInstantiationExpr() {
            for (auto& prop : Properties) {
                delete prop.second;
            }
        }
        void expr() override {}
        void Dump(int indent = 0, wostream& wcout_ = wcout) const override {
            wcout_ << GetName(indent) << endl;
        }
        wstring GetName(int indent = 0) const override {
            wstring str = wstring(indent * 2, L' ') + L"StructInstantiationExpr: " + StructName;
            for (const auto& prop : Properties) {
                str += L"\n" + wstring((indent + 1) * 2, L' ') + L"Property: " + prop.first;
                if (prop.second) str += L"\n" + wstring((indent + 2) * 2, L' ') + L"Value:\n" + prop.second->GetName(indent + 3);
            }
            return str;
        }
    };
    struct ArrayInstantiationExpr : public Expr {
        Type* UnderlyingType;
        vector<Expr*> Contents;
        ArrayInstantiationExpr(Type* underlyingType, const vector<Expr*>& contents) : UnderlyingType(underlyingType), Contents(contents) {}
        ~ArrayInstantiationExpr() {
            delete UnderlyingType;
            for (Expr* expr : Contents) {
                delete expr;
            }
        }
        void expr() override {}
        void Dump(int indent = 0, wostream& wcout_ = wcout) const override {
            wcout_ << GetName(indent) << endl;
        }
        wstring GetName(int indent = 0) const override {
            wstring str = wstring(indent * 2, L' ') + L"ArrayInstantiationExpr: ";
            if (UnderlyingType) str += L"\n" + wstring((indent + 1) * 2, L' ') + L"UnderlyingType: " + UnderlyingType->GetName();
            str += L"\n" + wstring((indent + 1) * 2, L' ') + L"Contents:";
            for (Expr* expr : Contents) {
                if (expr) str += L"\n" + expr->GetName(indent + 2);
            }
            return str;
        }
    };
}