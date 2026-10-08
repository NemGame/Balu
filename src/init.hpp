#pragma once

#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <map>
#include <bitset>
#include <algorithm>
#include <cstdint>
#include <locale>
#include <codecvt>
#include <type_traits>

using namespace std;

#define UNNECESSARY_PANIC_CHECK 0

struct CommandLineValuePair {
    wstring name;
    wstring value;
};

struct CommandLineArgs {
    wstring programName; // The first argument (the program name)
    wstring programPath; // The full path to the program (can be derived from programName if needed)
    vector<wstring> freeArgs;  // Without any prior dashes (not flags) -> wasd
    vector<wstring> flags;     // With double dashes or / (flags) -> --v, --verbose, --help, /v, /verbose, /help, /?, -?
    vector<CommandLineValuePair> keyValuePairs; // For inline key-value pairs; first is a singular dash, then the value (-filename=name.txt)
    wstring GetCommand() const {
        wstring command = programName;
        for (const wstring& arg : freeArgs) {
            command += L" " + arg;
        }
        for (const wstring& flag : flags) {
            command += L" " + flag;
        }
        for (const CommandLineValuePair& pair : keyValuePairs) {
            command += L" -" + pair.name + L"=" + pair.value;
        }
        return command;
    }
};

// Global variables
struct _CompilerOptions {
private:
    struct _IR {
        bool allowIRWarnings = true;  // Whether to allow warnings in the IR [true]
    };
    struct _User {
        CommandLineArgs Args;  // Command line arguments
    };
    struct _Warnings {
        bool mayLogWarnings = true;  // Whether to allow logging of warnings [true]
        bool undeletableVariableRemains = true;  // Whether to warn if an undeletable variable remains [true]
        bool unmatchedCharacter = true;  // Whether to warn if an unmatched character is encountered [true]
    };
    struct _Language {
        bool allowRedeclaration = true;  // Whether to allow redeclaration of variables [true]
    };
public:
    bool verbose = false;  // Whether to print verbose output [false]
    bool panic = true;  // Whether to panic on errors (exit immediately) [true]
    bool debug = false;  // Whether to print debug information [false]
    bool provideHelp = true;  // Whether to tell the user about the correct syntax when they make a syntax error [true]
    bool allowLexerErrors = true;  // Whether to allow lexer errors (if false, the lexer will panic on errors) [true]
    bool allowOptimization = true;  // Whether to allow optimizations [true]
    _IR IR;  // IR compiler options
    _User User;  // User-provided options
    _Warnings Warnings;  // Warning options
    _Language Language;  // Language options
};
_CompilerOptions CompilerOptions;
struct _OptimizationOptions {
private:
    struct _FastMath {
        bool finiteMathOnly = false;  // Whether to allow only finite math operations (no NaN or Inf) ; (x == x -> true) [false]
    };
    struct _IR {
        bool optimizeLabels = true;  // Whether to optimize labels in the IR [true]
        bool automaticVariableDeletion = true;  // Whether to automatically delete variables when they go out of scope [true]
    };
public:
    _FastMath FastMath;  // Fast math optimization options
    _IR IR;  // IR optimization options
};
_OptimizationOptions OptimizationOptions;

wostream& _wcout = wcout;

struct Error {
    wstring message;
    unsigned long long line, column;
    Error(const wstring& message, unsigned long long line, unsigned long long column) : message(message), line(line), column(column) {}
    Error(const wstring& message) : message(message), line(-1), column(-1) {}
    Error() : message(L""), line(-1), column(-1) {}
    bool isNull() const {
        return message.empty() && line == -1 && column == -1;
    }
};

#ifndef _O_U16TEXT
#define _O_U16TEXT 0x20000
#endif

#include "helper/init.hpp"
#include "lexer/init.hpp"
#include "ast/init.hpp"
#include "parser/init.hpp"
#include "IR/init.hpp"
#include "compiler/init.hpp"