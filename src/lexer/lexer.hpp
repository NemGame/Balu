#pragma once

#include <regex>
#include "tokens.hpp"
#include <functional>

#define byte wchar_t

namespace lexer {
    struct Lexer;

    // type regexHandler func (lex* lexer, regex *regexp.Regexp) in GO -> C++
    #define regexHandler function<void(Lexer* l, const wregex& regexp)> // Regex handler type
    struct RegexPattern {
        wregex pattern;
        regexHandler handler;
    };

    struct Lexer {
        int pos;
        wstring source;
        vector<Token> tokens;
        vector<RegexPattern> patterns;
        vector<Error> errors;
        unsigned long long line;
        unsigned long long column;

        void advanceN(int n) {
            for (int i = 0; i < n; i++) {
                if (pos < source.length()) {
                    if (source[pos] == L'\n') {
                        line++;
                        column = 1;
                    } else {
                        column++;
                    }
                    pos++;
                }
            }
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
        wstring position() const {
            return L'[' + to_wstring(line) + L':' + to_wstring(column) + L']';
        }
    };

    Lexer createLexer(const wstring& source);

    vector<Token> Tokenize(const wstring& source) {
        Lexer lex = createLexer(source);

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
                if (_showWarnings) _wcout << L"Warning: Unmatched character: " << lex.at() << L" at " << lex.position() << endl;
                lex.advanceN(1);
            }
        }

