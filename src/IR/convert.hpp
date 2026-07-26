#pragma once

namespace IR {
    Node* ConvertASTToIR(const ast::Stmt* stmt) {
        if (stmt == nullptr) return nullptr;
        if (auto blockStmt = dynamic_cast<const ast::BlockStmt*>(stmt)) {
            Node* head = nullptr;
            Node* tail = nullptr;
            size_t scope = Globals.GetNewScope();
            for (const auto& s : blockStmt->statements) {
                Node* currentNode = ConvertASTToIR(s);
                if (currentNode) {
                    if (!head) {
                        head = currentNode->getFirst();
                        tail = currentNode->getLast();
                    } else {
                        tail->next = currentNode->getFirst();
                        currentNode->getFirst()->prev = tail;
                        tail = currentNode->getLast();
                    }
                }
            }
            Node* setScopeNode = new Node(Instruction(Opcode::SET_SCOPE, vector<string>({ to_string(scope) })));
            size_t newScope = Globals.RemoveScope();
            Node* destroyScopeNode = new Node(Instruction(Opcode::SET_SCOPE, vector<string>({ to_string(newScope) })));
            setScopeNode->next = head;
            if (head) head->prev = setScopeNode;
            Node* lastNode = setScopeNode->getLast();
            lastNode->next = destroyScopeNode;
            destroyScopeNode->prev = lastNode;
            if (OptimizationOptions.IR.optimizeLabels) {
                Optimizer::OptimizeLabels(head);
            }
            if (OptimizationOptions.IR.automaticVariableDeletion) {
                Optimizer::AutoFreeVariables(setScopeNode);
            }
            return setScopeNode;
        } else if (auto ifStmt = dynamic_cast<const ast::IfStmt*>(stmt)) {
            return ConvertIfToIR(ifStmt);
        } else if (auto varDeclStmt = dynamic_cast<const ast::VarDeclStmt*>(stmt)) {
            const string varName = to_string(varDeclStmt->VariableName);
            const string varType = to_string(varDeclStmt->ExplicitType->GetName());
            const bool isliteral = varDeclStmt->AssignedValue && ast::optimizer::isLiteral(varDeclStmt->AssignedValue);
            Node* creation = new Node(Instruction(Opcode::CREATE_VAR, vector<string>({ varName, varType })));
            if (isliteral) {
                string value = to_string(ast::decompiler::DecompileExpression(varDeclStmt->AssignedValue));
                Node* storeValue = new Node(Instruction(Opcode::STORE_VAR, vector<string>({ varName, value })));
                creation->next = storeValue;
                storeValue->prev = creation;
            }
            return creation;
        } else if (auto whileStmt = dynamic_cast<const ast::WhileStmt*>(stmt)) {
            return ConvertWhileToIR(whileStmt);
        } else if (auto exprStmt = dynamic_cast<const ast::ExpressionStmt*>(stmt)) {
            return ConvertASTToIR(exprStmt->expression);
        }

        return new Node(Instruction(Opcode::NOP)); // Placeholder for unhandled statements
    }
    Node* ConvertASTToIR(const ast::Expr* expr) {
        if (expr == nullptr) return nullptr;
        if (auto assignmentExpr = dynamic_cast<const ast::AssignmentExpr*>(expr)) {
            return ConvertAssignmentToIR(assignmentExpr);
        }

        return new Node(Instruction(Opcode::NOP)); // Placeholder for unhandled expressions
    }
    // Adds the SOF and EOF nodes to the IR sequence generated from the AST
    Node* MConvert(const ast::Stmt* stmt) {
        Node* sof = new Node(Instruction(Opcode::SOF));
        Node* ir = ConvertASTToIR(stmt);
        Node* eof = new Node(Instruction(Opcode::EOF));
        sof->next = ir;
        if (ir) ir->prev = sof;
        if (ir) {
            Node* lastNode = ir->getLast();
            lastNode->next = eof;
            eof->prev = lastNode;
        } else {
            sof->next = eof;
            eof->prev = sof;
        }
        return sof;
    }
}