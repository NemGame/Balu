#pragma once

namespace IR {
    #undef EOF
    enum class Opcode {
        SOF = 0,      // Start of file
        EOF = 1,          // End of file
        NOP,          // No operation; does nothing
        CREATE_VAR,   // Create a variable with a given name on the stack [varName, varType]
        SET_SCOPE,    // Set the current scope for variable resolution [scopeName] ; The global scope is 1
        DESTROY_VAR,  // Destroy a variable with a given name on the stack [varName]
        LOAD_VAR,     // Load a variable's value into a register [varName]
        SAVE_VAR,     // Save a register's value into a variable [varName]
        STORE_VAR,    // Store a value into a variable [varName, value] ; *reg means the value is in the last used register
        ADD,          // Pop two values, add them, and push the result [a, b => a = a + b]
        SUB,          // Pop two values, subtract them, and push the result [a, b => a = a - b]
        MUL,          // Pop two values, multiply them, and push the result [a, b => a = a * b]
        DIV,          // Pop two values, divide them, and push the result [a, b => a = a / b]
        MOD,          // Pop two values, modulo them, and push the result [a, b => a = a % b]
        NEGATE,       // Pop one value, negate it, and push the result [ a => a = -a ]
        JUMP_IF_TRUE, // Pop one value; if it's true, jump to a given instruction index
        JUMP_IF_FALSE,// Pop one value; if it's false, jump to a given instruction index
        JUMP,         // Unconditionally jump to a given instruction index
        LABEL,        // Define a label for jump targets
        CALL_FUNC,    // Call a function with a given number of arguments
        RETURN_VALUE  // Return from a function with a value on top of the stack
    };
    wstring OpcodeToWString(Opcode opcode) {
        switch (opcode) {
            case Opcode::SOF: return L"SOF";
            case Opcode::EOF: return L"EOF";
            case Opcode::NOP: return L"NOP";
            case Opcode::CREATE_VAR: return L"CREATE_VAR";
            case Opcode::SET_SCOPE: return L"SET_SCOPE";
            case Opcode::DESTROY_VAR: return L"DESTROY_VAR";
            case Opcode::LOAD_VAR: return L"LOAD_VAR";
            case Opcode::SAVE_VAR: return L"SAVE_VAR";
            case Opcode::STORE_VAR: return L"STORE_VAR";
            case Opcode::ADD: return L"ADD";
            case Opcode::SUB: return L"SUB";
            case Opcode::MUL: return L"MUL";
            case Opcode::DIV: return L"DIV";
            case Opcode::MOD: return L"MOD";
            case Opcode::NEGATE: return L"NEGATE";
            case Opcode::JUMP_IF_TRUE: return L"JUMP_IF_TRUE";
            case Opcode::JUMP_IF_FALSE: return L"JUMP_IF_FALSE";
            case Opcode::JUMP: return L"JUMP";
            case Opcode::LABEL: return L"LABEL";
            case Opcode::CALL_FUNC: return L"CALL_FUNC";
            case Opcode::RETURN_VALUE: return L"RETURN_VALUE";
            default: return L"UNKNOWN_OPCODE";
        }
    }
    struct Instruction {
        Opcode opcode;
        vector<string> operands; // Operands can be variable names, constant values, or instruction indices (for jumps)
        Instruction(Opcode opcode, const vector<string>& operands = {}) : opcode(opcode), operands(operands) {}
        void Dump(wostream& wcout_ = _wcout) const {
            int indent = opcode == Opcode::LABEL ? 0 : 2;
            if (opcode == Opcode::SOF || opcode == Opcode::EOF) wcout_ << L"--";
            else if (opcode == Opcode::SET_SCOPE) wcout_ << L" -";
            else if (opcode == Opcode::NOP) wcout_ << L"? ";
            else if (opcode == Opcode::LABEL) wcout_ << L"\n" << to_wstring(operands.back()) << L":";
            else wcout_ << wstring(indent, L' ');
            if (indent != 0) {
                wcout_ << OpcodeToWString(opcode) << L" ";
                wstring all = L"";
                for (const auto& operand : operands) {
                    all += to_wstring(operand) + L", ";
                }
                if (!all.empty()) all.pop_back(), all.pop_back(); // Remove the last ", "
                wcout_ << all;
            }
            wcout_ << endl;
        }
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
        void freeAll() {
            Node* current = this->getFirst();
            while (current) {
                Node* toDelete = current;
                current = current->next;
                delete toDelete;
            }
        }
    };
}