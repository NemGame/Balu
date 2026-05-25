#pragma once

namespace ast::optimizer {
    void OptimizeIfStmt(ast::Stmt*& baseStmt) {
        ast::IfStmt* stmt = dynamic_cast<ast::IfStmt*>(baseStmt);
        if (!stmt) {
            return;
        }

        auto isEmptyBranch = [](ast::Stmt* branch) {
            if (!branch) {
                return true;
            }

            if (auto block = dynamic_cast<ast::BlockStmt*>(branch)) {
                return block->statements.empty();
            }

            return false;
        };

        if (isEmptyBranch(stmt->ThenBranch) && isEmptyBranch(stmt->ElseBranch)) {
            // If both branches are empty, we can remove the entire if statement
            delete stmt;
            baseStmt = nullptr;
            return;
        }
        if (stmt->ElseBranch) {
            if (dynamic_cast<ast::BlockStmt*>(stmt->ElseBranch) && static_cast<ast::BlockStmt*>(stmt->ElseBranch)->statements.empty()) {
                // If the else branch is an empty block, we can remove it
                stmt->ElseBranch = nullptr;
            }
            if (dynamic_cast<ast::BlockStmt*>(stmt->ElseBranch) && static_cast<ast::BlockStmt*>(stmt->ElseBranch)->statements.size() == 1) {
                // If the else branch is a block with only one statement, we can remove the block
                stmt->ElseBranch = static_cast<ast::BlockStmt*>(stmt->ElseBranch)->statements[0];
                Optimize(stmt->ElseBranch);
            }
        }
    }
    void OptimizeWhileStmt(ast::Stmt*& baseStmt) {
        ast::WhileStmt* stmt = dynamic_cast<ast::WhileStmt*>(baseStmt);
        if (!stmt) {
            return;
        }

        if (!stmt->Body) {
            // If the body is empty, we can remove the entire while statement
            delete stmt;
            baseStmt = nullptr;
            return;
        }
        if (auto innerWhile = dynamic_cast<ast::WhileStmt*>(stmt->Body)) {
            OptimizeWhileStmt(stmt->Body);

            if (!stmt->Body) {
                return;
            }

            innerWhile = dynamic_cast<ast::WhileStmt*>(stmt->Body);
            if (!innerWhile) {
                return;
            }

            // Merge the inner while into the current while
            stmt->Body = innerWhile->Body;
            stmt->Condition = new ast::BinaryExpr(stmt->Condition, innerWhile->Condition, lexer::NewToken(lexer::AND, L"&&"));
        }
    }
    void OptimizeVarDeclStmt(ast::Stmt*& baseStmt) {
        ast::VarDeclStmt* stmt = dynamic_cast<ast::VarDeclStmt*>(baseStmt);
        if (!stmt) {
            return;
        }
        Optimize(stmt->AssignedValue);
    }
}