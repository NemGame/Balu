#pragma once

namespace parser {
    struct ParserError : public Error {
        ParserError(const wstring& message, int line, int column) : Error(message, line, column) {}
    };
    struct Parser {
        vector<ParserError> errors;
        vector<lexer::Token> tokens;
        int pos;

        lexer::Token currentToken() const {
            return tokens[pos];
        }
        lexer::TokenKind currentTokenKind() const {
            return currentToken().kind;
        }
        lexer::Token advance() {
            lexer::Token token = currentToken();
            pos++;
            return token;
        }
        bool hasTokens() const {
            return pos < tokens.size() && currentToken().kind != lexer::EOF_TOKEN;
        }
        lexer::Token expectError(lexer::TokenKind expectedKind, Error error) {
            lexer::Token token = currentToken();
            lexer::TokenKind kind = token.kind;
            if (kind != expectedKind) {
                if (error.isNull()) {
                    wstring message = L"Expected token " + lexer::TokenKindString(expectedKind) + L" but found " + lexer::TokenKindString(kind);
                    error = Error(message);
                    wcout << message << endl;
                } else {
                    wcout << error.message << endl;
                }
                errors.push_back(ParserError(error.message, 0, 0));
                if (_panic) {
                    wcout << L"Panicing" << endl;
                    exit(1);
                }
            }
            return advance();
        }
        lexer::Token expect(lexer::TokenKind expectedKind) {
            return expectError(expectedKind, Error());
        }
    };
}

#include "lookup.hpp"
#include "expr.hpp"
#include "stmt.hpp"
#include "parser.hpp"