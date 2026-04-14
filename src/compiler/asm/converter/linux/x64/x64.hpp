#pragma once

namespace ASM::converter::Linux::x64 {
    class Converter : public ASM::ConverterInterface {
    public:
        vector<wstring> parse(const ast::Stmt* stmt) override {
            vector<wstring> assembly;
            return assembly;
        }
    };
    wstring exit(int code = 0) {
        wstringstream wss;
        wss << L"mov edi, 0x" << std::hex << code << L"\n"
            << L"push 60\n"  // syscall number for sys_exit
            << L"pop rax\n"  // syscall number into rax
            << L"syscall\n";  // call kernel
        return wss.str();
    }
}