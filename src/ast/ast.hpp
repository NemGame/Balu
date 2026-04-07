#pragma once

namespace ast {
    /* GO
    type Stmt interface {
        stmt()
    }
    */

    interface Stmt {
    public:
        virtual void stmt() = 0;
        virtual void Dump(int indent = 0) const = 0;
    };

    interface Expr {
    public:
        virtual void expr() = 0;
        virtual void Dump(int indent = 0) const = 0;
    };

    interface Type {
    public:
        virtual void type() = 0;
        virtual void Dump(int indent = 0) const = 0;
        virtual wstring GetName() const = 0;
    };
}