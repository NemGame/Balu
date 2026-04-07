#pragma once

namespace parser {
    struct ParserError : public Error {
        ParserError(const wstring& message, int line, int column) : Error(message, line, column) {}
        ParserError(const wstring& message) : Error(message) {}
        ParserError() : Error() {}
    };
    struct Parser {
        vector<ParserError> errors;
        vector<lexer::Token> tokens;
        int pos;

        lexer::Token currentToken() const {
            if (pos >= (int)tokens.size()) return lexer::Token{ lexer::EOF_TOKEN, L"EOF" };
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
        lexer::Token advanceBack() {
            pos--;
            return currentToken();
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
                    if (_debug) wcout << L"Panicing" << endl;
                    exit(1);
                }
            }
            return advance();
        }
        template<typename... TokenKinds>
        lexer::Token expect(TokenKinds... expectedKinds) {
            lexer::Token token = currentToken();
            lexer::TokenKind kind = token.kind;
            for (lexer::TokenKind expectedKind : {expectedKinds...}) {
                if (kind == expectedKind) {
                    return advance();
                }
            }
            wstring expectedKindsStr;
            for (lexer::TokenKind expectedKind : {expectedKinds...}) {
                expectedKindsStr += lexer::TokenKindString(expectedKind) + L", ";
            }
            expectedKindsStr = expectedKindsStr.substr(0, expectedKindsStr.length() - 2); // Remove trailing comma and space
            wstring message = L"Expected one of [" + expectedKindsStr + L"] but found " + lexer::TokenKindString(kind);
            wcout << message << endl;
            errors.push_back(ParserError(message, 0, 0));
            if (_panic) {
                if (_debug) wcout << L"Panicing" << endl;
                exit(1);
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
#include "types.hpp"
#include "parser.hpp"