#include <iostream>
#include <stdexcept>
#include <Windows.h>

#include "Detector.h"

static void Start() {
	StartDetect();
}

static void Cycle() {
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage (&msg);
	}
}

static void End() {
	EndDetect();
}

int main() {
	try {
		Start();
		Cycle();
		End();
	}
	catch (const std::exception& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
		return 1;
	}
	catch (...) {
		std::cerr << "UNKNOWN ERROR" << std::endl;
		return 1;
	}
	return 0;
}