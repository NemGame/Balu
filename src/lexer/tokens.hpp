#pragma once

#include <map>

namespace lexer {    
    enum TokenKind {
        EOF_TOKEN = 0,
        NUMBER,
        STRING,
        IDENTIFIER,

        OPEN_BRACKET,  // [
        CLOSE_BRACKET, // ]
        OPEN_CURLY,    // {
        CLOSE_CURLY,   // }
        OPEN_PAREN,    // (
        CLOSE_PAREN,   // )

        ASSIGNMENT,    // =
        EQUALS,        // ==
        NOT,           // !
        NOT_EQUALS,    // !=

        LESS,          // <
        LESS_EQUALS,   // <=
        GREATER,       // >
        GREATER_EQUALS, // >=

        OR,            // ||
        AND,           // &&

        DOT,           // .
        DOT_DOT,       // ..
        SEMICOLON,     // ;
        COLON,         // :
        COMMA,         // ,
        QUESTION,      // ?

        PLUS_PLUS,       // ++
        MINUS_MINUS,     // --
        PLUS_EQUALS,     // +=
        MINUS_EQUALS,    // -=
        SLASH_EQUALS,    // /=
        STAR_EQUALS,     // *=
        PERCENT_EQUALS,  // %=

        PLUS,            // +
        DASH,            // -
        SLASH,           // /
        STAR,            // *
        PERCENT,         // %

        // Reserved keywords
        MUT,            // mut
        CLASS,          // class
        NEW,            // new
        IMPORT,         // import
        FROM,           // from
        FN,             // fn
        IF,             // if
        ELSE,           // else
        FOREACH,        // foreach
        WHILE,          // while
        FOR,            // for
        EXPORT,         // export
        TYPEOF,         // typeof
        NAMEOF,         // nameof
        IN,             // in
        OUT,            // out
        EVAL,           // eval
        RETURN,         // return

        // Types
        STRING_TYPE,    // string
        CHAR_TYPE,      // char
        NUMBER_TYPE,    // number
        BOOL_TYPE,      // bool
        AUTO_TYPE,      // auto
        ANY_TYPE,        // any
        VOID_TYPE,       // void
    };

    wstring TokenKindString(TokenKind kind);

    map<wstring, TokenKind> reserved_lu = {
        {L"mut", MUT},
        {L"class", CLASS},
        {L"new", NEW},
        {L"import", IMPORT},
        {L"from", FROM},
        {L"fn", FN},
        {L"if", IF},
        {L"else", ELSE},
        {L"foreach", FOREACH},
        {L"while", WHILE},
        {L"for", FOR},
        {L"export", EXPORT},
        {L"typeof", TYPEOF},
        {L"nameof", NAMEOF},
        {L"in", IN},
        {L"out", OUT},
        {L"eval", EVAL},

        {L"string", STRING_TYPE},
        {L"char", CHAR_TYPE},
        {L"number", NUMBER_TYPE},
        {L"bool", BOOL_TYPE},
        {L"auto", AUTO_TYPE},
        {L"any", ANY_TYPE},
        {L"void", VOID_TYPE},
    };

    struct Token {
        TokenKind kind;
        wstring value;
        void Debug() const {
            if (isOneOfMany(NUMBER, STRING, IDENTIFIER)) {
                wcout << TokenKindString(kind) << L" (" << value << L")" << endl;
            } else {
                wcout << TokenKindString(kind) << L" ()" << endl;
            }
        }
        template<typename... TokenKinds>
        bool isOneOfMany(TokenKinds... tokenKinds) const {
            for (TokenKind kind : {tokenKinds...}) {
                if (this->kind == kind) {
                    return true;
                }
            }
            return false;
        }
    };

    Token NewToken(TokenKind kind, const wstring& value = L"") {
        return Token{kind, value};
    }

    wstring TokenKindString(TokenKind kind) {
        switch (kind) {
            case EOF_TOKEN: return L"EOF";
            case NUMBER: return L"NUMBER";
            case STRING: return L"STRING";
            case IDENTIFIER: return L"IDENTIFIER";
            case OPEN_BRACKET: return L"OPEN_BRACKET";
            case CLOSE_BRACKET: return L"CLOSE_BRACKET";
            case OPEN_CURLY: return L"OPEN_CURLY";
            case CLOSE_CURLY: return L"CLOSE_CURLY";
            case OPEN_PAREN: return L"OPEN_PAREN";
            case CLOSE_PAREN: return L"CLOSE_PAREN";
            case ASSIGNMENT: return L"ASSIGNMENT";
            case EQUALS: return L"EQUALS";
            case NOT: return L"NOT";
            case NOT_EQUALS: return L"NOT_EQUALS";
            case LESS: return L"LESS";
            case LESS_EQUALS: return L"LESS_EQUALS";
            case GREATER: return L"GREATER";
            case GREATER_EQUALS: return L"GREATER_EQUALS";
            case OR: return L"OR";
            case AND: return L"AND";
            case DOT: return L"DOT";
            case DOT_DOT: return L"DOT_DOT";
            case SEMICOLON: return L"SEMICOLON";
            case COLON: return L"COLON";
            case COMMA: return L"COMMA";
            case QUESTION: return L"QUESTION";
            case PLUS_PLUS: return L"PLUS_PLUS";
            case MINUS_MINUS: return L"MINUS_MINUS";
            case PLUS_EQUALS: return L"PLUS_EQUALS";
            case MINUS_EQUALS: return L"MINUS_EQUALS";
            case SLASH_EQUALS: return L"SLASH_EQUALS";
            case STAR_EQUALS: return L"STAR_EQUALS";
            case PERCENT_EQUALS: return L"PERCENT_EQUALS";
            case PLUS: return L"PLUS";
            case DASH: return L"DASH";
            case SLASH: return L"SLASH";
            case STAR: return L"STAR";
            case PERCENT: return L"PERCENT";

            // Reserved keywords
            case MUT: return L"MUT";
            case CLASS: return L"CLASS";
            case NEW: return L"NEW";
            case IMPORT: return L"IMPORT";
            case FROM: return L"FROM";
            case FN: return L"FN";
            case IF: return L"IF";
            case ELSE: return L"ELSE";
            case FOREACH: return L"FOREACH";
            case WHILE: return L"WHILE";
            case FOR: return L"FOR";
            case EXPORT: return L"EXPORT";
            case TYPEOF: return L"TYPEOF";
            case NAMEOF: return L"NAMEOF";
            case IN: return L"IN";
            case OUT: return L"OUT";
            case EVAL: return L"EVAL";
            case RETURN: return L"RETURN";

            // Types
            case STRING_TYPE: return L"STRING_TYPE";
            case CHAR_TYPE: return L"CHAR_TYPE";
            case NUMBER_TYPE: return L"NUMBER_TYPE";
            case BOOL_TYPE: return L"BOOL_TYPE";
            case AUTO_TYPE: return L"AUTO_TYPE";
            case ANY_TYPE: return L"ANY_TYPE";
            case VOID_TYPE: return L"VOID_TYPE";
            default: return L"UNKNOWN";
        }
    }
}