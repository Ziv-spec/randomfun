#include <stdio.h>
#include <windows.h>

#define APP_TITLE L"GPIB-ENET"

#define Assert(expr) do { if (!expr) __debugbreak(); } while (0)

static int window_height = 800; 
static int window_width = 600; 


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
        
    }
	
	return DefWindowProcW(window, message, wparam, lparam); // default way of handling incomming messages
}

#if 0
int WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR cmdline, int cmdshow)
#else 
int main()
#endif
{
		//LPSTR cmdline = "astar"; 
		int cmdshow = 1;
		
	WNDCLASSEXW window_class = { 
		.cbSize = sizeof(window_class),
		.lpfnWndProc = WinProc,
		.hInstance = GetModuleHandleW(NULL), 
		.lpszClassName = L"wcap_window_class",
	};
	
	RegisterClassExW(&window_class);
	
	HWND window = CreateWindowExW(0, // optional window styles 
								  window_class.lpszClassName, 
								  APP_TITLE, 
								  WS_OVERLAPPEDWINDOW, // window styles
								  CW_USEDEFAULT, CW_USEDEFAULT, window_width, window_height, 
								  NULL, NULL, GetModuleHandleW(NULL), NULL);
    if (window == NULL) return 0;
	
	if (ShowWindow(window, cmdshow) != 0) {
		printf("can not show thingys\n");
	}
	
	for (;;)
	{
		MSG Message;
		BOOL Result = GetMessageW(&Message, NULL, 0, 0);
		if (Result == 0)
		{
			ExitProcess(0);
		}
		Assert(Result > 0);
		
		TranslateMessage(&Message);
		DispatchMessageW(&Message);
	}
	
	
}