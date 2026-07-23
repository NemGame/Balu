#pragma once

namespace parser {
    map<lexer::TokenKind, wstring> CorrectSyntax = {
        {lexer::ALIAS, L"alias <aliasName> = <Expression/Variable/Type/Alias>;"},
    };
    wstring GetCorrectSyntax(lexer::TokenKind kind) {
        if (CompilerOptions.provideHelp && CorrectSyntax.find(kind) != CorrectSyntax.end()) {
            return L"\n=> Correct syntax: " + CorrectSyntax[kind];
        }
        return L"";
    }
}