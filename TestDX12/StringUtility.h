#include <memory>
#include <string>
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