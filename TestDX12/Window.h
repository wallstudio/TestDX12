#pragma once

#include <windows.h>
#include <memory>
#include <exception>
#include "Graphic.h"
#include <functional>

class Window
{
private:
    HMODULE m_ModuleHandle;
    HWND m_WindowHandle;
    std::unique_ptr<Graphic> m_Graphic;
public:
    Window();
    static MSG WaitApplicationQuit();
private:
    LRESULT WindowProcess(UINT message, WPARAM param, LPARAM longParam);
};

void HandleStructuredException(std::function<void()> *callback, unsigned int &code);