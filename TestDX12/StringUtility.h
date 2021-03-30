#pragma once

#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <Windows.h>
using namespace std;


namespace std
{
    #if UNICODE
    typedef wstring tstring;
    #else
    typedef string tstring;
    #endif
}

inline string WideToMultiByte(const WCHAR *source)
{
    auto wideString = wstring(source);
    auto multiByteLength = WideCharToMultiByte(CP_UTF8, 0, wideString.data(), wideString.size(), NULL, 0, NULL, NULL);
    auto multiByteBuffer = vector<char>(multiByteLength + 1);
    ZeroMemory(multiByteBuffer.data(), multiByteBuffer.size());
    WideCharToMultiByte(CP_UTF8, 0, wideString.data(), wideString.size(), multiByteBuffer.data(), multiByteBuffer.size(), NULL, NULL);
    auto multiByteString = string(multiByteBuffer.data());
    return multiByteString;
}

inline wstring MultiByteToWide(const char *source)
{
    auto multiByteString = string(source);
    auto wideLength = MultiByteToWideChar(CP_UTF8, 0, multiByteString.data(), multiByteString.size(), NULL, 0);
    auto wideBuffer = vector<WCHAR>(wideLength + 1);
    ZeroMemory(wideBuffer.data(), wideBuffer.size());
    MultiByteToWideChar(CP_UTF8, 0, multiByteString.data(), multiByteString.size(), wideBuffer.data(), wideBuffer.size());
    auto wideString = wstring(wideBuffer.data());
    return wideString;
}

#if UNICODE
inline tstring ToTString(const WCHAR *source) { return wstring(source); }
inline tstring ToTString(const char *source) { return MultiByteToWide(source); }
inline string ToMultiByte(const TCHAR *source) { return WideToMultiByte(source); }
inline wstring ToWide(const TCHAR *source) { return ToTString(source); }
#else
inline tstring ToTString(const WCHAR *source) { return WideToMultiByte(source); }
inline tstring ToTString(const char *source) { return string(source); }
inline string ToMultiByte(const TCHAR *source) { return ToTString(source); }
inline wstring ToWide(const TCHAR *source) { return MultiByteToWide(source); }
#endif

inline tstring ToTString(const HRESULT result)
{
    LPTSTR buffer = NULL;
    const auto flag = FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM;
    FormatMessage(flag, NULL, result, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (LPWSTR)&buffer, 0, NULL);
    const auto message = ToTString(buffer);
    LocalFree(buffer);
    return message;
}

#define AssertOK(operation) AssertOKImpl(operation, #operation)
inline void AssertOKImpl(HRESULT result, const char *code)
{
    if(FAILED(result))
    {
        auto sb = stringstream();
        sb << "Failed\n\n" << code << "\n\n" << ToMultiByte(ToTString(result).data());
        throw exception(sb.str().data());
    }
}