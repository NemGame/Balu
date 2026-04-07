#pragma once

// --------------------
// Pratt Parser
// --------------------

namespace parser {

    ast::Expr* parse_expr(Parser* parser, binding_power bp) {
        // NUD
        lexer::TokenKind tokenKind = parser->currentTokenKind();
        bool exists = nud_lu.find(tokenKind) != nud_lu.end();

        if (!exists) {
            wcout << L"NUD HANDLER EXPECTED FOR TOKEN " << lexer::TokenKindString(tokenKind) << L" BUT NOT FOUND" << endl;
            if (_panic) {
                if (_debug) wcout << L"Panicing" << endl;
                exit(1);
            }
            else return nullptr;
        }

        nud_handler nud_fn = nud_lu[tokenKind];

        ast::Expr* left = nud_fn(parser);

        while (bp_lu[parser->currentTokenKind()] > bp) {
            lexer::TokenKind tokenKind = parser->currentTokenKind();
            bool exists = led_lu.find(tokenKind) != led_lu.end();

            if (!exists) {
                wcout << L"LED HANDLER EXPECTED FOR TOKEN " << lexer::TokenKindString(tokenKind) << L" BUT NOT FOUND" << endl;
                if (_panic) {
                    if (_debug) wcout << L"Panicing" << endl;
                    exit(1);
                }
                else return left;
            }

            led_handler led_fn = led_lu[tokenKind];

            left = led_fn(parser, left, bp_lu[tokenKind]);
        }

        return left;
    }
    
    ast::Expr* parse_primary_expr(Parser* parser) {
        switch (parser->currentTokenKind()) {
            case lexer::NUMBER: {
                long double value = stold(parser->currentToken().value);
                parser->advance();
                return new ast::NumberExpr(value);
            }
            case lexer::BYTE: {
                unsigned char value = static_cast<unsigned char>(stoi(parser->currentToken().value));
                parser->advance();
                return new ast::ByteExpr(value);
            }
            case lexer::STRING: {
                return new ast::StringExpr(parser->advance().value);
            }
            case lexer::CHAR: {
                return new ast::CharExpr(parser->advance().value[0]);  // Represent char as string of length 1
            }
            case lexer::IDENTIFIER: {
                return new ast::SymbolExpr(parser->advance().value);
            }
            case lexer::RULE: {
                return new ast::RuleExpr(parser->advance().value);
            }
            default: {
                if (_verbose) wcout << "Cannot create primary expression from token: " << lexer::TokenKindString(parser->currentTokenKind()) << endl;
                parser->errors.push_back(ParserError(L"Unexpected token: " + lexer::TokenKindString(parser->currentTokenKind()), 0, 0));
                return nullptr;
            }
        }
    }

    ast::Expr* parse_binary_expr(Parser* parser, ast::Expr* left, binding_power bp) {
        lexer::Token op = parser->advance();
        ast::Expr* right = parse_expr(parser, bp_lu[op.kind]);
        return new ast::BinaryExpr(left, right, op);
    }

    ast::Expr* parse_prefix_expr(Parser* parser) {
        lexer::Token op = parser->advance();
        ast::Expr* right = parse_expr(parser, unary);
        return new ast::PrefixExpr(right, op);
    }

    ast::Expr* parse_assignment_expr(Parser* parser, ast::Expr* left, binding_power bp) {
        lexer::Token op = parser->advance();
        ast::Expr* right = parse_expr(parser, bp);  // Right-associative
        return new ast::AssignmentExpr(left, op, right);
    }

    ast::Expr* parse_grouping_expr(Parser* parser) {
        parser->expect(lexer::OPEN_PAREN);
        ast::Expr* expr = parse_expr(parser, default_bp);
        parser->expect(lexer::CLOSE_PAREN);
        return expr;
    }
}