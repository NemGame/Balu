#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>


using namespace std;

// Global variables
// They start with an underscore to indicate that they are global variables
bool _verbose = false;  // Whether to print verbose output [false]
bool _showWarnings = true;  // Whether to show warnings [true]
bool _panic = true;  // Whether to panic on errors (exit immediately) [true]
bool _debug = false;  // Whether to print debug information [false]

struct Error {
    wstring message;
    int line;
    int column;
    Error(const wstring& message, int line, int column) : message(message), line(line), column(column) {}
    Error(const wstring& message) : message(message), line(-1), column(-1) {}
    Error() : message(L""), line(-1), column(-1) {}
    bool isNull() const {
        return message.empty() && line == -1 && column == -1;
    }
};

#include "helper/init.hpp"
#include "lexer/init.hpp"
#include "ast/init.hpp"
#include "parser/init.hpp"