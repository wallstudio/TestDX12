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

std::shared_ptr<std::string> WideToMultiByte(const WCHAR *source);
std::shared_ptr<std::wstring> MultiByteToWide(const char *source);

std::shared_ptr<std::tstring> ToTString(const WCHAR *source);
std::shared_ptr<std::tstring> ToTString(const char *source);
std::shared_ptr<std::string> ToWide(const TCHAR *source);
std::shared_ptr<std::wstring> ToMultiByte(const TCHAR *source);