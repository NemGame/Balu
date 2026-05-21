#pragma once

namespace ast::optimizer {
    void Optimize(ast::Stmt*& block) {
        if (!block) {
            return;
        }
        if (auto blockStmt = dynamic_cast<ast::BlockStmt*>(block)) {
            vector<ast::Stmt*> optimizedBody;
            for (ast::Stmt*& stmt : blockStmt->statements) {
                Optimize(stmt);
                if (stmt) { // Only add non-null statements
                    optimizedBody.push_back(stmt);
                }
            }
            blockStmt->statements = optimizedBody;
        } else if (dynamic_cast<ast::IfStmt*>(block)) {
            OptimizeIfStmt(block);
        } else if (dynamic_cast<ast::WhileStmt*>(block)) {
            OptimizeWhileStmt(block);
        }
    }
}