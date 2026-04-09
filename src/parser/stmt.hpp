#pragma once

namespace parser {
    ast::Type* parse_type(Parser* parser, binding_power bp);
    ast::Stmt* parse_stmt(Parser* p) {
        if (_verbose) _wcout << L"Parsing statement at " << p->position() << endl;
        bool exists = stmt_lu[p->currentTokenKind()] != nullptr;
        if (exists) {
            return stmt_lu[p->currentTokenKind()](p);
        }

        ast::Expr* expr = parse_expr(p, default_bp); // This could return nullptr if an error occurred and _panic is false
        if (expr == nullptr) {
            // If expression parsing failed, we should not proceed to create an ExpressionStmt.
            // The error would have been reported by parse_expr.
            // We should try to recover by advancing past the current token, or until a semicolon.
            p->advanceUntil(lexer::SEMICOLON); // Skip to the end of the problematic statement
            p->expect(lexer::SEMICOLON); // Consume the semicolon
            return nullptr; // Return nullptr to indicate failure to parse a valid statement
        }

        p->expect(lexer::SEMICOLON);

        return new ast::ExpressionStmt( expr );
    }
    ast::Stmt* parse_block_stmt(Parser* p) {
        if (_verbose) _wcout << L"Parsing block statement at " << p->position() << endl;
        p->expect(lexer::OPEN_CURLY);  // consume '{'
        vector<ast::Stmt*> statements;
        while (p->hasTokens() && p->currentTokenKind() != lexer::CLOSE_CURLY) {
            ast::Stmt* stmt = parse_stmt(p);
            if (stmt != nullptr) statements.push_back(stmt);
        }
        p->expect(lexer::CLOSE_CURLY);  // consume '}'
        if (p->currentTokenKind() == lexer::SEMICOLON) p->advance();
        return new ast::BlockStmt(statements);
    };
    ast::Stmt* parse_var_decl_stmt(Parser* p) {
        if (_verbose) _wcout << L"Parsing variable declaration statement at " << p->position() << endl;
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

            // Check if we are looking at a type or just the variable name.
            // We parse a type if:
            // 1. It's a known keyword type (NUMBER, STRING, etc.)
            // 2. It's a prefix modifier (*, [)
            // 3. It's an identifier followed by a modifier or another identifier (the variable name)
            lexer::TokenKind nextKind = p->tokens.size() > p->pos + 1 ? p->tokens[p->pos + 1].kind : lexer::EOF_TOKEN;
            if (p->currentTokenKind() != lexer::IDENTIFIER || 
                (nextKind == lexer::IDENTIFIER || nextKind == lexer::STAR || nextKind == lexer::OPEN_BRACKET)) 
                explicitType = parse_type(p, default_bp);
        }

