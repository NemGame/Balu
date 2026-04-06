#pragma once

namespace parser {
    enum binding_power {
        default_bp = 0,
        comma,
        assignment,
        logical,
        relational,
        additive,
        multiplicative,
        unary,
        call,
        member,
        primary
    };

    // Forward declarations of parsing functions defined in expr.hpp
    ast::Expr* parse_primary_expr(Parser* parser);
    ast::Expr* parse_binary_expr(Parser* parser, ast::Expr* left, binding_power bp);

    using stmt_handler = ast::Stmt* (*)(Parser* p);
    using nud_handler = ast::Expr* (*)(Parser* p);
    using led_handler = ast::Expr* (*)(Parser* p, ast::Expr* left, binding_power bp);

    using stmt_lookup = map<lexer::TokenKind, stmt_handler>;
    using nud_lookup = map<lexer::TokenKind, nud_handler>;
    using led_lookup = map<lexer::TokenKind, led_handler>;
    using bp_lookup = map<lexer::TokenKind, binding_power>;

    bp_lookup bp_lu = {};
    nud_lookup nud_lu = {};
    led_lookup led_lu = {};
    stmt_lookup stmt_lu = {};

    void led(lexer::TokenKind kind, binding_power bp, led_handler led_fn) {
        bp_lu[kind] = bp;
        led_lu[kind] = led_fn;
    }
    void nud(lexer::TokenKind kind, binding_power bp, nud_handler nud_fn) {
        bp_lu[kind] = bp;
        nud_lu[kind] = nud_fn;
    }
    void stmt(lexer::TokenKind kind, stmt_handler stmt_fn) {
        bp_lu[kind] = default_bp;
        stmt_lu[kind] = stmt_fn;
    }

    void createTokenLookups() {
        // Logical
        led(lexer::AND, logical, parse_binary_expr);
        led(lexer::OR, logical, parse_binary_expr);
        led(lexer::DOT_DOT, logical, parse_binary_expr);

        // Relational
        led(lexer::LESS, relational, parse_binary_expr);
        led(lexer::LESS_EQUALS, relational, parse_binary_expr);
        led(lexer::GREATER, relational, parse_binary_expr);
        led(lexer::GREATER_EQUALS, relational, parse_binary_expr);
        led(lexer::EQUALS, relational, parse_binary_expr);
        led(lexer::NOT_EQUALS, relational, parse_binary_expr);

        // Additive & Multiplicative
        led(lexer::PLUS, additive, parse_binary_expr);
        led(lexer::DASH, additive, parse_binary_expr);
        
        led(lexer::STAR, multiplicative, parse_binary_expr);
        led(lexer::SLASH, multiplicative, parse_binary_expr);
        led(lexer::PERCENT, multiplicative, parse_binary_expr);

        // Literals and Symbols
        nud(lexer::NUMBER, primary, parse_primary_expr);
        nud(lexer::STRING, primary, parse_primary_expr);
        nud(lexer::IDENTIFIER, primary, parse_primary_expr);
    }
}