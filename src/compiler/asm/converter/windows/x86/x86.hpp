#pragma once

namespace ASM::converter::Windows::x86 {
    class Converter : public ASM::ConverterInterface {
    public:
        vector<wstring> parse(const ast::Stmt* stmt) override {
            vector<wstring> assembly;
            return assembly;
        }
    };
    wstring exit(int code = 0) {
        wstringstream wss;
        wss << L"push " << code << L"\n"
            << L"call ExitProcess\n";
        return wss.str();
    }
}