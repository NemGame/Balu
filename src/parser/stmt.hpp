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
        bool alias = p->currentTokenKind() == lexer::ALIAS;

        bool isConstant;
        ast::Type* explicitType = nullptr;
        ast::Expr* assignedValue = nullptr;
        if (letConst) {
            isConstant = p->currentTokenKind() == lexer::CONST;
            p->advance(); // consume 'let' or 'const'
        } else if (!alias) {  // Either 'mut' + typename or the typename itself
            isConstant = p->currentTokenKind() != lexer::MUT;  // If it's not 'mut', then it's a constant declaration
            if (!isConstant) p->advance(); // consume 'mut' if it exists

            // Check if we are looking at a type or just the variable name.
            // We parse a type if:
            // 1. It's a known keyword type (NUMBER, STRING, etc.)
            // 2. It's a prefix modifier (*, [)
            // 3. It's an identifier followed by a modifier or another identifier (the variable name)
            lexer::TokenKind nextKind = p->tokens.size() > p->pos + 1 ? p->tokens[p->pos + 1].kind : lexer::EOF_TOKEN;
            if (p->currentTokenKind() != lexer::IDENTIFIER || 
                (nextKind == lexer::IDENTIFIER || nextKind == lexer::STAR || nextKind == lexer::OPEN_BRACKET)) 
                explicitType = parse_type(p, default_bp);
        } else {
            p->advance(); // consume 'alias'
        }

        wstring varName = p->expectError(lexer::IDENTIFIER, Error(L"Expected variable name after declaration with " + (letConst ? (isConstant ? wstring(L"'const'") : wstring(L"'let'")) : (isConstant ? wstring(L"typename") : wstring(L"'mut'"))) + L", but got " + lexer::TokenKindString(p->currentTokenKind()) + L" at " + p->position())).value;
        if (p->currentTokenKind() == lexer::COLON) {
            p->advance();  // consume ':'
            if (explicitType != nullptr) {
                ast::Type* newExplicitType = parse_type(p, default_bp);
                wstring message = L"Variable declaration for '" + varName + L"' at " + p->position() + L" can only contain one type, but multiple were found (" + explicitType->GetName() + L" and " + newExplicitType->GetName() + L")";
                p->errors.push_back(ParserError(message));
                wcout << message << endl;
                if (_panic) {
                    if (_debug) wcout << L"Panicing" << endl;
                    exit(1);
                }
            } else explicitType = parse_type(p, default_bp);
        }

        // Default to type: auto (const) | any (let/mut)
        if (explicitType == nullptr) {
            if (isConstant) explicitType = new ast::SymbolType(L"auto");
            else explicitType = new ast::SymbolType(L"any");
        }

        if (p->currentTokenKind() != lexer::SEMICOLON) {
            p->expect(lexer::ASSIGNMENT); 
            assignedValue = parse_expr(p, assignment);
        }

        p->expect(lexer::SEMICOLON);
        
        if (isConstant) {
            if (explicitType->GetName() == L"any") {
                wstring message = L"Constant variable declaration for '" + varName + L"' at " + p->position() + L" cannot have type 'any' as it cannot be reassigned, did you mean 'auto'?";
                p->errors.push_back(ParserError(message));
                wcout << message << endl;
                if (_panic) {
                    if (_debug) wcout << L"Panicing" << endl;
                    exit(1);
                }
                explicitType = new ast::SymbolType(L"auto");
            }
            if (assignedValue == nullptr) {
                wstring message = L"Constant variable declaration for '" + varName + L"' at " + p->position() + L" must have an initializer";
                p->errors.push_back(ParserError(message));
                wcout << message << endl;
                if (_panic) {
                    if (_debug) wcout << L"Panicing" << endl;
                    exit(1);
                }
                assignedValue = new ast::NullExpr();
            }
        }

        if (alias) {
            if (assignedValue != nullptr) {
                wstring message = L"Alias declaration for '" + varName + L"' at " + p->position() + L" cannot have an initializer";
                p->errors.push_back(ParserError(message));
                wcout << message << endl;
                if (_panic) {
                    if (_debug) wcout << L"Panicing" << endl;
                    exit(1);
                }
            }
            explicitType = new ast::AliasType(assignedValue);
        }

        return new ast::VarDeclStmt{
            varName,
            isConstant,
            assignedValue,
            explicitType
        };
    }
}