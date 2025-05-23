#include <iostream>
#include <stdexcept>
#include <vector>
#include <Windows.h>

HWND W;
HWND Wm;

//==========================================================================================================

struct MonitorInfo { int x, y, w, h; };
std::vector<MonitorInfo> Monitors;

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
	MonitorInfo i;
	i.x = lprcMonitor->left;
	i.y = lprcMonitor->top;
	i.w = lprcMonitor->right - lprcMonitor->left;
	i.h = lprcMonitor->bottom - lprcMonitor->top;
	Monitors.push_back(i);
	return TRUE;
}

LRESULT CALLBACK WindowProc_Monitors(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateShot() {
	const wchar_t CLASS_NAME[] = L"WoowzRecorder_SnipAndSketch_Monitors";

	WNDCLASSW wc = {};
	wc.lpfnWndProc = WindowProc_Monitors;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = CLASS_NAME;

	RegisterClassW(&wc);

	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);

	int TotalW = 0;
	int MaxH   = 0;

	for (const auto& m : Monitors) {
		TotalW += m.w;
		if (m.h > MaxH) {
			MaxH = m.h;
		}
	}

	HWND hwnd = CreateWindowEx(
		WS_EX_TOPMOST,
		CLASS_NAME,
		L"",
		WS_POPUP,
		0, 0, TotalW, MaxH,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL
	);

	if(hwnd == NULL) { throw std::exception("Failed to create SnipAndSketch Monitors window!"); }

	Wm = hwnd;
}

void ScreenshotToWindow() {
	ShowWindow(W , SW_SHOW);
	ShowWindow(Wm, SW_SHOW);
	SetWindowPos(W, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

LRESULT CALLBACK WindowProc_Monitors(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_DESTROY:
		DestroyWindow(hwnd);
		if (W != NULL) { DestroyWindow(W); }
		Wm = NULL;
		return 0;
	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_INACTIVE) {
			SetWindowPos(W, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		HBRUSH redBrush = CreateSolidBrush(RGB(255, 0, 0));
		FillRect(hdc, &ps.rcPaint, redBrush);
		DeleteObject(redBrush);
		EndPaint(hwnd, &ps);
	}
	return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//==========================================================================================================

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND Button_Exit;
void CreateWindow_() {
	const wchar_t CLASS_NAME[] = L"WoowzRecorder_SnipAndSketch";

	WNDCLASSW wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = CLASS_NAME;

	RegisterClassW(&wc);

	int ScreenW = GetSystemMetrics(SM_CXSCREEN);
	int ScreenH = GetSystemMetrics(SM_CYSCREEN);

	int WindowW = 100;
	int WindowH = 100;
	
	int X = (ScreenW - WindowW) / 2;
	int Y = 0;

	HWND hwnd = CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
		CLASS_NAME,
		L"",
		WS_POPUP,
		X, Y, WindowW, WindowH,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL
	);

	if (hwnd == NULL) { throw std::exception("Failed to create SnipAndSketch window!"); }

	W = hwnd;

	Button_Exit = CreateWindowExW(
		0,
		L"BUTTON",
		L"X",
		WS_VISIBLE | WS_CHILD,
		25, 25, 50, 50,
		hwnd,
		(HMENU)1,
		GetModuleHandle(NULL),
		NULL
	);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
		case WM_DESTROY:
			DestroyWindow(hwnd);
			if (Wm != NULL) { DestroyWindow(Wm); }
			W = NULL;
			return 0;
		case WM_COMMAND:
			if (LOWORD(wParam) == 1) {
				DestroyWindow(hwnd);
			}
			break;
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hwnd, &ps);
				FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
				EndPaint(hwnd, &ps);
			}
			return 0;
		case WM_NCHITTEST:
			{
				POINT pt = { LOWORD(lParam), HIWORD(lParam) };
				ScreenToClient(hwnd, &pt);

				RECT rcClient;
				GetClientRect(hwnd, &rcClient);

				if (PtInRect(&rcClient, pt)) {
					return HTCAPTION;
				}
			}
			break;
		case WM_WINDOWPOSCHANGING:
			{
				WINDOWPOS* pPos = (WINDOWPOS*)lParam;

				int leftLimit = GetSystemMetrics(SM_XVIRTUALSCREEN);
				int topLimit = GetSystemMetrics(SM_YVIRTUALSCREEN);
				int rightLimit = leftLimit + GetSystemMetrics(SM_CXVIRTUALSCREEN);
				int bottomLimit = topLimit + GetSystemMetrics(SM_CYVIRTUALSCREEN);

				if (pPos->x < leftLimit) pPos->x = leftLimit;
				if (pPos->y < topLimit) pPos->y = topLimit;
				if (pPos->x + pPos->cx > rightLimit) pPos->x = rightLimit - pPos->cx;
				if (pPos->y + pPos->cy > bottomLimit) pPos->y = bottomLimit - pPos->cy;
			}
			break;
	}return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//==========================================================================================================

void WRSTART_SnipAndSketch() {
	try {
		if (!IsWindow(W)) {
			std::cout << "START 'SnipAndSketch'" << std::endl;
			CreateWindow_();
			CreateShot();
			ScreenshotToWindow();
		}
		else {
			DestroyWindow(W);
		}
	}
	catch (std::exception e) {
		std::cerr << "Error SnipAndSketch: " << e.what() << std::endl;
	}
}