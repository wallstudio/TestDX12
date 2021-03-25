#include <iostream>
#include <windows.h>
#include <vector>
#include "Window.h"

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
		auto result = Window().Run();
		return 0;
	}
	catch(std::exception e)
	{
		std::cout << e.what() << std::endl;
#if UNICODE
		auto message = std::string(e.what());
		auto wideMessage = std::vector<TCHAR>(message.size() + 1);
		ZeroMemory(wideMessage.data(), wideMessage.size());
		MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, message.data(), message.size(), wideMessage.data(), wideMessage.size());
		MessageBoxW(NULL, wideMessage.data(), TEXT("Error"), MB_OK);
#else
		MessageBoxA(NULL, e.what(), "Error", MB_OK);
#endif
		return 1;
	}

}