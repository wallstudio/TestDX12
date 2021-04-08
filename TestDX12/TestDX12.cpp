#include <iostream>
#include <filesystem>
#include "Window.h"

// /SUBSYSTEM: Console のエントリ
int main(int argc, char* argv[])
{
	using namespace std;

	cout << filesystem::current_path().string() << endl;
	SetConsoleOutputCP(CP_UTF8);
	return WinMain(NULL, NULL, NULL, 0);
}

// /WINDOS: WINDOWS のエントリ
int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd)
{
	using namespace std;

	try
	{
		auto window = unique_ptr<Window>(new Window());
		Window::WaitApplicationQuit();
		return 0;
	}
	catch(exception e)
	{
		cout << e.what() << endl;
		MessageBox(NULL, ToTString(e.what()).data(), TEXT("Error"), MB_OK);
		return 1;
	}
	catch(...)
	{
		MessageBox(NULL, TEXT("Unknown"), TEXT("Error"), MB_OK);
	}

}