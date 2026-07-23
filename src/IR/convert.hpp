#pragma once

namespace IR {
    Node* ConvertASTToIR(const ast::Stmt* stmt) {
        if (stmt == nullptr) return nullptr;
        if (auto ifStmt = dynamic_cast<const ast::IfStmt*>(stmt)) {
            // `operands` contains the jump target difference (number of instructions to jump over)
            Node* thenNode = ConvertASTToIR(ifStmt->ThenBranch);
            Node* elseNode = ifStmt->ElseBranch ? ConvertASTToIR(ifStmt->ElseBranch) : nullptr;

            bool statementTrue = true; // Default assumption
            if (ifStmt->Condition) {
                if (auto prefixExpr = dynamic_cast<const ast::PrefixExpr*>(ifStmt->Condition)) {
                    if (prefixExpr->Operator.kind == lexer::TokenKind::NOT) statementTrue = false;
                }
            }

            // TODO: add comparison

            Node* jumpNode = new Node(Instruction(statementTrue ? Opcode::JUMP_IF_FALSE : Opcode::JUMP_IF_TRUE));
            
            if (thenNode == nullptr && elseNode == nullptr) return jumpNode->getFirst();

            if (thenNode) {
                if (elseNode) {
                    Node* jumpOverElseNode = new Node(Instruction(Opcode::JUMP, vector<string>({ to_string(elseNode->getInstructionCount() + 1) })));
                    thenNode->getLast()->next = jumpOverElseNode;
                    jumpOverElseNode->prev = thenNode->getLast();
                }
                size_t thenLength = thenNode->getInstructionCount();
                jumpNode->instruction.operands.push_back(to_string(thenLength + 1)); // Jump to the instruction after the then branch
            }

            jumpNode->next = nullptr;
            if (thenNode) {
                jumpNode->next = thenNode->getFirst();
                thenNode->prev = jumpNode;
                if (elseNode) {
                    Node* lastThenNode = thenNode->getLast();
                    lastThenNode->next = elseNode->getFirst();
                    elseNode->prev = lastThenNode;
                }
            }
            return jumpNode;
        }

        return nullptr;
    }
}