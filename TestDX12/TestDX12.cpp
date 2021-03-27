#include <iostream>
#include <windows.h>
#include <vector>
#include <memory>
#include "Window.h"
#include "StringUtility.h"

// /SUBSYSTEM: Console のエントリ
int main(int argc, char* argv[])
{
	return WinMain(NULL, NULL, NULL, 0);
}

// /WINDOS: WINDOWS のエントリ
int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd)
{
	SetConsoleOutputCP(CP_UTF8);
	try
	{
		auto window = std::unique_ptr<Window>(new Window());
		Window::WaitApplicationQuit();
		return 0;
	}
	catch(std::exception e)
	{
		std::cout << e.what() << std::endl;
		MessageBox(NULL, ToTString(e.what()).data(), TEXT("Error"), MB_OK);
		return 1;
	}
	catch(...)
	{
		MessageBox(NULL, TEXT("Unknown"), TEXT("Error"), MB_OK);
	}

}