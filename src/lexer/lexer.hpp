#pragma once

#include <regex>
#include "tokens.hpp"
#include <functional>

#define byte wchar_t

namespace lexer {
    struct Lexer;

    // type regexHandler func (lex* lexer, regex *regexp.Regexp) in GO -> C++
    #define regexHandler function<void(Lexer* l, const Pattern& pattern)> // Regex handler type
    struct Pattern {
        wregex regexPattern;
        wstring wstringPattern;
        bool useRegex;
        Pattern(const wregex& regexPattern) : regexPattern(regexPattern), wstringPattern(L""), useRegex(true) {}
        Pattern(const wstring& wstringPattern) : regexPattern(wregex(L"")), wstringPattern(wstringPattern), useRegex(false) {}
        bool isRegex() const {
            return useRegex;
        }
        bool isWString() const {
            return !wstringPattern.empty();
        }
    };
    struct RegexPattern {
        Pattern pattern;
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
        if (_verbose) _wcout << L"[Lexer] Starting tokenization" << endl;
        Lexer lex = createLexer(source);

        while (!lex.at_eof())
        {
            bool matched = false;
            const wstring remaining = lex.remainder();
            for (const RegexPattern& rpattern : lex.patterns) {
                wsmatch match;
                if (rpattern.pattern.isRegex() && regex_search(remaining, match, rpattern.pattern.regexPattern)) {
                    rpattern.handler(&lex, rpattern.pattern);
                    matched = true;
                    break;
                } else if (rpattern.pattern.isWString() && remaining.find(rpattern.pattern.wstringPattern) == 0) {
                    rpattern.handler(&lex, rpattern.pattern);
                    matched = true;
                    break;
                }
            }
            if (!matched) {
                // Handle unmatched case, e.g., advance by one character or throw an error
                if (_showWarnings) _wcout << L"Warning: Unmatched character: " << lex.at() << L" at " << lex.position() << endl;
                lex.advanceN(1);
            }
            if (_verbose) _wcout << L"[Lexer] Current position: " << lex.position() << L", Tokens so far: " << lex.tokens.size() << endl;
        }

        lex.push(NewToken(EOF_TOKEN, L"EOF", lex.line, lex.column));
        return lex.tokens;
    }

    regexHandler defaultHandler(TokenKind kind, const wstring& value = L"") {
        return [kind, value](Lexer* l, const Pattern& pattern) {
            l->push(NewToken(kind, value, l->line, l->column));
            l->advanceN(value.length());
        };
    }

    
    bool tryParseUnsigned(const wstring& text, unsigned long long& out) {
        if (text.empty()) {
            return false;
        }
        unsigned long long value = 0;
        for (wchar_t c : text) {
            if (c < L'0' || c > L'9') {
                return false;
            }
            unsigned long long digit = static_cast<unsigned long long>(c - L'0');
            const unsigned long long maxBeforeMul = ULLONG_MAX / 10ULL;
            if (value > maxBeforeMul || (value == maxBeforeMul && digit > (ULLONG_MAX % 10ULL))) {
                return false;
            }
            value = value * 10ULL + digit;
        }
        out = value;
        return true;
    }

    // Expands decimal scientific notation to plain decimal text (e.g. 10e3 -> 10000).
    bool expandScientificNotation(wstring& value) {
        const size_t ePos = value.find(L'e');
        if (ePos == wstring::npos) {
            return true;
        }

        wstring basePart = value.substr(0, ePos);
        wstring exponentPart = value.substr(ePos + 1);
        if (basePart.empty() || exponentPart.empty()) {
            return false;
        }

        if (exponentPart[0] == L'+') {
            exponentPart = exponentPart.substr(1);
        }

        unsigned long long exponent = 0;
        if (!tryParseUnsigned(exponentPart, exponent)) {
            return false;
        }

        const size_t dotPos = basePart.find(L'.');
        wstring digitsOnly = basePart;
        size_t fractionalDigits = 0;

        if (dotPos != wstring::npos) {
            digitsOnly.erase(dotPos, 1);
            fractionalDigits = basePart.length() - dotPos - 1;
        }

        if (digitsOnly.empty()) {
            return false;
        }

        // Strip leading zeros but keep at least one digit.
        size_t firstNonZero = digitsOnly.find_first_not_of(L'0');
        if (firstNonZero == wstring::npos) {
            digitsOnly = L"0";
        } else if (firstNonZero > 0) {
            digitsOnly = digitsOnly.substr(firstNonZero);
        }

        if (exponent >= fractionalDigits) {
            digitsOnly.append(static_cast<size_t>(exponent - fractionalDigits), L'0');
            value = digitsOnly;
            return true;
        }

        const size_t decimalPos = digitsOnly.length() - static_cast<size_t>(fractionalDigits - exponent);
        value = digitsOnly.substr(0, decimalPos) + L"." + digitsOnly.substr(decimalPos);
        return true;
    }

