#pragma once

namespace BASM::converter::Windows::x86 {
    wstring exit(int code = 0) {
        wstringstream wss;
        wss << L"push " << code << L"\n"
            << L"call ExitProcess\n";
        return wss.str();
    }
}