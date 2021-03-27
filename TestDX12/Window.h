#pragma once

#include <windows.h>
#include <memory>
#include "Graphic.h"

class Window
{
private:
    HMODULE m_ModuleHandle;
    HWND m_WindowHandle;
    std::unique_ptr<Graphic> m_Graphic;
public:
    Window();
    ~Window();
    void Update();
    static MSG WaitApplicationQuit();
private:
    // static LRESULT CALLBACK GlobalWindowProcess(HWND window, UINT message, WPARAM param, LPARAM longParam);
    LRESULT WindowProcess(UINT message, WPARAM param, LPARAM longParam);
};