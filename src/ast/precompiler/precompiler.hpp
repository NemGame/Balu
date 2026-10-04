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
        uint64_t currentScope = 0;
        constexpr static size_t DEFAULT_DECL_INDEX = static_cast<size_t>(-1);
        VariableInfo& AddVariable(VarDeclStmt* varDecl, size_t declarationIndex = DEFAULT_DECL_INDEX) {
            VariableInfo varInfo;
            if (declarationIndex == DEFAULT_DECL_INDEX) {
                declarationIndex = size();
            }
            varInfo.variableName = varDecl->VariableName;
            varInfo.declaration = varDecl;
            varInfo.declarationIndex = declarationIndex;
            varInfo.type = varDecl->ExplicitType;
            varInfo.canBeDestroyed = varDecl->mayAutoDelete;
            varInfo.scope = currentScope;
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
        void UsedVariable(const wstring& variableName, Stmt* usageStmt) {
            for (auto& varInfo : *this) {
                if (varInfo.declaration && varInfo.variableName == variableName) {
                    varInfo.usage.push_back(usageStmt);
                    break;
                }
            }
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
        uint64_t AddScope() {
            return ++currentScope;
        }
        vector<VariableInfo*> GetVariablesInScope(size_t scope) {
            vector<VariableInfo*> varsInScope;
            for (auto& varInfo : *this) {
                if (varInfo.scope == scope) {
                    varsInScope.push_back(&varInfo);
                }
            }
            return varsInScope;
        }
        void RemoveScope(size_t scope, bool updateCurrentScope = true) {
            if (updateCurrentScope && currentScope == scope) {
                currentScope--;
            }
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
    };
    void PreCompile(Stmt* stmt, VariableRegistry& varReg);
    void PreCompile(Expr* expr, VariableRegistry& varReg, Stmt* stmt);

    void PreCompileBlockStmt(BlockStmt* blockStmt, VariableRegistry& varReg) {
        uint64_t scope = varReg.AddScope();

        for (auto& stmt : blockStmt->statements) {
            if (stmt != nullptr) PreCompile(stmt, varReg);
        }
        varReg.AdjustDeclarationIndices(blockStmt, 0);
        vector<VariableInfo*> remainingVars = varReg.GetVariablesInScope(scope);

        if (remainingVars.size() > 0) {
            for (const auto& varInfo : remainingVars) {
                if (!varInfo->canBeDestroyed) {
                    if (CompilerOptions.Warnings.undeletableVariableRemains) {
                        _wcout << L"Warning: Undeletable variable remains: \"" << varInfo->variableName << L"\" in scope " << scope << endl;
                    }
                }
                if (varInfo->usage.empty()) {
                    size_t declarationIndex = blockStmt->IndexOf(varInfo->declaration);
                    if (declarationIndex == static_cast<size_t>(-1)) {
                        throw logic_error("Variable declaration is missing from its block.");
                    }
                    blockStmt->statements[declarationIndex] = nullptr;
                    delete varInfo->declaration;
                    varInfo->declaration = nullptr;
                    continue;
                }
                UnusedStmt* unusedStmt = new UnusedStmt(varInfo->variableName);
                size_t insertIndex = blockStmt->statements.size();
                if (!varInfo->usage.empty()) {
                    size_t lastUsageIndex = blockStmt->IndexOf(varInfo->usage.back());
                    if (lastUsageIndex != static_cast<size_t>(-1)) {
                        insertIndex = lastUsageIndex + 1;
                    }
                }

                blockStmt->statements.insert(blockStmt->statements.begin() + insertIndex, unusedStmt);
            }
        }
        blockStmt->Fix();
        varReg.RemoveScope(scope);
    }

    void PreCompile(Stmt* stmt) {
        VariableRegistry varReg;
        PreCompile(stmt, varReg);
    }
    void PreCompile(Stmt* stmt, VariableRegistry& varReg) {
        if (CompilerOptions.verbose) _wcout << L"[PreCompiler] Pre-compiling statement..." << endl;
        if (stmt == nullptr) return;
        if (auto blockStmt = dynamic_cast<BlockStmt*>(stmt)) {
            PreCompileBlockStmt(blockStmt, varReg);
        } else if (auto varDecl = dynamic_cast<VarDeclStmt*>(stmt)) {
            varReg.AddVariable(varDecl);
            PreCompile(varDecl->AssignedValue, varReg, varDecl);
        } else if (auto unusedStmt = dynamic_cast<UnusedStmt*>(stmt)) {
            bool doesExist = varReg.GetVariable(unusedStmt->variableIdentifier) != nullptr;
            if (!doesExist) {
                wstring errorMessage = L"Unknown identifier: \"" + unusedStmt->variableIdentifier + L"\"";
                _wcout << L"Error: " << errorMessage << endl;
                if (CompilerOptions.panic) {
                    throw runtime_error(string(errorMessage.begin(), errorMessage.end()));
                }
                delete unusedStmt;
            }
            varReg.RemoveVariable(unusedStmt->variableIdentifier);
        } else if (auto exprStmt = dynamic_cast<ExpressionStmt*>(stmt)) {
            PreCompile(exprStmt->expression, varReg, stmt);
        } else if (auto typeChangeStmt = dynamic_cast<TypeChangeStmt*>(stmt)) {
            PreCompile(typeChangeStmt->NewExpr, varReg, stmt);
        } else if (auto aliasStmt = dynamic_cast<AliasDeclStmt*>(stmt)) {
            PreCompile(aliasStmt->AliasedValue, varReg, stmt);
        } else if (auto ifStmt = dynamic_cast<IfStmt*>(stmt)) {
            PreCompile(ifStmt->Condition, varReg, stmt);
            PreCompile(ifStmt->ThenBranch, varReg);
            PreCompile(ifStmt->ElseBranch, varReg);
        } else if (auto whileStmt = dynamic_cast<WhileStmt*>(stmt)) {
            PreCompile(whileStmt->Condition, varReg, stmt);
            PreCompile(whileStmt->Body, varReg);
            PreCompile(whileStmt->ElseBranch, varReg);
        } else if (auto funcDeclStmt = dynamic_cast<FuncDeclStmt*>(stmt)) {
            PreCompile(funcDeclStmt->Body, varReg);
        }
    }
    void PreCompile(Expr* expr, VariableRegistry& varReg, Stmt* stmt) {
        if (expr == nullptr) return;

        if (auto assignmentExpr = dynamic_cast<AssignmentExpr*>(expr)) {
            PreCompile(assignmentExpr->Assignee, varReg, stmt);
            PreCompile(assignmentExpr->Value, varReg, stmt);
        } else if (auto identifierExpr = dynamic_cast<IdentifierExpr*>(expr)) {
            varReg.UsedVariable(identifierExpr->value, stmt);
            wcout << L"Used variable: " << identifierExpr->value << endl;
        } else if (auto binaryExpr = dynamic_cast<BinaryExpr*>(expr)) {
            PreCompile(binaryExpr->left, varReg, stmt);
            PreCompile(binaryExpr->right, varReg, stmt);
        } else if (auto unaryExpr = dynamic_cast<UnaryExpr*>(expr)) {
            PreCompile(unaryExpr->RightExpr, varReg, stmt);
        } else if (auto returnExpr = dynamic_cast<ReturnExpr*>(expr)) {
            PreCompile(returnExpr->Value, varReg, stmt);
        }
    }
}