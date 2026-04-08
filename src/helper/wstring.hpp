#pragma once

template<typename T>
wstring to_wstring(T value) {
    wstringstream ss;
    ss << value;
    return ss.str();
}