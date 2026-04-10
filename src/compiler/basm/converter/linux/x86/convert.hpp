#pragma once

namespace BASM::converter::Linux::x86 {
    wstring exit(int code = 0) {
        wstringstream wss;
        wss << L"mov ebx, " << code << L"\n"  // First argument in EBX
            << L"mov eax, 1\n"  // syscall number for sys_exit
            << L"int 0x80\n";  // call kernel
        return wss.str();
    }
}