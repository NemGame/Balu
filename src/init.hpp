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


#include "lexer/init.hpp"
#include "ast/init.hpp"