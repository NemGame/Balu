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
    };

    interface Expr {
    public:
        virtual void expr() = 0;
    };
}