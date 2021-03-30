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

std::string WideToMultiByte(const WCHAR *source);
std::wstring MultiByteToWide(const char *source);

std::tstring ToTString(const WCHAR *source);
std::tstring ToTString(const char *source);
std::string ToMultiByte(const TCHAR *source);
std::wstring ToWide(const TCHAR *source);

std::tstring ToTString(const HRESULT result);


#define AssertOK(operation) AssertOKImpl(operation, #operation)
inline void AssertOKImpl(HRESULT result, const char *code)
{
    if(FAILED(result))
    {
        auto sb = std::stringstream();
        sb << "Failed\n\n" << code << "\n\n" << ToMultiByte(ToTString(result).data());
        throw std::exception(sb.str().data());
    }
}