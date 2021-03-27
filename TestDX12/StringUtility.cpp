#include <memory>
#include <string>
#include <vector>
#include <Windows.h>
#include "StringUtility.h"


std::string WideToMultiByte(const WCHAR *source)
{
    auto wideString = std::wstring(source);
    auto multiByteLength = WideCharToMultiByte(CP_UTF8, 0, wideString.data(), wideString.size(), NULL, 0, NULL, NULL);
    auto multiByteBuffer = std::vector<char>(multiByteLength + 1);
    ZeroMemory(multiByteBuffer.data(), multiByteBuffer.size());
    WideCharToMultiByte(CP_UTF8, 0, wideString.data(), wideString.size(), multiByteBuffer.data(), multiByteBuffer.size(), NULL, NULL);
    auto multiByteString = std::string(multiByteBuffer.data());
    return multiByteString;
}

std::wstring MultiByteToWide(const char *source)
{
    auto multiByteString = std::string(source);
    auto wideLength = MultiByteToWideChar(CP_UTF8, 0, multiByteString.data(), multiByteString.size(), NULL, 0);
    auto wideBuffer = std::vector<WCHAR>(wideLength + 1);
    ZeroMemory(wideBuffer.data(), wideBuffer.size());
    MultiByteToWideChar(CP_UTF8, 0, multiByteString.data(), multiByteString.size(), wideBuffer.data(), wideBuffer.size());
    auto wideString = std::wstring(wideBuffer.data());
    return wideString;
}

#if UNICODE
std::tstring ToTString(const WCHAR *source) { return std::wstring(source); }
std::tstring ToTString(const char *source) { return MultiByteToWide(source); }
std::string ToMultiByte(const TCHAR *source) { return WideToMultiByte(source); }
std::wstring ToWide(const TCHAR *source) { return ToTString(source); }
#else
std::tstring ToTString(const WCHAR *source) { return WideToMultiByte(source); }
std::tstring ToTString(const char *source) { return std::string(source); }
std::string ToMultiByte(const TCHAR *source) { return ToTString(source); }
std::wstring ToWide(const TCHAR *source) { return MultiByteToWide(source); }
#endif


std::tstring ToTString(const HRESULT result)
{
    LPTSTR buffer = NULL;
    const auto flag = FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM;
    FormatMessage(flag, NULL, result, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (LPWSTR)&buffer, 0, NULL);
    const auto message = ToTString(buffer);
    LocalFree(buffer);
    return message;
}