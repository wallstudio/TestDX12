#include <memory>
#include <string>
#include <vector>
#include <Windows.h>
#include "StringUtility.h"


std::shared_ptr<std::string> WideToMultiByte(const WCHAR *source)
{
    auto wideString = std::wstring(source);
    auto multiByteLength = WideCharToMultiByte(CP_UTF8, 0, wideString.data(), wideString.size(), NULL, 0, NULL, NULL);
    auto multiByteBuffer = std::vector<char>(multiByteLength);
    ZeroMemory(multiByteBuffer.data(), multiByteBuffer.size());
    WideCharToMultiByte(CP_UTF8, 0, wideString.data(), wideString.size(), multiByteBuffer.data(), multiByteBuffer.size(), NULL, NULL);
    auto multiByteString = std::shared_ptr<std::string>(new std::string(multiByteBuffer.data()));
    return multiByteString;
}

std::shared_ptr<std::wstring> MultiByteToWide(const char *source)
{
    auto multiByteString = std::string(source);
    auto wideLength = MultiByteToWideChar(CP_UTF8, 0, multiByteString.data(), multiByteString.size(), NULL, 0);
    auto wideBuffer = std::vector<WCHAR>(wideLength);
    ZeroMemory(wideBuffer.data(), wideBuffer.size());
    MultiByteToWideChar(CP_UTF8, 0, multiByteString.data(), multiByteString.size(), wideBuffer.data(), wideBuffer.size());
    auto wideString = std::shared_ptr<std::wstring>(new std::wstring(wideBuffer.data()));
    return wideString;
}

#if UNICODE
std::shared_ptr<std::tstring> ToTString(const WCHAR *source) { return std::shared_ptr<std::wstring>(new std::wstring(source)); }
std::shared_ptr<std::tstring> ToTString(const char *source) { return MultiByteToWide(source); }
std::shared_ptr<std::string> ToWide(const TCHAR *source) { return WideToMultiByte(ToTString(source)->data()); }
std::shared_ptr<std::wstring> ToMultiByte(const TCHAR *source) { return ToTString(source); }
#else
std::shared_ptr<std::tstring> ToTString(const WCHAR *source) { return WideToMultiByte(source); }
std::shared_ptr<std::tstring> ToTString(const char *source) { return std::shared_ptr<std::string>(new std::string(source)); }
std::shared_ptr<std::string> ToWide(const TCHAR *source) { return ToTString(source); }
std::shared_ptr<std::wstring> ToMultiByte(const TCHAR *source) { return MultiByteToWide(source); }
#endif
