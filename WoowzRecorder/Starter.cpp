#include <iostream>
#include <stdexcept>
#include <Windows.h>

#include "Detector.h"

const std::string Version = "0.1";

static bool ShouldExit = false;

HANDLE Mutex;

static void End() {
	EndDetect();

	if (Mutex) { CloseHandle(Mutex); Mutex = NULL; }
}

BOOL WINAPI ConsoleHandler(DWORD Event) {
	if (Event == CTRL_CLOSE_EVENT || Event == CTRL_C_EVENT || Event == CTRL_BREAK_EVENT) {
		End();
		ShouldExit = true;
		PostQuitMessage(0);
		return TRUE;
	}
	return FALSE;
}

static void Start() {
	Mutex = CreateMutexA(NULL, TRUE, "WoowzRecorder");
	if (!Mutex) { throw std::runtime_error("Failed to create mutex!"); }

	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		CloseHandle(Mutex);
		throw std::runtime_error("WoowzRecorder alredy running!");
	}

	AllocConsole();
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
	freopen_s((FILE**)stdin, "CONIN$", "r", stdin);

	SetConsoleCtrlHandler(ConsoleHandler, TRUE);

	HWND ConsoleWindow = GetConsoleWindow();

	std::cout << "WoowzRecorder " << Version << std::endl;
	StartDetect();
}

static void Cycle() {
	MSG msg;
	while (!ShouldExit) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				ShouldExit = true;
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Sleep(10);
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	try {
		Start();
		Cycle();
	}
	catch (const std::runtime_error& e) {
		MessageBoxA(NULL, e.what(), "ERROR", MB_ICONERROR | MB_OK);
		return 1;
	}
	catch (...) {
		MessageBoxA(NULL, "UNKNOWN ERROR", "ERROR", MB_ICONERROR | MB_OK);
		return 1;
	}
	End();
	return 0;
}