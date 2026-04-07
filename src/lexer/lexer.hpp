#pragma once

#include <regex>
#include "tokens.hpp"
#include <functional>

#define byte wchar_t

namespace lexer {
    struct lexer;

    // type regexHandler func (lex* lexer, regex *regexp.Regexp) in GO -> C++
    #define regexHandler function<void(lexer* l, const wregex& regexp)> // Regex handler type
    struct RegexPattern {
        wregex pattern;
        regexHandler handler;
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
                if (_showWarnings) wcout << L"Unmatched character: " << lex.at() << L" at position " << lex.pos << endl;
                lex.advanceN(1);
            }
        }

        lex.push(NewToken(EOF_TOKEN, L"EOF"));
        return lex.tokens;
    }

    regexHandler defaultHandler(TokenKind kind, const wstring& value = L"") {
        return [kind, value](lexer* l, const wregex& regexp) {
            l->advanceN(value.length());
            l->push(NewToken(kind, value));
        };
    }

    
    regexHandler numberHandler = [](lexer* l, const wregex& regexp) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, regexp);
        wstring value = match.str(0);
        l->advanceN(value.length());
        l->push(NewToken(NUMBER, value));
    };

    regexHandler symbolHandler = [](lexer* l, const wregex& regexp) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, regexp);
        wstring value = match.str(0);
        

        if (reserved_lu.find(value) != reserved_lu.end()) l->push(NewToken(reserved_lu[value], value));
        else l->push(NewToken(IDENTIFIER, value));


        
        l->advanceN(value.length());
    };

    regexHandler skipHandler = [](lexer* l, const wregex& regexp) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, regexp);
        wstring value = match.str(0);
        l->advanceN(value.length());
    };

    regexHandler multilineSkipHandler = [](lexer* l, const wregex& regexp) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, regexp);
        wstring value = match.str(0);
        l->advanceN(value.length());

        // Now skip until we find the closing comment tag
        wregex endCommentRegex(L"\\*/");
        while (!l->at_eof()) {
            wsmatch endMatch;
            const wstring newRemaining = l->remainder();
            if (regex_search(newRemaining, endMatch, endCommentRegex)) {
                l->advanceN(endMatch.position() + 2); // Advance past the closing tag
                break;
            } else {
                l->advanceN(newRemaining.length()); // Skip the rest if no closing tag is found
            }
        }
    };

    regexHandler stringHandler = [](lexer* l, const wregex& regexp) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, regexp);
        wstring value = match.str(0).substr(1, match.str(0).length() - 2); // Remove the surrounding quotes
        l->advanceN(value.length() + 2); // Advance past the string including the quotes
        l->push(NewToken(STRING, value));
    };

    regexHandler ruleHandler = [](lexer* l, const wregex& regexp) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, regexp);
        wstring value = match.str(0);
        if (_verbose) wcout << L"Found rule: " << value << endl;
        l->advanceN(value.length());
        l->push(NewToken(RULE, value.substr(6))); // Remove the "#rule " prefix
    };

    lexer createLexer(const wstring& source) {
        return lexer {
            0,
            source,
            vector<Token>(0),
            {
                {wregex(L"^[a-zA-Z_][a-zA-Z0-9_]*"), symbolHandler},
                {wregex(L"^[0-9]+(\\.[0-9]+)?"), numberHandler},
                {wregex(L"^\"[^\"]*\""), stringHandler},
                {wregex(L"^//.*"), skipHandler},
                {wregex(L"^/\\*"), multilineSkipHandler},
                {wregex(L"^\\s+"), skipHandler},
                {wregex(L"^#rule[^;\n]*"), ruleHandler},

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