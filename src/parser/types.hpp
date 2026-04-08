#pragma once

namespace parser {
    using type_nud_handler = ast::Type* (*)(Parser* p);
    using type_led_handler = ast::Type* (*)(Parser* p, ast::Type* left, binding_power bp);
    
    using type_nud_lookup = map<lexer::TokenKind, type_nud_handler>;
    using type_led_lookup = map<lexer::TokenKind, type_led_handler>;
    using type_bp_lookup = map<lexer::TokenKind, binding_power>;

    type_bp_lookup type_bp_lu = {};
    type_nud_lookup type_nud_lu = {};
    type_led_lookup type_led_lu = {};

    void type_led(lexer::TokenKind kind, binding_power bp, type_led_handler led_fn) {
        type_bp_lu[kind] = bp;
        type_led_lu[kind] = led_fn;
    }
    void type_nud(lexer::TokenKind kind, type_nud_handler nud_fn) {
        type_nud_lu[kind] = nud_fn;
    }
    
    ast::Type* parse_type(Parser* parser, binding_power bp) {
        // NUD
        lexer::TokenKind tokenKind = parser->currentTokenKind();
        bool exists = type_nud_lu.find(tokenKind) != type_nud_lu.end();

        if (!exists) {
            wcout << L"TYPE_NUD HANDLER EXPECTED FOR TOKEN " << lexer::TokenKindString(tokenKind) << L" BUT NOT FOUND" << endl;
            if (_panic) {
                if (_debug) wcout << L"Panicing" << endl;
                exit(1);
            }
            else return nullptr;
        }

        type_nud_handler nud_fn = type_nud_lu[tokenKind];

        ast::Type* left = nud_fn(parser);

        while (type_bp_lu[parser->currentTokenKind()] > bp) {
            lexer::TokenKind tokenKind = parser->currentTokenKind();
            bool exists = type_led_lu.find(tokenKind) != type_led_lu.end();

            if (!exists) {
                wcout << L"TYPE_LED HANDLER EXPECTED FOR TOKEN " << lexer::TokenKindString(tokenKind) << L" BUT NOT FOUND" << endl;
                if (_panic) {
                    if (_debug) wcout << L"Panicing" << endl;
                    exit(1);
                }
                else return left;
            }

            type_led_handler led_fn = type_led_lu[tokenKind];

            left = led_fn(parser, left, type_bp_lu[tokenKind]);
        }

        return left;
    }
    ast::Type* parse_symbol_type(Parser* parser) {
        return new ast::SymbolType(parser->advance().value);
    }
    ast::Type* parse_array_type(Parser* parser) {
        parser->expect(lexer::OPEN_BRACKET); // consume '['
        parser->expect(lexer::CLOSE_BRACKET); // consume ']'
        ast::Type* underlyingType = parse_type(parser, unary);
        return new ast::ArrayType(underlyingType);
    }
    ast::Type* parse_postfix_array_type(Parser* parser, ast::Type* left, binding_power bp) {
        parser->expect(lexer::OPEN_BRACKET); // consume '['
        parser->expect(lexer::CLOSE_BRACKET); // consume ']'
        return new ast::ArrayType(left);
    }
    void createTokenTypeLookups() {
        type_nud(lexer::IDENTIFIER, parse_symbol_type);
        type_nud(lexer::OPEN_BRACKET, parse_array_type);  // []T

        type_nud(lexer::STRING, parse_symbol_type);
        type_nud(lexer::BYTE, parse_symbol_type);
        type_nud(lexer::CHAR, parse_symbol_type);
        type_nud(lexer::NUMBER, parse_symbol_type);
        type_nud(lexer::BOOL, parse_symbol_type);
        type_nud(lexer::AUTO, parse_symbol_type);
        type_nud(lexer::ANY, parse_symbol_type);
        type_nud(lexer::VOID, parse_symbol_type);
        type_nud(lexer::NULL_TYPE, parse_symbol_type);

        type_led(lexer::OPEN_BRACKET, call, parse_postfix_array_type); // T[] (Higher precedence)
    }
}