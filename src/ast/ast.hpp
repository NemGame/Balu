#pragma once

namespace ast {
    /* GO
    type Stmt interface {
        stmt()
    }
    */

    interface Stmt {
    public:
        virtual ~Stmt() = default;
        virtual void stmt() = 0;
        virtual void Dump(int indent = 0, wostream& wcout_ = wcout) const = 0;
    };

    interface Expr {
    public:
        virtual ~Expr() = default;
        virtual void expr() = 0;
        virtual void Dump(int indent = 0, wostream& wcout_ = wcout) const = 0;
        virtual wstring GetName(int indent = 0) const = 0;
    };

    interface Type {
    public:
        virtual ~Type() = default;
        virtual void type() = 0;
        virtual void Dump(int indent = 0, wostream& wcout_ = wcout) const = 0;
        virtual wstring GetName() const = 0;
    };
}