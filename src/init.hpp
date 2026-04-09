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

using namespace std;

// Global variables
// They start with an underscore to indicate that they are global variables
bool _verbose = false;  // Whether to print verbose output [false]
bool _showWarnings = true;  // Whether to show warnings [true]
bool _panic = true;  // Whether to panic on errors (exit immediately) [true]
bool _debug = false;  // Whether to print debug information [false]
bool _provideHelp = true;  // Whether to tell the user about the correct syntax when they make a syntax error [true]

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