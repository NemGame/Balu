#pragma once

namespace parser {
    ast::Stmt* parse_stmt(Parser* p) {
        bool exists = stmt_lu[p->currentTokenKind()] != nullptr;
        if (exists) {
            return stmt_lu[p->currentTokenKind()](p);
        }

        ast::Expr* expr = parse_expr(p, default_bp);

        p->expect(lexer::SEMICOLON);

        return new ast::ExpressionStmt( expr );
    }
}