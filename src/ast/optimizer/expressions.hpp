#pragma once

namespace ast::optimizer {
    const map<vector<lexer::TokenKind>, lexer::TokenKind> OperatorLookup = {
        {{lexer::DASH, lexer::DASH}, lexer::PLUS},
        {{lexer::PLUS, lexer::PLUS}, lexer::PLUS},
        {{lexer::DASH, lexer::PLUS}, lexer::DASH},
        {{lexer::PLUS, lexer::DASH}, lexer::DASH},
        {{lexer::NOT, lexer::NOT}, lexer::PLUS},  // Skip double negation
    };
    const map<lexer::TokenKind, wstring> OperatorToString = {
        {lexer::PLUS, L"+"},
        {lexer::DASH, L"-"},
        {lexer::NOT, L"!"},
        {lexer::BITWISE_NOT, L"~"},
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
                expr->Operator.value = OperatorToString.at(it->second);
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
                } else if (expr->op.kind == lexer::EQUALS) {  // ==
                    bool value = leftNum->Equals(dynamic_cast<ast::NumberExpr*>(expr->right));
                    delete baseExpr;
                    baseExpr = new ast::BooleanExpr(value);
                    return;
                } else if (expr->op.kind == lexer::NOT_EQUALS) {  // !=
                    bool value = !leftNum->Equals(dynamic_cast<ast::NumberExpr*>(expr->right));
                    delete baseExpr;
                    baseExpr = new ast::BooleanExpr(value);
                    return;
                } else if (expr->op.kind == lexer::LESS) {  // <
                    bool value = leftNum->isLessthan(dynamic_cast<ast::NumberExpr*>(expr->right));
                    delete baseExpr;
                    baseExpr = new ast::BooleanExpr(value);
                    return;
                } else if (expr->op.kind == lexer::GREATER) {  // >
                    bool value = leftNum->isGreaterThan(dynamic_cast<ast::NumberExpr*>(expr->right));
                    delete baseExpr;
                    baseExpr = new ast::BooleanExpr(value);
                    return;
                } else if (expr->op.kind == lexer::LESS_EQUALS) {  // <=
                    bool value = leftNum->isLessThanOrEqual(dynamic_cast<ast::NumberExpr*>(expr->right));
                    delete baseExpr;
                    baseExpr = new ast::BooleanExpr(value);
                    return;
                } else if (expr->op.kind == lexer::GREATER_EQUALS) {  // >=
                    bool value = leftNum->isGreaterThanOrEqual(dynamic_cast<ast::NumberExpr*>(expr->right));
                    delete baseExpr;
                    baseExpr = new ast::BooleanExpr(value);
                    return;
                } else if (expr->op.kind == lexer::DASH) {  // -
                    leftNum->Minus(dynamic_cast<ast::NumberExpr*>(expr->right));
                } else if (expr->op.kind == lexer::BITWISE_AND) {  // &
                    leftNum->BitwiseAnd(dynamic_cast<ast::NumberExpr*>(expr->right));
                } else if (expr->op.kind == lexer::BITWISE_OR) {  // |
                    leftNum->BitwiseOr(dynamic_cast<ast::NumberExpr*>(expr->right));
                } else if (expr->op.kind == lexer::BITWISE_XOR) {  // ^
                    leftNum->BitwiseXor(dynamic_cast<ast::NumberExpr*>(expr->right));
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
    ast::Type* ExpressionToType(ast::Expr* expr) {
        // NumberExpr -> Number, StringExpr -> String, etc.
        if (typeid(*expr) == typeid(ast::NumberExpr)) {
            return new ast::SymbolType(L"number");
        } else if (typeid(*expr) == typeid(ast::StringExpr)) {
            switch (dynamic_cast<ast::StringExpr*>(expr)->byteSize) {
                case 1: return new ast::SymbolType(L"string8");
                case 2: return new ast::SymbolType(L"string16");
                case 4: return new ast::SymbolType(L"string32");
                default: return new ast::SymbolType(L"string32"); // Fallback for unknown byte sizes
            }
        } else if (typeid(*expr) == typeid(ast::CharExpr)) {
            switch (dynamic_cast<ast::CharExpr*>(expr)->byteSize) {
                case 1: return new ast::SymbolType(L"char8");
                case 2: return new ast::SymbolType(L"char16");
                case 4: return new ast::SymbolType(L"char32");
                default: return new ast::SymbolType(L"char32"); // Fallback for unknown byte sizes
            }
        } else if (typeid(*expr) == typeid(ast::BooleanExpr)) {
            return new ast::SymbolType(L"bool");
        } else if (typeid(*expr) == typeid(ast::ByteExpr)) {
            return new ast::SymbolType(L"byte");
        } else if (typeid(*expr) == typeid(ast::NullExpr)) {
            return new ast::SymbolType(L"null");
        } else {
            return nullptr; // Not a literal expression that can be converted to a type.
        }
    }
}