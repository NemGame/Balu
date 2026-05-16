#pragma once
#include <Windows.h>

template<typename T>
wstring to_wstring(T value) {
    wstringstream ss;
    ss << value;
    return ss.str();
}

string wstringToUTF8(const wstring& wstr) {
    if (wstr.empty()) return string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

wstring UTF8ToWstring(const string& str) {
    if (str.empty()) return wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

const wstring strip_left(const wstring& str, wchar_t charToStrip) {
    size_t start = 0;
    while (start < str.size() && str[start] == charToStrip) {
        start++;
    }
    return str.substr(start);
}