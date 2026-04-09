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
    void nud(lexer::TokenKind kind, nud_handler nud_fn) {
        nud_lu[kind] = nud_fn;
    }
    void stmt(lexer::TokenKind kind, stmt_handler stmt_fn) {
        bp_lu[kind] = default_bp;
        stmt_lu[kind] = stmt_fn;
    }
}