        wstring varName = p->expectError(lexer::IDENTIFIER, Error(L"Expected variable name after declaration with " + (letConst ? (isConstant ? wstring(L"'const'") : wstring(L"'let'")) : (isConstant ? wstring(L"typename") : wstring(L"'mut'"))) + L", but got " + lexer::TokenKindString(p->currentTokenKind()) + L" at " + p->position())).value;
        if (p->currentTokenKind() == lexer::COLON) {
            p->advance();  // consume ':'
            if (explicitType != nullptr) {
                ast::Type* newExplicitType = parse_type(p, default_bp);
                wstring message = L"Variable declaration for '" + varName + L"' at " + p->position() + L" can only contain one type, but multiple were found (" + explicitType->GetName() + L" and " + newExplicitType->GetName() + L")";
                p->errors.push_back(ParserError(message));
                _wcout << (_debug ? L"[Parser] " : L"") << message << endl;
                if (_panic) {
                    if (_debug) _wcout << L"[Parser] Panicing" << endl;
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
            if (p->currentTokenKind() != lexer::ASSIGNMENT) {
                wstring message = L"Expected token at " + p->position() + L" to be either an assignment operator '=' for variable initialization or a semicolon ';' to end the declaration, but found " + lexer::TokenKindString(p->currentTokenKind());
                p->errors.push_back(ParserError(message));
                _wcout << (_debug ? L"[Parser] " : L"") << message << endl;
                if (_panic) {
                    if (_debug) _wcout << L"[Parser] Panicing" << endl;
                    exit(1);
                }
                p->advanceUntil(lexer::SEMICOLON); // Skip to the end of the problematic statement
                p->expect(lexer::SEMICOLON); // Consume the semicolon
                return nullptr;
            }
            p->expect(lexer::ASSIGNMENT); 
            assignedValue = parse_expr(p, assignment);
        }

        p->expect(lexer::SEMICOLON);
        
        if (isConstant) {
            if (explicitType->GetName() == L"any") {
                wstring message = L"Constant variable declaration for '" + varName + L"' at " + p->position() + L" cannot have type 'any' as it cannot be reassigned, did you mean 'auto'?";
                p->errors.push_back(ParserError(message));
                _wcout << (_debug ? L"[Parser] " : L"") << message << endl;
                if (_panic) {
                    if (_debug) _wcout << L"[Parser] Panicing" << endl;
                    exit(1);
                }
                explicitType = new ast::SymbolType(L"auto");
            }
            if (assignedValue == nullptr) {
                wstring message = L"Constant variable declaration for '" + varName + L"' at " + p->position() + L" must have an initializer";
                p->errors.push_back(ParserError(message));
                _wcout << (_debug ? L"[Parser] " : L"") << message << endl;
                if (_panic) {
                    if (_debug) _wcout << L"[Parser] Panicing" << endl;
                    exit(1);
                }
                assignedValue = new ast::NullExpr();
            }
        }

        if (assignedValue == nullptr) assignedValue = new ast::NullExpr();
        return new ast::VarDeclStmt{
            varName,
            isConstant,
            assignedValue,
            explicitType
        };
    }
    ast::Stmt* parse_alias_decl_stmt(Parser* p) {
        if (_verbose) _wcout << L"Parsing alias declaration statement at " << p->position() << endl;
        p->expect(lexer::ALIAS);  // consume 'alias'

        const wstring correctSyntax = GetCorrectSyntax(lexer::ALIAS);

        if (p->currentTokenKind() == lexer::COLON) p->advance();  // consume ':'
        const bool currentIsTypename = p->currentToken().mightBeType() 
                                        && p->nextTokenKind() == lexer::IDENTIFIER;
        if (currentIsTypename) {
            wstring probableAliasName = currentIsTypename ? p->nextToken().value : L"?";
            wstring message = L"Alias declaration for '" + probableAliasName + L"' at " + p->position() + L" cannot have an explicit type ("+ lexer::TokenKindString(p->currentTokenKind()) + L")" + correctSyntax;
            p->errors.push_back(ParserError(message));
            _wcout << (_debug ? L"[Parser] " : L"") << message << endl;
            if (_panic) {
                if (_debug) _wcout << L"[Parser] Panicing" << endl;
                exit(1);
            }
            if (p->currentTokenKind() == lexer::COLON) p->advance();
            delete parse_type(p, default_bp);
        }

        wstring aliasName = p->expectError(lexer::IDENTIFIER, Error(L"Expected alias name after 'alias' keyword, but got " + lexer::TokenKindString(p->currentTokenKind()) + L" at " + p->position() + correctSyntax)).value;
        if (p->previousTokenKind() != lexer::IDENTIFIER) {
            p->advanceUntil(lexer::SEMICOLON); // Skip to the end of the problematic statement
            p->expect(lexer::SEMICOLON); // Consume the semicolon
            return nullptr;
        }
        if (p->currentTokenKind() == lexer::COLON) {
            wstring message = L"Alias declaration for '" + aliasName + L"' at " + p->position() + L" cannot have an explicit type";
            p->errors.push_back(ParserError(message));
            _wcout << (_debug ? L"[Parser] " : L"") << message << endl;
            if (_panic) {
                if (_debug) _wcout << L"[Parser] Panicing" << endl;
                exit(1);
            }
            p->advance();  // consume ':'

            if (p->currentToken().mightBeType()) parse_type(p, default_bp);
        }

        if (p->currentTokenKind() != lexer::ASSIGNMENT) {
            wstring message = L"Alias '" + aliasName + L"' must be initialized at " + p->position() + correctSyntax;
            p->errors.push_back(ParserError(message));
            _wcout << (_debug ? L"[Parser] " : L"") << message << endl;
            if (_panic) {
                if (_debug) _wcout << L"[Parser] Panicing" << endl;
                exit(1);
            }
            p->advanceUntil(lexer::SEMICOLON); // Skip to the end of the problematic statement
            p->expect(lexer::SEMICOLON); // Consume the semicolon
            return nullptr;
        }
        p->expect(lexer::ASSIGNMENT);

        ast::Expr* aliasedValue = parse_expr(p, assignment);
        p->expect(lexer::SEMICOLON);

        if (aliasedValue == nullptr) {
            wstring message = L"Alias declaration for '" + aliasName + L"' at " + p->position() + L" must have an initializer" + correctSyntax;
            p->errors.push_back(ParserError(message));
            _wcout << (_debug ? L"[Parser] " : L"") << message << endl;
            if (_panic) {
                if (_debug) _wcout << L"[Parser] Panicing" << endl;
                exit(1);
            }
            aliasedValue = new ast::NullExpr();
        }
        return new ast::AliasDeclStmt{
            aliasName,
            aliasedValue
        };
    }
    ast::Stmt* parse_struct_decl_stmt(Parser* p) {
        if (_verbose) _wcout << L"Parsing struct declaration statement at " << p->position() << endl;
        p->expect(lexer::STRUCT);  // consume 'struct'

        unordered_map<wstring, ast::StructProperty*> properties;
        unordered_map<wstring, ast::StructMethod*> methods;

        wstring structName = p->expectError(lexer::IDENTIFIER, 
            Error(L"Expected struct name after 'struct' keyword, but got " + lexer::TokenKindString(p->currentTokenKind()) + L" at " + p->position())
        ).value;

        p->expect(lexer::OPEN_CURLY);

        while (p->hasTokens() && p->currentTokenKind() != lexer::CLOSE_CURLY) {
            bool isStatic = false;
            if (p->currentTokenKind() == lexer::STATIC) {
                isStatic = true;
                p->advance();  // consume 'static'
            }
            
            ast::Type* ExpectedType = nullptr;
            bool isMethod = p->currentTokenKind() == lexer::FN || p->nextTokenKind(1) == lexer::OPEN_PAREN || p->nextTokenKind(2) == lexer::OPEN_PAREN;
            if (isMethod && p->nextTokenKind(1) != lexer::OPEN_PAREN) p->advance();  // consume 'fn' / typename

            // fn typename methodname() {} or static fn typename methodname() {}
            if ((p->currentTokenKind() == lexer::IDENTIFIER || p->currentToken().isType()) && p->nextTokenKind() == lexer::IDENTIFIER) {
                ExpectedType = parse_type(p, default_bp);
                if (ExpectedType == nullptr) ExpectedType = new ast::SymbolType(L"auto");
                if (ExpectedType->GetName() == L"any") {
                    wstring message = L"Struct method declaration for '" + p->currentToken().value + L"' at " + p->position() + L" cannot have return type 'any', did you mean 'auto'?";
                    p->errors.push_back(ParserError(message));
                    _wcout << (_debug ? L"[Parser] " : L"") << message << endl;
                    if (_panic) {
                        if (_debug) _wcout << L"[Parser] Panicing" << endl;
                        exit(1);
                    }
                    delete ExpectedType;
                    ExpectedType = new ast::SymbolType(L"auto");
                }
            }
            wstring propertyName = p->advance().value;

            // Name & collision check
            if (p->currentTokenKind() == lexer::IDENTIFIER) {
                if (properties.find(propertyName) != properties.end()) {
                    if (!isMethod) {
                        wstring message = L"Duplicate struct property '" + propertyName + L"' found in struct '" + structName + L"' declaration at " + p->position();
                        p->errors.push_back(ParserError(message));
                        _wcout << (_debug ? L"[Parser] " : L"") << message << endl;
                        if (_panic) {
                            if (_debug) _wcout << L"[Parser] Panicing" << endl;
                            exit(1);
                        }
                        p->advanceUntil(lexer::SEMICOLON);  // Skip until the end of the property declaration
                        continue;
                    }
                    // TODO: Method overloading based on parameter types and count
                }
            }

            // Interpret type
            if (p->currentTokenKind() == lexer::COLON) {
                p->advance();  // consume ':'
                if (ExpectedType != nullptr) {
                    wstring message = L"Struct property declaration for '" + propertyName + L"' at " + p->position() + L" cannot have an explicit type as it was already specified as '" + ExpectedType->GetName() + L"'";
                    p->errors.push_back(ParserError(message));
                    _wcout << (_debug ? L"[Parser] " : L"") << message << endl;
                    if (_panic) {
                        if (_debug) _wcout << L"[Parser] Panicing" << endl;
                        exit(1);
                    }
                    p->advance();  // consume the type
                } else ExpectedType = parse_type(p, default_bp);
            } else if (!isMethod && ExpectedType == nullptr) ExpectedType = new ast::SymbolType(L"any");  // Default to 'any' type for struct properties if no type is specified
            
            ast::Expr* AssignedValue = nullptr;
            ast::Stmt* MethodBody = nullptr;
            unordered_map<wstring, ast::MethodParameter*> parameters;
            
            // Method parameters
            if (isMethod) {  // Parse method
                p->expect(lexer::OPEN_PAREN);
                while (p->hasTokens() && p->currentTokenKind() != lexer::CLOSE_PAREN)
                {
                    wstring paramName = L"UnnamedParam";
                    ast::Type* paramType = nullptr;
                    ast::Expr* defaultValue = nullptr;
                    bool isConstant = false;
                    if (p->currentTokenKind() == lexer::CONST) {
                        isConstant = true;
                        p->advance();  // consume 'const'
                    }

                    if (p->currentTokenKind() == lexer::IDENTIFIER || p->currentToken().isType()) {
                        if (p->nextTokenKind() == lexer::IDENTIFIER) {
                            paramType = parse_type(p, default_bp);
                            if (paramType == nullptr) paramType = new ast::SymbolType(L"auto");
                        }
                        paramName = p->advance().value;
                    } else {
                        wstring message = L"Expected parameter name in method '" + propertyName + L"' declaration at " + p->position() + L", but got " + lexer::TokenKindString(p->currentTokenKind());
                        p->errors.push_back(ParserError(message));
                        _wcout << (_debug ? L"[Parser] " : L"") << message << endl;
                        if (_panic) {
                            if (_debug) _wcout << L"[Parser] Panicing" << endl;
                            exit(1);
                        }
                        p->advanceUntil(lexer::CLOSE_PAREN, lexer::COMMA);
                        continue;
                    }

                    if (p->currentTokenKind() == lexer::COLON) {
                        p->advance();  // consume ':'
                        if (paramType != nullptr) {
                            ast::Type* newParamType = parse_type(p, default_bp);
                            wstring message = L"Method parameter '" + paramName + L"' in method '" + propertyName + L"' declaration at " + p->position() + L" cannot have an explicit type as it was already specified as '" + paramType->GetName() + L"'";
                            p->errors.push_back(ParserError(message));
                            _wcout << (_debug ? L"[Parser] " : L"") << message << endl;
                            if (_panic) {
                                if (_debug) _wcout << L"[Parser] Panicing" << endl;
                                exit(1);
                            }
                            p->advance();  // consume the type
                        } else paramType = parse_type(p, default_bp);
                    }

                    if (p->currentTokenKind() == lexer::ASSIGNMENT) {
                        p->advance();  // consume '='
                        defaultValue = parse_expr(p, assignment);
                    }

                    if (parameters.find(paramName) != parameters.end()) {
                        wstring message = L"Duplicate parameter name '" + paramName + L"' found in method '" + propertyName + L"' declaration at " + p->position();
                        p->errors.push_back(ParserError(message));
                        _wcout << (_debug ? L"[Parser] " : L"") << message << endl;
                        if (_panic) {
                            if (_debug) _wcout << L"[Parser] Panicing" << endl;
                            exit(1);
                        }
                        p->advanceUntil(lexer::COMMA);  // Skip until the end of the parameter declaration
                        continue;
                    }

                    if (defaultValue == nullptr) defaultValue = new ast::NullExpr();
                    if (paramType == nullptr) paramType = new ast::SymbolType(isConstant ? L"auto" : L"any");

                    parameters[paramName] = new ast::MethodParameter(
                        paramName, 
                        paramType, 
                        defaultValue, 
                        isConstant
                    );
                    if (p->currentTokenKind() == lexer::COMMA) p->advance();
                }
                p->expect(lexer::CLOSE_PAREN);
                if (p->currentTokenKind() == lexer::COLON) {
                    p->advance();  // consume ':'
                    if (ExpectedType != nullptr) {
                        ast::Type* newExpectedType = parse_type(p, default_bp);
                        wstring message = L"Struct method declaration for '" + propertyName + L"' at " + p->position() + L" cannot have an explicit return type as it was already specified as '" + ExpectedType->GetName() + L"'";
                        p->errors.push_back(ParserError(message));
                        _wcout << (_debug ? L"[Parser] " : L"") << message << endl;
                        if (_panic) {
                            if (_debug) _wcout << L"[Parser] Panicing" << endl;
                            exit(1);
                        }
                        p->advance();  // consume the type
                    } else ExpectedType = parse_type(p, default_bp);
                }
                if (p->currentTokenKind() == lexer::ARROW) {  // fn <type> name() => <expr>;
                    p->advance();  // consume '=>'
                    MethodBody = new ast:: ExpressionStmt(new ast::ReturnExpr(parse_expr(p, assignment)));
                    p->expect(lexer::SEMICOLON);
                } else MethodBody = parse_block_stmt(p);
            } else {  // Parse property
                if (p->currentTokenKind() == lexer::ASSIGNMENT) {
                    p->advance();  // consume '='
                    AssignedValue = parse_expr(p, assignment);
                } else AssignedValue = new ast::NullExpr();
            }

            if (isMethod) {
                if (ExpectedType == nullptr) ExpectedType = new ast::SymbolType(L"auto");
                methods[propertyName] = new ast::StructMethod(
                    propertyName,
                    ExpectedType,
                    MethodBody,
                    isStatic,
                    parameters
                );
                if (p->currentTokenKind() == lexer::SEMICOLON) p->advance();  // Optional semicolon after method declaration
            } else {
                properties[propertyName] = new ast::StructProperty(
                    propertyName,
                    ExpectedType,
                    AssignedValue,
                    isStatic
                );
                p->expect(lexer::SEMICOLON);
            }
        }

        p->expect(lexer::CLOSE_CURLY);
        if (p->currentTokenKind() == lexer::SEMICOLON) p->advance();  // Optional semicolon after struct declaration

        return new ast::StructDeclStmt(
            structName,
            reverseUnorderedMap(properties),  // Properties
            reverseUnorderedMap(methods)   // Methods
        );
    }
    ast::Stmt* parse_if_stmt(Parser* p) {
        if (_verbose) _wcout << L"Parsing if statement at " << p->position() << endl;
        p->expect(lexer::IF, lexer::ELIF);
        ast::Expr* condition = nullptr;
        if (p->currentTokenKind() == lexer::OPEN_PAREN) {
            p->expect(lexer::OPEN_PAREN);
            condition = parse_expr(p, default_bp);
            p->expect(lexer::CLOSE_PAREN);
        } else condition = new ast::BooleanExpr(true);
        if (p->currentTokenKind() == lexer::SEMICOLON) {
            wstring message = L"Expected 'if' statement at " + p->position() + L" to have a condition expression in parentheses, but got ';'";
            p->errors.push_back(ParserError(message));
            _wcout << (_debug ? L"[Parser] " : L"") << message << endl;
            if (_panic) {
                if (_debug) _wcout << L"[Parser] Panicing" << endl;
                exit(1);
            }
            p->advance();
            return nullptr;
        }
        ast::Stmt* thenBranch = parse_block_stmt(p);
        ast::Stmt* elseBranch = nullptr;
        if (p->currentToken().isOneOfMany(lexer::ELSE, lexer::ELIF)) {
            if (p->currentTokenKind() == lexer::ELSE) p->advance();  // consume 'else'
            if (p->currentToken().isOneOfMany(lexer::IF, lexer::ELIF)) elseBranch = parse_if_stmt(p);  // else if
            else {
                if (p->currentTokenKind() != lexer::OPEN_CURLY) {
                    wstring message = L"Expected 'else' branch for if statement at " + p->position() + L" to be a block statement starting with '{', but got " + lexer::TokenKindString(p->currentTokenKind());
                    p->errors.push_back(ParserError(message));
                    _wcout << (_debug ? L"[Parser] " : L"") << message << endl;
                    if (_panic) {
                        if (_debug) _wcout << L"[Parser] Panicing" << endl;
                        exit(1);
                    }
                    p->advanceUntil(lexer::OPEN_CURLY, lexer::SEMICOLON);
                    if (p->currentTokenKind() != lexer::SEMICOLON) p->advanceDepth(lexer::OPEN_CURLY, lexer::CLOSE_CURLY);
                    p->advance();
                } else elseBranch = parse_block_stmt(p);  // else
            }
        }
        return new ast::IfStmt(condition, thenBranch, elseBranch);
    }
    ast::Stmt* parse_while_stmt(Parser* p) {
        if (_verbose) _wcout << L"Parsing while statement at " << p->position() << endl;
        p->expect(lexer::WHILE);
        ast::Expr* condition = nullptr;
        if (p->currentTokenKind() == lexer::OPEN_PAREN) {
            p->expect(lexer::OPEN_PAREN);
            condition = parse_expr(p, default_bp);
            p->expect(lexer::CLOSE_PAREN);
        }
        if (p->currentTokenKind() == lexer::SEMICOLON) {
            wstring message = L"Expected 'while' statement at " + p->position() + L" to have a condition expression in parentheses, but got ';'";
            p->errors.push_back(ParserError(message));
            _wcout << (_debug ? L"[Parser] " : L"") << message << endl;
            if (_panic) {
                if (_debug) _wcout << L"[Parser] Panicing" << endl;
                exit(1);
            }
            p->advance();
            return nullptr;
        }
        if (condition == nullptr) condition = new ast::BooleanExpr(true);
        ast::Stmt* body = parse_block_stmt(p);
        ast::Stmt* elseBranch = nullptr;
        if (p->currentTokenKind() == lexer::ELIF) {
            elseBranch = parse_if_stmt(p);
        } else if (p->currentTokenKind() == lexer::ELSE) {
            p->advance();  // consume 'else'
            if (p->currentTokenKind() == lexer::IF) elseBranch = parse_if_stmt(p);  // else if
            else {
                if (p->currentTokenKind() != lexer::OPEN_CURLY) {
                    wstring message = L"Expected 'else' branch for while statement at " + p->position() + L" to be a block statement starting with '{', but got " + lexer::TokenKindString(p->currentTokenKind());
                    p->errors.push_back(ParserError(message));
                    _wcout << (_debug ? L"[Parser] " : L"") << message << endl;
                    if (_panic) {
                        if (_debug) _wcout << L"[Parser] Panicing" << endl;
                        exit(1);
                    }
                    p->advanceUntil(lexer::OPEN_CURLY, lexer::SEMICOLON);
                    if (p->currentTokenKind() != lexer::SEMICOLON) p->advanceDepth(lexer::OPEN_CURLY, lexer::CLOSE_CURLY);
                    p->advance();
                } else elseBranch = parse_block_stmt(p);  // else
            }
        }
        if (p->currentTokenKind() == lexer::SEMICOLON) p->advance();  // Optional semicolon after while statement
        return new ast::WhileStmt(condition, body, elseBranch);
    }
}