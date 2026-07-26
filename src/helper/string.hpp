#pragma once
#include <Windows.h>

string wstringToUTF8(const wstring& wstr) {
    if (wstr.empty()) return string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

template<typename T>
string to_string(T value) {
    stringstream ss;
    ss << value;
    return ss.str();
}
string to_string(const wstring& wstr) {
    return wstringToUTF8(wstr);
}