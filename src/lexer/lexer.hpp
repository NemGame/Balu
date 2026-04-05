#pragma once

#include <regex>
#include "tokens.hpp"
#include <functional>

#define byte wchar_t

namespace lexer {
    struct lexer;

    // type regexHandler func (lex* lexer, regex *regexp.Regexp) in GO -> C++
    #define regexHandler void(*)(lexer* l, const wregex& regexp)
    struct RegexPattern {
        wregex pattern;
        function<void(lexer* l, const wregex& regexp)> handler;
    };

    struct lexer {
        int pos;
        wstring source;
        vector<Token> tokens;
        vector<RegexPattern> patterns;

        void advanceN(int n) {
            pos += n;
        }
        void push(Token token) {
            tokens.push_back(token);
        }
        byte at() {
            return source[pos];
        }
        wstring remainder() {
            return source.substr(pos);
        }
        bool at_eof() {
            return pos >= source.length();
        }
    };

    function<void(lexer* l, const wregex& regexp)> numberHandler = [](lexer* l, const wregex& regexp) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, regexp);
        wstring value = match.str(0);
        l->advanceN(value.length());
        l->push(NewToken(NUMBER, value));
    };

    function<void(lexer* l, const wregex& regexp)> skipHandler = [](lexer* l, const wregex& regexp) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, regexp);
        wstring value = match.str(0);
        l->advanceN(value.length());
    };

    lexer createLexer(const wstring& source);

    vector<Token> Tokenize(const wstring& source) {
        lexer lex = createLexer(source);

        while (!lex.at_eof())
        {
            bool matched = false;
            const wstring remaining = lex.remainder();
            for (const RegexPattern& pattern : lex.patterns) {
                wsmatch match;
                if (regex_search(remaining, match, pattern.pattern)) {
                    pattern.handler(&lex, pattern.pattern);
                    matched = true;
                    break;
                }
            }
            if (!matched) {
                // Handle unmatched case, e.g., advance by one character or throw an error
                wcout << L"Unmatched character: " << lex.at() << L" at position " << lex.pos << endl;
                lex.advanceN(1);
            }
        }

        lex.push(NewToken(EOF_TOKEN, L"EOF"));
        return lex.tokens;
    }

    function<void(lexer* l, const wregex& regexp)> defaultHandler(TokenKind kind, const wstring& value = L"") {
        return [kind, value](lexer* l, const wregex& regexp) {
            l->advanceN(value.length());
            l->push(NewToken(kind, value));
        };
    }

    lexer createLexer(const wstring& source) {
        return lexer {
            0,
            source,
            vector<Token>(0),
            {
                {wregex(L"^[0-9]+(\\.[0-9]+)?"), numberHandler},
                {wregex(L"^\\s+"), skipHandler},
                {wregex(L"^\\["), defaultHandler(OPEN_BRACKET, L"[")},
                {wregex(L"^\\]"), defaultHandler(CLOSE_BRACKET, L"]")},
                {wregex(L"^\\{"), defaultHandler(OPEN_CURLY, L"{")},
                {wregex(L"^\\}"), defaultHandler(CLOSE_CURLY, L"}")},
                {wregex(L"^\\("), defaultHandler(OPEN_PAREN, L"(")},
                {wregex(L"^\\)"), defaultHandler(CLOSE_PAREN, L")")},
                {wregex(L"^=="), defaultHandler(EQUALS, L"==")},
                {wregex(L"^!="), defaultHandler(NOT_EQUALS, L"!=")},
                {wregex(L"^="), defaultHandler(ASSIGNMENT, L"=")},
                {wregex(L"^!"), defaultHandler(NOT, L"!")},
                {wregex(L"^<="), defaultHandler(LESS_EQUALS, L"<=")},
                {wregex(L"^<"), defaultHandler(LESS, L"<")},
                {wregex(L"^>="), defaultHandler(GREATER_EQUALS, L">=")},
                {wregex(L"^>"), defaultHandler(GREATER, L">")},
                {wregex(L"^\\|\\|"), defaultHandler(OR, L"||")},
                {wregex(L"^&&"), defaultHandler(AND, L"&&")},
                {wregex(L"^\\.\\."), defaultHandler(DOT_DOT, L"..")},
                {wregex(L"^\\."), defaultHandler(DOT, L".")},
                {wregex(L"^;"), defaultHandler(SEMICOLON, L";")},
                {wregex(L"^:"), defaultHandler(COLON, L":")},
                {wregex(L"^\\?"), defaultHandler(QUESTION, L"?")},
                {wregex(L"^,"), defaultHandler(COMMA, L",")},
                {wregex(L"^\\+\\+"), defaultHandler(PLUS_PLUS, L"++")},
                {wregex(L"^--"), defaultHandler(MINUS_MINUS, L"--")},
                {wregex(L"^\\+="), defaultHandler(PLUS_EQUALS, L"+=")},
                {wregex(L"^-="), defaultHandler(MINUS_EQUALS, L"-=")},
                {wregex(L"^/="), defaultHandler(SLASH_EQUALS, L"/=")},
                {wregex(L"^\\*="), defaultHandler(STAR_EQUALS, L"*=")},
                {wregex(L"^%="), defaultHandler(PERCENT_EQUALS, L"%=")},
                {wregex(L"^\\+"), defaultHandler(PLUS, L"+")},
                {wregex(L"^-"), defaultHandler(DASH, L"-")},
                {wregex(L"^/"), defaultHandler(SLASH, L"/")},
                {wregex(L"^\\*"), defaultHandler(STAR, L"*")},
                {wregex(L"^%"), defaultHandler(PERCENT, L"%")},
            }
        };
    }

}