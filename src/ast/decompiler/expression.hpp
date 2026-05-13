#pragma once

namespace ast::decompiler {
    wstring DecompileStatement(ast::Stmt* stmt, int indent = 0);
    wstring DecompileExpression(ast::Expr* expr, int indent = 0);
    wstring vectorToWstring(const vector<wstring>& vec, wchar_t delimiter=L',') {
        wstring result;
        for (size_t i = 0; i < vec.size(); ++i) {
            result += vec[i];
            if (i < vec.size() - 1) {
                result += delimiter;
            }
        }
        return result;
    }
    wstring DecompileBlockStmt(ast::BlockStmt* block, int indent = 0) {
        wstring result = wstring(indent * 4, L' ') + L"{\n";
        for (ast::Stmt* stmt : block->statements) {
            result += DecompileStatement(stmt, indent + 1) + L";\n";
        }
        result += wstring(indent * 4, L' ') + L"}";
        return result;
    }
    wstring DecompileExpressionStmt(ast::ExpressionStmt* stmt, int indent = 0) {
        return wstring(indent * 4, L' ') + DecompileExpression(stmt->expression, indent);
    }
    wstring DecompileVarDeclStmt(ast::VarDeclStmt* stmt, int indent = 0) {
        wstring result = wstring(indent * 4, L' ') + stmt->VariableName;
        if (!stmt->isConstant) {
            result = L"let " + result;
        } else {
            result = L"const " + result;
        }
        result += stmt->ExplicitType ? L": " + stmt->ExplicitType->GetName() : L"";
        if (stmt->AssignedValue) {
            result += L" = " + DecompileExpression(stmt->AssignedValue, indent);
        }
        return result;
    }
    wstring DecompileTypeChangeStmt(ast::TypeChangeStmt* stmt, int indent = 0) {
        return wstring(indent * 4, L' ') + L"typeof " + stmt->VariableName + L" = " + DecompileExpression(stmt->NewExpr, indent);
    }
    wstring DecompileIfStmt(ast::IfStmt* stmt, int indent = 0) {
        wstring result = wstring(indent * 4, L' ') + L"if (" + DecompileExpression(stmt->Condition, indent) + L") " + DecompileStatement(stmt->ThenBranch, indent);
        if (stmt->ElseBranch) {
            result += L" else " + DecompileStatement(stmt->ElseBranch, indent);
        }
        return result;
    }
    wstring DecompileExpression(ast::Expr* expr, int indent) {
        if (auto s = dynamic_cast<ast::BinaryExpr*>(expr)) {
            wstring left = DecompileExpression(s->left, indent);
            wstring right = DecompileExpression(s->right, indent);
            return L'(' + left + L" " + s->op.value + L" " + right + L')';
        } else if (auto s = dynamic_cast<ast::NumberExpr*>(expr)) {
            if (s->isPrecise()) {
                return s->preciseValue + L"@";
            }
            return to_wstring(s->value);
        } else if (auto s = dynamic_cast<ast::RuleExpr*>(expr)) {
            return L"#" + vectorToWstring(s->value, L' ');
        } else if (auto s = dynamic_cast<ast::IdentifierExpr*>(expr)) {
            return wstring(indent * 4, L' ') + s->value;
        } else if (auto s = dynamic_cast<ast::StringExpr*>(expr)) {
            return wstring(indent * 4, L' ') + L"\"" + s->value + L"\"";
        } else if (auto s = dynamic_cast<ast::CharExpr*>(expr)) {
            return wstring(indent * 4, L' ') + L"'" + wstring(1, s->value) + L"'";
        } else if (auto s = dynamic_cast<ast::ByteExpr*>(expr)) {
            return wstring(indent * 4, L' ') + to_wstring(s->value) + L"b";
        } else if (auto s = dynamic_cast<ast::BooleanExpr*>(expr)) {
            return wstring(indent * 4, L' ') + (s->value ? L"true" : L"false");
        } else if (auto s = dynamic_cast<ast::NullExpr*>(expr)) {
            return wstring(indent * 4, L' ') + L"null";
        } else if (auto s = dynamic_cast<ast::PrefixExpr*>(expr)) {
            return wstring(indent * 4, L' ') + s->Operator.value + DecompileExpression(s->RightExpr, indent);
        } else if (auto s = dynamic_cast<ast::TypeExpr*>(expr)) {
            return s->value->GetName();
        } else {
            return L"<unknown expr>";
        }
    }
    wstring DecompileStatement(ast::Stmt* stmt, int indent) {
        if (auto s = dynamic_cast<ast::BlockStmt*>(stmt)) {
            return DecompileBlockStmt(s, indent);
        } else if (auto s = dynamic_cast<ast::ExpressionStmt*>(stmt)) {
            return DecompileExpressionStmt(s, indent);
        } else if (auto s = dynamic_cast<ast::VarDeclStmt*>(stmt)) {
            return DecompileVarDeclStmt(s, indent);
        } else if (auto s = dynamic_cast<ast::TypeChangeStmt*>(stmt)) {
            return DecompileTypeChangeStmt(s, indent);
        } else if (auto s = dynamic_cast<ast::IfStmt*>(stmt)) {
            return DecompileIfStmt(s, indent);
        } else {
            return L"<unknown stmt>";
        }
    }
}