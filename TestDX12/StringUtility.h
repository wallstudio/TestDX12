#pragma once

#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <Windows.h>

namespace std
{
    #if UNICODE
    typedef wstring tstring;
    #else
    typedef string tstring;
    #endif
}

inline std::string WideToMultiByte(const WCHAR *source)
{
    using namespace std;

    auto wideString = wstring(source);
    auto multiByteLength = WideCharToMultiByte(CP_UTF8, 0, wideString.data(), static_cast<int>(wideString.size()), NULL, 0, NULL, NULL);
    auto multiByteBuffer = vector<char>(multiByteLength + 1);
    std::memset(multiByteBuffer.data(), 0, multiByteBuffer.size());
    WideCharToMultiByte(CP_UTF8, 0, wideString.data(), static_cast<int>(wideString.size()), multiByteBuffer.data(), static_cast<int>(multiByteBuffer.size()), NULL, NULL);
    auto multiByteString = string(multiByteBuffer.data());
    return multiByteString;
}

inline std::wstring MultiByteToWide(const char *source)
{
    using namespace std;

    auto multiByteString = string(source);
    auto wideLength = MultiByteToWideChar(CP_UTF8, 0, multiByteString.data(), static_cast<int>(multiByteString.size()), NULL, 0);
    auto wideBuffer = vector<WCHAR>(wideLength + 1);
    std::memset(wideBuffer.data(), 0, wideBuffer.size());
    MultiByteToWideChar(CP_UTF8, 0, multiByteString.data(), static_cast<int>(multiByteString.size()), wideBuffer.data(), static_cast<int>(wideBuffer.size()));
    auto wideString = wstring(wideBuffer.data());
    return wideString;
}

#if UNICODE
inline std::tstring ToTString(const WCHAR *source) { return std::wstring(source); }
inline std::tstring ToTString(const char *source) { return MultiByteToWide(source); }
inline std::string ToMultiByte(const TCHAR *source) { return WideToMultiByte(source); }
inline std::wstring ToWide(const TCHAR *source) { return ToTString(source); }
#else
inline std::tstring ToTString(const WCHAR *source) { return WideToMultiByte(source); }
inline std::tstring ToTString(const char *source) { return std::string(source); }
inline std::string ToMultiByte(const TCHAR *source) { return ToTString(source); }
inline std::wstring ToWide(const TCHAR *source) { return MultiByteToWide(source); }
#endif

inline std::tstring ToTString(const HRESULT result)
{
    LPTSTR buffer = NULL;
    const auto flag = FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM;
    FormatMessage(flag, NULL, result, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (LPWSTR)&buffer, 0, NULL);
    const auto message = ToTString(buffer);
    LocalFree(buffer);
    return message;
}