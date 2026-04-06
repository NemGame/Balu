#pragma once

namespace ast {
    struct SymbolType : public Type {
        wstring name;  // T
        SymbolType(const wstring& n) : name(n) {}
        void type() override {}
        void Dump(int indent = 0) const override {
            wcout << wstring(indent * 2, L' ') << L"SymbolType: " << name << endl;
        }
    };
    struct ArrayType : public Type {
        Type* Underlying;  // T[]
        ArrayType(Type* t) : Underlying(t) {}
        void type() override {}
        void Dump(int indent = 0) const override {
            wcout << wstring(indent * 2, L' ') << L"ArrayType:" << endl;
            Underlying->Dump(indent + 1);
        }
    };
}