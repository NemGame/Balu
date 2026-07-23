#pragma once

namespace ast::optimizer {
    // If `x` may be assigned a value of type `y`
    // ```ts
    // let foo:y;
    // let bar:x = foo;
    // ```
    const map<wstring, vector<wstring>> TokenTypeCompatibility = {
        {L"number", {L"byte", L"char32", L"char16", L"char8", L"bool"}},
        {L"char32", {L"byte", L"number", L"bool"}},
        {L"char16", {L"byte", L"number", L"char32", L"bool"}},
        {L"char8", {L"byte", L"number", L"char16", L"char32", L"bool"}},
        {L"string32", {L"bool"}},
        {L"string16", {L"bool", L"string32"}},
        {L"string8", {L"bool", L"string16", L"string32"}},
        {L"bool", {L"number", L"byte"}},
        {L"byte", {L"number", L"char8", L"bool"}},
        {L"void", {L"null"}},
        {L"null", {L"void"}},
    };
    struct Helper {
        static bool isEmptyBranch(ast::Stmt* branch) {
            if (!branch) {
                return true;
            }

            if (auto block = dynamic_cast<ast::BlockStmt*>(branch)) {
                return block->statements.empty();
            }

            return false;
        };
    };
    void OptimizeIfStmt(ast::Stmt*& baseStmt) {
        ast::IfStmt* stmt = dynamic_cast<ast::IfStmt*>(baseStmt);
        if (!stmt) {
            return;
        }

        Optimize(stmt->Condition);
        if (auto booleanExpr = dynamic_cast<ast::BooleanExpr*>(stmt->Condition)) {
            if (booleanExpr->value) {
                // Convert to block statement and ignore else branch

                // Copy then branch to avoid deleting it when we delete the if statement
                ast::Stmt* copiedThenBranch = stmt->ThenBranch ? stmt->ThenBranch->Clone() : nullptr;
                delete stmt;
                baseStmt = copiedThenBranch;
                if (baseStmt) Optimize(baseStmt);
                return;
            } else {
                // Copy else branch to avoid deleting it when we delete the if statement
                ast::Stmt* copiedElseBranch = stmt->ElseBranch ? stmt->ElseBranch->Clone() : nullptr;
                delete stmt;
                baseStmt = copiedElseBranch;
                if (baseStmt) Optimize(baseStmt);
                return;
            }
        }
        
        if (Helper::isEmptyBranch(stmt->ThenBranch) && Helper::isEmptyBranch(stmt->ElseBranch)) {
            // If both branches are empty, we can remove the entire if statement
            delete stmt;
            baseStmt = nullptr;
            return;
        }
        if (!Helper::isEmptyBranch(stmt->ThenBranch)) Optimize(stmt->ThenBranch);
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
            if (Helper::isEmptyBranch(stmt->ThenBranch)) {
                if (auto thenBranch = dynamic_cast<ast::BlockStmt*>(stmt->ThenBranch)) {
                    if (thenBranch->statements.empty()) {
                        delete stmt->ThenBranch;
                        stmt->ThenBranch = stmt->ElseBranch;
                        stmt->Condition = new ast::PrefixExpr(stmt->Condition, lexer::NewToken(lexer::NOT, L"!"));
                        Optimize(stmt->Condition);
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
        const bool isBodyEmpty = Helper::isEmptyBranch(stmt->Body);
        if (Helper::isEmptyBranch(stmt->ElseBranch)) {
            delete stmt->ElseBranch;
            stmt->ElseBranch = nullptr;
            if (isBodyEmpty) {
                ast::Expr* condition = stmt->Condition;
                stmt->Condition = nullptr;
                delete stmt;
                ast::ExpressionStmt* exprStmt = new ast::ExpressionStmt(condition);
                baseStmt = exprStmt;
                Optimize(baseStmt);
                if (isLiteral(exprStmt->expression)) {
                    delete baseStmt;
                    baseStmt = nullptr;
                }
            }
        } else Optimize(stmt->ElseBranch);
        Optimize(stmt->Condition);
        if (auto booleanExpr = dynamic_cast<ast::BooleanExpr*>(stmt->Condition)) {
            if (!booleanExpr->value) {
                ast::Stmt* elseBranch = stmt->ElseBranch;
                stmt->ElseBranch = nullptr;
                delete stmt;
                baseStmt = elseBranch;
                if (baseStmt) Optimize(baseStmt);
                return;
            }
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
            ast::Stmt* mergedBody = innerWhile->Body;
            ast::Expr* mergedCondition = innerWhile->Condition;
            innerWhile->Body = nullptr;
            innerWhile->Condition = nullptr;
            delete innerWhile;

            stmt->Body = mergedBody;
            stmt->Condition = new ast::BinaryExpr(stmt->Condition, mergedCondition, lexer::NewToken(lexer::AND, L"&&"));
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
                const wstring assignedValueTypeName = assignedValueType->GetName();
                const wstring explicitTypeName = stmt->ExplicitType->GetName();
                const bool isTypeNull = assignedValueTypeName == L"null";
                const bool isCompatible = isTypeNull || explicitTypeName == assignedValueTypeName ||
                    explicitTypeName == L"any" ||
                    (
                        TokenTypeCompatibility.count(assignedValueTypeName) > 0 &&
                        find(
                            TokenTypeCompatibility.at(assignedValueTypeName).begin(), 
                            TokenTypeCompatibility.at(assignedValueTypeName).end(), 
                            explicitTypeName
                        ) != TokenTypeCompatibility.at(assignedValueTypeName).end()
                    );
                if (isCompatible) {
                    delete assignedValueType; // No need for this since it's compatible
                } else {
                    const wstring message = L"Type mismatch: cannot assign value of type " + assignedValueType->GetName() + L" to variable of type " + stmt->ExplicitType->GetName();
                    _wcout << message << endl;
                    if (CompilerOptions.panic) {
                        if (CompilerOptions.debug) _wcout << L"[Parser] Panicing" << endl;
                        exit(1);
                    }
                    delete stmt->ExplicitType;
                    stmt->ExplicitType = assignedValueType;
                }
            }
        }
    }
}