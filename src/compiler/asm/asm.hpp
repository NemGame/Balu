#pragma once

namespace ASM {
    interface ConverterInterface {
        virtual vector<wstring> parse(const ast::Stmt* stmt) = 0;
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
    ConverterInterface* getConverter(compiler::CompileTarget target) {
        return nullptr;
    }  // TODO: fix
    // ConverterInterface* getConverter(compiler::CompileTarget target) {
    //     switch (target) {
    //         case compiler::CompileTarget::Windows_x86:
    //             return new converter::Windows::x86::Converter();
    //         case compiler::CompileTarget::Windows_x64:
    //             return new converter::Windows::x64::Converter();
    //         case compiler::CompileTarget::Linux_x86:
    //             return new converter::Linux::x86::Converter();
    //         case compiler::CompileTarget::Linux_x64:
    //             return new converter::Linux::x64::Converter();
    //         default:
    //             throw runtime_error("Unsupported compile target");
    //     }
    // }
    int convertToAssembly(compiler::CompileTarget target, const ast::Stmt* stmt) {
        ConverterInterface* converter = getConverter(target);
        vector<wstring> assembly = converter->parse(stmt);
        delete converter;
        return 0; // Return success
    }
}