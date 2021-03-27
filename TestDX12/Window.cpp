#include <windows.h>
#include <iostream>
#include "Window.h"
#include "Resource.h"
#include "winres.h"
#include "Graphic.h"


Window::Window()
{
    m_ModuleHandle = GetModuleHandle(NULL);

    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = [](HWND window, UINT message, WPARAM param, LPARAM longParam)-> LRESULT
    {
        const auto _this = reinterpret_cast<Window *>(GetWindowLongPtr(window, GWLP_USERDATA));
        return nullptr != _this ? _this->WindowProcess(message, param, longParam) : DefWindowProc(window, message, param, longParam);
    };
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = m_ModuleHandle;
    windowClass.hIcon = LoadIcon(m_ModuleHandle, MAKEINTRESOURCE(APP_ICON));
    windowClass.hIconSm = LoadIcon(m_ModuleHandle, MAKEINTRESOURCE(APP_ICON));
    windowClass.hCursor = LoadCursor(m_ModuleHandle, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = TEXT("MY_WINDOW_CLASS");
    if(!RegisterClassEx(&windowClass))
    {
        throw std::exception("Failed register window class怪.");
    }
    
    // 制御が戻るまでに次のメッセージが消費される
    // WM_GETMINMAXINFO, WM_NCCREA, WM_NCCALCSIZE, WM_CREATE
    m_WindowHandle = CreateWindow(
        windowClass.lpszClassName,
        TEXT("TestDX12"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        m_ModuleHandle,
        NULL);

    SetWindowLongPtr(m_WindowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    m_Graphic = std::unique_ptr<Graphic>(new Graphic(m_WindowHandle));

    ShowWindow(m_WindowHandle, SW_SHOW);
}

Window::~Window() {}

LRESULT Window::WindowProcess(UINT message, WPARAM param, LPARAM longParam)
{
    switch(message)
    {
        case WM_PAINT:
            m_Graphic->Rendring();
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            DefWindowProc(m_WindowHandle, message, param, longParam);
            break;
    }
    return 0;
}

void Window::Update() { m_Graphic->Rendring(); }

MSG Window::WaitApplicationQuit()
{
    MSG message;
    while (GetMessage(&message, NULL, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
    return message;
}