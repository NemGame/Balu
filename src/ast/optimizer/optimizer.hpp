#pragma once

namespace ast::optimizer {
    void Optimize(ast::Stmt*& block) {
        if (!block) {
            return;
        }
        if (auto blockStmt = dynamic_cast<ast::BlockStmt*>(block)) {
            if (blockStmt->statements.empty()) {
                delete block;
                block = nullptr;
                return;
            }
            if (blockStmt->statements.size() == 1) {
                auto innerBlock = dynamic_cast<ast::BlockStmt*>(blockStmt->statements[0]);
                if (innerBlock) {
                    // If the block has only one statement and that statement is a block, we can merge the inner block into the current block
                    blockStmt->statements = innerBlock->statements;
                    innerBlock->statements.clear(); // Prevent the destructor from deleting the statements
                    Optimize(block); // Optimize the merged block
                    delete innerBlock; // Delete the inner block
                }
            }
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
        } else if (dynamic_cast<ast::VarDeclStmt*>(block)) {
            OptimizeVarDeclStmt(block);
        } else if (dynamic_cast<ast::ExpressionStmt*>(block)) {
            ast::ExpressionStmt* exprStmt = dynamic_cast<ast::ExpressionStmt*>(block);
            Optimize(exprStmt->expression);
        }
    }
    void Optimize(ast::Expr*& expr) {
        if (!expr) {
            return;
        }
        if (auto prefixExpr = dynamic_cast<ast::PrefixExpr*>(expr)) {
            OptimizePrefixExpr(expr);
        } else if (auto binaryExpr = dynamic_cast<ast::BinaryExpr*>(expr)) {
            Optimize(binaryExpr->left);
            Optimize(binaryExpr->right);
            MergeLiterals(expr);
        }
    }
}