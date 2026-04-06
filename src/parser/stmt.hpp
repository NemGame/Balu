#pragma once

namespace parser {
    ast::Type* parse_type(Parser* parser, binding_power bp);
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
        // Declared using the let or const keyword, otherwise it's either a mut, or a typename declaration
        bool letConst = p->currentTokenKind() == lexer::LET || p->currentTokenKind() == lexer::CONST;

        bool isConstant;
        ast::Type* explicitType = nullptr;
        ast::Expr* assignedValue = nullptr;
        if (letConst) {
            isConstant = p->currentTokenKind() == lexer::CONST;
            p->advance(); // consume 'let' or 'const'
        } else {  // Either 'mut' + typename or the typename itself
            isConstant = p->currentTokenKind() != lexer::MUT;  // If it's not 'mut', then it's a constant declaration
            if (!isConstant) p->advance(); // consume 'mut' if it exists

            explicitType = parse_type(p, default_bp);
        }

        wstring varName = p->expectError(lexer::IDENTIFIER, Error(L"Expected variable name after 'let' or 'const'")).value;
        if (letConst && p->currentTokenKind() == lexer::COLON) {
            p->advance();  // consume ':'
            explicitType = parse_type(p, default_bp);
        }

        if (p->currentTokenKind() != lexer::SEMICOLON) {
            p->expect(lexer::ASSIGNMENT); 
            assignedValue = parse_expr(p, assignment);
        } else if (explicitType == nullptr) {
            p->errors.push_back(ParserError(L"Variable declaration must have an initializer or an explicit type"));
            wcout << L"Variable declaration must have an initializer or an explicit type" << endl;
            if (_panic) {
                wcout << L"Panicing" << endl;
                exit(1);
            }
        }
        
        p->expect(lexer::SEMICOLON);
        
        if (isConstant && assignedValue == nullptr) {
            p->errors.push_back(ParserError(L"Constant variable declaration must have an initializer"));
            wcout << L"Constant variable declaration must have an initializer" << endl;
            if (_panic) {
                wcout << L"Panicing" << endl;
                exit(1);
            }
        }

        return new ast::VarDeclStmt{
            varName,
            isConstant,
            assignedValue,
            explicitType
        };
    }
}