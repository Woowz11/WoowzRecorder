#include <iostream>
#include <stdexcept>
#include <vector>
#include <Windows.h>
#include <intrin.h>

bool Created = false;

HWND W;
HWND Wm;

//==========================================================================================================

struct MonitorInfo
{ 
	int x, y, w, h;
	HBITMAP Colors;
};
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

void GetMonitorsColors() {
	HDC hdcScreen = GetDC(NULL);

	for (auto& m : Monitors) {
		HDC hdc = CreateCompatibleDC(hdcScreen);
		m.Colors = CreateCompatibleBitmap(hdcScreen, m.w, m.h);
		SelectObject(hdc, m.Colors);

		BitBlt(hdc, 0, 0, m.w, m.h, hdcScreen, m.x, m.y, SRCCOPY);

		DeleteDC(hdc);
	}

	ReleaseDC(NULL, hdcScreen);
}

LRESULT CALLBACK WindowProc_Monitors(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateShot() {
	const wchar_t CLASS_NAME[] = L"WoowzRecorder_SnipAndSketch_Monitors";

	WNDCLASSW wc = {};
	wc.lpfnWndProc = WindowProc_Monitors;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = CLASS_NAME;
	wc.hCursor = LoadCursor(NULL, IDC_CROSS);

	if (!RegisterClassW(&wc)) {
		DWORD e = GetLastError();
		if (e != ERROR_CLASS_ALREADY_EXISTS) { throw std::exception("Failed to register WoowzRecorder_SnipAndSketch_Monitors!"); }
	}

	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);
	GetMonitorsColors();

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
	UpdateWindow(Wm);
}

void DrawScreenshot(HDC hdcBuffer) {
	int X = 0;
	for (const auto& m : Monitors) {
		HDC hdc = CreateCompatibleDC(hdcBuffer);
		HBITMAP Old = (HBITMAP)SelectObject(hdc, m.Colors);

		BITMAP bm;
		GetObject(m.Colors, sizeof(bm), &bm);

		BitBlt(hdcBuffer, X, 0, bm.bmWidth, bm.bmHeight, hdc, 0, 0, SRCCOPY);

		SelectObject(hdc, Old);
		X += m.w;

		DeleteDC(hdc);
	}
}

void CopyToClipboard(HBITMAP Image) {
	if (OpenClipboard(NULL)) {
		EmptyClipboard();
		SetClipboardData(CF_BITMAP, Image);
		CloseClipboard();
	}
}

void DrawSemiTransparentRect(HDC hdc, int x, int y, int width, int height) {
	BYTE alpha = 128;

	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP hbmMem = CreateCompatibleBitmap(hdc, width, height);
	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

	BitBlt(hdcMem, 0, 0, width, height, hdc, x, y, SRCCOPY);

	BITMAP bm;
	GetObject(hbmMem, sizeof(BITMAP), &bm);

	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = bm.bmWidth;
	bmi.bmiHeader.biHeight = -bm.bmHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* bits;
	HBITMAP hbmAlpha = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
	if (!hbmAlpha) {
		SelectObject(hdcMem, hbmOld);
		DeleteObject(hbmMem);
		DeleteDC(hdcMem);
		return;
	}

	HDC hdcAlpha = CreateCompatibleDC(hdc);
	HBITMAP hbmOldAlpha = (HBITMAP)SelectObject(hdcAlpha, hbmAlpha);

	BitBlt(hdcAlpha, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);

	DWORD* pixels = (DWORD*)bits;
	int pixelCount = bm.bmWidth * bm.bmHeight;

	__m128i fgColor = _mm_set1_epi32(0x00000000);
	__m128i alphaMask = _mm_set1_epi32(alpha);

	for (int i = 0; i < pixelCount; i += 4) {
		__m128i bgPixels = _mm_loadu_si128((__m128i*)(pixels + i));

		__m128i bgR = _mm_and_si128(_mm_srli_epi32(bgPixels, 16), _mm_set1_epi32(0x000000FF));
		__m128i bgG = _mm_and_si128(_mm_srli_epi32(bgPixels, 8), _mm_set1_epi32(0x000000FF));
		__m128i bgB = _mm_and_si128(bgPixels, _mm_set1_epi32(0x000000FF));

		__m128i blendedR = _mm_add_epi32(
			_mm_srli_epi32(_mm_mullo_epi16(bgR, alphaMask), 8),
			_mm_srli_epi32(_mm_mullo_epi16(_mm_set1_epi32(0), _mm_sub_epi32(_mm_set1_epi32(255), alphaMask)), 8)
		);
		__m128i blendedG = _mm_add_epi32(
			_mm_srli_epi32(_mm_mullo_epi16(bgG, alphaMask), 8),
			_mm_srli_epi32(_mm_mullo_epi16(_mm_set1_epi32(0), _mm_sub_epi32(_mm_set1_epi32(255), alphaMask)), 8)
		);
		__m128i blendedB = _mm_add_epi32(
			_mm_srli_epi32(_mm_mullo_epi16(bgB, alphaMask), 8),
			_mm_srli_epi32(_mm_mullo_epi16(_mm_set1_epi32(0), _mm_sub_epi32(_mm_set1_epi32(255), alphaMask)), 8)
		);

		__m128i blendedPixels = _mm_or_si128(
			_mm_or_si128(
				_mm_slli_epi32(blendedR, 16),
				_mm_slli_epi32(blendedG, 8)
			),
			blendedB
		);

		_mm_storeu_si128((__m128i*)(pixels + i), blendedPixels);
	}

	BitBlt(hdc, x, y, width, height, hdcAlpha, 0, 0, SRCCOPY);

	SelectObject(hdcAlpha, hbmOldAlpha);
	DeleteObject(hbmAlpha);
	DeleteDC(hdcAlpha);

	SelectObject(hdcMem, hbmOld);
	DeleteObject(hbmMem);
	DeleteDC(hdcMem);
}

