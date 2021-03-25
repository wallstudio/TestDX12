#include <iostream>
#include <windows.h>
#include <tchar.h>

// /SUBSYSTEM: Console のエントリ
int main(int argc, char* argv[])
{
	std::cout << "HelloWorld";
	return WinMain(NULL, NULL, NULL, 0);
}

// /WINDOS: WINDOWS のエントリ
int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd)
{
	return 0;
}