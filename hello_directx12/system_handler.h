// Liam Wynn, 9/26/2024, Hello DirectX 12

//
// The system_handler is in charge of setting up the application
// window and running the 
//

#pragma once

#include "stdafx.h"
#include "application.h"

struct system_handler {
	LPCWSTR application_name;
	HINSTANCE instance;
	HWND window_handler;

	application* app;
};

bool initialize_system(
	system_handler* system,
	const uint32_t screen_w,
	const uint32_t screen_h,
	const bool use_warp
);

void initialize_window(
	system_handler* system,
	const uint32_t screen_w,
	const uint32_t screen_h
);

void run_system(system_handler* system);

void shutdown_system(system_handler* system);

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
