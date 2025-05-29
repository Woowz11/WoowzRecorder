#include <iostream>
#include <Windows.h>
#include <stdexcept>
#include <unordered_set>

#include "WR_Recorder.h"
#include "WR_SnipAndSketch.h"

const bool TestKeys = false;

bool ConsoleVisible = false;

HHOOK MouseHook;

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= 0) {
		WR_SnipAndSketch_MousePress(wParam, lParam);
	}
	return CallNextHookEx(MouseHook, nCode, wParam, lParam);
}

HHOOK KeyboardHook;
std::unordered_set<int> PressedKeys;

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION) {
		if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
			KBDLLHOOKSTRUCT* Key = (KBDLLHOOKSTRUCT*)lParam;
			PressedKeys.insert(Key->vkCode);
			
			if (!TestKeys) {
				if ( // LSHIFT + LWIN + S
					PressedKeys.find(VK_LSHIFT) != PressedKeys.end() &&
					PressedKeys.find(VK_LWIN) != PressedKeys.end() &&
					PressedKeys.find(0x53) != PressedKeys.end()
					) {
					if (WRSTART_SnipAndSketch()) { return 1; }
				}

				if ( // LALT + Z
					PressedKeys.find(VK_LMENU) != PressedKeys.end() &&
					PressedKeys.find(0x5A) != PressedKeys.end()
					) {
					if (WRSTART_Recorder()) { return 1; }
				}
			}
			else {
				if (Key->vkCode == VK_F1) {
					if (WRSTART_SnipAndSketch()) { return 1; }
				}

				if (Key->vkCode == VK_F2) {
					if (WRSTART_Recorder()) { return 1; }
				}
			}

			if (Key->vkCode == VK_LWIN) {
				if (WR_SnipAndSketch_Cancel()) { return 1; }
			}

			if (Key->vkCode == VK_SPACE || Key->vkCode == VK_ESCAPE) {
				if (WREND_SnipAndSketch()) { return 1; }
			}

			if ( // LALT + W + R
				PressedKeys.find(VK_LMENU) != PressedKeys.end() &&
				PressedKeys.find(0x57    ) != PressedKeys.end() &&
				PressedKeys.find(0x52    ) != PressedKeys.end()
				) {
				HWND ConsoleWindow = GetConsoleWindow();
				ConsoleVisible = !ConsoleVisible;
				ShowWindow(ConsoleWindow, ConsoleVisible ? SW_SHOW : SW_HIDE);
				return 1;
			}
		}
		else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
			KBDLLHOOKSTRUCT* Key = (KBDLLHOOKSTRUCT*)lParam;
			PressedKeys.erase(Key->vkCode);
		}
	}
	return CallNextHookEx(KeyboardHook, nCode, wParam, lParam);
}

void StartDetect() {
	if (TestKeys) { std::cout << "Enabled TestKeys!" << std::endl; }

	KeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
	if (KeyboardHook == NULL) { throw std::exception("KeyboardHook failed to set!"); }

	MouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, NULL, 0);
	if (MouseHook == NULL) { throw std::exception("MouseHook failed to set!"); }
}

void EndDetect() {
	if (MouseHook    != NULL) { UnhookWindowsHookEx(MouseHook   ); MouseHook    = NULL; }
	if (KeyboardHook != NULL) { UnhookWindowsHookEx(KeyboardHook); KeyboardHook = NULL; }
}