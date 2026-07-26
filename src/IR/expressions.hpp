#pragma once

namespace IR {
    Node* ConvertAssignmentToIR(const ast::AssignmentExpr* assignmentExpr) {
        const string varName = to_string(assignmentExpr->Assignee->GetValue());
        const string value = to_string(ast::decompiler::DecompileExpression(assignmentExpr->Value));
        if (assignmentExpr->Operator.kind != lexer::ASSIGNMENT) {
            // Handle compound assignments like +=, -=, etc.
            string op;
            switch (assignmentExpr->Operator.kind) {
                case lexer::PLUS_EQUALS : op = "ADD"; break;
                case lexer::MINUS_EQUALS : op = "SUB"; break;
                case lexer::STAR_EQUALS : op = "MUL"; break;
                case lexer::SLASH_EQUALS : op = "DIV"; break;
                case lexer::PERCENT_EQUALS : op = "MOD"; break;
                default:
                    throw std::runtime_error("Unsupported assignment operator: " + to_string(lexer::TokenKindString(assignmentExpr->Operator.kind)));
            }
            Node* loadVarNode = new Node(Instruction(Opcode::LOAD_VAR, vector<string>({ varName })));
            Node* operationNode = new Node(Instruction(
                static_cast<Opcode>(
                    (Opcode)((int)(Opcode::ADD) + (op == "SUB" ? 1 : op == "MUL" ? 2 : op == "DIV" ? 3 : op == "MOD" ? 4 : 0))), 
                    vector<string>({ varName, value }
                    )
                )
            );
            Node* storeValueNode = new Node(Instruction(Opcode::STORE_VAR, vector<string>({ varName, "*reg" })));

            loadVarNode->next = operationNode;
            operationNode->prev = loadVarNode;
            operationNode->next = storeValueNode;
            storeValueNode->prev = operationNode;

            return loadVarNode;
        }
        Node* storeValueNode = new Node(Instruction(Opcode::STORE_VAR, vector<string>({ varName, value })));
        return storeValueNode;
    }
}