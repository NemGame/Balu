#pragma once

namespace ast::precompiler {
    struct VariableInfo {
        wstring variableName = L"";
        Stmt* declaration = nullptr;
        size_t declarationIndex = 0;
        vector<Stmt*> usage = {};
        size_t scope = 0;
        Type* type = nullptr;
        bool canInline = false;
        bool canBeDestroyed = false;
    };
    struct VariableRegistry : vector<VariableInfo> {
        VariableInfo& AddVariable(VarDeclStmt* varDecl, size_t declarationIndex) {
            VariableInfo varInfo;
            varInfo.variableName = varDecl->VariableName;
            varInfo.declaration = varDecl;
            varInfo.declarationIndex = declarationIndex;
            varInfo.type = varDecl->ExplicitType;
            varInfo.canBeDestroyed = varDecl->mayAutoDelete;
            push_back(varInfo);
            return back();
        }
        bool HasVariable(VarDeclStmt* varDecl) {
            for (const auto& varInfo : *this) {
                if (varInfo.declaration == varDecl) {
                    return true;
                }
            }
            return false;
        }
        bool HasVariable(const wstring& variableName) {
            for (const VariableInfo& varInfo : *this) {
                if (varInfo.declaration && varInfo.variableName == variableName) {
                    return true;
                }
            }
            return false;
        }
        VariableInfo* GetVariable(const wstring& variableName) {
            for (auto& varInfo : *this) {
                if (varInfo.declaration && varInfo.variableName == variableName) {
                    return &varInfo;
                }
            }
            return nullptr;
        }
        void RemoveVariable(VarDeclStmt* varDecl) {
            this->erase(std::remove_if(this->begin(), this->end(),
                [varDecl](const VariableInfo& varInfo) { return varInfo.declaration == varDecl; }),
                this->end());
        }
        void RemoveVariable(const wstring& variableName) {
            this->erase(std::remove_if(this->begin(), this->end(),
                [variableName](const VariableInfo& varInfo) { return varInfo.variableName == variableName; }),
                this->end());
        }
        void fix() {
            this->erase(std::remove_if(this->begin(), this->end(),
                [](const VariableInfo& varInfo) { return varInfo.declaration == nullptr; }),
                this->end());
        }
        void RemoveScope(size_t scope) {
            this->erase(std::remove_if(this->begin(), this->end(),
                [scope](const VariableInfo& varInfo) { return varInfo.scope == scope; }),
                this->end());
        }
        void AdjustDeclarationIndices(BlockStmt* blockStmt, size_t startIndex) {
            for (size_t i = startIndex; i < blockStmt->statements.size(); i++) {
                if (auto varDecl = dynamic_cast<VarDeclStmt*>(blockStmt->statements[i])) {
                    if (VariableInfo* varInfo = GetVariable(varDecl->VariableName)) {
                        varInfo->declarationIndex = i;
                    }
                }
            }
        }
        void UsedIdentifier(const wstring& variableName, Stmt* stmt) {
            if (VariableInfo* varInfo = GetVariable(variableName)) {
                varInfo->usage.push_back(stmt);
            }
        }
    };
    void InlineVariables(Expr* expr, Stmt* exprStmt, VariableRegistry& variableInfos) {
        if (auto binaryExpr = dynamic_cast<BinaryExpr*>(expr)) {
            auto leftIdentifier = dynamic_cast<IdentifierExpr*>(binaryExpr->left);
            if (leftIdentifier) {
                variableInfos.UsedIdentifier(leftIdentifier->value, exprStmt);
            }
            auto rightIdentifier = dynamic_cast<IdentifierExpr*>(binaryExpr->right);
            if (rightIdentifier) {
                variableInfos.UsedIdentifier(rightIdentifier->value, exprStmt);
            }
        }
    }
    void InlineVariables(BlockStmt* blockStmt, size_t currentScope, VariableRegistry& variableInfos) {
        if (CompilerOptions.verbose) _wcout << L"[PreCompiler] Inlining variables in block statement..." << endl;

        for (size_t i = 0; i < blockStmt->statements.size(); i++) {
            Stmt* stmt = blockStmt->statements[i];

            if (auto varDecl = dynamic_cast<VarDeclStmt*>(stmt)) {
                if (variableInfos.HasVariable(varDecl->VariableName)) {
                    if (VariableInfo* existingVarInfo = variableInfos.GetVariable(varDecl->VariableName)) {
                        if (existingVarInfo->usage.size() == 0) {
                            blockStmt->statements.erase(blockStmt->statements.begin() + existingVarInfo->declarationIndex);
                            variableInfos.RemoveVariable(varDecl->VariableName);
                            variableInfos.AdjustDeclarationIndices(blockStmt, existingVarInfo->declarationIndex);
                            i--;
                            goto firstDeclaration;
                        }
                        if (existingVarInfo->type->GetName() == varDecl->ExplicitType->GetName()) {
                            AssignmentExpr* assignmentExpr = new AssignmentExpr(new IdentifierExpr(existingVarInfo->variableName), lexer::NewToken(lexer::ASSIGNMENT, L"="), varDecl->AssignedValue->Clone());
                            delete varDecl;
                            blockStmt->statements[i] = new ExpressionStmt(assignmentExpr);
                            return;
                        }
                    }
                    UnusedStmt* unusedStmt = new UnusedStmt(varDecl->VariableName);
                    blockStmt->statements.insert(blockStmt->statements.begin() + i, unusedStmt);
                    variableInfos.RemoveVariable(varDecl->VariableName);
                    variableInfos.AdjustDeclarationIndices(blockStmt, i);
                    i++;
                }
        firstDeclaration:

                InlineVariables(varDecl->AssignedValue, varDecl, variableInfos);

                VariableInfo varInfo;
                varInfo.declaration = varDecl;
                varInfo.declarationIndex = i;
                varInfo.type = varDecl->ExplicitType;
                varInfo.canBeDestroyed = varDecl->mayAutoDelete;
                varInfo.scope = currentScope;
                varInfo.variableName = varDecl->VariableName;
                variableInfos.push_back(varInfo);
            } else if (auto _blockStmt = dynamic_cast<BlockStmt*>(stmt)) {
                InlineVariables(_blockStmt, currentScope + 1, variableInfos);
                variableInfos.RemoveScope(currentScope + 1);
            } else if (auto unusedStmt = dynamic_cast<UnusedStmt*>(stmt)) {
                variableInfos.RemoveVariable(unusedStmt->variableIdentifier);
            } else if (auto exprStmt = dynamic_cast<ExpressionStmt*>(stmt)) {
                InlineVariables(exprStmt->expression, exprStmt, variableInfos);
            }
        }
    }
    void InlineVariables(BlockStmt* blockStmt, size_t currentScope) {
        VariableRegistry variableInfos;
        InlineVariables(blockStmt, currentScope, variableInfos);
    }

    void PreCompile(Stmt* stmt) {
        if (CompilerOptions.verbose) _wcout << L"[PreCompiler] Pre-compiling statement..." << endl;
        if (auto blockStmt = dynamic_cast<BlockStmt*>(stmt)) {
            InlineVariables(blockStmt, 0);
        }
    }
}