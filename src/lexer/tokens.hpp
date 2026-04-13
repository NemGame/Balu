#pragma once

namespace lexer {    
    enum TokenKind {
        EOF_TOKEN = 0,
        NUMBER,       // long double
        STRING,       // e.g., "hello", "world", etc.
        CHAR,         // e.g., 'a', 'b', etc.
        BOOL,         // e.g., true, false
        BYTE,         // 0-255
        ANY,          // Any type
        AUTO,         // Type inferred from context
        VOID,         // Void type
        NULL_TYPE,    // Null type

        IDENTIFIER,    // e.g., variable names, function names, etc.
        RULE,          // #rule
        FSTRING,       // Format string, e.g., $"Hello {name}"

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

        ARROW,          // =>

        LESS,          // <
        LESS_EQUALS,   // <=
        GREATER,       // >
        GREATER_EQUALS, // >=

        OR,            // ||
        AND,           // &&

        BITWISE_OR,    // |
        BITWISE_AND,   // &
        BITWISE_XOR,   // ^
        BITWISE_NOT,   // ~
        BITWISE_LEFT,  // <<
        BITWISE_RIGHT, // >>

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

        /// Reserved keywords
        // Variable declaration and modifiers
        LET,            // let
        CONST,          // const
        MUT,            // mut
        CLASS,          // class
        STRUCT,         // struct
        ALIAS,          // alias
        NEW,            // new
        FN,             // fn
        STATIC,         // static
        GET,            // get
        SET,            // set

        // Modules and imports
        IMPORT,         // import
        FROM,           // from
        EXPORT,         // export

        // Control flow
        IF,             // if
        ELSE,           // else
        ELIF,           // elif
        FOREACH,        // foreach
        WHILE,          // while
        FOR,            // for
        RETURN,         // return

        // Reflection and evaluation
        TYPEOF,         // typeof
        NAMEOF,         // nameof
        IN,             // in
        OUT,            // out
        EVAL,           // eval
    };
    vector<TokenKind> TokenTypes = {
        NUMBER, STRING, CHAR, BOOL, BYTE, ANY, AUTO, VOID, NULL_TYPE
    };

    wstring TokenKindString(TokenKind kind);

    map<wstring, TokenKind> reserved_lu = {
        {L"let", LET},
        {L"const", CONST},
        {L"mut", MUT},
        {L"class", CLASS},
        {L"struct", STRUCT},
        {L"alias", ALIAS},
        {L"new", NEW},
        {L"fn", FN},
        {L"static", STATIC},
        {L"get", GET},
        {L"set", SET},

        {L"import", IMPORT},
        {L"from", FROM},
        {L"export", EXPORT},

        {L"if", IF},
        {L"else", ELSE},
        {L"elif", ELIF},
        {L"foreach", FOREACH},
        {L"while", WHILE},
        {L"for", FOR},
        
        {L"typeof", TYPEOF},
        {L"nameof", NAMEOF},
        {L"in", IN},
        {L"out", OUT},
        {L"eval", EVAL},
        {L"return", RETURN},
        
        {L"string", STRING},
        {L"char", CHAR},
        {L"number", NUMBER},
        {L"byte", BYTE},
        {L"bool", BOOL},
        {L"auto", AUTO},
        {L"any", ANY},
        {L"void", VOID},
        {L"null", NULL_TYPE},

        {L"#rule", RULE},
    };
    
    struct Token {
        TokenKind kind;
        wstring value;
        unsigned long long line;
        unsigned long long column;
        void Debug(wostream& wcout_ = _wcout, bool endWithNewLine = true) const { // Change ostream to wostream, cout to wcout
            wcout_ << L"[" << line << L":" << column << L"] ";
            if (isOneOfMany(NUMBER, STRING, CHAR, BOOL, BYTE, ANY, AUTO, IDENTIFIER, RULE, FSTRING)) {
                wcout_ << TokenKindString(kind) << L" (" << value << L")";
            } else {
                wcout_ << TokenKindString(kind) << L" ()";
            }
            if (endWithNewLine) wcout_ << endl;
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
        bool isType() const {
            for (TokenKind tokenType : TokenTypes) {
                if (this->kind == tokenType) return true;
            }
            return false;
        }
        bool mightBeType() const {
            return isType() || kind == IDENTIFIER;
        }
    };

    Token NewToken(TokenKind kind, const wstring& value = L"", unsigned long long line = 0, unsigned long long column = 0) {
        return Token{kind, value, line, column};
    }

    wstring TokenKindString(TokenKind kind) {
        switch (kind) {
            case EOF_TOKEN: return L"EOF";
            case NUMBER: return L"NUMBER";
            case STRING: return L"STRING";
            case CHAR: return L"CHAR";
            case BOOL: return L"BOOL";
            case BYTE: return L"BYTE";
            case ANY: return L"ANY";
            case AUTO: return L"AUTO";
            case VOID: return L"VOID";
            case NULL_TYPE: return L"NULL_TYPE";

            case IDENTIFIER: return L"IDENTIFIER";
            case RULE: return L"RULE";
            case FSTRING: return L"FSTRING";

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

            case ARROW: return L"ARROW";

            case LESS: return L"LESS";
            case LESS_EQUALS: return L"LESS_EQUALS";
            case GREATER: return L"GREATER";
            case GREATER_EQUALS: return L"GREATER_EQUALS";

            case OR: return L"OR";
            case AND: return L"AND";

            case BITWISE_OR: return L"BITWISE_OR";
            case BITWISE_AND: return L"BITWISE_AND";
            case BITWISE_XOR: return L"BITWISE_XOR";
            case BITWISE_NOT: return L"BITWISE_NOT";
            case BITWISE_LEFT: return L"BITWISE_LEFT";
            case BITWISE_RIGHT: return L"BITWISE_RIGHT";
            
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
            case LET: return L"LET";
            case CONST: return L"CONST";
            case MUT: return L"MUT";
            case CLASS: return L"CLASS";
            case ALIAS: return L"ALIAS";
            case STRUCT: return L"STRUCT";
            case STATIC: return L"STATIC";
            case GET: return L"GET";
            case SET: return L"SET";
            case NEW: return L"NEW";
            case FN: return L"FN";

            case IMPORT: return L"IMPORT";
            case FROM: return L"FROM";
            case EXPORT: return L"EXPORT";

            case IF: return L"IF";
            case ELSE: return L"ELSE";
            case ELIF: return L"ELIF";
            case FOREACH: return L"FOREACH";
            case WHILE: return L"WHILE";
            case FOR: return L"FOR";
            case RETURN: return L"RETURN";

            case TYPEOF: return L"TYPEOF";
            case NAMEOF: return L"NAMEOF";
            case IN: return L"IN";
            case OUT: return L"OUT";
            case EVAL: return L"EVAL";
            default: return L"UNKNOWN";
        }
    }
}