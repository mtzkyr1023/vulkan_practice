
#include <iostream>
#include <cstdlib>

#include "defines.h"
#include "native_window.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {

	AllocConsole();
	FILE* stream;

	_wfreopen_s(&stream, L"CONOUT$", L"w+", stdout);

	NativeWindow window(hInstance);

	window.initializeWindow();

	while (window.pollEvents()) {
		window.render();
	}


	window.cleanupWindow();

	return 0;
}