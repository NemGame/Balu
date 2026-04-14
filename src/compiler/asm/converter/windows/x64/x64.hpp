#pragma once

namespace ASM::converter::Windows::x64 {
    class Converter : public ASM::ConverterInterface {
    public:
        vector<wstring> parse(const ast::Stmt* stmt) override {
            vector<wstring> assembly;
            return assembly;
        }
    };
    wstring exit(int code = 0) {
        wstringstream wss;
        wss << L"sub rsp, 40\n"  // 32 bytes shadow space + 8 bytes for alignment
            << L"mov rcx, " << code << L"\n"  // First argument in RCX
            << L"call ExitProcess\n";
        return wss.str();
    }
}