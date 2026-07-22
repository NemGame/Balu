#pragma once

namespace IR {
    enum class Opcode {
        NOP,
        LOAD_CONST,   // Load a constant value onto the stack
        LOAD_VAR,     // Load a variable's value onto the stack
        STORE_VAR,    // Store the top of the stack into a variable
        ADD,          // Pop two values, add them, and push the result
        SUB,          // Pop two values, subtract them, and push the result
        MUL,          // Pop two values, multiply them, and push the result
        DIV,          // Pop two values, divide them, and push the result
        MOD,          // Pop two values, modulo them, and push the result
        NEGATE,       // Pop one value, negate it, and push the result
        JUMP_IF_TRUE, // Pop one value; if it's true, jump to a given instruction index
        JUMP_IF_FALSE,// Pop one value; if it's false, jump to a given instruction index
        JUMP,         // Unconditionally jump to a given instruction index
        CALL_FUNC,    // Call a function with a given number of arguments
        RETURN_VALUE  // Return from a function with a value on top of the stack
    };
    struct Instruction {
        Opcode opcode;
        vector<string> operands; // Operands can be variable names, constant values, or instruction indices (for jumps)
        Instruction(Opcode opcode, const vector<string>& operands = {}) : opcode(opcode), operands(operands) {}
    };
    struct Node {
        Instruction instruction;
        Node* next; // Pointer to the next instruction in the sequence
        Node* prev; // Pointer to the previous instruction in the sequence
        Node(const Instruction& instruction) : instruction(instruction), next(nullptr), prev(nullptr) {}
        Node* getLast() {
            Node* current = this;
            while (current->next) current = current->next;
            return current;
        }
        Node* getFirst() {
            Node* current = this;
            while (current->prev) current = current->prev;
            return current;
        }
        size_t getInstructionCount() {
            size_t count = 0;
            Node* current = this->getFirst();
            while (current) {
                count++;
                current = current->next;
            }
            return count;
        }
        size_t getInstructionsLeft() {
            size_t count = 0;
            Node* current = this;
            while (current) {
                count++;
                current = current->next;
            }
            return count;
        }
    };
}