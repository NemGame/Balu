#pragma once

namespace ast::decompiler {
    wstring Decompile(const ast::BlockStmt& ast, const wstring& filename) {
        wstring result = L"// Decompiled from " + filename + L"\n";
        for (ast::Stmt* stmt : ast.statements) {
            result += DecompileStatement(stmt) + L";\n";
        }
        return result;
    }
}