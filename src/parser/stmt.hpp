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

            if (p->currentTokenKind() != lexer::IDENTIFIER) explicitType = parse_type(p, default_bp);
        }

        wstring varName = p->expectError(lexer::IDENTIFIER, Error(L"Expected variable name after declaration with " + (letConst ? (isConstant ? wstring(L"'const'") : wstring(L"'let'")) : (isConstant ? wstring(L"typename") : wstring(L"'mut'"))) + L", but got " + lexer::TokenKindString(p->currentTokenKind()) + L".")).value;
        if (p->currentTokenKind() == lexer::COLON) {
            p->advance();  // consume ':'
            if (explicitType != nullptr) {
                ast::Type* newExplicitType = parse_type(p, default_bp);
                wstring message = L"Variable declaration for '" + varName + L"' can only contain one type, but multiple were found (" + explicitType->GetName() + L" and " + newExplicitType->GetName() + L")";
                p->errors.push_back(ParserError(message));
                wcout << message << endl;
                if (_panic) {
                    if (_debug) wcout << L"Panicing" << endl;
                    exit(1);
                }
            } else explicitType = parse_type(p, default_bp);
        }

        // Default to type: any
        if (explicitType == nullptr) {
            explicitType = new ast::SymbolType(L"any");
        }

        if (p->currentTokenKind() != lexer::SEMICOLON) {
            p->expect(lexer::ASSIGNMENT); 
            assignedValue = parse_expr(p, assignment);
        }

        p->expect(lexer::SEMICOLON);
        
        if (isConstant && assignedValue == nullptr) {
            wstring message = L"Constant variable declaration for '" + varName + L"' must have an initializer";
            p->errors.push_back(ParserError(message));
            wcout << message << endl;
            if (_panic) {
                if (_debug) wcout << L"Panicing" << endl;
                exit(1);
            }
            explicitType = new ast::SymbolType(L"any");
        }

        return new ast::VarDeclStmt{
            varName,
            isConstant,
            assignedValue,
            explicitType
        };
    }
}