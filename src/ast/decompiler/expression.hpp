#pragma once

namespace ast::decompiler {
    wstring DecompileStatement(ast::Stmt* stmt);
    wstring DecompileExpression(ast::Expr* expr);
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
    wstring DecompileBlockStmt(ast::BlockStmt* block) {
        wstring result = L"{\n";
        for (ast::Stmt* stmt : block->statements) {
            result += DecompileStatement(stmt) + L"\n";
        }
        result += L"}";
        return result;
    }
    wstring DecompileExpressionStmt(ast::ExpressionStmt* stmt) {
        return DecompileExpression(stmt->expression);
    }
    wstring DecompileVarDeclStmt(ast::VarDeclStmt* stmt) {
        wstring result = stmt->VariableName;
        if (!stmt->isConstant) {
            result = L"let " + result;
        } else {
            result = L"const " + result;
        }
        result += stmt->ExplicitType ? L": " + stmt->ExplicitType->GetName() : L"";
        if (stmt->AssignedValue) {
            result += L" = " + DecompileExpression(stmt->AssignedValue);
        }
        return result;
    }
    wstring DecompileTypeChangeStmt(ast::TypeChangeStmt* stmt) {
        return L"typeof " + stmt->VariableName + L" = " + DecompileExpression(stmt->NewExpr);
    }
    wstring DecompileExpression(ast::Expr* expr) {
        if (auto s = dynamic_cast<ast::BinaryExpr*>(expr)) {
            wstring left = DecompileExpression(s->left);
            wstring right = DecompileExpression(s->right);
            return L'(' + left + L" " + s->op.value + L" " + right + L')';
        } else if (auto s = dynamic_cast<ast::NumberExpr*>(expr)) {
            if (s->isPrecise()) {
                return s->preciseValue + L"@";
            }
            return to_wstring(s->value);
        } else if (auto s = dynamic_cast<ast::RuleExpr*>(expr)) {
            return L"#" + vectorToWstring(s->value, L' ');
        } else if (auto s = dynamic_cast<ast::IdentifierExpr*>(expr)) {
            return s->value;
        } else if (auto s = dynamic_cast<ast::StringExpr*>(expr)) {
            return L"\"" + s->value + L"\"";
        } else if (auto s = dynamic_cast<ast::CharExpr*>(expr)) {
            return L"'" + wstring(1, s->value) + L"'";
        } else if (auto s = dynamic_cast<ast::ByteExpr*>(expr)) {
            return to_wstring(s->value) + L"b";
        } else if (auto s = dynamic_cast<ast::BooleanExpr*>(expr)) {
            return s->value ? L"true" : L"false";
        } else if (auto s = dynamic_cast<ast::NullExpr*>(expr)) {
            return L"null";
        } else if (auto s = dynamic_cast<ast::PrefixExpr*>(expr)) {
            return s->Operator.value + DecompileExpression(s->RightExpr);
        } else {
            return L"<unknown expr>";
        }
    }
    wstring DecompileStatement(ast::Stmt* stmt) {
        if (auto s = dynamic_cast<ast::BlockStmt*>(stmt)) {
            return DecompileBlockStmt(s);
        } else if (auto s = dynamic_cast<ast::ExpressionStmt*>(stmt)) {
            return DecompileExpressionStmt(s);
        } else if (auto s = dynamic_cast<ast::VarDeclStmt*>(stmt)) {
            return DecompileVarDeclStmt(s);
        } else if (auto s = dynamic_cast<ast::TypeChangeStmt*>(stmt)) {
            return DecompileTypeChangeStmt(s);
        } else {
            return L"<unknown stmt>";
        }
    }
}