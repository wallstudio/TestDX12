#include <windows.h>

class Window
{
private:
    HMODULE m_AppHandle;
    HWND m_Window;
public:
    Window();
    ~Window();
    WPARAM Run();
private:
    static LRESULT CALLBACK WindowProcess(HWND window, UINT message, WPARAM param, LPARAM longParam);
};