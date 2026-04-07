#pragma once

namespace helper {    
    const wstring lastOfPath(const wstring& path) {
        size_t lastSlashPos = path.find_last_of(L"\\");
        if (lastSlashPos != wstring::npos) {
            return path.substr(lastSlashPos + 1);
        }
        return path;
    }
}