#pragma once

#include <functional>

#include "Graphic.h"
#include "Resource.h"


inline void HandleStructuredException(std::function<void()> *callback, unsigned int &code)
{
    __try { callback->operator()(); }
    __except (EXCEPTION_EXECUTE_HANDLER) { code = GetExceptionCode(); }
}

class Window
{
private:
    HMODULE m_ModuleHandle;
    WNDCLASSEX m_WindowClass;
    HWND m_WindowHandle;
    std::unique_ptr<Graphic> m_Graphic;

public:
    Window()
    {
        using namespace std;

        m_ModuleHandle = GetModuleHandle(NULL);
        
        m_WindowClass =
        {
            .cbSize = sizeof(WNDCLASSEX),
            .style = CS_HREDRAW | CS_VREDRAW,
            // エラーハンドリングしてWindow::WindowProcessに流す（例外拾ったらすぐClose）
            .lpfnWndProc = [](HWND window, UINT message, WPARAM param, LPARAM longParam)-> LRESULT
            {
                const auto _this = reinterpret_cast<Window *>(GetWindowLongPtr(window, GWLP_USERDATA));
                if(nullptr != _this)
                {
                    try
                    {
                        unsigned int seCode = 0;
                        LRESULT result = 0;
                        function<void()> callback = [&]() -> void { result = _this->WindowProcess(message, param, longParam); };
                        HandleStructuredException(&callback, seCode);
                        if(seCode != 0)
                        {
                            // 多分この辺（DX12のはどこにあるのかわからない）
                            // https://github.com/Alexpux/mingw-w64/blob/master/mingw-w64-headers/include/minwinbase.h#L284
                            // https://github.com/wine-mirror/wine/blob/master/include/winnt.h#L611
                            // https://github.com/apitrace/dxsdk/blob/master/Include/d3d9.h#L1981 （これはDX9多分違う）
                            stringstream sstream;
                            sstream << "Structured Exception: " << hex << seCode;
                            throw exception(sstream.str().data());
                        }
                        return result;
                    }
                    catch(exception e)
                    {
                        cout << e.what() << endl;
                        MessageBox(NULL, ToTString(e.what()).data(), TEXT("Exception"), MB_OK);
                        DestroyWindow(window);
                    }
                    catch(...)
                    {
                        cout << "Unknown Error" << endl;
                        MessageBox(NULL, TEXT("Unknown Error"), TEXT("Exception"), MB_OK);
                        DestroyWindow(window);
                    }
                }
                return DefWindowProc(window, message, param, longParam);
            },
            .cbClsExtra = 0,
            .cbWndExtra = 0,
            .hInstance = m_ModuleHandle,
            .hIcon = LoadIcon(m_ModuleHandle, MAKEINTRESOURCE(APP_ICON)),
            .hCursor = LoadCursor(m_ModuleHandle, IDC_ARROW),
            .hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)),
            .lpszMenuName = NULL,
            .lpszClassName = TEXT("MY_WINDOW_CLASS"),
            .hIconSm = LoadIcon(m_ModuleHandle, MAKEINTRESOURCE(APP_ICON)),
        };
        if(!RegisterClassEx(&m_WindowClass))
        {
            throw exception("Failed register window class怪.");
        }
        // 制御が戻るまでに次のメッセージが消費される
        // WM_GETMINMAXINFO, WM_NCCREA, WM_NCCALCSIZE, WM_CREATE
        m_WindowHandle = CreateWindow(
            m_WindowClass.lpszClassName,
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
        ShowWindow(m_WindowHandle, SW_SHOW);
    }

    ~Window()
    {
        UnregisterClass(m_WindowClass.lpszClassName, m_ModuleHandle);
    }
    
    static MSG WaitApplicationQuit()
    {
        MSG message;
        while (GetMessage(&message, NULL, 0, 0))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        return message;
    }

private:
    LRESULT WindowProcess(UINT message, WPARAM param, LPARAM longParam)
    {
        switch(message)
        {
            case WM_SHOWWINDOW:
                m_Graphic.reset(new Graphic(m_WindowHandle));
                break;
            case WM_PAINT:
            case WM_MOUSEMOVE:
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
};
