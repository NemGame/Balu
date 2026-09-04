#pragma once

namespace ast::precompiler {

    struct InlineVariableInfo {
        Stmt* declaration = nullptr;
        size_t declarationIndex = 0;
        vector<Stmt*> usage = {};
        bool canInline = false;
    };
    struct DestroyVariableInfo {
        bool canDestroy = false;
    };
    unordered_map<wstring, size_t> GetIdentifiers(Expr* expr, unordered_map<wstring, size_t>& identifiers) {
        if (auto identifierExpr = dynamic_cast<IdentifierExpr*>(expr)) {
            identifiers[identifierExpr->value]++;
        } else if (auto binaryExpr = dynamic_cast<BinaryExpr*>(expr)) {
            GetIdentifiers(binaryExpr->left, identifiers);
            GetIdentifiers(binaryExpr->right, identifiers);
        }
        else if (auto unaryExpr = dynamic_cast<UnaryExpr*>(expr)) {
            GetIdentifiers(unaryExpr->RightExpr, identifiers);
        }
        return identifiers;
    }
    unordered_map<wstring, size_t> GetIdentifiers(Stmt* stmt, unordered_map<wstring, size_t>& identifiers) {
        if (auto varDecl = dynamic_cast<VarDeclStmt*>(stmt)) {
            unordered_map<wstring, size_t> initIdentifiers;
            GetIdentifiers(varDecl->AssignedValue, initIdentifiers);
            for (const auto& [key, value] : initIdentifiers) {
                identifiers[key] += value;
            }
        } else if (auto unusedStmt = dynamic_cast<UnusedStmt*>(stmt)) {
            identifiers[unusedStmt->variableIdentifier]++;
        } else if (auto exprStmt = dynamic_cast<ExpressionStmt*>(stmt)) {
            unordered_map<wstring, size_t> exprIdentifiers;
            GetIdentifiers(exprStmt->expression, exprIdentifiers);
            for (const auto& [key, value] : exprIdentifiers) {
                identifiers[key] += value;
            }
        }
        return identifiers;
    }
    void AdjustDeclarationIndices(unordered_map<wstring, InlineVariableInfo>& inlineVariableMap, size_t startIndex, int difference) {
        for (auto& [variableName, info] : inlineVariableMap) {
            if (info.declarationIndex >= startIndex) {
                info.declarationIndex += difference;
            }
        }
    }
    void InlineVariables(BlockStmt* blockStmt) {
        if (CompilerOptions.verbose) _wcout << L"[PreCompiler] Inlining variables in block statement..." << endl;
        unordered_map<wstring, InlineVariableInfo> inlineVariableMap;

        unordered_map<wstring, bool> destroyVariableList;

        unordered_map<wstring, size_t> exprIdentifiers;
        GetIdentifiers(static_cast<Stmt*>(blockStmt), exprIdentifiers);
        for (size_t i = 0; i < blockStmt->statements.size(); i++) {
            Stmt* stmt = blockStmt->statements[i];
            if (auto varDecl = dynamic_cast<VarDeclStmt*>(stmt)) {
                const bool isUsed = exprIdentifiers.find(varDecl->VariableName) != exprIdentifiers.end();
                if (inlineVariableMap.find(varDecl->VariableName) != inlineVariableMap.end() || !isUsed) {
                    if (!CompilerOptions.IR.allowRedeclaration && isUsed) {
                        const wstring message = L"Variable " + varDecl->VariableName + L" is already declared.";
                        _wcout << message << endl;
                        if (CompilerOptions.panic) {
                            throw runtime_error(string(message.begin(), message.end()));
                        }
                    }
                    // TODO: Make it look good
                    if (!isUsed) {
                        blockStmt->statements.erase(blockStmt->statements.begin() + i);
                        delete varDecl;
                        AdjustDeclarationIndices(inlineVariableMap, i, -1);
                        i--;
                    }
                    if (inlineVariableMap[varDecl->VariableName].usage.empty()) {
                        _wcout << L"Destroying variable " << varDecl->VariableName << L" as it was never used" << endl;
                        blockStmt->statements.erase(blockStmt->statements.begin() + inlineVariableMap[varDecl->VariableName].declarationIndex);
                        delete inlineVariableMap[varDecl->VariableName].declaration;
                        inlineVariableMap.erase(varDecl->VariableName);
                        AdjustDeclarationIndices(inlineVariableMap, i, -1);
                        i--;
                    } else {
                        UnusedStmt* unusedStmt = new UnusedStmt(varDecl);
                        blockStmt->statements.insert(blockStmt->statements.begin() + i, unusedStmt);
                        i++;
                    }
                }
                inlineVariableMap[varDecl->VariableName] = InlineVariableInfo{varDecl, i, {}, ast::optimizer::isLiteral(varDecl->AssignedValue)};
            } else if (auto unusedStmt = dynamic_cast<UnusedStmt*>(stmt)) {
                if (destroyVariableList.find(unusedStmt->variableIdentifier) == destroyVariableList.end()) {
                    destroyVariableList.erase(unusedStmt->variableIdentifier);
                }
            } else if (auto exprStmt = dynamic_cast<ExpressionStmt*>(stmt)) {
                
            }
        }

        for (auto& [variableName, canDestroy] : destroyVariableList) {
            if (canDestroy) {
                UnusedStmt* unusedStmt = new UnusedStmt(variableName);
                blockStmt->statements.push_back(unusedStmt);
                continue;
            }
            const wstring message = L"Variable " + variableName + L" was never destroyed.";
            _wcout << message << endl;
            if (CompilerOptions.panic) {
                throw runtime_error(string(message.begin(), message.end()));
            }
        }
    }

    void PreCompile(Stmt* stmt) {
        if (CompilerOptions.verbose) _wcout << L"[PreCompiler] Pre-compiling statement..." << endl;
        if (auto blockStmt = dynamic_cast<BlockStmt*>(stmt)) {
            InlineVariables(blockStmt);
        }
    }
}