static POINT StartPoint  = { 0,0 };
static POINT EndPoint    = { 0,0 };
static bool  IsSelecting = false;
static RECT  Selection   = { 0,0,0,0 };
LRESULT CALLBACK WindowProc_Monitors(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static HDC hdcBuffer = NULL;
	static HBITMAP hbmBuffer = NULL;
	static HBITMAP hbmOldBuffer = NULL;

	switch (uMsg) {
	case WM_DESTROY:
		Monitors.clear();
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

            if (!hdcBuffer) {
                hdcBuffer = CreateCompatibleDC(hdc);
                hbmBuffer = CreateCompatibleBitmap(hdc, ps.rcPaint.right, ps.rcPaint.bottom);
                hbmOldBuffer = (HBITMAP)SelectObject(hdcBuffer, hbmBuffer);
            }

            DrawScreenshot(hdcBuffer);

            if (IsSelecting) {
				SetROP2(hdcBuffer, R2_XORPEN);
				HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
				HPEN oPen = (HPEN)SelectObject(hdcBuffer, hPen);
				HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
				HBRUSH oBrush = (HBRUSH)SelectObject(hdcBuffer, hBrush);

				Rectangle(hdcBuffer, min(StartPoint.x, EndPoint.x) - 1, min(StartPoint.y, EndPoint.y) - 1, max(StartPoint.x, EndPoint.x) + 1, max(StartPoint.y, EndPoint.y) + 1);

				SelectObject(hdcBuffer, oPen);
				SelectObject(hdcBuffer, oBrush);
				DeleteObject(hPen);
				SetROP2(hdcBuffer, R2_COPYPEN);

				RECT rcOutside = ps.rcPaint;
				RECT rcInside = { min(StartPoint.x, EndPoint.x) - 1, min(StartPoint.y, EndPoint.y) - 1, max(StartPoint.x, EndPoint.x) + 1, max(StartPoint.y, EndPoint.y) + 1 };

				DrawSemiTransparentRect(hdcBuffer, rcOutside.left , rcOutside.top   , rcInside .left  - rcOutside.left , rcOutside.bottom - rcOutside.top   );
				DrawSemiTransparentRect(hdcBuffer, rcInside .right, rcOutside.top   , rcOutside.right - rcInside .right, rcOutside.bottom - rcOutside.top   );
				DrawSemiTransparentRect(hdcBuffer, rcInside .left , rcOutside.top   , rcInside .right - rcInside .left , rcInside .top    - rcOutside.top   );
				DrawSemiTransparentRect(hdcBuffer, rcInside .left , rcInside .bottom, rcInside .right - rcInside .left , rcOutside.bottom - rcInside .bottom);
			} else {
				DrawSemiTransparentRect(hdcBuffer, 0, 0, ps.rcPaint.right, ps.rcPaint.bottom);
			}

            BitBlt(hdc, 0, 0, ps.rcPaint.right, ps.rcPaint.bottom, hdcBuffer, 0, 0, SRCCOPY);

            EndPaint(hwnd, &ps);
		}
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//==========================================================================================================

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND Button_Exit;
HWND Button_ChangeType;
void CreateWindow_() {
	const wchar_t CLASS_NAME[] = L"WoowzRecorder_SnipAndSketch";

	WNDCLASSW wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = CLASS_NAME;

	if (!RegisterClassW(&wc)) {
		DWORD e = GetLastError();
		if (e != ERROR_CLASS_ALREADY_EXISTS) { throw std::exception("Failed to register WoowzRecorder_SnipAndSketch!"); }
	}

	int ScreenW = GetSystemMetrics(SM_CXSCREEN);
	int ScreenH = GetSystemMetrics(SM_CYSCREEN);

	int WindowW = 200;
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
	Created = true;

	Button_ChangeType = CreateWindowExW(
		0,
		L"BUTTON",
		L"T",
		WS_VISIBLE | WS_CHILD,
		25, 25, 50, 50,
		hwnd,
		(HMENU)2,
		GetModuleHandle(NULL),
		NULL
	);

	Button_Exit = CreateWindowExW(
		0,
		L"BUTTON",
		L"X",
		WS_VISIBLE | WS_CHILD,
		125, 25, 50, 50,
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
			Created = false;
			return 0;
		case WM_COMMAND:
			if (LOWORD(wParam) == 1) {
				DestroyWindow(hwnd);
			}
			if (LOWORD(wParam) == 2) {
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

void WR_SnipAndSketch_MousePress(WPARAM w, LPARAM l) {
	if (Created) {
		MOUSEHOOKSTRUCT* i = (MOUSEHOOKSTRUCT*)l;

		switch (w)
		{
			case WM_LBUTTONDOWN:
				{
					POINT CurPos;
					GetCursorPos(&CurPos);
					HWND hwndUnderCursor = WindowFromPoint(CurPos);
					if (hwndUnderCursor != W) {
						IsSelecting = true;
						StartPoint.x = CurPos.x;
						StartPoint.y = CurPos.y;
						EndPoint = StartPoint;

						ShowWindow(W, SW_HIDE);
					}
				}
				break;
			case WM_MOUSEMOVE:
				if (IsSelecting) {
					POINT CurPos;
					GetCursorPos(&CurPos);
					EndPoint.x = CurPos.x;
					EndPoint.y = CurPos.y;

					InvalidateRect(Wm, NULL, FALSE);
				}
				break;
			case WM_LBUTTONUP:
				if (IsSelecting) {
					IsSelecting = false;

					Selection.left = min(StartPoint.x, EndPoint.x);
					Selection.top = min(StartPoint.y, EndPoint.y);
					Selection.right = max(StartPoint.x, EndPoint.x);
					Selection.bottom = max(StartPoint.y, EndPoint.y);

					HDC hdcScreen = GetDC(NULL);
					HDC hdc = CreateCompatibleDC(hdcScreen);
					HBITMAP Colors = CreateCompatibleBitmap(hdcScreen, Selection.right - Selection.left, Selection.bottom - Selection.top);
					SelectObject(hdc, Colors);

					BitBlt(hdc, 0, 0, Selection.right - Selection.left, Selection.bottom - Selection.top, hdcScreen, Selection.left, Selection.top, SRCCOPY);

					CopyToClipboard(Colors);

					DeleteObject(Colors);
					DeleteDC(hdc);
					ReleaseDC(NULL, hdcScreen);

					DestroyWindow(W);
				}
				break;
		}
	}
}

//==========================================================================================================

void WREND_SnipAndSketch() {
	if (Created) {
		DestroyWindow(W);
	}
}

void WRSTART_SnipAndSketch() {
	try {
		if (!Created) {
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