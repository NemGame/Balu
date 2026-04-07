#pragma once

namespace helper {
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
}