    // If ends in '@', it MUST be precise, like 120e120@ must use a fuck ton of bytes, so
    // 120e120 == 120e120 + 1
    // BUT
    // 120e120@ <1 120e120@ + 1
    regexHandler numberHandler = [](Lexer* l, const Pattern& pattern) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, pattern.regexPattern);
        const wstring matchedValue = match.str(0);
        wstring value = matchedValue;
        // Remove underscores from the number for easier parsing later.
        value.erase(remove(value.begin(), value.end(), L'_'), value.end());
        TokenKind kind = NUMBER;
        if (value.back() == L'@') {
            value.pop_back();
            kind = PNUMBER;

            // Preserve precise literals as exact decimal text for AST/decompiler and big-number ops.
            if (!expandScientificNotation(value)) {
                if (_allowLexerErrors) {
                    wstring message = L"Invalid precise number literal at " + l->position() + L": " + match.str(0);
                    _wcout << (_debug ? L"[Lexer] " : L"") << message << endl;
                    l->errors.push_back(Error(message, l->line, l->column));
                    if (_panic) {
                        if (_debug) _wcout << L"[Lexer] Panicing" << endl;
                        exit(1);
                    }
                }
            }
        }
        l->push(NewToken(kind, value, l->line, l->column));
        l->advanceN(static_cast<int>(matchedValue.length()));
    };

    // 0b01 - binary
    regexHandler binaryNumberHandler = [](Lexer* l, const Pattern& pattern) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, pattern.regexPattern);
        bool isByte = false;
        wstring value = match.str(0); // Remove the "0b" prefix
        if (value.back() == L'b') {
            isByte = true;
            value.pop_back();
        }
        bool isPrecise = false;
        if (value.back() == L'@') {
            isPrecise = true;
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
        l->push(NewToken(isByte ? BYTE : (isPrecise ? PNUMBER : NUMBER), to_wstring(numberValue), l->line, l->column));
        l->advanceN(value.length()); 
        if (isByte) l->advanceN(1); // Advance past the trailing 'b' if it's a byte literal
        if (isPrecise) l->advanceN(1); // Advance past the trailing '@' if it's a precise number literal
    };

    // 010 - octal
    regexHandler octalNumberHandler = [](Lexer* l, const Pattern& pattern) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, pattern.regexPattern);
        bool isByte = false;
        wstring value = match.str(0); // Remove the "0" prefix
        if (value.back() == L'b') {
            isByte = true;
            value.pop_back();
        }
        bool isPrecise = false;
        if (value.back() == L'@') {
            isPrecise = true;
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
        l->push(NewToken(isByte ? BYTE : (isPrecise ? PNUMBER : NUMBER), to_wstring(numberValue), l->line, l->column));
        l->advanceN(value.length());
        if (isByte) l->advanceN(1); // Advance past the trailing 'b' if it's a byte literal
        if (isPrecise) l->advanceN(1); // Advance past the trailing '@' if it's a precise number literal
    };

    // 0x1 - hexadecimal
    regexHandler hexNumberHandler = [](Lexer* l, const Pattern& pattern) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, pattern.regexPattern);
        wstring value = match.str(0); // Remove the "0x" prefix
        bool isPrecise = false;
        if (value.back() == L'@') {
            isPrecise = true;
            value.pop_back();
        }
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
        l->push(NewToken(isPrecise ? PNUMBER : NUMBER, to_wstring(numberValue), l->line, l->column));
        l->advanceN(value.length());
        if (isPrecise) l->advanceN(1); // Advance past the trailing '@' if it's a precise number literal
    };

    regexHandler byteHandler = [](Lexer* l, const Pattern& pattern) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, pattern.regexPattern);
        wstring value = match.str(0);
        value.erase(remove(value.begin(), value.end(), L'_'), value.end());
        bool isPrecise = false;
        if (value.back() == L'@') {
            isPrecise = true;
            value.pop_back();
            if (_allowLexerErrors) {
                const wstring bRemoved = value.back() == L'b' ? value.substr(0, value.length() - 1) : value;
                const wstring message = L"Invalid byte literal at " + l->position() + L": " + match.str(0) + L" | Byte literals cannot be precise. Did you mean to use a precise number literal (e.g., " + bRemoved + L"@) instead?";
                _wcout << (_debug ? L"[Lexer] " : L"") << message << endl;
                l->errors.push_back(Error(message, l->line, l->column));
                if (_panic) {
                    if (_debug) _wcout << L"[Lexer] Panicing" << endl;
                    exit(1);
                }
            }
        }
        l->push(NewToken(isPrecise ? PNUMBER : BYTE, value.substr(0, value.length() - 1), l->line, l->column)); // Remove the trailing 'b'
        l->advanceN(value.length());
        if (isPrecise) l->advanceN(1); // Advance past the trailing '@' if it's a precise number literal
    };

    regexHandler symbolHandler = [](Lexer* l, const Pattern& pattern) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, pattern.regexPattern);
        wstring value = match.str(0);
        unsigned long long line = l->line;
        unsigned long long col = l->column;
        
        if (reserved_lu.find(value) != reserved_lu.end()) 
            l->push(NewToken(reserved_lu[value], value, line, col));
        else 
            l->push(NewToken(IDENTIFIER, value, line, col));
        
        l->advanceN(value.length());
    };

    regexHandler skipHandler = [](Lexer* l, const Pattern& pattern) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, pattern.regexPattern);
        wstring value = match.str(0);
        l->advanceN(value.length());
    };

    regexHandler multilineSkipHandler = [](Lexer* l, const Pattern& pattern) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, pattern.regexPattern);
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

    regexHandler semicolonHandler = [](Lexer* l, const Pattern& pattern) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, pattern.regexPattern);
        wstring value = match.str(0);
        l->push(NewToken(SEMICOLON, L";", l->line, l->column));
        l->advanceN(value.length()); // Advance past the semicolon
    };

    regexHandler stringHandler = [](Lexer* l, const Pattern& pattern) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, pattern.regexPattern);
        wstring value = match.str(0).substr(1, match.str(0).length() - 2); // Remove the surrounding quotes
        l->push(NewToken(STRING, value, l->line, l->column));
        l->advanceN(value.length() + 2); // Advance past the string including the quotes
    };
    
    regexHandler fstringHandler = [](Lexer* l, const Pattern& pattern) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, pattern.regexPattern);
        wstring value = match.str(0).substr(2, match.str(0).length() - 3); // Remove the $" and " surrounding the format string
        l->push(NewToken(FSTRING, value, l->line, l->column));
        l->advanceN(value.length() + 3); // Advance past the string including the $" and " surrounding the format string
    };

    regexHandler characterHandler = [](Lexer* l, const Pattern& pattern) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, pattern.regexPattern);
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

    regexHandler ruleHandler = [](Lexer* l, const Pattern& pattern) {
        wsmatch match;
        const wstring remaining = l->remainder();
        regex_search(remaining, match, pattern.regexPattern);
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
                {Pattern(L"true"), defaultHandler(BOOL, L"true")},
                {Pattern(L"false"), defaultHandler(BOOL, L"false")},
                {Pattern(wregex(L"^[\\$f]\"[^\"]*\"")), fstringHandler},
                {Pattern(wregex(L"^[a-zA-Z_\\u0080-\\uFFFF\\$][a-zA-Z0-9_\\u0080-\\uFFFF\\$]*")), symbolHandler},
                {Pattern(wregex(L"^0b[0-1_]+b?@?")), binaryNumberHandler},
                {Pattern(wregex(L"^0[0-7_]+b?@?")), octalNumberHandler},
                {Pattern(wregex(L"^0x[0-9a-fA-F_]+@?")), hexNumberHandler},
                {Pattern(wregex(L"^[0-9][0-9_]*b@?")), byteHandler},
                {Pattern(wregex(L"^[0-9][0-9_]*(\\.[0-9_]*)?(e[0-9_+]+)?@?")), numberHandler},
                {Pattern(wregex(L"^\"[^\"]*\"")), stringHandler},
                {Pattern(wregex(L"^\'[^\']*\'")), characterHandler},
                {Pattern(wregex(L"^//.*")), skipHandler},
                {Pattern(wregex(L"^/\\*")), multilineSkipHandler},
                {Pattern(wregex(L"^;+")), semicolonHandler},
                {Pattern(wregex(L"^\\s+")), skipHandler},
                {Pattern(wregex(L"^#[^;\n\\s]*")), ruleHandler},
                {Pattern(wregex(L"^%:[^;\n\\s]*")), ruleHandler},
                {Pattern(wregex(L"^\\?\\?=[^;\n\\s]*")), ruleHandler},

                {Pattern(L"["), defaultHandler(OPEN_BRACKET, L"[")},
                {Pattern(L"]"), defaultHandler(CLOSE_BRACKET, L"]")},
                {Pattern(L"{"), defaultHandler(OPEN_CURLY, L"{")},
                {Pattern(L"}"), defaultHandler(CLOSE_CURLY, L"}")},
                {Pattern(L"("), defaultHandler(OPEN_PAREN, L"(")},
                {Pattern(L")"), defaultHandler(CLOSE_PAREN, L")")},
                {Pattern(L"=="), defaultHandler(EQUALS, L"==")},
                {Pattern(L"!="), defaultHandler(NOT_EQUALS, L"!=")},
                {Pattern(L"=>"), defaultHandler(ARROW, L"=>")},
                {Pattern(L"="), defaultHandler(ASSIGNMENT, L"=")},
                {Pattern(L"!"), defaultHandler(NOT, L"!")},
                {Pattern(L">>>="), defaultHandler(BITWISE_RIGHT_ROTATE_EQUALS, L">>>=")},
                {Pattern(L"<<<="), defaultHandler(BITWISE_LEFT_ROTATE_EQUALS, L"<<<=")},
                {Pattern(L"<<<"), defaultHandler(BITWISE_LEFT_ROTATE, L"<<<")},
                {Pattern(L">>>"), defaultHandler(BITWISE_RIGHT_ROTATE, L">>>")},
                {Pattern(L">>="), defaultHandler(BITWISE_RIGHT_EQUALS, L">>=")},
                {Pattern(L"<<="), defaultHandler(BITWISE_LEFT_EQUALS, L"<<=")},
                {Pattern(L"<<"), defaultHandler(BITWISE_LEFT, L"<<")},
                {Pattern(L">>"), defaultHandler(BITWISE_RIGHT, L">>")},
                // {Pattern(L"??-"), defaultHandler(BITWISE_NOT, L"??-")},
                // {Pattern(L"??'"), defaultHandler(BITWISE_XOR, L"??'")},
                // {Pattern(L"??!"), defaultHandler(BITWISE_OR, L"??!")}, 
                // {Pattern(L"??<"), defaultHandler(OPEN_CURLY, L"??<")},
                // {Pattern(L"??>"), defaultHandler(CLOSE_CURLY, L"??>")},
                // {Pattern(L"??("), defaultHandler(OPEN_BRACKET, L"??(")},
                // {Pattern(L"??)"), defaultHandler(CLOSE_BRACKET, L"??)")},
                {Pattern(L"<="), defaultHandler(LESS_EQUALS, L"<=")},
                // {Pattern(L"<%"), defaultHandler(OPEN_CURLY, L"<%")},
                // {Pattern(L"<:"), defaultHandler(OPEN_BRACKET, L"<:")},
                {Pattern(L"<"), defaultHandler(LESS, L"<")},
                {Pattern(L">="), defaultHandler(GREATER_EQUALS, L">=")},
                // {Pattern(L"%>"), defaultHandler(CLOSE_CURLY, L"%>")},
                // {Pattern(L":>"), defaultHandler(CLOSE_BRACKET, L":>")},
                {Pattern(L">"), defaultHandler(GREATER, L">")},
                {Pattern(L"||"), defaultHandler(OR, L"||")},
                {Pattern(L"&&"), defaultHandler(AND, L"&&")},
                {Pattern(L".."), defaultHandler(DOT_DOT, L"..")},
                {Pattern(L"."), defaultHandler(DOT, L".")},
                {Pattern(L":"), defaultHandler(COLON, L":")},
                {Pattern(L"?"), defaultHandler(QUESTION, L"?")},
                {Pattern(L","), defaultHandler(COMMA, L",")},
                {Pattern(L"++"), defaultHandler(PLUS_PLUS, L"++")},
                {Pattern(L"--"), defaultHandler(MINUS_MINUS, L"--")},
                {Pattern(L"+="), defaultHandler(PLUS_EQUALS, L"+=")},
                {Pattern(L"-="), defaultHandler(MINUS_EQUALS, L"-=")},
                {Pattern(L"/="), defaultHandler(SLASH_EQUALS, L"/=")},
                {Pattern(L"*="), defaultHandler(STAR_EQUALS, L"*=")},
                {Pattern(L"%="), defaultHandler(PERCENT_EQUALS, L"%=")},
                {Pattern(L"+"), defaultHandler(PLUS, L"+")},
                {Pattern(L"-"), defaultHandler(DASH, L"-")},
                {Pattern(L"/"), defaultHandler(SLASH, L"/")},
                {Pattern(L"*"), defaultHandler(STAR, L"*")},
                {Pattern(L"%"), defaultHandler(PERCENT, L"%")},
                {Pattern(L"|="), defaultHandler(BITWISE_OR_EQUALS, L"|=")},
                {Pattern(L"|"), defaultHandler(BITWISE_OR, L"|")},
                {Pattern(L"&="), defaultHandler(BITWISE_AND_EQUALS, L"&=")},
                {Pattern(L"&"), defaultHandler(BITWISE_AND, L"&")},
                {Pattern(L"^="), defaultHandler(BITWISE_XOR_EQUALS, L"^=")},
                {Pattern(L"^"), defaultHandler(BITWISE_XOR, L"^")},
                {Pattern(L"~="), defaultHandler(BITWISE_NOT_EQUALS, L"~=")},
                {Pattern(L"~"), defaultHandler(BITWISE_NOT, L"~")},
            },
            vector<Error>(0),
            1, // line
            1  // column
        };
    }
}