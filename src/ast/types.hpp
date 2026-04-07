#pragma once

namespace ast {
    struct SymbolType : public Type {
        wstring name;  // T
        SymbolType(const wstring& n) : name(n) {}
        void type() override {}
        void Dump(int indent = 0) const override {
            wcout << wstring(indent * 2, L' ') << L"SymbolType: " << name << endl;
        }
        wstring GetName() const {
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
        void Dump(int indent = 0) const override {
            wcout << wstring(indent * 2, L' ') << L"ArrayType:" << endl;
            Underlying->Dump(indent + 1);
        }
        wstring GetName() const {
            return Underlying->GetName() + L"[]";
        }
    };
    struct PointerType : public Type {
        Type* Underlying;  // *T
        PointerType(Type* t) : Underlying(t) {}
        ~PointerType() {
            delete Underlying;
        }
        void type() override {}
        void Dump(int indent = 0) const override {
            wcout << wstring(indent * 2, L' ') << L"PointerType:" << endl;
            Underlying->Dump(indent + 1);
        }
        wstring GetName() const {
            return L"*" + Underlying->GetName();
        }
    };
}