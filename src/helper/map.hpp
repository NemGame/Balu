#pragma once

template<typename K, typename V>
bool mapContainsKey(const map<K, V>& m, const K& key) {
    return m.find(key) != m.end();
}