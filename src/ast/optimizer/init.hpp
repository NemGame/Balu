#pragma once

namespace ast::optimizer {
    void Optimize(ast::Stmt*& block);
    void Optimize(ast::Expr*& expr);
}

#include "expressions.hpp"
#include "statements.hpp"
#include "optimizer.hpp"