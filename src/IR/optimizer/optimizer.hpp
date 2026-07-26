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
    struct VariableInfo {
        string name;
        string type;
        size_t scope;
        Node* lastUsed = nullptr;
        Node* declaration = nullptr;
    };
    Node* AutoFreeVariables(Node* root) {
        if (!root) return nullptr;

        Node* current = root;
        vector<VariableInfo> variables;
        size_t scopeCounter = 0;
        while (current) {
            if (current->instruction.opcode == Opcode::SET_SCOPE) {
                scopeCounter = stoul(current->instruction.operands[0]);
                for (auto it = variables.begin(); it != variables.end();) {
                    if (it->scope > scopeCounter) {
                        // If the variable's scope is greater than the current scope, destroy it
                        Node* destroyNode = new Node(Instruction(Opcode::DESTROY_VAR, vector<string>({ it->name })));
                        destroyNode->next = current;
                        destroyNode->prev = current->prev;
                        if (current->prev) {
                            current->prev->next = destroyNode;
                        }
                        current->prev = destroyNode;
                        it = variables.erase(it);
                    } else {
                        ++it;
                    }
                }
            } else if (current->instruction.opcode == Opcode::CREATE_VAR) {
                const string varName = current->instruction.operands[0], varType = current->instruction.operands[1];
                auto it = find_if(variables.begin(), variables.end(), [&](const VariableInfo& v) { return v.name == varName; });
                if (it != variables.end()) {
                    // If the variable already exists, update its scope and declaration
                    if (CompilerOptions.IR.allowIRWarnings && !CompilerOptions.IR.allowRedeclaration) {
                        const wstring message = L"Warning: Variable " + to_wstring(varName) + L" is being redeclared in the same scope. " + to_wstring(it->type) + L" -> " + to_wstring(varType);
                        _wcout << message << endl;
                    } else if (CompilerOptions.IR.allowRedeclaration) {
                        Node* destroyNode = new Node(Instruction(Opcode::DESTROY_VAR, vector<string>({ varName })));
                        destroyNode->next = current;
                        destroyNode->prev = current->prev;
                        if (current->prev) {
                            current->prev->next = destroyNode;
                        }
                        current->prev = destroyNode;
                        it->scope = scopeCounter;
                        it->type = varType;
                        it->declaration = current;
                    }
                } else {
                    VariableInfo varInfo;
                    varInfo.name = current->instruction.operands[0];
                    varInfo.type = current->instruction.operands[1];
                    varInfo.scope = scopeCounter;
                    varInfo.declaration = current;
                    variables.push_back(varInfo);
                }
            } else if (current->instruction.opcode == Opcode::DESTROY_VAR) {
                string varName = current->instruction.operands[0];
                auto it = find_if(variables.begin(), variables.end(), [&](const VariableInfo& v) { return v.name == varName; });
                if (it != variables.end()) {
                    variables.erase(it);
                }
            } else if (current->instruction.operands.size() > 0) {
                for (auto& operand : current->instruction.operands) {
                    if (operand[0] != '*') { // Skip register references
                        auto it = find_if(variables.begin(), variables.end(), [&](const VariableInfo& v) { return v.name == operand; });
                        if (it != variables.end()) {
                            it->lastUsed = current;
                        }
                    }
                }
            }
            current = current->next;
        }
        for (auto& var : variables) {
            if (var.lastUsed) {
                Node* destroyNode = new Node(Instruction(Opcode::DESTROY_VAR, vector<string>({ var.name })));
                destroyNode->next = var.lastUsed->next;
                if (var.lastUsed->next) {
                    var.lastUsed->next->prev = destroyNode;
                }
                var.lastUsed->next = destroyNode;
                destroyNode->prev = var.lastUsed;
            } else {
                Node* destroyNode = new Node(Instruction(Opcode::DESTROY_VAR, vector<string>({ var.name })));
                Node* afterDeclaration = var.declaration->next;
                var.declaration->next = destroyNode;
                destroyNode->prev = var.declaration;
                destroyNode->next = afterDeclaration;
                if (afterDeclaration) {
                    afterDeclaration->prev = destroyNode;
                }
            }
        }
        return root;
    }
}