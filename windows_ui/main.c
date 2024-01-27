#define UNICODE
#define COBJMACROS
#define _CRT_SECURE_NO_DEPRECATE
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>
#include <stdio.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")

#define APP_TITLE L"GPIB-ENET"

#define Assert(expr) do { if (!expr) __debugbreak(); } while (0)

static int window_width = 800; 
static int window_height = 600; 

static HWND g_window;
static HWND g_groupbox1;
static HWND g_groupbox2;
static HWND g_button;

static LRESULT CALLBACK WinProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
	
	switch (message)
    {
	case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
		} break;
		
		case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(window, &ps);
			
            // All painting occurs here, between BeginPaint and EndPaint.
			
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
			
            EndPaint(window, &ps);
			return 0;
		}
		
		case WM_COMMAND: {
			
			if (HIWORD(wparam) == BN_CLICKED && (HWND)lparam == g_button) { 
			MessageBoxA(0, "Hello", "HI", 0);
			}
			
			return 0;
		}
		
    }
	
	return DefWindowProcW(window, message, wparam, lparam); // default way of handling incomming messages
}

#ifdef _DEBUG
//int WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR cmdline, int cmdshow)
int main()
#else
int WinMainCRTStartup()
#endif
{
	WNDCLASSEXW window_class = { 
		.cbSize = sizeof(window_class),
		.lpfnWndProc = WinProc,
		.hInstance = GetModuleHandleW(NULL), 
		.hIcon = LoadIcon(GetModuleHandleW(NULL), MAKEINTRESOURCE(1)),
		.lpszClassName = L"wcap_window_class",
	};
	
	RegisterClassExW(&window_class);
	
	g_window = CreateWindowExW(0, // optional window styles 
								  window_class.lpszClassName,
								  APP_TITLE, 
								  WS_OVERLAPPEDWINDOW, // window styles
								  CW_USEDEFAULT, CW_USEDEFAULT, window_width, window_height, 
								  NULL, NULL, window_class.hInstance, NULL);
    if (g_window == NULL) return 0;
	
	
	g_groupbox1 = CreateWindowEx(0, WC_BUTTON, 
							   L"GroupBox 1", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 
							   10, 10, 305, 460, g_window, NULL, NULL, NULL);
	g_groupbox2 = CreateWindowEx(0, WC_BUTTON, 
								 L"", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 
							   325, 10, 305, 460, g_window, NULL, NULL, NULL);
	g_button = CreateWindowEx(0, WC_BUTTON, 
							  L"Click Me", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
								 20, 20, 100, 20, g_window, NULL, NULL, NULL);
	
	
	if (ShowWindow(g_window, SW_SHOW) != 0) {
		printf("can not show thingys\n");
	}
	
	for (;;)
	{
		MSG Message;
		BOOL Result = GetMessageW(&Message, NULL, 0, 0);
		if (Result == 0) {
			ExitProcess(0);
		}
		Assert(Result > 0);
		
		TranslateMessage(&Message);
		DispatchMessageW(&Message);
	}
	return 0;
}
