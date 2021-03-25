#include <windows.h>
#include <iostream>
#include "Window.h"
#include "Resource.h"
#include "winres.h"
#include "Graphic.h"


Window::Window()
{
    m_AppHandle = GetModuleHandle(NULL);

    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = Window::WindowProcess;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = m_AppHandle;
    windowClass.hIcon = LoadIcon(m_AppHandle, MAKEINTRESOURCE(APP_ICON));
    windowClass.hIconSm = LoadIcon(m_AppHandle, MAKEINTRESOURCE(APP_ICON));
    windowClass.hCursor = LoadCursor(m_AppHandle, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = TEXT("MY_WINDOW_CLASS");
    if(!RegisterClassEx(&windowClass))
    {
        throw std::exception("Failed register window class怪.");
    }
    
    m_Window = CreateWindow(
        windowClass.lpszClassName,
        TEXT("TestDX12"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        m_AppHandle,
        NULL);

    SetWindowLong(m_Window, GWLP_USERDATA, reinterpret_cast<long>(this));

    m_Graphic.reset(new Graphic(m_Window));
}

Window::~Window()
{
}

WPARAM Window::Run()
{
    ShowWindow(m_Window, SW_SHOW);
    
    MSG message;
    while (GetMessage(&message, NULL, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
    
    return message.wParam;
}

LRESULT CALLBACK Window::WindowProcess(HWND window, UINT message, WPARAM param, LPARAM longParam)
{
    auto _this = reinterpret_cast<Window *>(GetWindowLong(window, GWLP_USERDATA));
    switch(message)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(window, message, param, longParam);
    }

    return 0;
}