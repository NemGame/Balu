#pragma once

namespace ast::optimizer {
    map<vector<lexer::TokenKind>, lexer::TokenKind> OperatorLookup = {
        {{lexer::DASH, lexer::DASH}, lexer::PLUS},
        {{lexer::PLUS, lexer::PLUS}, lexer::PLUS},
        {{lexer::DASH, lexer::PLUS}, lexer::DASH},
        {{lexer::PLUS, lexer::DASH}, lexer::DASH},
        {{lexer::NOT, lexer::NOT}, lexer::PLUS},  // Skip double negation
    };
    void OptimizePrefixExpr(ast::Expr*& baseExpr) {
        ast::PrefixExpr* expr = dynamic_cast<ast::PrefixExpr*>(baseExpr);
        if (!expr) {
            return;
        }
        lexer::Token op = expr->Operator;
        if (auto rightPrefix = dynamic_cast<ast::PrefixExpr*>(expr->RightExpr)) {
            auto key = vector<lexer::TokenKind>{op.kind, rightPrefix->Operator.kind};
            if (OperatorLookup.count(key)) {
                // Fold nested prefix operators into a single one.
                expr->Operator.kind = OperatorLookup[key];
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
}