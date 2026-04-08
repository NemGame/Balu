#pragma once

// [{k1: v1}, {k2: v2}] -> [{k2: v2}, {k1: v1}]
template<typename T>
unordered_map<wstring, T> reverseUnorderedMap(const unordered_map<wstring, T>& map) {
    unordered_map<wstring, T> reversedMap;
    for (const auto& pair : map) {
        reversedMap[pair.first] = pair.second;
    }
    return reversedMap;
}