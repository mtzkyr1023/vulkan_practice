#include "native_window.h"
#include "defines.h"
#include "mk_exception.h"

#include "imgui/backends/imgui_impl_win32.h"
#include "util/input.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void NativeWindow::initializeWindow() {
	WNDCLASSEX wc{};
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)windowProc;
	wc.hInstance = hinstance_;
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = kClassName;

	if (!RegisterClassEx(&wc)) {
		throw MkException("window creation error");
	}

	RECT wr = { 0, 0, kScreenWidth, kScreenHeight };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	int nativeWidth = GetSystemMetrics(SM_CXSCREEN);
	int nativeHeight = GetSystemMetrics(SM_CYSCREEN);
	hwnd_ = CreateWindowEx(0,
		kClassName,
		L"Vulkan Totorial",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU,
		(nativeWidth - kScreenWidth) / 2,
		(nativeHeight - kScreenHeight) / 2 - GetSystemMetrics(SM_CYSIZE),
		wr.right - wr.left,		// width
		wr.bottom - wr.top,		// height
		nullptr,				// handle to parent
		nullptr,				// handle to menu
		hinstance_,			// hInstance
		nullptr);
	if (!hwnd_) {
		throw MkException("window creation error");
	}

	SetWindowPos(
		hwnd_,
		HWND_TOP,
		-GetSystemMetrics(SM_CXSIZE) / 4,
		-GetSystemMetrics(SM_CYSIZE) / 2,
		wr.right - wr.left,
		wr.bottom - wr.top,
		0);

	engine_.initializeContext();
	engine_.initializeSurface(hwnd_, hinstance_);
	engine_.initializeSwapchain(kScreenWidth, kScreenHeight, false);
	engine_.initializeRenderSettings();

	application_.initialize(&engine_, hwnd_);

	Input::Instance().Initialize(hwnd_);
}

void NativeWindow::cleanupWindow() {
	UnregisterClass(kClassName, hinstance_);

	application_.cleanup(&engine_);

	engine_.cleanupRenderSettings();
	engine_.cleanupSwapchain();
	engine_.cleanupContext();
}


bool NativeWindow::pollEvents() {
	MSG msg;

	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT || Input::Instance().Trigger(DIK_ESCAPE)) {
			return false;
		}
		else {
			Input::Instance().Updata();

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return true;
}

void NativeWindow::render() {
	application_.render(&engine_);
}

static LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	switch (uMsg)
	{
	case WM_SETFOCUS:
	{
		return 0;
	}

	case WM_KILLFOCUS:
	{
		return 0;
	}

	case WM_SYSCOMMAND:
	{
		switch (wParam & 0xfff0)
		{
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
		{
			break;
		}

		case SC_KEYMENU:
			return 0;
		}
		break;
	}

	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		// TODO: キー入力に対する処理
		break;
	}

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
	{
		// TODO: マウス操作に対する処理
		return 0;
	}

	case WM_MOUSEMOVE:
	{
		// TODO: マウス移動に対する処理
		return 0;
	}

	case WM_MOUSELEAVE:
	{
		return 0;
	}

	case WM_MOUSEWHEEL:
	{
		return 0;
	}

	case WM_MOUSEHWHEEL:
	{
		return 0;
	}

	case WM_SIZE:
	{
		// TODO: サイズ変更時の処理
		return 0;
	}

	case WM_MOVE:
	{
		return 0;
	}

	case WM_PAINT:
	{
		//_glfwInputWindowDamage(window);
		break;
	}

	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}