        lex.push(NewToken(EOF_TOKEN, L"EOF", lex.line, lex.column));
        return lex.tokens;
    }

    regexHandler defaultHandler(TokenKind kind, const wstring& value = L"") {
        return [kind, value](Lexer* l, const wregex& regexp) {
            l->push(NewToken(kind, value, l->line, l->column));
            l->advanceN(value.length());
        };
    }

    
    // If ends in '@', it MUST be precise, like 120e120@ must use a fuck ton of bytes, so
    // 120e120 == 120e120 + 1
    // BUT
    // 120e120@ <1 120e120@ + 1
    regexHandler numberHandler = [](Lexer* l, const wregex& regexp) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, regexp);
        wstring value = match.str(0);
        // Remove underscores from the number for easier parsing later
        long double numberOfUnderscores = count(value.begin(), value.end(), L'_');
        value.erase(remove(value.begin(), value.end(), L'_'), value.end());
        // Check if 'e'
        if (value.find(L'e') != wstring::npos) {
            // Split the number into the base and the exponent parts
            size_t ePos = value.find(L'e');
            wstring basePart = value.substr(0, ePos);
            wstring exponentPart = value.substr(ePos + 1);

            // Remove any underscores from the exponent part as well
            exponentPart.erase(remove(exponentPart.begin(), exponentPart.end(), L'_'), exponentPart.end());

            // Reconstruct the number in a standard format for easier parsing later
            value = basePart + L"e" + exponentPart;
        }
        TokenKind kind = NUMBER;
        if (value.back() == L'@') {
            value.pop_back();
            kind = PNUMBER;
        }
        l->push(NewToken(kind, value, l->line, l->column));
        l->advanceN(value.length() + numberOfUnderscores);
        if (kind == PNUMBER) l->advanceN(1); // Advance past the trailing '@' if it's a precise number literal
    };

    // 0b01 - binary
    regexHandler binaryNumberHandler = [](Lexer* l, const wregex& regexp) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, regexp);
        bool isByte = false;
        wstring value = match.str(0); // Remove the "0b" prefix
        if (value.back() == L'b') {
            isByte = true;
            value.pop_back();
        }
        // Turn to binary number
        unsigned long long numberValue = 0;
        for (char c : value.substr(2)) {
            if (c == L'_') continue; // Skip underscores
            numberValue <<= 1; // Shift left by 1 (multiply by 2)
            if (c == L'1') {
                numberValue |= 1; // Set the last bit if it's '1'
            }
        }
        l->push(NewToken(isByte ? BYTE : NUMBER, to_wstring(numberValue), l->line, l->column));
        l->advanceN(value.length()); 
        if (isByte) l->advanceN(1); // Advance past the trailing 'b' if it's a byte literal
    };

    // 010 - octal
    regexHandler octalNumberHandler = [](Lexer* l, const wregex& regexp) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, regexp);
        bool isByte = false;
        wstring value = match.str(0); // Remove the "0" prefix
        if (value.back() == L'b') {
            isByte = true;
            value.pop_back();
        }
        // Turn to octal number
        unsigned long long numberValue = 0;
        for (char c : value.substr(1)) {
            if (c == L'_') continue; // Skip underscores
            numberValue <<= 3; // Shift left by 3 (multiply by 8)
            if (c >= L'0' && c <= L'7') {
                numberValue |= (c - L'0'); // Add the digit value
            }
        }
        l->push(NewToken(isByte ? BYTE : NUMBER, to_wstring(numberValue), l->line, l->column));
        l->advanceN(value.length());
        if (isByte) l->advanceN(1); // Advance past the trailing 'b' if it's a byte literal
    };

    // 0x1 - hexadecimal
    regexHandler hexNumberHandler = [](Lexer* l, const wregex& regexp) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, regexp);
        wstring value = match.str(0); // Remove the "0x" prefix
        // Turn to hexadecimal number
        unsigned long long numberValue = 0;
        for (char c : value.substr(2)) {
            if (c == L'_') continue; // Skip underscores
            numberValue <<= 4; // Shift left by 4 (multiply by 16)
            if (c >= L'0' && c <= L'9') {
                numberValue |= (c - L'0'); // Add the digit value
            } else if (c >= L'a' && c <= L'f') {
                numberValue |= (c - L'a' + 10); // Add the digit value
            } else if (c >= L'A' && c <= L'F') {
                numberValue |= (c - L'A' + 10); // Add the digit value
            }
        }
        l->push(NewToken(NUMBER, to_wstring(numberValue), l->line, l->column));
        l->advanceN(value.length());
    };

    regexHandler byteHandler = [](Lexer* l, const wregex& regexp) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, regexp);
        wstring value = match.str(0);
        value.erase(remove(value.begin(), value.end(), L'_'), value.end());
        l->push(NewToken(NULL_, value.substr(0, value.length() - 1), l->line, l->column)); // Remove the trailing 'b'
        l->advanceN(value.length());
    };

    regexHandler symbolHandler = [](Lexer* l, const wregex& regexp) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, regexp);
        wstring value = match.str(0);
        unsigned long long line = l->line;
        unsigned long long col = l->column;
        
        if (reserved_lu.find(value) != reserved_lu.end()) 
            l->push(NewToken(reserved_lu[value], value, line, col));
        else 
            l->push(NewToken(IDENTIFIER, value, line, col));
        
        l->advanceN(value.length());
    };

    regexHandler skipHandler = [](Lexer* l, const wregex& regexp) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, regexp);
        wstring value = match.str(0);
        l->advanceN(value.length());
    };

    regexHandler multilineSkipHandler = [](Lexer* l, const wregex& regexp) {
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

    regexHandler semicolonHandler = [](Lexer* l, const wregex& regexp) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, regexp);
        wstring value = match.str(0);
        l->push(NewToken(SEMICOLON, L";", l->line, l->column));
        l->advanceN(value.length()); // Advance past the semicolon
    };

    regexHandler stringHandler = [](Lexer* l, const wregex& regexp) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, regexp);
        wstring value = match.str(0).substr(1, match.str(0).length() - 2); // Remove the surrounding quotes
        l->push(NewToken(STRING, value, l->line, l->column));
        l->advanceN(value.length() + 2); // Advance past the string including the quotes
    };
    
    regexHandler fstringHandler = [](Lexer* l, const wregex& regexp) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, regexp);
        wstring value = match.str(0).substr(2, match.str(0).length() - 3); // Remove the $" and " surrounding the format string
        l->push(NewToken(FSTRING, value, l->line, l->column));
        l->advanceN(value.length() + 3); // Advance past the string including the $" and " surrounding the format string
    };

    regexHandler characterHandler = [](Lexer* l, const wregex& regexp) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, regexp);
        wstring value = match.str(0).substr(1, match.str(0).length() - 2); // Extract the character(s) between the single quotes
        unsigned long long line = l->line;
        unsigned long long col = l->column;
        if (value.length() != 1) {
            if (_allowLexerErrors) {
                wstring message = L"Invalid character literal at " + l->position() + L": " + match.str(0) + L". Character literals must contain exactly one character.";
                _wcout << (_debug ? L"[Lexer] " : L"") << message << endl;
                l->errors.push_back(Error(message, line, col));
                if (_panic) {
                    if (_debug) _wcout << L"[Lexer] Panicing" << endl;
                    exit(1);
                }
            }
            l->advanceN(match.str(0).length()); // Advance past the invalid character literal
            value = value.substr(0, 1); // Default to the first character if multiple are found
        } else l->advanceN(3); // Advance past the character including the surrounding single quotes
        l->push(NewToken(CHAR, value, line, col));
    };

    regexHandler ruleHandler = [](Lexer* l, const wregex& regexp) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, regexp);
        wstring value = match.str(0);
        if (_verbose) _wcout << L"Found rule: " << value << endl;
        l->push(NewToken(RULE, value.substr(1), l->line, l->column)); // Remove the "#" prefix
        l->advanceN(value.length());
    };

    Lexer createLexer(const wstring& source) {
        return Lexer {
            0,
            source,
            vector<Token>(0),
            {
                {wregex(L"^true"), defaultHandler(BOOL, L"true")},
                {wregex(L"^false"), defaultHandler(BOOL, L"false")},
                {wregex(L"^[\\$f]\"[^\"]*\""), fstringHandler},
                {wregex(L"^[a-zA-Z_\\u0080-\\uFFFF\\$][a-zA-Z0-9_\\u0080-\\uFFFF\\$]*"), symbolHandler},
                {wregex(L"^0b[0-1_]+b?"), binaryNumberHandler},
                {wregex(L"^0[0-7_]+b?"), octalNumberHandler},
                {wregex(L"^0x[0-9a-fA-F_]+"), hexNumberHandler},
                {wregex(L"^[0-9][0-9_]*b"), byteHandler},
                {wregex(L"^[0-9][0-9_]*(\\.[0-9_]*)?(e[0-9_+]+)?@?"), numberHandler},
                {wregex(L"^\"[^\"]*\""), stringHandler},
                {wregex(L"^\'[^\']*\'"), characterHandler},
                {wregex(L"^//.*"), skipHandler},
                {wregex(L"^/\\*"), multilineSkipHandler},
                {wregex(L"^;+"), semicolonHandler},
                {wregex(L"^\\s+"), skipHandler},
                {wregex(L"^#[^;\n\\s]*"), ruleHandler},
                {wregex(L"^%:[^;\n\\s]*"), ruleHandler},
                {wregex(L"^\\?\\?=[^;\n\\s]*"), ruleHandler},

                {wregex(L"^\\["), defaultHandler(OPEN_BRACKET, L"[")},
                {wregex(L"^\\]"), defaultHandler(CLOSE_BRACKET, L"]")},
                {wregex(L"^\\{"), defaultHandler(OPEN_CURLY, L"{")},
                {wregex(L"^\\}"), defaultHandler(CLOSE_CURLY, L"}")},
                {wregex(L"^\\("), defaultHandler(OPEN_PAREN, L"(")},
                {wregex(L"^\\)"), defaultHandler(CLOSE_PAREN, L")")},
                {wregex(L"^=="), defaultHandler(EQUALS, L"==")},
                {wregex(L"^!="), defaultHandler(NOT_EQUALS, L"!=")},
                {wregex(L"^=>"), defaultHandler(ARROW, L"=>")},
                {wregex(L"^="), defaultHandler(ASSIGNMENT, L"=")},
                {wregex(L"^!"), defaultHandler(NOT, L"!")},
                {wregex(L"^>>>="), defaultHandler(BITWISE_RIGHT_ROTATE_EQUALS, L">>>=")},
                {wregex(L"^<<<="), defaultHandler(BITWISE_LEFT_ROTATE_EQUALS, L"<<<=")},
                {wregex(L"^<<<"), defaultHandler(BITWISE_LEFT_ROTATE, L"<<<")},
                {wregex(L"^>>>"), defaultHandler(BITWISE_RIGHT_ROTATE, L">>>")},
                {wregex(L"^>>="), defaultHandler(BITWISE_RIGHT_EQUALS, L">>=")},
                {wregex(L"^<<="), defaultHandler(BITWISE_LEFT_EQUALS, L"<<=")},
                {wregex(L"^<<"), defaultHandler(BITWISE_LEFT, L"<<")},
                {wregex(L"^>>"), defaultHandler(BITWISE_RIGHT, L">>")},
                {wregex(L"^\\?\\?-"), defaultHandler(BITWISE_NOT, L"\?\?-")},
                {wregex(L"^\\?\\?'"), defaultHandler(BITWISE_XOR, L"\?\?'")},
                {wregex(L"^\\?\\?!"), defaultHandler(BITWISE_OR, L"\?\?!")}, 
                {wregex(L"^\\?\\?<"), defaultHandler(OPEN_CURLY, L"\?\?<")},
                {wregex(L"^\\?\\?>"), defaultHandler(CLOSE_CURLY, L"\?\?>")},
                {wregex(L"^\\?\\?\\("), defaultHandler(OPEN_BRACKET, L"\?\?(")},
                {wregex(L"^\\?\\?\\)"), defaultHandler(CLOSE_BRACKET, L"\?\?)")},
                {wregex(L"^<="), defaultHandler(LESS_EQUALS, L"<=")},
                {wregex(L"^<%"), defaultHandler(OPEN_CURLY, L"<%")},
                {wregex(L"^<:"), defaultHandler(OPEN_BRACKET, L"<:")},
                {wregex(L"^<"), defaultHandler(LESS, L"<")},
                {wregex(L"^>="), defaultHandler(GREATER_EQUALS, L">=")},
                {wregex(L"^%>"), defaultHandler(CLOSE_CURLY, L"%>")},
                {wregex(L"^:>"), defaultHandler(CLOSE_BRACKET, L":>")},
                {wregex(L"^>"), defaultHandler(GREATER, L">")},
                {wregex(L"^\\|\\|"), defaultHandler(OR, L"||")},
                {wregex(L"^&&"), defaultHandler(AND, L"&&")},
                {wregex(L"^\\.\\."), defaultHandler(DOT_DOT, L"..")},
                {wregex(L"^\\."), defaultHandler(DOT, L".")},
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
                {wregex(L"^\\|="), defaultHandler(BITWISE_OR_EQUALS, L"|=")},
                {wregex(L"^\\|"), defaultHandler(BITWISE_OR, L"|")},
                {wregex(L"^&="), defaultHandler(BITWISE_AND_EQUALS, L"&=")},
                {wregex(L"^&"), defaultHandler(BITWISE_AND, L"&")},
                {wregex(L"^\\^="), defaultHandler(BITWISE_XOR_EQUALS, L"^=")},
                {wregex(L"^\\^"), defaultHandler(BITWISE_XOR, L"^")},
                {wregex(L"^~="), defaultHandler(BITWISE_NOT_EQUALS, L"~=")},
                {wregex(L"^~"), defaultHandler(BITWISE_NOT, L"~")},
            },
            vector<Error>(0),
            1, // line
            1  // column
        };
    }
}