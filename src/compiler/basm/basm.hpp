#pragma once

namespace BASM {
    struct BASMStruct {
        // This structure holds the translated BASM instructions/data
    };

    enum TokenKind {
        EXIT = 0,       // Exit the program
        CREATE_VAR,    // Create a variable
        SET_VAR,       // Set a variable's value
        GET_VAR,       // Get a variable's value
        ADD,           // Add two values
        SUB,           // Subtract two values
        MUL,           // Multiply two values
        DIV,           // Divide two values
        MOD,           // Modulo of two values
        AND,           // Logical AND of two values
        OR,            // Logical OR of two values
        NOT,           // Logical NOT of a value
        JUMP,          // Unconditional jump
        JUMP_IF_ZERO,  // Jump if a value is zero
        JUMP_IF_NOT_ZERO, // Jump if a value is not zero
        CALL,          // Call a function
        RET,           // Return from a function
        PRINT,         // Print a value
        READ,          // Read a value from input
    };
}