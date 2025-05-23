#include <iostream>
#include <Windows.h>
#include <stdexcept>
#include <unordered_set>

#include "WR_Recorder.h"
#include "WR_SnipAndSketch.h"

const bool TestKeys = true;

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
				if (
					PressedKeys.find(VK_LSHIFT) != PressedKeys.end() &&
					PressedKeys.find(VK_LWIN) != PressedKeys.end() &&
					PressedKeys.find(0x53) != PressedKeys.end()
					) {
					WRSTART_SnipAndSketch();
				}

				if (
					PressedKeys.find(VK_LMENU) != PressedKeys.end() &&
					PressedKeys.find(0x5A) != PressedKeys.end()
					) {
					WRSTART_Recorder();
				}
			}
			else {
				if (Key->vkCode == VK_F1) {
					WRSTART_SnipAndSketch();
				}

				if (Key->vkCode == VK_F2) {
					WRSTART_Recorder();
				}
			}

			if (Key->vkCode == VK_LWIN) {
				WREND_SnipAndSketch();
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
	KeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
	if (KeyboardHook == NULL) { throw std::exception("KeyboardHook failed to set!"); }

	MouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, NULL, 0);
	if (MouseHook == NULL) { throw std::exception("MouseHook failed to set!"); }
}

void EndDetect() {
	if (MouseHook    != NULL) { UnhookWindowsHookEx(MouseHook   ); MouseHook    = NULL; }
	if (KeyboardHook != NULL) { UnhookWindowsHookEx(KeyboardHook); KeyboardHook = NULL; }
}