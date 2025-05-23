#include <iostream>
#include <Windows.h>
#include <stdexcept>
#include <unordered_set>

#include "WR_Recorder.h"
#include "WR_SnipAndSketch.h"

HHOOK KeyboardHook;
std::unordered_set<int> PressedKeys;
bool TestKeys = true;

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
}

void EndDetect() {
	UnhookWindowsHookEx(KeyboardHook);
}