#pragma once
#include <Windows.h>

template<typename T>
wstring to_wstring(T value) {
    wstringstream ss;
    ss << value;
    return ss.str();
}

string wstringToUTF8(const wstring& wstr) {
    if (wstr.empty()) return string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

wstring UTF8ToWstring(const string& str) {
    if (str.empty()) return wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

const wstring strip_left(const wstring& str, wchar_t charToStrip) {
    size_t start = 0;
    while (start < str.size() && str[start] == charToStrip) {
        start++;
    }
    return str.substr(start);
}

const wstring NumberToBinary(wstring decimal_str) {
    if (decimal_str == L"0" || decimal_str.empty()) return L"0";

    wstring binary_result = L"";

    while (decimal_str != L"0" && !decimal_str.empty()) {
        wstring next_decimal = L"";
        int remainder = 0;

        for (wchar_t c : decimal_str) {
            int current_digit = remainder * 10 + (c - L'0');
            int next_digit = current_digit / 2;
            remainder = current_digit % 2;

            // Prevent leading zeros in the intermediate quotient string
            if (!next_decimal.empty() || next_digit > 0) {
                next_decimal += to_wstring(next_digit);
            }
        }

        // The remainder of the division by 2 becomes our binary bit
        binary_result += (remainder == 1) ? L"1" : L"0";
        
        // Update the string for the next iteration loop
        decimal_str = next_decimal.empty() ? L"0" : next_decimal;
    }

    // Bits were collected in reverse order (little-endian), so flip them
    std::reverse(binary_result.begin(), binary_result.end());
    return binary_result;
}

const wstring BinaryAdd(const wstring& a, const wstring& b) {
    wstring result;
    int carry = 0;
    int maxLength = max(a.size(), b.size());

    for (int i = 0; i < maxLength; ++i) {
        int bitA = (i < a.size()) ? a[a.size() - 1 - i] - L'0' : 0;
        int bitB = (i < b.size()) ? b[b.size() - 1 - i] - L'0' : 0;
        int sum = bitA + bitB + carry;
        carry = sum / 2;
        result += (sum % 2) ? L'1' : L'0';
    }

    if (carry) {
        result += L'1';
    }

    std::reverse(result.begin(), result.end());
    return result;
}

const wstring BinaryNot(const wstring& binary_str) {
    wstring result = binary_str;
    for (wchar_t& c : result) {
        c = (c == L'0') ? L'1' : L'0';
    }
    return result;
}

const wstring BinaryMultiply(const wstring& a, const wstring& b) {
    wstring result = L"0";
    for (size_t i = 0; i < b.size(); ++i) {
        if (b[b.size() - 1 - i] == L'1') {
            result = BinaryAdd(result, a + wstring(i, L'0')); // Shift a left by i
        }
    }
    return result;
}

const wstring DecimalAdd(const wstring& a, const wstring& b) {
    wstring result;
    int carry = 0;
    int maxLength = max(a.size(), b.size());

    for (int i = 0; i < maxLength; ++i) {
        int digitA = (i < a.size()) ? a[a.size() - 1 - i] - L'0' : 0;
        int digitB = (i < b.size()) ? b[b.size() - 1 - i] - L'0' : 0;
        int sum = digitA + digitB + carry;
        carry = sum / 10;
        result += wchar_t(L'0' + (sum % 10));
    }

    if (carry) {
        result += wchar_t(L'0' + carry);
    }

    std::reverse(result.begin(), result.end());
    size_t firstNonZero = result.find_first_not_of(L'0');
    if (firstNonZero == wstring::npos) {
        return L"0";
    }
    return result.substr(firstNonZero);
}

const wstring DecimalMultiplyByTwo(const wstring& value) {
    return DecimalAdd(value, value);
}

const wstring BinaryToDecimalWstring(const wstring& binary_str) {
    size_t firstBit = binary_str.find_first_not_of(L'0');
    if (firstBit == wstring::npos) {
        return L"0";
    }

    wstring decimal_result = L"0";
    for (size_t i = firstBit; i < binary_str.size(); ++i) {
        decimal_result = DecimalMultiplyByTwo(decimal_result);
        if (binary_str[i] == L'1') {
            decimal_result = DecimalAdd(decimal_result, L"1");
        }
    }

    return decimal_result;
}