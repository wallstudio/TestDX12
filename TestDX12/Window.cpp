#include <windows.h>
#include <iostream>
#include <exception>
#include <sstream>
#include "Window.h"
#include "Resource.h"
#include "winres.h"
#include "Graphic.h"
#include "StringUtility.h"


Window::Window()
{
    m_ModuleHandle = GetModuleHandle(NULL);
    
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = [](HWND window, UINT message, WPARAM param, LPARAM longParam)-> LRESULT
    {
        const auto _this = reinterpret_cast<Window *>(GetWindowLongPtr(window, GWLP_USERDATA));
        if(nullptr != _this)
        {
            try
            {
                unsigned int seCode = 0;
                LRESULT result = 0;
                std::function<void()> callback = [&]() -> void { result = _this->WindowProcess(message, param, longParam); };
                HandleStructuredException(&callback, seCode);
                if(seCode != 0)
                {
                    // 多分この辺（DX12のはどこにあるのかわからない）
                    // https://github.com/Alexpux/mingw-w64/blob/master/mingw-w64-headers/include/minwinbase.h#L284
                    // https://github.com/wine-mirror/wine/blob/master/include/winnt.h#L611
                    // https://github.com/apitrace/dxsdk/blob/master/Include/d3d9.h#L1981 （これはDX9多分違う）
                    std::stringstream sstream;
                    sstream << "Structured Exception: " << std::hex << seCode;
                    throw std::exception(sstream.str().data());
                }
                return result;
            }
            catch(std::exception e)
            {
                std::cout << e.what() << std::endl;
                MessageBox(NULL, ToTString(e.what()).data(), TEXT("Exception"), MB_OK);
                DestroyWindow(window);
            }
            catch(...)
            {
                std::cout << "Unknown Error" << std::endl;
                MessageBox(NULL, TEXT("Unknown Error"), TEXT("Exception"), MB_OK);
                DestroyWindow(window);
            }
        }
        return DefWindowProc(window, message, param, longParam);
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
            break;
    }
    return DefWindowProc(m_WindowHandle, message, param, longParam);
}

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

void HandleStructuredException(std::function<void()> *callback, unsigned int &code)
{
    __try { callback->operator()(); }
    __except (EXCEPTION_EXECUTE_HANDLER) { code = GetExceptionCode(); }
}