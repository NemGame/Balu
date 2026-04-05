#include <Windows.h>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

#pragma region Structs
struct CommandLineValuePair {
    wstring name;
    wstring value;
};

wostream& operator<<(wostream& os, const CommandLineValuePair& pair) {
    os << pair.name << "=" << pair.value;
    return os;
}

struct CommandLineArgs {
    vector<wstring> freeArgs;  // Without any prior dashes (not flags) -> wasd
    vector<wstring> flags;     // With double dashes or / (flags) -> --v, --verbose, --help, /v, /verbose, /help, /?, -?
    vector<CommandLineValuePair> keyValuePairs; // For inline key-value pairs; first is a singular dash, then the value (-filename=name.txt)
};

#pragma endregion

#pragma region Command Line Parsing
vector<wstring> GetCommandLineArgs(int& count) {
    LPWSTR* argList = CommandLineToArgvW(GetCommandLineW(), &count);
    vector<wstring> args;
    
    if (argList != nullptr) {
        for (int i = 0; i < count; ++i) {
            args.push_back(wstring(argList[i]));
        }
        LocalFree(argList);
    }
    
    return args;
}

CommandLineArgs ParseCommandLineArgs(vector<wstring> args) {
    CommandLineArgs result;
    
    for (const auto& arg : args) {
        if (arg.rfind(L"--", 0) == 0 || arg.rfind(L"/", 0) == 0) {
            result.flags.push_back(arg);
        } else if (arg.rfind(L"-", 0) == 0) {
            size_t equalPos = arg.find(L'=');
            if (equalPos != wstring::npos) {
                CommandLineValuePair pair;
                pair.name = arg.substr(1, equalPos - 1); // Skip the leading dash
                pair.value = arg.substr(equalPos + 1);
                result.keyValuePairs.push_back(pair);
            } else {
                result.flags.push_back(arg);
            }
        } else {
            result.freeArgs.push_back(arg);
        }
    }
    
    return result;
}

void DisplayCommandLineArgs(const CommandLineArgs& args) {
    wcout << L"Free Arguments:" << endl;
    for (const auto& arg : args.freeArgs) {
        wcout << L"  " << arg << endl;
    }
    wcout << L"Flags:" << endl;
    for (const auto& flag : args.flags) {
        wcout << L"  " << flag << endl;
    }

    wcout << L"Key-Value Pairs:" << endl;
    for (const auto& pair : args.keyValuePairs) {
        wcout << L"  " << pair << endl;
    }
}

#pragma endregion

#pragma region Utility Functions
bool vectorContains(const vector<wstring>& vec, const wstring& value) {
    for (const auto& item : vec) {
        if (item == value) {
            return true;
        }
    }
    return false;
}
bool vectorContains(const vector<wstring>& vec, const vector<wstring>& values) {
    for (const auto& value : values) {
        if (vectorContains(vec, value)) {
            return true;
        }
    }
    return false;
}
#pragma endregion

#pragma region User Interaction

void DisplayHelp() {
    wcout << L"Usage: main.exe [options] [arguments]" << endl;
    wcout << L"Options:" << endl;
    wcout << L"  --help, /help, /?   Display this help message" << endl;
    wcout << L"  --verbose, /verbose Enable verbose output" << endl;
    wcout << L"  -key=value          Specify a key-value pair (e.g., -filename=name.txt)" << endl;
}

#pragma endregion

int main(int argc, char* argv[]) {
    int count = 0;
    vector<wstring> args = GetCommandLineArgs(count);
    CommandLineArgs parsedArgs = ParseCommandLineArgs(args);

    vector<wstring> *freeArgs = &parsedArgs.freeArgs;
    vector<wstring> *flags = &parsedArgs.flags;
    vector<CommandLineValuePair> *keyValuePairs = &parsedArgs.keyValuePairs;

    DisplayCommandLineArgs(parsedArgs);

    if (vectorContains(*flags, {L"--help", L"/help", L"/?"})) {
        DisplayHelp();
        return 0;
    }

    return 0;
}