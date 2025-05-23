#include <iostream>
#include <stdexcept>
#include <Windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateWindow_() {
	const wchar_t CLASS_NAME[] = L"WoowzRecorder_SniAndSketch";

	WNDCLASSW wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = CLASS_NAME;

	RegisterClassW(&wc);

	HWND hwnd = CreateWindowEx(
		WS_EX_TOOLWINDOW,
		CLASS_NAME,
		L"Hi)",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL
	);

	if (hwnd == NULL) { throw std::exception("Failed to create SnipAndSketch window!"); }

	ShowWindow(hwnd, SW_SHOW);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
		case WM_DESTROY:
			DestroyWindow(hwnd);
			return 0;
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hwnd, &ps);
				FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
				EndPaint(hwnd, &ps);
			}
			return 0;
	}return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void WRSTART_SnipAndSketch() {
	std::cout << "START 'SnipAndSketch'" << std::endl;
	try {
		CreateWindow_();
	}
	catch (std::exception e) {
		std::cerr << "Error SnipAndSketch: " << e.what() << std::endl;
	}
}