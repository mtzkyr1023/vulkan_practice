#ifndef _NATIVE_WINDOW_H_
#define _NATIVE_WINDOW_H_

#include <Windows.h>

#include "render_engine.h"
#include "application.h"

class NativeWindow {
public:
	NativeWindow(HINSTANCE hinstance) :
		hwnd_(nullptr),
		hinstance_(hinstance)
	{}

	void initializeWindow();
	void cleanupWindow();

	bool pollEvents();
	void render();

private:
	HWND hwnd_;
	HINSTANCE hinstance_;

	RenderEngine engine_;
	Application application_;

	const wchar_t* kClassName = L"Vulkan Tutorial";
};

static LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif