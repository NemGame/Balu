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

using namespace std;


// Global variables
struct _CompilerOptions {
    bool verbose = false;  // Whether to print verbose output [false]
    bool showWarnings = true;  // Whether to show warnings [true]
    bool panic = true;  // Whether to panic on errors (exit immediately) [true]
    bool debug = false;  // Whether to print debug information [false]
    bool provideHelp = true;  // Whether to tell the user about the correct syntax when they make a syntax error [true]
    bool allowLexerErrors = true;  // Whether to allow lexer errors (if false, the lexer will panic on errors) [true]
    bool allowOptimization = true;  // Whether to allow optimizations [true]
};
_CompilerOptions CompilerOptions;
struct _OptimizationOptions {
private:
    struct _FastMath {
        bool finiteMathOnly = false;  // Whether to allow only finite math operations (no NaN or Inf) ; (x == x -> true) [false]
    };
public:
    _FastMath FastMath;  // Fast math optimization options
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