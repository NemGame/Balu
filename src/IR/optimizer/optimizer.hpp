#pragma once

namespace IR::Optimizer {
    const vector<Opcode> jumpOpcodes = { Opcode::JUMP, Opcode::JUMP_IF_TRUE, Opcode::JUMP_IF_FALSE };
    Node* ChangeAllLabelReferences(Node* root, const string& oldLabel, const string& newLabel) {
        if (!root) return nullptr;

        Node* current = root;
        while (current) {
            if (vectorContains(jumpOpcodes, current->instruction.opcode)) {
                for (auto& operand : current->instruction.operands) {
                    if (operand == oldLabel) {
                        operand = newLabel;
                    }
                }
            }
            current = current->next;
        }
        return root;
    }
    Node* OptimizeLabels(Node* root) {
        if (!root) return nullptr;

        Node* current = root;
        while (current && current->next) {
            if (current->instruction.opcode == Opcode::LABEL) {
                Node* nextNode = current->next;
                while (nextNode && nextNode->instruction.opcode == Opcode::LABEL) {
                    // Change all references to the current label to the next label
                    ChangeAllLabelReferences(root, current->instruction.operands[0], nextNode->instruction.operands[0]);
                    // Remove the current label
                    if (current->prev) {
                        current->prev->next = nextNode;
                        nextNode->prev = current->prev;
                    } else {
                        // If current is the root, update root to nextNode
                        root = nextNode;
                        nextNode->prev = nullptr;
                    }
                    delete current;
                    current = nextNode;
                    nextNode = current->next;
                }
            }
            current = current->next;
        }
        return root;
    }
}