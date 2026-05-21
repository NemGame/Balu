#pragma once

namespace ast::optimizer {
    void Optimize(ast::Stmt*& block);
}

#include "statements.hpp"
#include "optimizer.hpp"