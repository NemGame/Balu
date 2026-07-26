#pragma once

template<typename T, typename U>
bool vectorContains(const vector<T>& vec, const U& value) {
    for (const auto& item : vec) {
        if (item == value) {
            return true;
        }
    }
    return false;
}
template<typename T>
bool vectorContains(const vector<T>& vec, const vector<T>& values) {
    for (const auto& value : values) {
        if (vectorContains(vec, value)) {
            return true;
        }
    }
    return false;
}
vector<wstring> wstringToVector(const wstring& str, wchar_t delimiter=L',') {
    vector<wstring> result;
    wstring::size_type start = 0;
    wstring::size_type end = str.find(delimiter);
    while (end != wstring::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }
    result.push_back(str.substr(start)); // Add the last segment
    return result;
}
wstring vectorToWstring(const vector<wstring>& vec, wchar_t delimiter=L',') {
    if (vec.empty()) return L"";
    if (vec.size() == 1) return vec[0];
    wstring result;
    for (size_t i = 0; i < vec.size(); ++i) {
        result += vec[i];
        if (i < vec.size() - 1) {
            result += delimiter;
        }
    }
    return result;
}
string vectorToString(const vector<string>& vec, char delimiter=',') {
    if (vec.empty()) return "";
    if (vec.size() == 1) return vec[0];
    string result;
    for (size_t i = 0; i < vec.size(); ++i) {
        result += vec[i];
        if (i < vec.size() - 1) {
            result += delimiter;
        }
    }
    return result;
}