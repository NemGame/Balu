#pragma once

namespace ast::optimizer {
    struct TypeInfoBefore {
        bool operator()(const type_info* a, const type_info* b) const {
            return a->before(*b);
        }
    };

    const map<const type_info*, vector<const type_info*>, TypeInfoBefore> TokenTypeCompatibility = {
        {&typeid(ast::NumberExpr), {&typeid(ast::ByteExpr), &typeid(ast::CharExpr)}},
        {&typeid(ast::CharExpr), {&typeid(ast::ByteExpr), &typeid(ast::NumberExpr)}},
        {&typeid(ast::BooleanExpr), {&typeid(ast::NumberExpr), &typeid(ast::ByteExpr)}},
        {&typeid(ast::ByteExpr), {&typeid(ast::NumberExpr), &typeid(ast::CharExpr), &typeid(ast::BooleanExpr)}},
    };
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
        
        Optimize(stmt->Condition);
        if (auto booleanExpr = dynamic_cast<ast::BooleanExpr*>(stmt->Condition)) {
            if (booleanExpr->value) {
                // Convert to block statement and ignore else branch

                // Copy then branch to avoid deleting it when we delete the if statement
                ast::Stmt* copiedThenBranch = stmt->ThenBranch->Clone();
                delete stmt;
                baseStmt = copiedThenBranch;
                Optimize(baseStmt);
                return;
            } else {
                // Copy else branch to avoid deleting it when we delete the if statement
                ast::Stmt* copiedElseBranch = stmt->ElseBranch->Clone();
                delete stmt;
                baseStmt = copiedElseBranch;
                Optimize(baseStmt);
                return;
            }
        }
        
        if (isEmptyBranch(stmt->ThenBranch) && isEmptyBranch(stmt->ElseBranch)) {
            // If both branches are empty, we can remove the entire if statement
            delete stmt;
            baseStmt = nullptr;
            return;
        }
        if (!isEmptyBranch(stmt->ThenBranch)) Optimize(stmt->ThenBranch);
        if (stmt->ElseBranch) {
            Optimize(stmt->ElseBranch);
            if (dynamic_cast<ast::BlockStmt*>(stmt->ElseBranch) && static_cast<ast::BlockStmt*>(stmt->ElseBranch)->statements.empty()) {
                // If the else branch is an empty block, we can remove it
                stmt->ElseBranch = nullptr;
            }
            if (dynamic_cast<ast::BlockStmt*>(stmt->ElseBranch) && static_cast<ast::BlockStmt*>(stmt->ElseBranch)->statements.size() == 1) {
                // If the else branch is a block with only one statement, we can remove the block
                stmt->ElseBranch = static_cast<ast::BlockStmt*>(stmt->ElseBranch)->statements[0];
                Optimize(stmt->ElseBranch);
            }
            if (isEmptyBranch(stmt->ThenBranch)) {
                if (auto thenBranch = dynamic_cast<ast::BlockStmt*>(stmt->ThenBranch)) {
                    if (thenBranch->statements.empty()) {
                        delete stmt->ThenBranch;
                        stmt->ThenBranch = stmt->ElseBranch;
                        stmt->Condition = new ast::PrefixExpr(stmt->Condition, lexer::NewToken(lexer::NOT, L"!"));
                        stmt->ElseBranch = nullptr;
                    }
                }
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
        if (stmt->ExplicitType->GetName() == L"auto" && isLiteral(stmt->AssignedValue)) {
            delete stmt->ExplicitType;
            stmt->ExplicitType = ExpressionToType(stmt->AssignedValue);
        } else {
            ast::Type* assignedValueType = ExpressionToType(stmt->AssignedValue);
            if (assignedValueType) {
                // Check if assigned value type is compatible with the variable's explicit type
                const bool isCompatible = stmt->ExplicitType->GetName() == assignedValueType->GetName() ||
                    stmt->ExplicitType->GetName() == L"any" ||
                    (TokenTypeCompatibility.count(&typeid(*assignedValueType)) > 0 &&
                     find(TokenTypeCompatibility.at(&typeid(*assignedValueType)).begin(), TokenTypeCompatibility.at(&typeid(*assignedValueType)).end(), &typeid(*stmt->ExplicitType)) != TokenTypeCompatibility.at(&typeid(*assignedValueType)).end());
                if (isCompatible) {
                    delete assignedValueType; // No need for this since it's compatible
                } else {
                    const wstring message = L"Type mismatch: cannot assign value of type " + assignedValueType->GetName() + L" to variable of type " + stmt->ExplicitType->GetName();
                    wcout << message << endl;
                    if (_panic) {
                        if (_debug) wcout << L"[Parser] Panicing" << endl;
                        exit(1);
                    }
                    delete stmt->ExplicitType;
                    stmt->ExplicitType = assignedValueType;
                }
            }
        }
    }
}