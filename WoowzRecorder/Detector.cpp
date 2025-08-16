#include <iostream>
#include <Windows.h>
#include <stdexcept>
#include <unordered_set>

#include "WR_Recorder.h"
#include "WR_SnipAndSketch.h"

bool TestKeys = false;

bool ConsoleVisible = false;

std::unordered_set<int> PressedKeys;

bool KeyPressed(int VK) {
    return (GetAsyncKeyState(VK) & 0x8000) != 0;
}

bool KeyComboPressed(std::initializer_list<int> Keys) {
    for (int VK : Keys) {
        if (PressedKeys.find(VK) == PressedKeys.end()) {
            return false;
        }
    }
    return true;
}

HHOOK KeyboardHook = NULL;
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        auto* Key = (KBDLLHOOKSTRUCT*)lParam;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            PressedKeys.insert(Key->vkCode);

            bool Blocked = false;

            if (!TestKeys) {
                if (KeyComboPressed({ VK_LSHIFT, VK_LWIN, 0x53 })) { // LSHIFT + LWIN + S
                    Blocked = WRSTART_SnipAndSketch();
                }
                if (KeyComboPressed({ VK_LMENU, 0x5A })) { // LALT + Z
                    Blocked = WRSTART_Recorder();
                }
            } else {
                if (Key->vkCode == VK_F1) { Blocked = WRSTART_SnipAndSketch(); }
                if (Key->vkCode == VK_F2) { Blocked = WRSTART_Recorder     (); }
            }

            if (Key->vkCode == VK_LWIN) {
                Blocked = WR_SnipAndSketch_Cancel();
            }

            if (Key->vkCode == VK_SPACE || Key->vkCode == VK_ESCAPE) {
                Blocked = WREND_SnipAndSketch();
            }

            if (KeyComboPressed({ VK_LMENU, 0x57, 0x52 })) { // LALT + W + R
                HWND ConsoleWindow = GetConsoleWindow();
                ConsoleVisible = !ConsoleVisible;
                ShowWindow(ConsoleWindow, ConsoleVisible ? SW_SHOW : SW_HIDE);
                Blocked = true;
            }

            if (Blocked) { return 1; }

        } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            PressedKeys.erase(Key->vkCode);
        }
    }

    return CallNextHookEx(KeyboardHook, nCode, wParam, lParam);
}

HHOOK MouseHook = NULL;
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) { WR_SnipAndSketch_MousePress(wParam, lParam); }

    return CallNextHookEx(MouseHook, nCode, wParam, lParam);
}

void StartDetect() {
	if (TestKeys) { std::cout << "Enabled TestKey's!" << std::endl; }

	KeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
	if (KeyboardHook == NULL) { throw std::runtime_error("KeyboardHook failed to set!"); }

	MouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, NULL, 0);
	if (MouseHook == NULL) { throw std::runtime_error("MouseHook failed to set!"); }
}

void EndDetect() {
	if (MouseHook    != NULL) { UnhookWindowsHookEx(MouseHook   ); MouseHook    = NULL; }
	if (KeyboardHook != NULL) { UnhookWindowsHookEx(KeyboardHook); KeyboardHook = NULL; }

    ConsoleVisible = true;
    ShowWindow(GetConsoleWindow(), SW_SHOW);
}