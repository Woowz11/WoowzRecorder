#include <iostream>
#include <Windows.h>
#include <stdexcept>
#include <unordered_set>

HHOOK KeyboardHook;
std::unordered_set<int> PressedKeys;

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION) {
		if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
			KBDLLHOOKSTRUCT* Key = (KBDLLHOOKSTRUCT*)lParam;
			PressedKeys.insert(Key->vkCode);
			
			if (
				PressedKeys.find(VK_LSHIFT) != PressedKeys.end() &&
				PressedKeys.find(VK_LWIN) != PressedKeys.end() &&
				PressedKeys.find(0x53) != PressedKeys.end()
				) {
				std::cout << "LSHIFT + WIN + S" << std::endl;
			}

			if (
				PressedKeys.find(VK_LMENU) != PressedKeys.end() &&
				PressedKeys.find(0x5A    ) != PressedKeys.end()
				) {
				std::cout << "LALT + Z" << std::endl;
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