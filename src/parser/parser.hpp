#pragma once

namespace parser {
    ast::Stmt* parse_stmt(Parser* p);
    
    // Returns a new parser instance with the given tokens (free manually)
    Parser* createParser(const vector<lexer::Token>& tokens) {
        createTokenLookups();
        createTokenTypeLookups();
        Parser* p = new Parser{ {}, tokens, 0 };
        lexer::Token current = p->currentToken();
        p->line = current.line;
        p->column = current.column;
        return p;
    }

    ast::BlockStmt Parse(const vector<lexer::Token>& tokens) {
        vector<ast::Stmt*> Body;
        
        unique_ptr<Parser> p(createParser(tokens));

        while (p->hasTokens())
        {
            Body.push_back(parse_stmt(p.get()));
        }
        

        return ast::BlockStmt{
            Body,
        };
    }
}