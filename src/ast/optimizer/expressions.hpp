#pragma once

namespace ast::optimizer {
    const map<vector<lexer::TokenKind>, lexer::TokenKind> OperatorLookup = {
        {{lexer::DASH, lexer::DASH}, lexer::PLUS},
        {{lexer::PLUS, lexer::PLUS}, lexer::PLUS},
        {{lexer::DASH, lexer::PLUS}, lexer::DASH},
        {{lexer::PLUS, lexer::DASH}, lexer::DASH},
        {{lexer::NOT, lexer::NOT}, lexer::PLUS},  // Skip double negation
    };
    const vector<const type_info*> TypeLiterals = {&typeid(ast::NumberExpr), &typeid(ast::NullExpr), &typeid(ast::BooleanExpr), &typeid(ast::ByteExpr), &typeid(ast::StringExpr), &typeid(ast::CharExpr)};
    const vector<const type_info*> NumberLiterals = {&typeid(ast::NumberExpr), &typeid(ast::ByteExpr), &typeid(ast::CharExpr)};
    bool isLiteral(Expr* e) {
        for (const auto* lit : TypeLiterals) {
            if (typeid(*e) == *lit) return true;
        }
        return false;
    }
    bool isNumberLiteral(Expr* e) {
        for (const auto* lit : NumberLiterals) {
            if (typeid(*e) == *lit) return true;
        }
        return false;
    }
    void OptimizePrefixExpr(ast::Expr*& baseExpr) {
        ast::PrefixExpr* expr = dynamic_cast<ast::PrefixExpr*>(baseExpr);
        if (!expr) {
            return;
        }
        lexer::Token op = expr->Operator;
        if (auto rightPrefix = dynamic_cast<ast::PrefixExpr*>(expr->RightExpr)) {
            auto key = vector<lexer::TokenKind>{op.kind, rightPrefix->Operator.kind};
            auto it = OperatorLookup.find(key);
            if (it != OperatorLookup.end()) {
                // Fold nested prefix operators into a single one.
                expr->Operator.kind = it->second;
                expr->RightExpr = rightPrefix->RightExpr;
                rightPrefix->RightExpr = nullptr;
                delete rightPrefix;

                if (expr->Operator.kind == lexer::PLUS) {
                    // +x -> x
                    baseExpr = expr->RightExpr;
                    expr->RightExpr = nullptr;
                    delete expr;
                }
                Optimize(baseExpr);
                return;
            }
        }

        if (expr->Operator.kind == lexer::PLUS) {
            // +x -> x
            baseExpr = expr->RightExpr;
            expr->RightExpr = nullptr;
            delete expr;
            Optimize(baseExpr);
        }
    }
    void MergeLiterals(ast::Expr*& baseExpr) {
        ast::BinaryExpr* expr = dynamic_cast<ast::BinaryExpr*>(baseExpr);
        if (!expr) {
            return;
        }
        if (typeid(*expr->left) == typeid(ast::NumberExpr)) {
            ast::NumberExpr* leftNum = dynamic_cast<ast::NumberExpr*>(expr->left);
            if (typeid(*expr->right) == typeid(ast::NumberExpr)) {
                if (expr->op.kind == lexer::PLUS) {
                    leftNum->Add(dynamic_cast<ast::NumberExpr*>(expr->right));
                } else if (expr->op.kind == lexer::STAR) {
                    leftNum->Multiply(dynamic_cast<ast::NumberExpr*>(expr->right));
                } else {
                    return; // Unsupported operator for merging literals
                }
                delete expr->right;
                expr->right = nullptr;
                expr->left = nullptr;  // Ownership moves to baseExpr.
                baseExpr = leftNum;
                delete expr;
            }
        }
    }
}