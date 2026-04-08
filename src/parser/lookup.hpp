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

    // Forward declarations
    ast::Expr* parse_expr(Parser* parser, binding_power bp);
    ast::Expr* parse_primary_expr(Parser* parser);
    ast::Expr* parse_binary_expr(Parser* parser, ast::Expr* left, binding_power bp);
    ast::Expr* parse_prefix_expr(Parser* parser);
    ast::Expr* parse_assignment_expr(Parser* parser, ast::Expr* left, binding_power bp);
    ast::Expr* parse_grouping_expr(Parser* parser);
    ast::Stmt* parse_var_decl_stmt(Parser* parser);

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

    void createTokenLookups() {
        // Assignment
        led(lexer::ASSIGNMENT, assignment, parse_assignment_expr);
        led(lexer::PLUS_EQUALS, assignment, parse_assignment_expr);
        led(lexer::MINUS_EQUALS, assignment, parse_assignment_expr);
        led(lexer::STAR_EQUALS, assignment, parse_assignment_expr);
        led(lexer::SLASH_EQUALS, assignment, parse_assignment_expr);
        led(lexer::PERCENT_EQUALS, assignment, parse_assignment_expr);

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
        nud(lexer::NUMBER, parse_primary_expr);
        nud(lexer::BYTE, parse_primary_expr);
        nud(lexer::STRING, parse_primary_expr);
        nud(lexer::CHAR, parse_primary_expr);
        nud(lexer::BOOL, parse_primary_expr);
        nud(lexer::AUTO, parse_primary_expr);
        nud(lexer::ANY, parse_primary_expr);
        nud(lexer::VOID, parse_primary_expr);
        nud(lexer::NULL_TYPE, parse_primary_expr);
        nud(lexer::IDENTIFIER, parse_primary_expr);
        nud(lexer::ALIAS, parse_primary_expr);
        nud(lexer::OPEN_PAREN, parse_grouping_expr);
        nud(lexer::DASH, parse_prefix_expr);

        nud(lexer::RULE, parse_primary_expr);
        
        // Statements
        stmt(lexer::LET, parse_var_decl_stmt);
        stmt(lexer::CONST, parse_var_decl_stmt);
        stmt(lexer::MUT, parse_var_decl_stmt);
        stmt(lexer::ALIAS, parse_var_decl_stmt);
        stmt(lexer::OPEN_BRACKET, parse_var_decl_stmt);
        
        // Types
        stmt(lexer::NUMBER, parse_var_decl_stmt);
        stmt(lexer::BYTE, parse_var_decl_stmt);
        stmt(lexer::STRING, parse_var_decl_stmt);
        stmt(lexer::BOOL, parse_var_decl_stmt);
        stmt(lexer::CHAR, parse_var_decl_stmt);
        stmt(lexer::AUTO, parse_var_decl_stmt);
        stmt(lexer::ANY, parse_var_decl_stmt);
        stmt(lexer::VOID, parse_var_decl_stmt);
        stmt(lexer::NULL_TYPE, parse_var_decl_stmt);
    }
}