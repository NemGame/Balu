#pragma once

namespace BASM::converter::Windows::x64 {
    wstring exit(int code = 0) {
        wstringstream wss;
        wss << L"sub rsp, 40\n"  // 32 bytes shadow space + 8 bytes for alignment
            << L"mov rcx, " << code << L"\n"  // First argument in RCX
            << L"call ExitProcess\n";
        return wss.str();
    }
}