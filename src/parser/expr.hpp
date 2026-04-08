#pragma once

// --------------------
// Pratt Parser
// --------------------

namespace parser {
    // Forward declarations
    ast::Type* parse_type(Parser* parser, binding_power bp);

    ast::Expr* parse_expr(Parser* parser, binding_power bp) {
        if (_verbose) _wcout << L"[Parser] Parsing expression at " << parser->position() << endl;
        // NUD
        lexer::TokenKind tokenKind = parser->currentTokenKind();
        bool exists = nud_lu.find(tokenKind) != nud_lu.end();

        if (!exists) {
            _wcout << (_debug ? L"[Parser] " : L"") << L"NUD HANDLER EXPECTED FOR TOKEN " << lexer::TokenKindString(tokenKind) << L" BUT NOT FOUND" << endl;
            if (_panic) {
                if (_debug) _wcout << L"[Parser] Panicing" << endl;
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
                _wcout << (_debug ? L"[Parser] " : L"") << L"LED HANDLER EXPECTED FOR TOKEN " << lexer::TokenKindString(tokenKind) << L" BUT NOT FOUND" << endl;
                if (_panic) {
                    if (_debug) _wcout << L"[Parser] Panicing" << endl;
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
        if (_verbose) _wcout << L"[Parser] Parsing primary expression at " << parser->position() << endl;
        switch (parser->currentTokenKind()) {
            case lexer::NUMBER: {
                long double value = 0;
                try {
                    value = stold(parser->currentToken().value);
                } catch (const std::exception& e) {
                    wstring message = L"Invalid number literal at " + parser->position() + L": " + parser->currentToken().value;
                    parser->errors.push_back(ParserError(message, parser->currentToken().line, parser->currentToken().column));
                    _wcout << (_debug ? L"[Parser] " : L"") << message << endl;
                    if (_panic) {
                        if (_debug) _wcout << L"[Parser] Panicing" << endl;
                        exit(1);
                    }
                    parser->advance();
                    return nullptr;
                }
                parser->advance();
                return new ast::NumberExpr(value);
            }
            case lexer::BYTE: {
                unsigned char value = 0;
                try {
                    value = static_cast<unsigned char>(stoi(parser->currentToken().value));
                } catch (const std::exception& e) {
                    wstring message = L"Invalid byte literal at " + parser->position() + L": " + parser->currentToken().value;
                    parser->errors.push_back(ParserError(message, parser->currentToken().line, parser->currentToken().column));
                    _wcout << (_debug ? L"[Parser] " : L"") << message << endl;
                    if (_panic) {
                        if (_debug) _wcout << L"[Parser] Panicing" << endl;
                        exit(1);
                    }
                    parser->advance();
                    return nullptr;
                }
                parser->advance();
                return new ast::ByteExpr(value);
            }
            case lexer::STRING: {
                return new ast::StringExpr(parser->advance().value);
            }
            case lexer::CHAR: {
                return new ast::CharExpr(parser->advance().value[0]);
            }
            case lexer::BOOL: {
                bool value = parser->currentToken().value == L"true";
                parser->advance();
                return new ast::BooleanExpr(value);
            }
            case lexer::NULL_TYPE: {
                parser->advance();
                return new ast::NullExpr();
            }
            case lexer::IDENTIFIER: {
                return new ast::SymbolExpr(parser->advance().value);
            }
            case lexer::RULE: {
                return new ast::RuleExpr(parser->advance().value);
            }
            default: {
                lexer::Token token = parser->currentToken();
                if (_verbose || _debug) _wcout << L"[Parser] Cannot create primary expression from token: " << lexer::TokenKindString(parser->currentTokenKind()) << endl;
                wstring message = L"Unexpected token at " + parser->position() + L": " + lexer::TokenKindString(token.kind);
                parser->errors.push_back(ParserError(message, token.line, token.column));
                return nullptr;
            }
        }
    }

    ast::Expr* parse_binary_expr(Parser* parser, ast::Expr* left, binding_power bp) {
        if (_verbose) _wcout << L"[Parser] Parsing binary expression at " << parser->position() << endl;
        lexer::Token op = parser->advance();
        ast::Expr* right = parse_expr(parser, bp_lu[op.kind]);
        return new ast::BinaryExpr(left, right, op);
    }

    ast::Expr* parse_prefix_expr(Parser* parser) {
        if (_verbose) _wcout << L"[Parser] Parsing prefix expression at " << parser->position() << endl;
        lexer::Token op = parser->advance();
        ast::Expr* right = parse_expr(parser, unary);
        return new ast::PrefixExpr(right, op);
    }

    ast::Expr* parse_assignment_expr(Parser* parser, ast::Expr* left, binding_power bp) {
        if (_verbose) _wcout << L"[Parser] Parsing assignment expression at " << parser->position() << endl;
        lexer::Token op = parser->advance();
        ast::Expr* right = parse_expr(parser, bp);  // Right-associative
        return new ast::AssignmentExpr(left, op, right);
    }

    ast::Expr* parse_grouping_expr(Parser* parser) {
        if (_verbose) _wcout << L"[Parser] Parsing grouping expression at " << parser->position() << endl;
        parser->expect(lexer::OPEN_PAREN);
        ast::Expr* expr = parse_expr(parser, default_bp);
        parser->expect(lexer::CLOSE_PAREN);
        return expr;
    }

    ast::Expr* parse_struct_instantiation_expr(Parser* parser, ast::Expr* left, binding_power bp) {
        if (_verbose) _wcout << L"[Parser] Parsing struct instantiation expression at " << parser->position() << endl;
        wstring structName;
        if (auto symbolExpr = dynamic_cast<ast::SymbolExpr*>(left)) {
            structName = symbolExpr->value;
        } else {
            wstring message = L"Expected struct name before '{' in struct instantiation expression at " + parser->position();
            parser->errors.push_back(ParserError(message, parser->line, parser->column));
            _wcout << (_debug ? L"[Parser] " : L"") << message << endl;
            if (_panic) {
                if (_debug) _wcout << L"[Parser] Panicing" << endl;
                exit(1);
            }
            parser->advanceDepth(lexer::OPEN_CURLY, lexer::CLOSE_CURLY);  // Skip the struct body
            parser->expect(lexer::CLOSE_CURLY);
            return nullptr;
        }
        unordered_map<wstring, ast::Expr*> fieldValues;
        parser->expect(lexer::OPEN_CURLY);

        while (parser->hasTokens() && parser->currentTokenKind() != lexer::CLOSE_CURLY) {
            if (parser->currentToken().isOneOfMany(lexer::SEMICOLON, lexer::COMMA)) {
                // Allow optional commas between fields
                parser->advance();  // consume comma and continue to next field
                continue;
            }
            wstring fieldName = parser->expect(lexer::IDENTIFIER).value;
            parser->expect(lexer::COLON, lexer::ASSIGNMENT);
            ast::Expr* fieldValue = parse_expr(parser, logical);
            fieldValues[fieldName] = fieldValue;

            if (parser->currentTokenKind() != lexer::CLOSE_CURLY) {
                parser->expect(lexer::SEMICOLON, lexer::COMMA);
            }
        }

        parser->expect(lexer::CLOSE_CURLY);
        return new ast::StructInstantiationExpr(structName, reverseUnorderedMap(fieldValues));
    }
    ast::Expr* parse_array_instantiation_expr(Parser* parser) {
        vector<ast::Expr*> contents;

        parser->expect(lexer::OPEN_BRACKET);
        parser->expect(lexer::CLOSE_BRACKET);
        
        ast::Type* underlyingType = parse_type(parser, unary);

        parser->expect(lexer::OPEN_CURLY);

        while (parser->hasTokens() && parser->currentTokenKind() != lexer::CLOSE_CURLY) {
            contents.push_back(parse_expr(parser, default_bp));
            if (parser->currentTokenKind() != lexer::CLOSE_CURLY) {
                parser->expect(lexer::COMMA);
            }
        }

        parser->expect(lexer::CLOSE_CURLY);
        return new ast::ArrayInstantiationExpr(underlyingType, contents);
    }
}