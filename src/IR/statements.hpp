#pragma once

namespace IR {
    Node* ConvertIfToIR(const ast::IfStmt* ifStmt) {
        const size_t id = Globals.GetUniqueID();
        Node* thenNode = ConvertASTToIR(ifStmt->ThenBranch);
        Node* elseNode = ifStmt->ElseBranch ? ConvertASTToIR(ifStmt->ElseBranch) : nullptr;

        bool statementTrue = true; // Default assumption
        if (ifStmt->Condition) {
            if (auto prefixExpr = dynamic_cast<const ast::PrefixExpr*>(ifStmt->Condition)) {
                if (prefixExpr->Operator.kind == lexer::TokenKind::NOT) statementTrue = false;
            }
        }

        // TODO: add comparison
        const string sid = to_string(id);
        const string elseJumpName = "ifelse_" + sid, endJumpName = "ifend_" + sid;
        Node* jumpNode = new Node(Instruction(statementTrue ? Opcode::JUMP_IF_FALSE : Opcode::JUMP_IF_TRUE, vector<string>({ elseJumpName })));

        if (thenNode == nullptr && elseNode == nullptr) {
            Node* endLabelNode = new Node(Instruction(Opcode::LABEL, vector<string>({ endJumpName })));
            jumpNode->next = endLabelNode;
            endLabelNode->prev = jumpNode;
            jumpNode->instruction.operands = vector<string>({ endJumpName });
            return jumpNode;
        }

        if (thenNode) {
            if (elseNode) {
                Node* jumpOverElseNode = new Node(Instruction(Opcode::JUMP, vector<string>({ endJumpName })));
                jumpOverElseNode->prev = thenNode->getLast();
                thenNode->getLast()->next = jumpOverElseNode;
                Node* elseLabelNode = new Node(Instruction(Opcode::LABEL, vector<string>({ elseJumpName })));
                elseLabelNode->prev = thenNode->getLast();
                thenNode->getLast()->next = elseLabelNode;
            } else {
                jumpNode->instruction.operands = vector<string>({ endJumpName });
            }
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
        Node* endLabelNode = new Node(Instruction(Opcode::LABEL, vector<string>({ endJumpName })));
        endLabelNode->prev = jumpNode->getLast();
        jumpNode->getLast()->next = endLabelNode;
        return jumpNode;
    }
    Node* ConvertWhileToIR(const ast::WhileStmt* whileStmt) {
        const size_t id = Globals.GetUniqueID("while");
        const string sid = to_string(id);
        const string startLabelName = "while_start_" + sid, endLabelName = "while_end_" + sid;

        // TODO: add comparison

        Node* startLabelNode = new Node(Instruction(Opcode::LABEL, vector<string>({ startLabelName })));
        // comparison after start label
        Node* jumpNode = new Node(Instruction(Opcode::JUMP_IF_FALSE, vector<string>({ endLabelName })));
        Node* bodyNode = ConvertASTToIR(whileStmt->Body);
        Node* endLabelNode = new Node(Instruction(Opcode::LABEL, vector<string>({ endLabelName })));

        Node* jumpToTopNode = new Node(Instruction(Opcode::JUMP, vector<string>({ startLabelName })));

        // Link the nodes together
        startLabelNode->next = jumpNode;
        jumpNode->prev = startLabelNode;
        jumpNode->next = bodyNode;
        if (bodyNode) {
            bodyNode->prev = jumpNode;
            Node* lastBodyNode = bodyNode->getLast();
            lastBodyNode->next = jumpToTopNode;
            jumpToTopNode->prev = lastBodyNode;
            jumpToTopNode->next = endLabelNode;
            endLabelNode->prev = jumpToTopNode;
        } else {
            jumpNode->next = endLabelNode;
            endLabelNode->prev = jumpNode;
        }
        if (OptimizationOptions.IR.optimizeLabels) {
            Optimizer::OptimizeLabels(startLabelNode);
        }
        return startLabelNode;
    }
}