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
    ast::Stmt* parse_var_decl_stmt(Parser* p) {
        bool letConst = p->currentTokenKind() == lexer::LET || p->currentTokenKind() == lexer::CONST;

        bool isConstant;
        if (letConst) {
            isConstant = p->currentTokenKind() == lexer::CONST;
            p->advance(); // consume 'let' or 'const'
        } else {  // Either 'mut' + typename or the typename itself
            isConstant = p->currentTokenKind() != lexer::MUT;  // If it's not 'mut', then it's a constant declaration
            if (!isConstant) p->advance(); // consume 'mut' if it exists

            p->advance();  // Ignore typename for now
        }
        
        wstring varName = p->expectError(lexer::IDENTIFIER, Error(L"Expected variable name after 'let' or 'const'")).value;
        p->expect(lexer::ASSIGNMENT);
        ast::Expr* assignedValue = parse_expr(p, assignment);

        p->expect(lexer::SEMICOLON);

        return new ast::VarDeclStmt{
            varName,
            isConstant,
            assignedValue
        };
    }
}