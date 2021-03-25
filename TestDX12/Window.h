#pragma once

#include <windows.h>
#include <memory>
#include "Graphic.h"

class Window
{
private:
    HMODULE m_AppHandle;
    HWND m_Window;
    std::unique_ptr<Graphic> m_Graphic;
public:
    Window();
    ~Window();
    WPARAM Run();
private:
    static LRESULT CALLBACK WindowProcess(HWND window, UINT message, WPARAM param, LPARAM longParam);
};