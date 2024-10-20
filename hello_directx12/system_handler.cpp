// Liam Wynn, 9/26/2024, Hello DirectX 12

#include "system_handler.h"
#include "utils.h"

// Needed for WndProc
static system_handler* application_system = NULL;

bool initialize_system(
	system_handler* system,
	const uint32_t screen_w,
	const uint32_t screen_h,
	const bool use_warp
) {
	bool success;
	HRESULT result;

	//
	// Sets the default application name and acquires the instance.
	//

	system->application_name = L"Hello DirectX 12";
	system->instance = GetModuleHandle(NULL);

	//
	// Need to call this to initialize DirectX 12 (Specifically DirectXTex)
	//

	result = CoInitializeEx(NULL, COINITBASE_MULTITHREADED);
	throw_if_failed(result);

	//
	// Initialize the render window.
	//

	initialize_window(system, screen_w, screen_h);

	//
	// Initialize the application.
	//

	system->app = new application;
	success = initialize_application(
		system->app,
		system->window_handler,
		screen_w,
		screen_h,
		use_warp
	);

	application_system = system;

	return success;
}

void initialize_window(
	system_handler* system,
	const uint32_t screen_w,
	const uint32_t screen_h
) {
	WNDCLASSEX window;
	ATOM result;
	int display_monitor_w;
	int display_monitor_h;
	RECT desired_client_window;
	int window_w;
	int window_h;
	int window_x;
	int window_y;
	DWORD window_style;

	//
	// Register a window class for creating our render window.
	//

	window = {};

	window.cbSize = sizeof(WNDCLASSEX);
	window.style = CS_HREDRAW | CS_VREDRAW;
	window.lpfnWndProc = WndProc;
	window.cbClsExtra = 0;
	window.cbWndExtra = 0;
	window.hInstance = system->instance;
	window.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	window.hIconSm = window.hIcon;
	window.hCursor = LoadCursor(NULL, IDC_ARROW);
	window.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	window.lpszMenuName = NULL;
	window.lpszClassName = system->application_name;

	result = RegisterClassEx(&window);
	assert(result > 0);

	//
	// Now compute the window dimensions and position.
	//

	// Capture the primary monitor display size in pixels.
	display_monitor_w = GetSystemMetrics(SM_CXSCREEN);
	display_monitor_h = GetSystemMetrics(SM_CYSCREEN);

	// Define a rect for the application window dimensions.
	// For now, we place it in the top-left corner of the
	// window (this point is (0, 0)).
	desired_client_window = {
		0, 0, (long)screen_w, (long)screen_h
	};

	// This style specifies that the window cannot be resized nor
	// maximized.
	window_style =
		  (WS_OVERLAPPEDWINDOW)
		& (~WS_MAXIMIZEBOX)
		& (~WS_THICKFRAME)
		;

	// Adjust the window (position and size) to accomodate the desired
	// client window size. WS_OVERLAPPEDWINDOW says "treat these
	// dimensions as though they came from a window that can be minimized,
	// maximized, and has a thick window frame".
	AdjustWindowRect(&desired_client_window, window_style, FALSE);

	// Now compute the window dimensions.
	window_w = desired_client_window.right - desired_client_window.left;
	window_h = desired_client_window.bottom - desired_client_window.top;

	// Place the window in the middle of the screen.
	window_x = (display_monitor_w - window_w) / 2;
	if (window_x < 0) {
		window_x = 0;
	}

	window_y = (display_monitor_h - window_h) / 2;
	if (window_y < 0) {
		window_y = 0;
	}

	//
	// Finally, create the resulting window.
	//

	system->window_handler = CreateWindowEx(
		NULL,
		system->application_name,
		system->application_name,
		window_style,
		window_x,
		window_y,
		window_w,
		window_h,
		NULL,
		NULL,
		system->instance,
		NULL
	);

	assert(system->window_handler != NULL);
}

void run_system(system_handler* system) {
	MSG msg;
	bool peek_result;

	ShowWindow(system->window_handler, SW_SHOW);

	msg = {};

	while (msg.message != WM_QUIT) {
		peek_result = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);

		if (peek_result) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//
		// Process the application's next frame
		//

		frame(system->app);
	}
}

void shutdown_system(system_handler* system) {

	//
	// Shutdown the application
	// 

	if (system->app) {
		shutdown_application(system->app);
		delete system->app;
		system->app = NULL;
	}

	//
	// Shutdown and destroy the window.
	//

	ShowCursor(true);

	DestroyWindow(system->window_handler);
	system->window_handler = NULL;

	UnregisterClass(system->application_name, system->instance);
	system->instance = NULL;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
}