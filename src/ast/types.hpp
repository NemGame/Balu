#pragma once

namespace ast {
    struct SymbolType : public Type {
        wstring name;  // T
        SymbolType(const wstring& n) : name(n) {}
        void type() override {}
        void Dump(int indent = 0, wostream& wcout_ = _wcout) const override {
            wcout_ << wstring(indent * 2, L' ') << L"SymbolType: " << name << endl;
        }
        wstring GetName(int indent = 0) const {
            return name;
        }
    };
    struct ArrayType : public Type {
        Type* Underlying;  // T[]
        ArrayType(Type* t) : Underlying(t) {}
        ~ArrayType() {
            delete Underlying;
        }
        void type() override {}
        void Dump(int indent = 0, wostream& wcout_ = _wcout) const override {
            wcout_ << wstring(indent * 2, L' ') << L"ArrayType:" << endl;
            Underlying->Dump(indent + 1, wcout_);
        }
        wstring GetName(int indent = 0) const override {
            return Underlying->GetName(indent) + L"[]";
        }
    };
}