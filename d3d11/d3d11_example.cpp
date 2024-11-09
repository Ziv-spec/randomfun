//#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <d3d11_1.h>
#include <dxgi1_3.h>
#include <dxgidebug.h>
#include <d3dcompiler.h>
//#include <gameinput.h>

#include <stdint.h>
#define _USE_MATH_DEFINES
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "d3dcompiler")
//#pragma comment(lib, "gameinput")

#define APP_TITLE "D3D11 application!!!"

// default starting width and height for the window
static int window_width = 800;
static int window_height = 600;

#define Assert(expr) do { if (!(expr)) __debugbreak(); } while (0)
#define AssertHR(hr) Assert(SUCCEEDED(hr))
#define ArrayLength(arr) (sizeof(arr) / sizeof(arr[0]))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define STR2(x) #x
#define STR(x) STR2(x)

typedef int64_t s64; 
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t  s8;
typedef int64_t u64; 
typedef int32_t u32;
typedef int16_t u16;
typedef int8_t  u8;
typedef int b32; 

// #define USE_GAMEINPUT

// Resources to look at:
// https://bgolus.medium.com/the-quest-for-very-wide-outlines-ba82ed442cd9 - the quest for wide outlines
// https://ameye.dev/notes/rendering-outlines/                       - 5 ways to draw an outline
// https://ameye.dev/notes/stylized-water-shader/                     - stylized water shader
// https://wwwtyro.net/2019/11/18/instanced-lines.html               - instanced line rendering
// https://w3.impa.br/~diego/projects/GanEtAl14/                     - massively parallel vector graphics (paper)
// https://www.3dgep.com/understanding-quaternions/                  - understanding quarternions
// https://gist.github.com/vurtun/d41914c00b6608da3f6a73373b9533e5   - camera gist for understanding all about cameras 
// https://lxjk.github.io/2016/10/29/A-Different-Way-to-Understand-Quaternion-and-Rotation.html
// https://www.youtube.com/watch?v=Jhopq2lkzMQ&list=PLplnkTzzqsZS3R5DjmCQsqupu43oS9CFN&index=1

/* 
* TODO(ziv):
* [x] Obj loading (with position, textures, normals)
* [x] Obj dynamic transformation
* [x] Obj dynamic lighting(global illumination + point light)
* [x] Texture mapping
	* [x] Camera
		* [x] Normal Camera
		* [x] Free Camera (The only thing left is free movement for which raw input/direct input needed
* [x] Face Culling
* [x] z-buffer
* [ ] Shadow Mapping
* [ ] Normal Mapping
* [ ] Input
* [ ] Obj Outlines
* [ ] Mouse screen to world projection
* [ ] Font Rendering (With real fonts and not a bitmap)
* [ ] General UI (This has many steps which I will detail when I will begin working on it)
* [ ] Update on Resize for fluid screen resize handling
			* [ ] Pixelated look

@IMPORTANT  These have a higher importance level right now
* [ ] Create a "Renderer" seperating d3d11
* [ ] Create a "Drawing" library using the renderer for effient simple object rendering
*/


static void FatalError(const char* message)
{
    MessageBoxA(NULL, message, "Error", MB_ICONEXCLAMATION);
    ExitProcess(0);
}

//~
// Math
//

struct matrix { float m[4][4]; };
struct float3 { float x, y, z; };

inline static float3 operator+=(const float3 v1, const float3 v2){ return float3{ v1.x+v2.x, v1.y+v2.y, v1.z+v2.z }; }
inline static float3 operator+(const float3 v1, const float3 v2) { return float3{ v1.x+v2.x, v1.y+v2.y, v1.z+v2.z }; }
inline static float3 operator-(const float3 v1, const float3 v2) { return float3{ v1.x-v2.x, v1.y-v2.y, v1.z-v2.z }; }
inline static float3 operator*(const float3 v1, const float3 v2) { return float3{ v1.x*v2.x, v1.y*v2.y, v1.z*v2.z }; }
inline static float3 operator*(const float3 v, const float c)    { return float3{ v.x*c, v.y*c, v.z*c }; }

inline static float3 
f3normalize(float3 v) {
	float len = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z); 
	float inv_len = 1/len; 
	return float3{ v.x*inv_len, v.y*inv_len, v.z*inv_len }; 
}

inline static float3 
f3cross(const float3& a, const float3& b) {
	return float3{
		a.y*b.z - a.z*b.y, 
		a.z*b.x - a.x*b.z, 
		a.x*b.y - a.y*b.x
	};
}

inline static float 
f3dot(const float3& a, const float3& b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline static matrix 
operator*(const matrix& m1, const matrix& m2)
{
    return {
        m1.m[0][0] * m2.m[0][0] + m1.m[0][1] * m2.m[1][0] + m1.m[0][2] * m2.m[2][0] + m1.m[0][3] * m2.m[3][0],
        m1.m[0][0] * m2.m[0][1] + m1.m[0][1] * m2.m[1][1] + m1.m[0][2] * m2.m[2][1] + m1.m[0][3] * m2.m[3][1],
        m1.m[0][0] * m2.m[0][2] + m1.m[0][1] * m2.m[1][2] + m1.m[0][2] * m2.m[2][2] + m1.m[0][3] * m2.m[3][2],
        m1.m[0][0] * m2.m[0][3] + m1.m[0][1] * m2.m[1][3] + m1.m[0][2] * m2.m[2][3] + m1.m[0][3] * m2.m[3][3],
        m1.m[1][0] * m2.m[0][0] + m1.m[1][1] * m2.m[1][0] + m1.m[1][2] * m2.m[2][0] + m1.m[1][3] * m2.m[3][0],
        m1.m[1][0] * m2.m[0][1] + m1.m[1][1] * m2.m[1][1] + m1.m[1][2] * m2.m[2][1] + m1.m[1][3] * m2.m[3][1],
        m1.m[1][0] * m2.m[0][2] + m1.m[1][1] * m2.m[1][2] + m1.m[1][2] * m2.m[2][2] + m1.m[1][3] * m2.m[3][2],
        m1.m[1][0] * m2.m[0][3] + m1.m[1][1] * m2.m[1][3] + m1.m[1][2] * m2.m[2][3] + m1.m[1][3] * m2.m[3][3],
        m1.m[2][0] * m2.m[0][0] + m1.m[2][1] * m2.m[1][0] + m1.m[2][2] * m2.m[2][0] + m1.m[2][3] * m2.m[3][0],
        m1.m[2][0] * m2.m[0][1] + m1.m[2][1] * m2.m[1][1] + m1.m[2][2] * m2.m[2][1] + m1.m[2][3] * m2.m[3][1],
        m1.m[2][0] * m2.m[0][2] + m1.m[2][1] * m2.m[1][2] + m1.m[2][2] * m2.m[2][2] + m1.m[2][3] * m2.m[3][2],
        m1.m[2][0] * m2.m[0][3] + m1.m[2][1] * m2.m[1][3] + m1.m[2][2] * m2.m[2][3] + m1.m[2][3] * m2.m[3][3],
        m1.m[3][0] * m2.m[0][0] + m1.m[3][1] * m2.m[1][0] + m1.m[3][2] * m2.m[2][0] + m1.m[3][3] * m2.m[3][0],
        m1.m[3][0] * m2.m[0][1] + m1.m[3][1] * m2.m[1][1] + m1.m[3][2] * m2.m[2][1] + m1.m[3][3] * m2.m[3][1],
        m1.m[3][0] * m2.m[0][2] + m1.m[3][1] * m2.m[1][2] + m1.m[3][2] * m2.m[2][2] + m1.m[3][3] * m2.m[3][2],
        m1.m[3][0] * m2.m[0][3] + m1.m[3][1] * m2.m[1][3] + m1.m[3][2] * m2.m[2][3] + m1.m[3][3] * m2.m[3][3],
    };
}

static matrix 
matrix_inverse_transpose(matrix A) {
	matrix r = {0};
	
	float determinant = 
		 +A.m[0][0]*(A.m[1][1]*A.m[2][2] - A.m[2][1]*A.m[1][2])
		-A.m[0][1]*(A.m[1][0]*A.m[2][2] - A.m[1][2]*A.m[2][0])
		+A.m[0][2]*(A.m[1][0]*A.m[2][1] - A.m[1][1]*A.m[2][0]);
		
	float invdet = 1/determinant;
	r.m[0][0] =  (A.m[1][1]*A.m[2][2] - A.m[2][1]*A.m[1][2])*invdet;
	r.m[1][0] = -(A.m[0][1]*A.m[2][2] - A.m[0][2]*A.m[2][1])*invdet;
	r.m[2][0] =  (A.m[0][1]*A.m[1][2] - A.m[0][2]*A.m[1][1])*invdet;
	r.m[0][1] = -(A.m[1][0]*A.m[2][2] - A.m[1][2]*A.m[2][0])*invdet;
	r.m[1][1] =  (A.m[0][0]*A.m[2][2] - A.m[0][2]*A.m[2][0])*invdet;
	r.m[2][1] = -(A.m[0][0]*A.m[1][2] - A.m[1][0]*A.m[0][2])*invdet;
	r.m[0][2] =  (A.m[1][0]*A.m[2][1] - A.m[2][0]*A.m[1][1])*invdet;
	r.m[1][2] = -(A.m[0][0]*A.m[2][1] - A.m[2][0]*A.m[0][1])*invdet;
	r.m[2][2] =  (A.m[0][0]*A.m[1][1] - A.m[1][0]*A.m[0][1])*invdet;
	
	return r;
}

inline static matrix 
matrix_transpose(matrix A) {
	matrix r = {
		A.m[0][0], A.m[1][0], A.m[2][0], A.m[3][0], 
		A.m[0][1], A.m[1][1], A.m[2][1], A.m[3][1], 
		A.m[0][2], A.m[1][2], A.m[2][2], A.m[3][2], 
		A.m[0][3], A.m[1][3], A.m[2][3], A.m[3][3], 
	};
	return r; 
}

static inline float 
float_clamp(float val, float min, float max) {
	float temp = MIN(val, max);
	return MAX(temp, min);
}

static matrix 
get_model_view_matrix(float3 rotation, float3 translation, float3 scale) {
#if 1
	matrix rx = { 
		1, 0,                0,                 0, 
		0, cosf(rotation.x), -sinf(rotation.x), 0, 
		0, sinf(rotation.x), cosf(rotation.x),  0, 
		0, 0,                0,                 1 
	};
	matrix ry = { 
		cosf(rotation.y), 0, sinf(rotation.y), 0, 
		0,                1, 0,                0, 
		-sinf(rotation.y),0, cosf(rotation.y), 0, 
		0,                0, 0,                1
	};
	matrix rz = { 
		cosf(rotation.z), -sinf(rotation.z), 0, 0, 
		sinf(rotation.z), cosf(rotation.z), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 
	};
	matrix scale_xyz = { 
		scale.x, 0, 0, 0,
			 0, scale.y, 0, 0, 
		0, 0, scale.z, 0, 
		0, 0, 0, 1 
	};
	matrix translate_xyz = { 
		1, 0, 0, 0, 
		0, 1, 0, 0, 
		0, 0, 1, 0, 
		translation.x, translation.y, translation.z, 1 
	};
	return rx * ry * rz * scale_xyz * translate_xyz;
#endif 
}


//~
// Input
//

typedef enum {
	MouseNone           = 0x00000000,
	MouseLeftButton     = 0x00000001,
	MouseRightButton    = 0x00000002,
	MouseMiddleButton   = 0x00000004,
	MouseButton4        = 0x00000008,
	MouseButton5        = 0x00000010,
	MouseWheelTiltLeft  = 0x00000020,
	MouseWheelTiltRight = 0x00000040
} Mouse_Buttons;

typedef struct {
	s64 px, py; // position detlta x, delta y
	s64 wx, wy; // wheel x,y
	Mouse_Buttons buttons;
} Mouse;

typedef enum {
	GamepadNone            = 0x00000000,
    GamepadMenu            = 0x00000001,
    GamepadView            = 0x00000002,
    GamepadA               = 0x00000004,
    GamepadB               = 0x00000008,
    GamepadX               = 0x00000010,
    GamepadY               = 0x00000020,
    GamepadDPadUp          = 0x00000040,
    GamepadDPadDown        = 0x00000080,
    GamepadDPadLeft        = 0x00000100,
    GamepadDPadRight       = 0x00000200,
    GamepadLeftShoulder    = 0x00000400,
    GamepadRightShoulder   = 0x00000800,
    GamepadLeftThumbstick  = 0x00001000,
    GamepadRightThumbstick = 0x00002000
} Gamepad_Buttons; 

typedef struct {
    Gamepad_Buttons buttons;
    float left_trigger;
    float right_trigger;
    float left_thumbstick_x;;
    float left_thumbstick_y;
    float right_thumbstick_x;
    float right_thumbstick_y;
} Gamepad;

typedef struct {
	//Keyboard keyboard;
	Mouse mouse;
	Gamepad gamepads[4]; 
} Game_Input;

static Game_Input g_raw_input_state;


// RAWINPUT implementation (should work on all windows pc's)

// Terms used by rawinput api
// HID - Human Interface Device

// In the RawInput API you must register the devices you 
// want to get data from. Then you can poll for data which 
// happens from WM_INPUT
//
// By default no application recieves rawinput. You must 
// register the device to begin recieving rawinput. 

static void 
InputInitialize(HWND window) {
	
	// Rawinput API
	{
		
		RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
	Rid[0].usUsage = 0x02;              // HID_USAGE_GENERIC_MOUSE
	Rid[0].dwFlags = 0;    // adds mouse and also ignores legacy mouse messages
	Rid[0].hwndTarget = window;
	
	if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE) {
		//registration failed. Call GetLastError for the cause of the error.
		FatalError("Rawinput couldn't register a mouse\n"); 
		}
		
	}
	
	
	// Gameinput
	{
		
	}
	
	
}

static bool
InputUpdate(LPARAM lparam) {
	
	UINT dwSize;
	
	GetRawInputData((HRAWINPUT)lparam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
	
	LPBYTE lpb = new BYTE[dwSize];
	if (lpb == NULL) {
		return 0; 
	} 
	
	if (GetRawInputData((HRAWINPUT)lparam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
		OutputDebugString(TEXT("GetRawInputData does not return correct size !\n")); 
		return 0;
	}
	
	RAWINPUT* raw = (RAWINPUT*)lpb;
	if (raw->header.dwType == RIM_TYPEMOUSE) {
		
		Mouse mouse = g_raw_input_state.mouse;
		if (raw->data.mouse.usButtonFlags == RI_MOUSE_WHEEL) {
			int delta_m = (int)raw->data.mouse.usButtonData; 
			//printf("wheel: %d\n", delta_m);
			
			mouse.px += raw->data.mouse.lLastX;
		mouse.py += raw->data.mouse.lLastY;
		if (raw->data.mouse.usButtonFlags == RI_MOUSE_WHEEL) {
			mouse.wy += (int)raw->data.mouse.usButtonData;
		}
		}
		
		int buttons = 0;
		switch (raw->data.mouse.ulButtons) {
			case RI_MOUSE_BUTTON_1_DOWN: buttons |= MouseLeftButton; break;
			case RI_MOUSE_BUTTON_1_UP:   buttons &= ~MouseLeftButton; break;
			case RI_MOUSE_BUTTON_2_DOWN: buttons |= MouseRightButton; break;
			case RI_MOUSE_BUTTON_2_UP:   buttons &= ~MouseRightButton; break;
			case RI_MOUSE_BUTTON_3_DOWN: buttons |= MouseMiddleButton; break;
			case RI_MOUSE_BUTTON_3_UP:   buttons &= ~MouseMiddleButton; break;
			}
			
		mouse.buttons = (Mouse_Buttons)buttons;
		g_raw_input_state.mouse = mouse;

	delete[] lpb; 
	}
	
	return 1;
}

static Game_Input
InputGetState() {
	return g_raw_input_state;
}

static void
InputShutdown() {
	
	// Rawinput
	{
	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
	Rid[0].usUsage = 0x02;              // HID_USAGE_GENERIC_MOUSE
	Rid[0].dwFlags = RIDEV_REMOVE;      // adds mouse and also ignores legacy mouse messages
	Rid[0].hwndTarget = 0;
	
	if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE)
	{
		//registration failed. Call GetLastError for the cause of the error.
		FatalError("Couldn't register a mouse\n"); 
	}
	}
	
	
}

#if 0
// GAMEINPUT implementation (for some reason doesn't work on my pc)

IGameInput *g_gameinput = 0; 
IGameInputDevice *g_gamepads[4]; // game supports upto 4 gamepads
IGameInputDevice *g_mouse = 0;

static HRESULT initialize_input() {
	HRESULT result = GameInputCreate(&g_gameinput); 
	return result;
}

static void shutdown_input() {
	
	for (int i = 0; i < ArrayLength(g_gamepads); i++) {
		if (g_gamepads[i]) { g_gamepads[i]->Release(); }
	}
	
	if (g_gameinput) {
		g_gameinput->Release();
	}
}

static void poll_gameinput(Game_Input *input) {
	// NOTE(ziv): GameInput is "Input-Centric" API. It finds the types 
	// of input the user is interested in and then optionally query 
	// the device from which it came from. 
	
	// Ask for the latest reading from devices that provide fix-format
	// gamepad state. If a device has beend assigned to g_gamepad, filter
	// readings to just the ones coming from that device. Otherwise, if 
	// g_gamepad is null, it will allow readings from any device.
	
	IGameInputReading *reading;
	for (int i = 0; i < ArrayLength(g_gamepads); i++) {
		
		HRESULT success = g_gameinput->GetCurrentReading(GameInputKindGamepad, g_gamepads[i], &reading);
		if (SUCCEEDED(success)) {
			
			// If not device has been assigned to g_gamepad yet, set it
			// to the first device we recieve input from. (This must be 
			// the one the player is using because it's generating input.)
			if (!g_gamepads[i]) {
				reading->GetDevice(&g_gamepads[i]); 
			}
			
			if (i > 0 && g_gamepads[i] == g_gamepads[i-1]) {
				g_gamepads[i] = 0; 
				reading->Release();
				break;
			}
			
			// Retrive the fixed-format gamepad state from the reading
			GameInputGamepadState state; 
			if (reading->GetGamepadState(&state)) { 
				
				// Application specific code to process the gamepad state goes here: 
				input->gamepads[i].buttons = (Gamepad_Buttons)state.buttons;
				input->gamepads[i].left_trigger  = state.leftTrigger;
				input->gamepads[i].right_trigger = state.rightTrigger;
				
				float threshold = 0.2f;
#define THRESHOLD_INPUT(val, t) (-t > val || val > t) ? val : 0
				input->gamepads[i].left_thumbstick_x = THRESHOLD_INPUT(state.leftThumbstickX, threshold); 
				input->gamepads[i].left_thumbstick_y = THRESHOLD_INPUT(state.leftThumbstickY, threshold); 
				input->gamepads[i].right_thumbstick_x = THRESHOLD_INPUT(state.rightThumbstickX, threshold); 
				input->gamepads[i].right_thumbstick_y = THRESHOLD_INPUT(state.rightThumbstickY, threshold); 
				
				reading->Release();
			}
			
		}
		else { 
			if (g_gamepads[i]) {
				g_gamepads[i]->Release(); 
				g_gamepads[i] = 0;
			}
			//printf("Couldn't get any readings from gamepad\n"); 
			break;
		}
		
		
	}
	
	//
	// Mouse Input
	// 
	
	HRESULT success = g_gameinput->GetCurrentReading(GameInputKindMouse, g_mouse, &reading);
	if (!SUCCEEDED(success)) {
		if (g_mouse) {
			g_mouse->Release(); 
			g_mouse= 0;
		}
		return; 
	}
	
	if (!g_mouse) {
		reading->GetDevice(&g_mouse); 
	}
	
	GameInputMouseState mouse_state;
	if (reading->GetMouseState(&mouse_state)) {
		input->mouse.buttons = (Mouse_Buttons)mouse_state.buttons; 
		input->mouse.px = mouse_state.positionX; 
		input->mouse.py = mouse_state.positionY;
		input->mouse.wx = mouse_state.wheelX;
		input->mouse.wy = mouse_state.wheelY;
		
#if 0
		printf("mouse input "); 
		printf("%d, %d,  %d, %d | %d\n", (int)input->mouse.px, (int)input->mouse.px,
			   (int)mouse_state.wheelX, (int)mouse_state.wheelY, input->mouse.buttons);
#endif 
		
		reading->Release();
	}
	
}
#endif 


static bool key_w; // up 
static bool key_s; // down
static bool key_a; // right
static bool key_d; // left

static bool key_space; 
static bool key_ctrl; 
static bool key_tab; 
static bool key_tab_pressed;  // this will escape into free camera mode

static bool key_up; 
static bool key_down;
static bool key_right;
static bool key_left;


static int mouse_pos[2];

bool show_free_camera = false; 

static LRESULT CALLBACK WinProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
	
	switch (message)
    {
		case WM_DESTROY: {
			PostQuitMessage(0);
			return 0;
		} break;
		
		case WM_KEYDOWN: 
		{
			if ((char)wparam == 'W') { key_w = true; }
			if ((char)wparam == 'S') { key_s = true; }
			if ((char)wparam == 'A') { key_a = true; }
			if ((char)wparam == 'D') { key_d = true; }
			
			if ((char)wparam == VK_UP)    { key_up = true; }
			if ((char)wparam == VK_DOWN)  { key_down = true; }
			if ((char)wparam == VK_RIGHT) { key_right = true; }
			if ((char)wparam == VK_LEFT)  { key_left = true; }
			
			if ((char)wparam == VK_SPACE)   { key_space = true; }
			if ((char)wparam == VK_CONTROL) { key_ctrl = true; }
			
			if (wparam == VK_TAB) { key_tab = true; }
		} break; 
		
		case WM_KEYUP: 
		{
			key_w = (char)wparam == 'W' ? false : key_w;
			key_s = (char)wparam == 'S' ? false : key_s;
			key_a = (char)wparam == 'A' ? false : key_a;
			key_d = (char)wparam == 'D' ? false : key_d;
			
			key_up    = (char)wparam == VK_UP ? false : key_up;
			key_down  = (char)wparam == VK_DOWN ? false : key_down;
			key_right = (char)wparam == VK_RIGHT ? false : key_right;
			key_left  = (char)wparam == VK_LEFT ? false : key_left;
			
			key_space  = (char)wparam == VK_SPACE ? false : key_space;
			key_ctrl   = (char)wparam == VK_CONTROL ? false : key_ctrl;
			key_tab    = (wparam == VK_TAB) ? false : key_tab;
			
			if (wparam == VK_TAB) key_tab_pressed = true;
			
		} break;
		
		case WM_MOUSEMOVE: {
			POINTS p = MAKEPOINTS(lparam);
			mouse_pos[0] = p.x; mouse_pos[1] = p.y;
		} break;
		
		
		
		//
		// Rawinput 
		//
		
		case WM_INPUT: {
			
			if (!InputUpdate(lparam)) {
				FatalError("Program ran out of memory!");
				return 0; 
			}
			
		} break; 
		
	}
	return DefWindowProcA(window, message, wparam, lparam);
}

//~
// Camera
//
// Resources:
// [1] https://learnopengl.com/Getting-started/Camera
// [2] https://gist.github.com/vurtun/d41914c00b6608da3f6a73373b9533e5
//

typedef struct {
	
	/// In
	// camera
	float3 pos;
	float pitch, yaw;
	
	// projection
	float fov;  // field of view
	float n, f; // near, far plaines
	float aspect_ratio;
	
	/// Out
	matrix view; // obj world->view space
	matrix proj; // obj view->screen space
	matrix norm; // normals world->view
	matrix view_inv;
	matrix proj_inv;
	
	// movement vectors
	float3 right, left;
	float3 up, down; 
	float3 forward, backward;
} Camera; 

static void 
CameraInit(Camera *c) {
	c->fov = 0.25f*(float)M_PI;
	c->n = .01f; 
	c->f = 1000.f; 
	
	c->proj = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
	c->view = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
}

static void 
CameraBuild(Camera *c) {
	Assert(c);

	// Build the lookat matrix 
	float3 some_up_vector = { 0, 1, 0 }; 

	// The lookat matrix is a matrix created from a 'single' vector 
	// pointing in the forward direction of the player looking at the 
	// object we want to point the camera towards. 
	// In this case I create the forward vector using the pitch and yaw
	// values and from it I compute the space in which the camera lies. 
	// The view matrix created from this space is the inverse translation
	// and inverse rotations done by the camera itself [2].

	// this camera is pointing in the inverse direction of it's target
	float3 fv = f3normalize({ cosf(c->yaw)*cosf(c->pitch), sinf(c->pitch), sinf(c->yaw)*cosf(c->pitch) });
	float3 rv = f3normalize(f3cross(some_up_vector, fv)); 
	float3 uv = f3normalize(f3cross(fv, rv)); 

	float3 p = c->pos;

	matrix cmat = { 
		rv.x, uv.x, fv.x, 0,
		rv.y, uv.y, fv.y, 0,
		rv.z, uv.z, fv.z, 0,
		-(rv.x*p.x+rv.y*p.y+rv.z*p.z), -(uv.x*p.x+uv.y*p.y+uv.z*p.z), -(fv.x*p.x+fv.y*p.y+fv.z*p.z), 1
	};

	c->view = cmat; 
	c->view_inv = matrix_transpose(cmat);

	c->forward.x = c->view_inv.m[2][0];
	c->forward.y = c->view_inv.m[2][1];
	c->forward.z = c->view_inv.m[2][2];

	c->backward.x = -c->view_inv.m[2][0];
	c->backward.y = -c->view_inv.m[2][1];
	c->backward.z = -c->view_inv.m[2][2];

	c->right.x = c->view_inv.m[0][0];
	c->right.y = c->view_inv.m[0][1];
	c->right.z = c->view_inv.m[0][2];

	c->left.x = -c->view_inv.m[0][0];
	c->left.y = -c->view_inv.m[0][1];
	c->left.z = -c->view_inv.m[0][2];

	c->up.x = c->view_inv.m[1][0];
	c->up.y = c->view_inv.m[1][1];
	c->up.z = c->view_inv.m[1][2];

	c->down.x = -c->view_inv.m[1][0];
	c->down.y = -c->view_inv.m[1][1];
	c->down.z = -c->view_inv.m[1][2];



	// TODO(ziv): @IMPORTANT Fix the projection matrix that I use in here 
    // Build the projection matrix 

	// In this case the projection matrix which we are building 
	// is mapping object's in the players thrustum inside a 
	// -1 to 1 cordinate space [2].
	float hfov = 1.0f/tanf(c->fov*0.5f); 
		
	c->proj = matrix{0}; 
	c->proj.m[0][0] = 1.0f/(c->aspect_ratio*hfov);
	c->proj.m[1][1] = 1.0f/hfov; 
	c->proj.m[2][3] = -1.0f;

	 // cn = -1 and cf = 1:
	c->proj.m[2][2] = -(c->f + c->n) / (c->f - c->n);
	c->proj.m[3][2] = -(2.0f * c->f * c->n) / (c->f - c->n);

	/* 	
	 //cn = 0 and cf = 1: 
	c->proj.m[2][2] = -(c->f) / (c->f - c->n);
	c->proj.m[3][2] = -(c->f * c->n) / (c->f - c->n);

	 //cn = -1 and cf = 0:
	c->proj.m[2][2] = (c->n) / (c->n - c->f);
	c->proj.m[3][2] = (c->f * c->n) / (c->n - c->f);
    */

	// Inverse of the Projection Matrix
	memset(c->proj_inv.m, 0, sizeof(c->proj_inv.m));
	c->proj_inv.m[0][0] = 1.0f/c->proj.m[0][0];
	c->proj_inv.m[1][1] = 1.0f/c->proj.m[1][1];
	c->proj_inv.m[2][3] = 1.0f/c->proj.m[3][2];
	c->proj_inv.m[3][2] = 1.0f/c->proj.m[2][3];
	c->proj_inv.m[3][3] = -c->proj.m[2][2];
	c->proj_inv.m[3][3] /= (c->proj.m[3][2] * c->proj.m[2][3]);
}

static void 
CameraMove(Camera *c, float x, float y, float z) {
	// Here we use the right up and forward vectors aligned to 
	// the camera space. We do so such that every movement we 
	// let the player have is one which respects the camera 
	// viewing angle.
	
	// NOTE(ziv): Player movement is defined here
	// Currently using first person shooter movement.
	float cy = c->pos.y;
	
	c->pos = c->pos + c->right * x; 
	c->pos = c->pos + c->up * y; 
	c->pos = c->pos + c->forward * z; 
	
	if (1) { 
		// in a first person shooter camera we don't change the players
		// y position because he looked in that direction. Only when the 
		// player requrests.
		c->pos.y = cy;
	}
}


//~
// Win32 API
//

static HWND 
CreateWin32Window() {
	
	HINSTANCE instance_handle = GetModuleHandleW(NULL);
	
	WNDCLASSEXA window_class = {0};
	window_class.cbSize = sizeof(window_class);
	window_class.lpfnWndProc = WinProc;
	window_class.hInstance = instance_handle ;
	window_class.hIcon = LoadIcon(instance_handle, MAKEINTRESOURCE(1));
	window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	window_class.lpszClassName = "d3d11 example";
	
	ATOM atom = RegisterClassExA(&window_class); 
	Assert(atom && "Could not register class"); 
	
	HWND window = CreateWindowExA(CS_VREDRAW | CS_HREDRAW, 
					  window_class.lpszClassName, 
					  APP_TITLE, 
					  WS_OVERLAPPEDWINDOW,
					  CW_USEDEFAULT, CW_USEDEFAULT, window_width, window_height, 
					  NULL, NULL, window_class.hInstance, NULL);
	Assert(window && "Failed to create a window"); 
	
	return window;
}

//~
// D3D11 Renderer
// 


// API 
#define D3D11_BUFFER_SIZE       0x100
#define D3D11_PIXELSHADER_SIZE  0x100
#define D3D11_VERTEXSHADER_SIZE 0x100
#define D3D11_LAYOUT_SIZE       0x100

typedef struct { int x; } R_Buffer; 
typedef struct { int x; } R_PixelShader; 
typedef struct { int x; } R_VertexShader; 

typedef enum { 
	// two bits
	BF_USAGE_IMMUTABLE = 1 << 0,
	BF_USAGE_DYNAMIC   = 1 << 1,
	// two bits
	BF_CPU_ACESS_WRITE = 1 << 2,
	BF_CPU_ACESS_READ  = 1 << 3, 
	// three bits
	BF_VERTEX_BUFFER   = 1 << 4,
	BF_INDEX_BUFFER    = 1 << 5,
	BF_CONSTNAT_BUFFER = (1 << 4) | (1 << 5),
} Buffer_Flag;

typedef struct {
	int aaaaa;
} Renderer_Command;

typedef struct {
	HWND window;
	
	ID3D11Device1 *device; 
	ID3D11DeviceContext1*context;
	IDXGISwapChain1* swap_chain;
	ID3D11RenderTargetView *frame_buffer_view; 
	
	ID3D11DepthStencilState* depth_stencil_state;
	ID3D11DepthStencilView *zbuffer;
	ID3D11Texture2D *zbuffer_texture; 
	ID3D11RasterizerState1* rasterizer_cull_back;
	
	
	// Data uploaded to the GPU already would get reused through 
	// handles to the data (essentially this will be the user faceing info)
	struct { 
		R_Buffer handle;
		ID3D11Buffer *buf;
	} buffers[D3D11_BUFFER_SIZE];
	size_t buffer_count;
	
	struct { 
		R_VertexShader handle;
		ID3D11InputLayout *layout;
		ID3D11VertexShader *vs;
	} vertex_shaders[D3D11_VERTEXSHADER_SIZE];
	size_t vertex_shader_count;
	
	struct { 
		R_PixelShader handle;
		ID3D11PixelShader *ps;
	} pixel_shaders[D3D11_PIXELSHADER_SIZE];
	size_t pixel_shader_count;
	
	} Renderer;

// Initialization and DeInitialization
static void RendererCreate(Renderer *r, HWND window); 
static void RendererDestroy(Renderer *r);
static void RendererResize(Renderer *r, unsigned int width, unsigned int height); 


// Resource Aquasition (Sending data to GPU)
static R_Buffer RendererCreateBuffer(Renderer *r, const void *data, u32 size, u16 flags); 
static ID3D11Buffer *RendererGetBufferFromHandle(Renderer *r, R_Buffer handle);

static R_VertexShader RendererCreateVertexShader(Renderer *r, const char *hlsl, D3D11_INPUT_ELEMENT_DESC format[], // TODO(ziv): This is d3d11 specific for a unified API generalize this 
												 u8 flags); 

static R_PixelShader RendererCreatePixelShader(Renderer *r, const char *hlsl, u8 flags); 

// Drawing Functions
static void RendererDraw(Renderer *r, R_Buffer vbuffer, R_VertexShader vs, R_PixelShader ps, R_Buffer cbuffer, u8 flags);



// 
// Implementation
// 

struct Vertex {
	float pos[3];
	float norm[3];
	float uv[2];
};

static void 
RendererCreate(Renderer *r, HWND window) {
	
	//
	// Create D3D11 Device used for resource creation and Device Context for pipline setup and rendering
	//
	
	HRESULT hr; 
	ID3D11Device1 *device; 
	ID3D11DeviceContext1*context;
	{ 

		UINT flags = 0; 
		
#if _DEBUG
		// this enables VERY USEFUL debug messages in debugger output
		flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif 

		D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

		ID3D11Device* base_device;
		ID3D11DeviceContext* base_context;
		
		hr = D3D11CreateDevice(NULL, // Default adapter
				   D3D_DRIVER_TYPE_HARDWARE,  // Use GPU
				   NULL, // Software renderer if specified to use CPU
				   flags | D3D11_CREATE_DEVICE_BGRA_SUPPORT, 
				   featureLevels, ARRAYSIZE(featureLevels), 
				   D3D11_SDK_VERSION, &base_device, NULL, &base_context);
		AssertHR(hr); 
		
		base_device->QueryInterface(__uuidof(ID3D11Device1), (void **)&device);
		base_device->Release();
		base_context->QueryInterface(__uuidof(ID3D11DeviceContext1), (void **)&context);
		base_context->Release();
	}
	
#if _DEBUG
	// for debug builds enable VERY USEFUL debug break on API errors
    {
        ID3D11InfoQueue* info;
        device->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&info);
        info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
        info->Release();
    }
	
    // enable debug break for DXGI too
    {
        IDXGIInfoQueue* dxgi_info;
        hr = DXGIGetDebugInterface1(0, __uuidof(IDXGIInfoQueue), (void**)&dxgi_info);
        AssertHR(hr);
        dxgi_info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        dxgi_info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
        dxgi_info->Release();
    }

    // after this there's no need to check for any errors on device functions manually
    // so all HRESULT return  values in this code will be ignored
    // debugger will break on errors anyway
#endif

	// Create D3D11 Swap Chain
	IDXGISwapChain1* swap_chain;
	{

		IDXGIDevice1* dxgiDevice;
		device->QueryInterface(__uuidof(IDXGIDevice1), (void **)&dxgiDevice);

		IDXGIAdapter* dxgiAdapter;
		dxgiDevice->GetAdapter(&dxgiAdapter);
		dxgiDevice->Release();

		IDXGIFactory2* dxgiFactory;
		dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void **)&dxgiFactory);
		dxgiAdapter->Release(); 

		//

		DXGI_SWAP_CHAIN_DESC1 swap_chain_desc;
		swap_chain_desc.Width              = 0; // use window width
		swap_chain_desc.Height             = 0; // use window height
		swap_chain_desc.Format             = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		swap_chain_desc.Stereo             = FALSE;
		swap_chain_desc.SampleDesc.Count   = 1;
		swap_chain_desc.SampleDesc.Quality = 0;
		swap_chain_desc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swap_chain_desc.BufferCount        = 2;
		swap_chain_desc.Scaling            = DXGI_SCALING_STRETCH;
		swap_chain_desc.SwapEffect         = DXGI_SWAP_EFFECT_DISCARD;
		swap_chain_desc.AlphaMode          = DXGI_ALPHA_MODE_UNSPECIFIED;
		swap_chain_desc.Flags              = 0;

		dxgiFactory->CreateSwapChainForHwnd(device, window, &swap_chain_desc, NULL, NULL, &swap_chain);
		dxgiFactory->Release();
	}

	// Create a Target View
	ID3D11RenderTargetView *frame_buffer_view; 
	{
		ID3D11Texture2D* frame_buffer;
		swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&frame_buffer);

		D3D11_RENDER_TARGET_VIEW_DESC frame_buffer_desc = {};
		frame_buffer_desc.Format        = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; // ... so do this to get _SRGB swapchain (rendertarget view)
		frame_buffer_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		
		device->CreateRenderTargetView(frame_buffer, &frame_buffer_desc, &frame_buffer_view);
		frame_buffer->Release();
	}

	// 
	// Zbuffer and BackFace Culling are provided by default and disabled/enabled by the user
	// 

	// Create Depth Sentcil
	ID3D11DepthStencilState* depth_stencil_state;
	{
		D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = {};
		depth_stencil_desc.DepthEnable    = TRUE;
		depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depth_stencil_desc.DepthFunc      = D3D11_COMPARISON_LESS;

		device->CreateDepthStencilState(&depth_stencil_desc, &depth_stencil_state);
	}

	// Create Z-Buffer
	ID3D11DepthStencilView *zbuffer;
	ID3D11Texture2D *zbuffer_texture; 
	{
		D3D11_TEXTURE2D_DESC depth_buffer_desc = {};

		depth_buffer_desc.Width = window_height;
		depth_buffer_desc.Height = window_width;
		depth_buffer_desc.MipLevels = 1;
		depth_buffer_desc.ArraySize = 1;
		depth_buffer_desc.Format = DXGI_FORMAT_D32_FLOAT;
		depth_buffer_desc.SampleDesc.Count = 1;
		depth_buffer_desc.SampleDesc.Quality = 0;
		depth_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		depth_buffer_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		device->CreateTexture2D(&depth_buffer_desc, NULL, &zbuffer_texture);

		D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = {}; 
		depth_stencil_view_desc.Format = DXGI_FORMAT_D32_FLOAT;
		depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depth_stencil_view_desc.Texture2D.MipSlice = 0; 
		device->CreateDepthStencilView(zbuffer_texture, &depth_stencil_view_desc, &zbuffer);
	}

	// Create a Rasterizer
	ID3D11RasterizerState1* rasterizer_cull_back;
	{
		D3D11_RASTERIZER_DESC1 rasterizer_desc = {};
		rasterizer_desc.FillMode = D3D11_FILL_SOLID; // D3D11_FILL_WIREFRAME;
		rasterizer_desc.CullMode = D3D11_CULL_BACK;

		device->CreateRasterizerState1(&rasterizer_desc, &rasterizer_cull_back);

		// NOTE(ziv): For shadowmap it will be useful to have a rasterizer which will cull the front triangles
		//rasterizer_desc.CullMode = D3D11_CULL_FRONT;
		//device->CreateRasterizerState1(&rasterizer_desc, &rasterizer_cull_back);
	}

	// Set all d3d11 com objects
	r->window = window;
	r->device = device; 
	r->context = context; 
	r->swap_chain = swap_chain; 
	r->frame_buffer_view = frame_buffer_view; 
	r->depth_stencil_state = depth_stencil_state; 
	r->zbuffer = zbuffer; 
	r->zbuffer_texture = zbuffer_texture; 
	r->rasterizer_cull_back = rasterizer_cull_back; 
}

static void 
RendererDestroy(Renderer *r) {
	r->device->Release();
	r->context->Release();
	r->swap_chain->Release();
	r->frame_buffer_view->Release(); 
	r->depth_stencil_state->Release();
	r->zbuffer->Release();
	r->zbuffer_texture->Release(); 
	r->rasterizer_cull_back->Release();
	
	// TODO(ziv): Deallocate all user allocated resources
}

static void 
RendererResize(Renderer *r, unsigned int width, unsigned int height) {
	Assert(r); 
	
	if (r->frame_buffer_view) {
		r->context->ClearState(); 
		r->frame_buffer_view->Release(); 
		r->frame_buffer_view = NULL;
		
		r->zbuffer_texture->Release(); 
		r->zbuffer->Release(); 
	}
	
	HRESULT hr;
	if (width != 0 && height != 0) {
		hr = r->swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
		if (FAILED(hr)) FatalError("Failed to resize the swap chain!");
		
		// Create a new RenderTarget for the new backbuffer texture
		ID3D11Texture2D *backbuffer; 
		r->swap_chain->GetBuffer(0, __uuidof(ID3D11Resource), (void **)&backbuffer);
		r->device->CreateRenderTargetView(backbuffer, NULL, &r->frame_buffer_view);
		backbuffer->Release();
		
		// Create Z-Buffer
		D3D11_TEXTURE2D_DESC depth_buffer_desc = {};
		
		depth_buffer_desc.Width = width;
		depth_buffer_desc.Height = height;
		depth_buffer_desc.MipLevels = 1;
		depth_buffer_desc.ArraySize = 1;
		depth_buffer_desc.Format = DXGI_FORMAT_D32_FLOAT;
		depth_buffer_desc.SampleDesc.Count = 1;
		depth_buffer_desc.SampleDesc.Quality = 0;
		depth_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		depth_buffer_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		r->device->CreateTexture2D(&depth_buffer_desc, NULL, &r->zbuffer_texture);
		
		D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = {}; 
		depth_stencil_view_desc.Format = DXGI_FORMAT_D32_FLOAT;
		depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depth_stencil_view_desc.Texture2D.MipSlice = 0; 
		r->device->CreateDepthStencilView(r->zbuffer_texture, &depth_stencil_view_desc, &r->zbuffer);
	}
	
}

static R_Buffer 
RendererCreateBuffer(Renderer *r, const void *data, u32 size, u16 flags) {
	Assert((flags & 0xfc) && "Must supply the type of buffer and it's usage type");
	
	static D3D11_USAGE usage[] = {  D3D11_USAGE_DEFAULT, D3D11_USAGE_IMMUTABLE, D3D11_USAGE_DYNAMIC, };
	static D3D11_BIND_FLAG bind[] = { D3D11_BIND_VERTEX_BUFFER, D3D11_BIND_INDEX_BUFFER, D3D11_BIND_CONSTANT_BUFFER, };
	static D3D11_CPU_ACCESS_FLAG cpu_access[] = { (D3D11_CPU_ACCESS_FLAG)0, D3D11_CPU_ACCESS_WRITE, D3D11_CPU_ACCESS_READ };
	
	ID3D11Buffer *buffer;
	{
		D3D11_BUFFER_DESC descriptor = {}; 
		descriptor.ByteWidth = (size + 0xf) & 0xffffff0;
		descriptor.Usage = usage[(u8)flags & 0x3];
		descriptor.BindFlags = bind[((u8)(flags >> 4) & 0x7)-1];
		descriptor.CPUAccessFlags = cpu_access[((u8)flags >> 2) & 0x3];
		
		if (data) {
		D3D11_SUBRESOURCE_DATA buffer_data  = { data };
			r->device->CreateBuffer(&descriptor, &buffer_data, &buffer);
		}
		else {
			r->device->CreateBuffer(&descriptor, NULL, &buffer);
		}
		
	}
	
	// The user will use the resource through it's handle
	// Format:    0000 0000 0000 kkkk iiii iiii iiii iiii
	// i - index, k - kind
	
	Assert(r->buffer_count < D3D11_BUFFER_SIZE); 
	
	u32 idx = (u16)r->buffer_count;
	u32 kind = flags >> 4;
	R_Buffer handle = { (kind << 16) | (idx) };
	
	// Copy of the handle is used to know if data has changed
	r->buffers[r->buffer_count++] = { handle, buffer };
	
	// TODO(ziv): Make allocation of new buffers smarter
	// Currently I just allocate statically a buffer and 
	// I don't deallocate or do anything to free resources
	// Since the option of adding smarter allocation should 
	// be easy and a smart move forward, implement a nice 
	// system for allocation/deallocation of these slots
	
	return handle; 
}

static ID3D11Buffer *
RendererGetBufferFromHandle(Renderer *r, R_Buffer handle) {
	Assert(r);
	Assert((handle.x >> 4) > 0 && "Invalid Buffer!!! Must contain a kind");
	
	int idx = handle.x & 0xffff; 
	Assert(0 <= idx && idx <= D3D11_BUFFER_SIZE); 
	
	ID3D11Buffer *result = NULL; 
	if (handle.x == r->buffers[idx].handle.x) {
	result = r->buffers[idx].buf;
	}
	return result;
}

static R_VertexShader 
RendererCreateVertexShader(Renderer *r, const char *hlsl, u32 length, const D3D11_INPUT_ELEMENT_DESC format[], u32 format_elem_count, u8 flags) {
	Assert(r && hlsl && format && format_elem_count > 0); 
	
    HRESULT hr; 
	ID3D11InputLayout  *layout = NULL;
	ID3D11VertexShader *vshader = NULL; 
	{
		
        UINT flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#ifdef _DEBUG
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
		ID3DBlob *error;
		ID3DBlob *vblob;
		hr = D3DCompile(hlsl, length, NULL, NULL, NULL, "vs_main", "vs_5_0", flags, 0, &vblob, &error);
		if (FAILED(hr)) {
			const char *message = (const char *)error->GetBufferPointer(); 
			OutputDebugStringA(message); 
			Assert(!"Failed to compile pertex shader!");
		}
		r->device->CreateVertexShader(vblob->GetBufferPointer(), vblob->GetBufferSize(), NULL, &vshader);
		
		r->device->CreateInputLayout(format, format_elem_count, 
									 vblob->GetBufferPointer(), vblob->GetBufferSize(), &layout);
		
		vblob->Release();
	}
	
	
	
	// TODO(ziv): Improve allocation and deallocation scheme here too 
	u16 idx = (u16)r->vertex_shader_count;
	R_VertexShader handle = { idx }; 
	
	r->vertex_shaders[r->vertex_shader_count++] = { handle, layout, vshader };
	
	return handle;
}

static ID3D11VertexShader*
RendererGetVertexShaderFromHandle(Renderer *r, R_VertexShader handle) {
	Assert(r);
	
	int idx = handle.x & 0xffff; 
	Assert(0 <= idx && idx <= D3D11_VERTEXSHADER_SIZE); 
	
	ID3D11VertexShader *result = NULL; 
	if (handle.x == r->vertex_shaders[idx].handle.x) {
		result = r->vertex_shaders[idx].vs;
	}
	return result;
}

static ID3D11InputLayout*
RendererGetLayoutFromHandle(Renderer *r, R_VertexShader handle) {
	Assert(r);
	
	int idx = handle.x & 0xffff; 
	Assert(0 <= idx && idx <= D3D11_VERTEXSHADER_SIZE); 
	
	ID3D11InputLayout  *result = NULL; 
	if (handle.x == r->vertex_shaders[idx].handle.x) {
		result = r->vertex_shaders[idx].layout;
	}
	return result;
}

static R_PixelShader 
RendererCreatePixelShader(Renderer *r, const char *hlsl, u32 length, u8 flags) {
	
	HRESULT hr; 
	ID3D11PixelShader *pshader;
	{
		
        UINT flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#ifdef _DEBUG
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
		ID3DBlob *error;
		ID3DBlob *pblob;
	hr = D3DCompile(hlsl, length, NULL, NULL, NULL, "ps_main", "ps_5_0", flags, 0, &pblob, &error);
	if (FAILED(hr)) {
		const char *message = (const char *)error->GetBufferPointer(); 
		OutputDebugStringA(message); 
		Assert(!"Failed to compile pixel shader!");
	}
	r->device->CreatePixelShader(pblob->GetBufferPointer(), pblob->GetBufferSize(), NULL, &pshader);
	pblob->Release(); 
	}
	
	
	// TODO(ziv): Improve allocation and deallocation scheme here too 
	
	u16 idx = (u16)r->pixel_shader_count;
	R_PixelShader handle = { idx }; 
	
	r->pixel_shaders[r->pixel_shader_count++] = { handle, pshader };
	
	return handle;
}

static ID3D11PixelShader*
RendererGetPixelShaderFromHandle(Renderer *r, R_PixelShader handle) {
	Assert(r);
	
	int idx = handle.x & 0xffff; 
	Assert(0 <= idx && idx <= D3D11_PIXELSHADER_SIZE); 
	
	ID3D11PixelShader *result = NULL; 
	if (handle.x == r->pixel_shaders[idx].handle.x) {
		result = r->pixel_shaders[idx].ps;
	}
	return result;
}

static void 
RendererDraw(Renderer *r, R_Buffer vbuffer, R_VertexShader vs, R_PixelShader ps, R_Buffer cbuffer, u8 flags) {
	Assert((vbuffer.x >> 16) == 1);
	Assert((cbuffer.x >> 16) == 3);
	
	ID3D11Buffer *verticies = RendererGetBufferFromHandle(r, vbuffer);
	ID3D11Buffer *constant_buffer = RendererGetBufferFromHandle(r, cbuffer);
	ID3D11InputLayout *layout = RendererGetLayoutFromHandle(r, vs);
	ID3D11VertexShader *vshader = RendererGetVertexShaderFromHandle(r, vs);
	ID3D11PixelShader *pshader = RendererGetPixelShaderFromHandle(r, ps);
	
	RECT rect; 
	GetClientRect(r->window, &rect);
	LONG width = rect.right - rect.left;
	LONG height = rect.bottom - rect.top;
	
	D3D11_VIEWPORT viewport = {0};
	viewport.Width = (FLOAT)width; 
	viewport.Height = (FLOAT)height;
	viewport.MaxDepth = 1;
	
    // Input Assembler
    r->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    r->context->IASetInputLayout(layout);
    const UINT stride = 2*sizeof(float); 
    const UINT offset = 0;
    r->context->IASetVertexBuffers(0, 1, &verticies, &stride, &offset);
    
    // Vertex Shader
    r->context->VSSetShader(vshader, NULL, 0);
    
    // Pixel Shader
    r->context->PSSetShader(pshader, NULL, 0);
    r->context->PSSetConstantBuffers(0, 1, &constant_buffer); 
    
    // Rasterizer
    r->context->RSSetViewports(1, &viewport);
    
    // Output Merger
    r->context->OMSetRenderTargets(1, &r->frame_buffer_view, 0);
    
    r->context->Draw(4,0);
	
}


/*
typedef enum {
	RTF_SRGB      = 1 << 0, 
	RTF_IMMUTABLE = 1 << 1, 
} R_Texture_Flags;

typedef struct {
	unsigned int width; 
	unsigned int height;
	ID3D11ShaderResourceView *tex;
} R_Texture; 


static R_Texture 
RendererGetTexture(Renderer *r, const char *bytes, unsigned int width, unsigned int height, unsigned char flags) {
	// Upload the texture to the GPU and keep the resource around
	ID3D11ShaderResourceView *texture_view;
	
	// TODO(ziv): Allow user to use mipmapping with the texture
	D3D11_TEXTURE2D_DESC texture_desc = {};
	texture_desc.Width              = width;
	texture_desc.Height             = height;
	texture_desc.MipLevels          = 1;
	texture_desc.ArraySize          = 1;
	texture_desc.Format             = (flags & RTF_SRGB) ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : (DXGI_FORMAT)0;
	texture_desc.SampleDesc.Count   = 1;
	texture_desc.Usage              = (flags & RTF_IMMUTABLE) ? D3D11_USAGE_IMMUTABLE : (D3D11_USAGE)0;
	texture_desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
	
	D3D11_SUBRESOURCE_DATA texture_data = {};
	texture_data.pSysMem = bytes;
	texture_data.SysMemPitch = 4*width; // TODO(ziv): Stop assuming all textures are SRGB
	
	ID3D11Texture2D* texture;
	r->device->CreateTexture2D(&texture_desc, &texture_data, &texture);
	r->device->CreateShaderResourceView(texture, NULL, &texture_view);
	texture->Release();
	
	R_Texture result = { width, height, texture_view };
	return result;
}

static void
RendererDestroyTexture(R_Texture *t) {
	t->tex->Release(); 
}
*/








typedef struct { int minx, miny, maxx, maxy; } Box; 
typedef struct { float r, g, b, a; } Color;
typedef struct { Box box, Color; } DrawQuadCommand; 
static Color RED      = { 1, 0, 0, 1 };
static Color LIGHTRED = { 1, .1f, .1f, 1};
static Color LIGHTERRED = { 1, .3f, .3f, 1};

typedef struct { 
	Renderer *renderer; 
	
	// For Quad Rendering
	R_VertexShader vshader; 
	R_PixelShader pshader; 
	R_Buffer vertex_buffer; 
	R_Buffer constant_buffer;
	
} Draw_Context; 

static void 
DrawBegin(Draw_Context *ctx, Renderer *r) { 
    
    float sz = .5;
    float verticies[] = {
		-1, -1,  -1, 1,
		1, -1,   1, 1,
    };
	
    static char hlsl[] = 
        "#line " STR(__LINE__)                                "\n"
        "                                                      \n"
        "float4 vs_main(float2 p : Position) : SV_Position {   \n" 
        "    return float4(p, 0., 1.);                         \n"
        "}                                                     \n"
        "                                                      \n" 
        "cbuffer DrawColor {                                   \n" 
        "    float4 color;                                     \n" 
        "};                                                    \n" 
        "                                                      \n" 
        "float4 ps_main(float4 p : SV_Position) : SV_Target {  \n" 
        "    return color;                                     \n"
        "}                                                     \n";
	
	R_Buffer vbuffer = RendererCreateBuffer(r, verticies, sizeof(verticies), BF_VERTEX_BUFFER | BF_USAGE_DYNAMIC | BF_CPU_ACESS_WRITE);
	R_Buffer cbuffer = RendererCreateBuffer(r, &RED, sizeof(Color), BF_CONSTNAT_BUFFER | BF_USAGE_DYNAMIC | BF_CPU_ACESS_WRITE);
	
	// Vertex Shader Fromat Descriptor
	const D3D11_INPUT_ELEMENT_DESC format[] = {
		{ "Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(struct Vertex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	
	R_VertexShader vs = RendererCreateVertexShader(r, hlsl, sizeof(hlsl), format, ARRAYSIZE(format), 0);
	R_PixelShader ps = RendererCreatePixelShader(r, hlsl, sizeof(hlsl), 0);
	
	// TODO(ziv): This should change for sure
	ctx->renderer = r;
	ctx->vshader = vs; 
	ctx->pshader = ps; 
	ctx->vertex_buffer = vbuffer;
	ctx->constant_buffer = cbuffer;
	
	}
    
static void 
DrawEnd(Draw_Context *ctx) {
    // Take all common commands and draw instanced
    // RendererDrawInstanced()
	
}

static void 
DrawBox(Draw_Context *ctx, Box box, Color color) { 
    // Post a drawquad command
	
	RECT rect; 
	GetClientRect(ctx->renderer->window, &rect);
	float width = (float)(rect.right - rect.left);
	float height = (float)(rect.bottom - rect.top);
	
	// Don't draw if the box is not only the screen
	if (!(0 < box.maxx && 0 < box.maxy && box.minx < width && box.miny < height)) {
		return;
	}
	
	float minx = (2*box.minx / width) -1; 
	float miny = (2*box.miny / height)-1;
	float maxx = (2*box.maxx / width) -1; 
	float maxy = (2*box.maxy / height)-1;
	
	float verticies[] = {
		minx, miny, minx, maxy, 
		maxx, miny, maxx, maxy, 
    };
	
	// TODO(ziv): Make a renderer wrapping for this changing of buffer data dynamically.
	ID3D11Buffer *vbuffer = RendererGetBufferFromHandle(ctx->renderer, ctx->vertex_buffer); 
	ID3D11Buffer *cbuffer = RendererGetBufferFromHandle(ctx->renderer, ctx->constant_buffer);
	Renderer *r = ctx->renderer;
	
	D3D11_MAPPED_SUBRESOURCE mapped;
	r->context->Map((ID3D11Resource *)vbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, &verticies, sizeof(verticies)); 
	r->context->Unmap((ID3D11Resource *)vbuffer, 0);
	
		r->context->Map((ID3D11Resource *)cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		memcpy(mapped.pData, &color, sizeof(Color)); 
	r->context->Unmap((ID3D11Resource *)cbuffer, 0);
	
	RendererDraw(ctx->renderer, 
				 ctx->vertex_buffer, 
				 ctx->vshader, 
				 ctx->pshader, 
				 ctx->constant_buffer, 0); 
}


//~
// UI
// 

typedef enum {
	UI_HOVERABLE = 1,   // can be hovered
	UI_CLICKABLE = 2,   // can be clicked 
    UI_SLIDERABLE = 4,  // slider property (also allows for mouse movement)
    UI_INPUTABLE = 8,   // accepts keyboard input
} UI_Behaviour; 

typedef struct {
	bool clicked : 1; 
	bool hovered : 1; 
    float slider_value; 
} UI_Output;

typedef struct { 
	u16 behaviour;
	Box box; 
} UI_Widget; 

typedef int UI_Widget_Handle; 

#define WIDGETS_COUNT 0x400

typedef struct {
	Game_Input *input;
	
	Draw_Context *draw_ctx;
	
    //UI_Widget widgets[WIDGETS_COUNT];
    UI_Widget_Handle last;
    UI_Widget_Handle hot, active; 
} UI_Context; 

#define UI_INVALID_WIDGET -1

static UI_Output
UIBuildWidget(UI_Context *ctx, Box box, u16 behaviour) {
	
	UI_Output output;
    Mouse mouse = ctx->input->mouse;
    UI_Widget_Handle handle = ctx->last; 
	
	
    // Find if mouse is inside UI hit-box
    if (box.minx <= mouse.px && 
            mouse.px <= box.maxx && 
            box.miny <= mouse.py && 
            mouse.py <= box.maxy) { 

        // widget is hot
        if ((behaviour & UI_HOVERABLE) && ctx->hot == UI_INVALID_WIDGET) { 
            ctx->hot = handle; 
            output.hovered = 1;
        }
		
        // widget is active?
        if ((behaviour & UI_CLICKABLE) && 
                ctx->active == UI_INVALID_WIDGET && 
                ctx->hot == handle && 
                (mouse.buttons & MouseLeftButton)) {
            ctx->active = handle; 
            output.clicked = 1;
			}

        // widget slider behaviour
        if ((behaviour & UI_SLIDERABLE)) {

        }
    } 

    ctx->last++; 
    Assert(ctx->last < WIDGETS_COUNT); 

	return output;
}

static bool
UIButton(UI_Context *ctx, int x, int y, int w, int h) {
    
	// Input handling
	Box box = { x, y, x+w, y+h };
	UI_Output output = UIBuildWidget(ctx, box, UI_HOVERABLE | UI_CLICKABLE); 

    // Drawing 
    Color color = output.hovered ? LIGHTRED : RED;
    color = output.clicked ? LIGHTERRED : color;
	
	//printf("button draw\n");
	
	 DrawBox(ctx->draw_ctx, box, color);

	return output.clicked;
}


//~
// Object File Loader 
// 

static float ObjParseFloat(char* str, size_t *length) {
	float num = 0.0, mul = 1.0;
	int len = 0, dec = 0;
	
	while (str[len] == ' ' || str[len] == '\n') len++;
	
	if (str[len] == '-') len++;
	while (str[len] && (('0' <= str[len] && str[len] <= '9') ||  str[len] == '.')) if (str[len++] == '.') dec = 1;
	
	for (int idx = len - 1; idx >= 0; idx--)
	{
		char chr = str[idx] - '0';
			
		if      (chr == '-' - '0') num = -num;
		else if (chr == '.' - '0') dec = 0; 
		else if (dec)
		{
		    num += chr;
		    num *= 0.1f;
		}
		else
		{
		    num += chr * mul;
		    mul *= 10.0;
		}
	}
	
	*length = len;
	return num; 
}

static unsigned int ObjParseUINT(char *str, size_t *length) {
	unsigned int len = 0, result = 0;
	
	while (str[len] == ' ' && str[len] != '\0') len++; 
	
	while (((unsigned int)str[len] - '0') < 10) {
		result = 10*result + str[len++]-'0';
	}
	
	*length = len;
	return result; 
}


static bool ObjLoadFile(char *path, Vertex *vdest, size_t *v_cnt, unsigned short *idest, size_t *i_cnt) {
	
	if (v_cnt == NULL || i_cnt == NULL) 
		return false;
	
	// Read .obj file
	char *obj_buf;
	{
		HANDLE file = CreateFileA(path,
					  GENERIC_READ,          // open for reading
					  FILE_SHARE_READ,       // share for reading
					  NULL,                  // default security
					  OPEN_EXISTING,         // existing file only
					  FILE_ATTRIBUTE_NORMAL, // normal file
					  NULL);
		Assert(file != INVALID_HANDLE_VALUE); // TODO(ziv): Make it trip the assertion when the file just doesn't exist
		
		DWORD file_size = GetFileSize(file, NULL);
		obj_buf = (char *)malloc((file_size+1) * sizeof(char));
		
		DWORD bytes_read_cnt;
		bool success = ReadFile(file, obj_buf, file_size, &bytes_read_cnt, NULL);
		if (!success || file_size != bytes_read_cnt) {
			return false;
		}
		
		obj_buf[file_size] = '\0'; // NULL terminate the string
		
		// TODO(ziv): Close the file? 
	}

	int verticies_pos_count = 0; 
	int indicies_count = 0; 
	int normals_count = 0; 
	int texture_cords_count = 0; 
	
	// count amount of verticies and indicies needed
	{
		
		char *s = obj_buf; 
		while (*s) {
			
			if (*s == 'v') {
				s++;
				if (*s == ' ') verticies_pos_count++; 
				else if (*s == 'n') normals_count++; 
				else if (*s == 't') texture_cords_count++; 
			}
			else if (*s == 'f') {
				s++;
				indicies_count += 3; // 3 indicies per face
			}
			
			while(*s != '\0' && *s++ != '\n');
		}
		
	}
	
	// when there is no output buffer, give the user the sizes of buffers required
	if (idest == NULL || vdest == NULL) {
		*v_cnt = indicies_count; 
		*i_cnt = indicies_count; 
		return true;
	}
	
	float *shared = (float *)malloc( 1+(3*verticies_pos_count+3*normals_count+2*texture_cords_count)*sizeof(float) );
	float *normals_buff = shared;
	float *uv_buff      = normals_buff + (3*normals_count); 
	float *pos_buff     = uv_buff + (2*texture_cords_count);
	int pos_idx = 0, uv_idx = 0, norm_idx = 0;
	unsigned short idx = 0;
	
	Vertex *v = vdest; 
	
	// parse obj file
	{
		size_t len = 0;
		char *s = obj_buf;
		while (*s) {

			if (*s == 'v') {
				s++;
				if (*s == ' ') {
					s++;
					pos_buff[pos_idx++] = ObjParseFloat(s, &len); s+=len+1;
					pos_buff[pos_idx++] = ObjParseFloat(s, &len); s+=len+1;
					pos_buff[pos_idx++] = ObjParseFloat(s, &len); s+=len;
				}
				else if (*s == 't') {
					s+=2;
					uv_buff[uv_idx++] = ObjParseFloat(s, &len); s+=len+1;
					uv_buff[uv_idx++] = ObjParseFloat(s, &len); s+=len;
				}
				else if (*s == 'n') {
					s++; s++;
					normals_buff[norm_idx++] = ObjParseFloat(s, &len); s+=len+1;
					normals_buff[norm_idx++] = ObjParseFloat(s, &len); s+=len+1;
					normals_buff[norm_idx++] = ObjParseFloat(s, &len); s+=len;
				}
				
			}
			else if (*s == 'f') {
				s++;

				unsigned int vi, vt, vn;
				for (int i = 0; i < 3; i++) {
					vi = ObjParseUINT(s, &len); s+=len+1;
					vt = ObjParseUINT(s, &len); s+=len+1;
					vn = ObjParseUINT(s, &len); s+=len+1;
					
					memcpy(&v->pos, &pos_buff[(vi-1)*3], 3*sizeof(float));
					memcpy(&v->norm,&normals_buff[(vn-1)*3], 3*sizeof(float));
					memcpy(&v->uv,  &uv_buff[(vt-1)*2], 2*sizeof(float));
					// Search for existing vertex for it's index
					unsigned short match_index = -1;
					if (idx > 0) { 
						for (int j = 0; j < idx; j++) {
							if (memcmp(&vdest[j], v, sizeof(Vertex)) == 0) {
								match_index = j;
								break;
							}
						}
					}
					
					if (match_index == (unsigned short)-1) { v++; *idest++ = idx++; }
					else { *idest++ = match_index; }
				}
				s--;
				
			}
			
			while(*s != '\0' && *s++ != '\n');
		}
		
	}

	// update new vertex count
	*v_cnt = idx;
	return true;
}




#ifdef _DEBUG
int main()
#else
int WINAPI WinMain(HINSTANCE instance, HINSTANCE previouse, LPSTR CmdLine, int ShowCmd)
#endif
{
	
	// Typical WIN32 Window creation
	HWND window = CreateWin32Window();
	
	// Renderer Initialization
	Renderer r = {0};
	RendererCreate(&r, window);
	
	InputInitialize(window);
	

	// Initialize drawing library for Quad rendering
	Draw_Context ctx = {0};
	DrawBegin(&ctx, &r); 

	
	
	
	//~
	// Model Data
	//
	
	HRESULT hr; 
	char model_path[] = "../resources/cube.obj";
	
	size_t verticies_count, indicies_count; 
	bool success = ObjLoadFile(model_path,NULL, &verticies_count, NULL, &indicies_count); 
	Assert(success && "Failed extracting buffer sizes for vertex and index buffers");
	
	Vertex *verticies = (Vertex *)malloc(verticies_count*sizeof(Vertex)); 
	unsigned short *indicies = (unsigned short *)malloc(indicies_count*sizeof(unsigned short));
	success = ObjLoadFile(model_path, verticies, &verticies_count, indicies, &indicies_count); 
	Assert(success && "Failed extracting model data");
	
	// Create Vertex And Index Buffers
	ID3D11Buffer *vertex_buffer, *index_buffer;
	{
		D3D11_BUFFER_DESC vertex_descriptor = {}; 
		vertex_descriptor.ByteWidth = (UINT)verticies_count*sizeof(Vertex);
		vertex_descriptor.Usage = D3D11_USAGE_DEFAULT;
		vertex_descriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		D3D11_SUBRESOURCE_DATA vertex_data  = { verticies };
		r.device->CreateBuffer(&vertex_descriptor, &vertex_data, &vertex_buffer);
		
		D3D11_BUFFER_DESC index_descriptor = {};
		index_descriptor.ByteWidth = (UINT)indicies_count*sizeof(unsigned short); 
		index_descriptor.StructureByteStride = sizeof(unsigned short); // TODO(ziv): check whether I will need to move to 32 bit
		index_descriptor.Usage = D3D11_USAGE_DEFAULT;
		index_descriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;
		D3D11_SUBRESOURCE_DATA index_data = { indicies };
		r.device->CreateBuffer(&index_descriptor, &index_data, &index_buffer);
	}
	
	// Create Vertex And Pixel Shaders
	ID3D11InputLayout  *layout = NULL;
	ID3D11VertexShader *vshader = NULL; 
	ID3D11PixelShader  *pshader = NULL; 
	{
		
		// Vertex Shader Fromat Descriptor
		const D3D11_INPUT_ELEMENT_DESC vs_input_desc[] = {
			{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct Vertex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "Normal",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct Vertex, norm), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "Texture",  0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(struct Vertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		
        UINT flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#ifdef _DEBUG
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
		ID3DBlob *error;
		ID3DBlob *vblob;
		hr = D3DCompileFromFile(L"../shaders.hlsl", NULL, NULL, "vs_main", "vs_5_0", flags, 0, &vblob, &error);
		if (FAILED(hr)) {
			const char *message = (const char *)error->GetBufferPointer(); 
			OutputDebugStringA(message); 
			Assert(!"Failed to compile vertex shader!");
		}
		r.device->CreateVertexShader(vblob->GetBufferPointer(), vblob->GetBufferSize(), NULL, &vshader);
		
		r.device->CreateInputLayout(vs_input_desc, ARRAYSIZE(vs_input_desc), 
								  vblob->GetBufferPointer(), vblob->GetBufferSize(), &layout);
		
		
		ID3DBlob *pblob;
		hr = D3DCompileFromFile(L"../shaders.hlsl", NULL, NULL, "ps_main", "ps_5_0", flags, 0, &pblob, &error);
		if (FAILED(hr)) {
			const char *message = (const char *)error->GetBufferPointer(); 
			OutputDebugStringA(message); 
			Assert(!"Failed to compile pixel shader!");
		}
		r.device->CreatePixelShader(pblob->GetBufferPointer(), pblob->GetBufferSize(), NULL, &pshader);
		
		vblob->Release(); 
		pblob->Release();
	}
	
	struct VSConstantBuffer { 
		matrix transform; 
		matrix projection; 
		matrix normal_transform;
		float3 lightposition;
	};
	
	// Vertex shader constant buffer 
	ID3D11Buffer *cbuffer; 
	{
		D3D11_BUFFER_DESC cbuffer_descriptor = {};
		cbuffer_descriptor.ByteWidth = (sizeof(VSConstantBuffer) + 0xf) & 0xffffff0;
		cbuffer_descriptor.Usage = D3D11_USAGE_DYNAMIC;
		cbuffer_descriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbuffer_descriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		r.device->CreateBuffer(&cbuffer_descriptor, NULL, &cbuffer); 
	}
	
	struct PSConstantBuffer { 
		float3 point_light_position;
		float3 sun_light_direction;
	};
	
	// Pixel shader constant buffer 
	ID3D11Buffer *ps_constant_buffer; 
	{
		D3D11_BUFFER_DESC cbuffer_descriptor = {};
		cbuffer_descriptor.ByteWidth = (sizeof(PSConstantBuffer) + 0xf) & 0xffffff0;
		cbuffer_descriptor.Usage = D3D11_USAGE_DYNAMIC;
		cbuffer_descriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbuffer_descriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		r.device->CreateBuffer(&cbuffer_descriptor, NULL, &ps_constant_buffer); 
	}
	
	// Create Sampler State
	ID3D11SamplerState* sampler_state;
	{
		D3D11_SAMPLER_DESC sampler_desc = {};
		sampler_desc.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT;
		sampler_desc.AddressU       = D3D11_TEXTURE_ADDRESS_BORDER;
		sampler_desc.AddressV       = D3D11_TEXTURE_ADDRESS_BORDER;
		sampler_desc.AddressW       = D3D11_TEXTURE_ADDRESS_BORDER;
		sampler_desc.BorderColor[0] = 1.0f;
		sampler_desc.BorderColor[1] = 1.0f;
		sampler_desc.BorderColor[2] = 1.0f;
		sampler_desc.BorderColor[3] = 1.0f;
		sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			
		r.device->CreateSamplerState(&sampler_desc, &sampler_state);
	}

	// Texture
	ID3D11ShaderResourceView *texture_view;
	{
		// Load Image
		int tex_w, tex_h, tex_num_channels;
		unsigned char* bytes = stbi_load("../resources/test.png", &tex_w, &tex_h, &tex_num_channels, 4);
		Assert(bytes);
		int pitch = 4 * tex_w;
			
		D3D11_TEXTURE2D_DESC texture_desc = {};
		texture_desc.Width              = tex_w;
		texture_desc.Height             = tex_h;
		texture_desc.MipLevels          = 1;
		texture_desc.ArraySize          = 1;
		texture_desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		texture_desc.SampleDesc.Count   = 1;
		texture_desc.Usage              = D3D11_USAGE_IMMUTABLE;
		texture_desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
		
		D3D11_SUBRESOURCE_DATA texture_data = {};
		texture_data.pSysMem = bytes;
		texture_data.SysMemPitch = pitch;
		
		ID3D11Texture2D* texture;
		r.device->CreateTexture2D(&texture_desc, &texture_data, &texture);
		r.device->CreateShaderResourceView(texture, NULL, &texture_view);
		
		texture->Release();
		free(bytes);
	}

	ShowWindow(window, SW_SHOW);

	//~
	// Main Game Loop
	//
	
	LONG curr_width = window_width, curr_height = window_height;
	D3D11_VIEWPORT viewport = {0};
	viewport.Width = (FLOAT)window_width; 
	viewport.Height = (FLOAT)window_height;
	viewport.MaxDepth = 1;
	
	
	FLOAT background_color[4] = { 0.025f, 0.025f, 0.025f, 1.0f };
	
	// projection matrix variables
	float w = viewport.Width / viewport.Height; // width (aspect ratio)
	float h = 1.0f;                             // height
	float n = 1.0f;                             // near
	float f = 90.0f;                            // far

	float3 model_rotation    = { 0.0f, 0.0f, 0.0f };
	float3 model_scale       = { 1.5f, 1.5f, 1.5f };
	float3 model_translation = { 0.0f, 0.0f, 4.0f };
    
	// global directional light
	float3 sun_direction = { 0, 0, 1 }; 
	// point light
	float3 lightposition = {  0, 0, 2 };

	// TODO(ziv): MOVE THIS CODE!!!
	RECT rcClip;           // new area for ClipCursor
	RECT rcOldClip;        // previous area for ClipCursor
	// Record the area in which the cursor can move. 
	GetClipCursor(&rcOldClip); 
	// Get the dimensions of the application's window. 
	GetWindowRect(window, &rcClip); 


	// more things that I need I guess...
	LARGE_INTEGER freq, start_frame, end_frame;
	QueryPerformanceFrequency(&freq); 
	QueryPerformanceCounter(&start_frame); 
	int last_mouse_pos[2] = {window_width/2, window_height/2};
	
	
	// Camera 
	Camera c = {0};
	CameraInit(&c);
	c.aspect_ratio = (float)window_width/(float)window_height;
	c.pos.z -= 5;
	
#ifdef USE_GAMEINPUT
	initialize_input();
#endif 
	
	for (;;) {
		
		// 
		// Handle Input
		//
		
		MSG msg; 
		while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				goto release_resources;
			}
			
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
		
		Game_Input input = InputGetState();
		
#ifdef USE_GAMEINPUT
		Game_Input input = {0}; 
		poll_gameinput(&input); 

		for (int i = 0; i < ArrayLength(input.gamepads); i++) {
			Gamepad gamepad = input.gamepads[i];
			if (gamepad.buttons & GamepadA) printf("gamepad button a\n");
		}

		mouse_pos[0] = (int)input.mouse.px; 
		mouse_pos[1] = (int)input.mouse.py; 
#endif 

		//
		// Handle window resize
		//
		
		RECT rect; 
		GetClientRect(window, &rect);
		LONG width = rect.right - rect.left;
		LONG height = rect.bottom - rect.top;
		if (width != (LONG)viewport.Width || height != (LONG)viewport.Height) {
			RendererResize(&r, width, height); 
			
			// TODO(ziv): Remove viewport from here
			viewport.Width = (FLOAT)width; 
			viewport.Height = (FLOAT)height;
			c.aspect_ratio = viewport.Width/viewport.Height;
		}
		
		
		//
		// Enable/Disable Free Camera Mode
		//
		
		// Update Mouse Clip area
		GetWindowRect(window, &rcClip);
		
		if (key_tab_pressed) {
			show_free_camera = !show_free_camera; 
			
			if (show_free_camera) {
				// Confine the cursor to the application's window. 
				Assert(ClipCursor(&rcClip));
				bool success = ShowCursor(false);
				Assert(success); 
			}
			else {
				// Restore the cursor to its previous area. 
				ClipCursor(&rcOldClip);
				ShowCursor(true);
			}
		}
		key_tab_pressed = false;
		
			
		
		
		//~
		// Update Game State
		//
		
		QueryPerformanceCounter(&end_frame);
		float dt =  (float)((double)(end_frame.QuadPart - start_frame.QuadPart) / (double)freq.QuadPart);
		
		// Update Camera
		float dx = (float)(mouse_pos[0] - last_mouse_pos[0]);
		float dy = (float)(last_mouse_pos[1] - mouse_pos[1]); // NOTE(ziv): flipped y axis so up is positive
		
		float speed = 5;
		
		for (int i = 0; i < 4; i++) {
			dx += input.gamepads[i].right_thumbstick_x*speed;
			dy += input.gamepads[i].right_thumbstick_y*speed;
		}
		
		c.yaw   = fmodf(c.yaw - dx/window_width*2*3.14f, (float)(2*M_PI));; 
		c.pitch = float_clamp(c.pitch + dy/window_height, -(float)M_PI/2.f, (float)M_PI/2.f); 
			
		CameraMove(&c, (key_d-key_a)*dt*speed, 0, (key_w-key_s)*dt*speed); 
		CameraBuild(&c); 

		float3 translate_vector = { -c.view.m[3][0], -c.view.m[3][1], -c.view.m[3][2] };
		
		// Don't render when minimized
		if (width == 0 && height == 0) {
			Sleep(16); continue;
		}
		
		
		// 
		// Render Game
		//
		
		// ===== RendererBeginFrame
		
		// Update model-view matrix
		{
			matrix model_view_matrix = get_model_view_matrix(model_rotation, model_translation, model_scale) * c.view;
			
			// fov
			float w = viewport.Width / viewport.Height; // width (aspect ratio)
			matrix projection = { 
				2 * n / w, 0, 0, 0,
				0, 2 * n / h, 0, 0, 
				0, 0, f / (f - n), 1, 0, 
				0, n * f / (n - f), 0  
			};
			
			
			// Vertex Contstant Buffer
			VSConstantBuffer vs_cbuf; 
			vs_cbuf.transform        = model_view_matrix;  
			vs_cbuf.projection       = projection; 
			vs_cbuf.normal_transform = matrix_inverse_transpose(model_view_matrix);
			
			D3D11_MAPPED_SUBRESOURCE mapped;
			r.context->Map((ID3D11Resource *)cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			memcpy(mapped.pData, &vs_cbuf, sizeof(vs_cbuf)); 
			r.context->Unmap((ID3D11Resource *)cbuffer, 0);
			
			// Pixel Contstant Buffer
			PSConstantBuffer ps_cbuf; 
			ps_cbuf.point_light_position = lightposition - translate_vector;
			ps_cbuf.sun_light_direction = sun_direction;
			
			D3D11_MAPPED_SUBRESOURCE ps_mapped;
			r.context->Map((ID3D11Resource *)ps_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ps_mapped);
			memcpy(ps_mapped.pData, &ps_cbuf, sizeof(ps_cbuf)); 
			r.context->Unmap((ID3D11Resource *)ps_constant_buffer, 0);
		}
		
		
		r.context->ClearRenderTargetView(r.frame_buffer_view, background_color);
		r.context->ClearDepthStencilView(r.zbuffer, D3D11_CLEAR_DEPTH, 1.0f, 0);
		
		// Input Assembler
		r.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		r.context->IASetInputLayout(layout);
		const UINT stride = sizeof(Vertex); 
		const UINT offset = 0;
		r.context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
		r.context->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R16_UINT, 0);
		
		// Vertex Shader
		r.context->VSSetShader(vshader, NULL, 0);
		r.context->VSSetConstantBuffers(0, 1, &cbuffer);
		
		// Rasterizer Stage
		r.context->RSSetViewports(1, &viewport);
		r.context->RSSetState(r.rasterizer_cull_back);
		
		// Pixel Shader
		r.context->PSSetShader(pshader, NULL, 0);
		r.context->PSSetConstantBuffers(0, 1, &ps_constant_buffer); 
		r.context->PSSetShaderResources(0, 1, &texture_view);
		r.context->PSSetSamplers(0, 1, &sampler_state);
		
		// Output Merger
		r.context->OMSetDepthStencilState(r.depth_stencil_state, 0);
		r.context->OMSetRenderTargets(1, &r.frame_buffer_view, r.zbuffer);
		
		r.context->DrawIndexed((UINT)indicies_count, 0, 0); 
		
		
		// NOTE(ziv): Testing for UI & API purposes
		 // 

		static int val = 10;

        DrawBox(&ctx, { val, 10, 100+val, 100 }, RED);
		DrawBox(&ctx, { 110, 10, 200, 200 }, LIGHTRED);
		DrawBox(&ctx, { 210, 10, 300, 300 }, LIGHTRED);

		UI_Context ui = { }; 
		ui.hot = UI_INVALID_WIDGET;
		ui.active = UI_INVALID_WIDGET;
		ui.draw_ctx = &ctx;
		ui.input = &input;
		
		input.mouse.px = mouse_pos[0];
		input.mouse.py = height-mouse_pos[1];
		
		//printf("mp %lld:%lld\n", input.mouse.px, input.mouse.py);
		
		bool clicked = UIButton(&ui, 400, 10, 100, 100);
		if (clicked) {
			printf("button clicked\n");
		}
		val++;
		
		
		
		// ===== RendererEndFrame
		
		// present the backbuffer to the screen
		r.swap_chain->Present(1, 0);
		
		
		// end of frame
		last_mouse_pos[0] = mouse_pos[0]; last_mouse_pos[1] = mouse_pos[1]; 
		start_frame = end_frame; // update time for dt calc
	}
	
	
	release_resources:
	
	DrawEnd(&ctx);
	
	InputShutdown(); // rawinput
	
#ifdef USE_GAMEINPUT
	shutdown_input(); 
#endif 
	
	index_buffer->Release();
	vertex_buffer->Release();
	cbuffer->Release();
	ps_constant_buffer->Release();
	texture_view->Release();
	sampler_state->Release();
	layout->Release();
	pshader->Release();
	vshader->Release();
	
	RendererDestroy(&r);
	
	return 0; 
}

/*

//
// Font Atlas 
//
#define CHARACTER_COUNT   95

#define ATLAS_WIDTH      475
#define ATLAS_HEIGHT       7

static UINT atlas[] = {
	0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff,
	0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff,
	0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
	0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
	0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff,
	0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
	0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff,
	0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
	0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
	0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff,
	0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
	0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff,
	0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff,
	0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff,
	0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff,
	0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff,
	0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff,
	0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff,
	0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff,
	0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
	0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff,
	0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff,
	0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
	0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
	0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
	0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff,
	0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff,
	0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
	0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff,
	0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff,
	0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
};



//~
// Text Data
// 

// Font Constant Buffer
ID3D11Buffer* font_constant_buffer;
{
	// one-time calc here to make it easier for the shader later (float2 rn_screensize, r_atlassize)
	float font_constant_data[4] = { 
		2.0f / window_width,
		-2.0f / window_height, 
		1.0f / ATLAS_WIDTH, 
		1.0f / ATLAS_HEIGHT 
	}; 
	
	D3D11_BUFFER_DESC constant_buffer_desc = {};
	constant_buffer_desc.ByteWidth = sizeof(font_constant_data) + 0xf & 0xfffffff0; // ensure constant buffer size is multiple of 16 bytes
	constant_buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE; // constant buffer doesn't need updating in this example
	constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	
	D3D11_SUBRESOURCE_DATA font_data = { font_constant_data };
	device->CreateBuffer(&constant_buffer_desc, &font_data, &font_constant_buffer);
}

// Texture Atlas
ID3D11ShaderResourceView* atlas_resource_view;
{
    D3D11_TEXTURE2D_DESC texture_atlas_desc = {};
    texture_atlas_desc.Width             = ATLAS_WIDTH;
    texture_atlas_desc.Height            = ATLAS_HEIGHT;
    texture_atlas_desc.MipLevels         = 1;
    texture_atlas_desc.ArraySize         = 1;
    texture_atlas_desc.Format            = DXGI_FORMAT_B8G8R8A8_UNORM;
    texture_atlas_desc.SampleDesc.Count  = 1;
    texture_atlas_desc.Usage             = D3D11_USAGE_IMMUTABLE;
    texture_atlas_desc.BindFlags         = D3D11_BIND_SHADER_RESOURCE;
	
    D3D11_SUBRESOURCE_DATA atlas_data = {};
    atlas_data.pSysMem     = atlas;
    atlas_data.SysMemPitch = ATLAS_WIDTH * sizeof(UINT);
	
    ID3D11Texture2D* atlas_texture;
    device->CreateTexture2D(&texture_atlas_desc, &atlas_data, &atlas_texture);
	device->CreateShaderResourceView(atlas_texture, NULL, &atlas_resource_view);
	atlas_texture->Release();
}

#define MAX_SPRITES 4096
struct int2 { 
	int x, y; 
};
struct Sprite { 
	int2 screen_pos, size, atlas_pos; 
};

// Sprite Buffer
ID3D11Buffer* sprite_buffer;
ID3D11ShaderResourceView* sprite_resource_view;
{
    D3D11_BUFFER_DESC sprite_buffer_desc = {};
    sprite_buffer_desc.ByteWidth           = MAX_SPRITES * sizeof(Sprite);
    sprite_buffer_desc.Usage               = D3D11_USAGE_DYNAMIC;
    sprite_buffer_desc.BindFlags           = D3D11_BIND_SHADER_RESOURCE;
    sprite_buffer_desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
    sprite_buffer_desc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED; // Structured Buffer contains elements of equal size. Inside the shader you can access members using an index
    sprite_buffer_desc.StructureByteStride = sizeof(Sprite);
    device->CreateBuffer(&sprite_buffer_desc, NULL, &sprite_buffer);
	
	// A shader resource view wraps textures in a format that the shaders can access them
    D3D11_SHADER_RESOURCE_VIEW_DESC sprite_shader_resource_view_desc = {};
    sprite_shader_resource_view_desc.Format             = DXGI_FORMAT_UNKNOWN; 
    sprite_shader_resource_view_desc.ViewDimension      = D3D11_SRV_DIMENSION_BUFFER; // indicates the resource is a buffer
    sprite_shader_resource_view_desc.Buffer.NumElements = MAX_SPRITES;
	device->CreateShaderResourceView(sprite_buffer, &sprite_shader_resource_view_desc, &sprite_resource_view);
}

// Point Sampler
ID3D11SamplerState* font_sampler;
{
    D3D11_SAMPLER_DESC font_sampler_desc = {};
    font_sampler_desc.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT;
    font_sampler_desc.AddressU       = D3D11_TEXTURE_ADDRESS_CLAMP;
    font_sampler_desc.AddressV       = D3D11_TEXTURE_ADDRESS_CLAMP;
    font_sampler_desc.AddressW       = D3D11_TEXTURE_ADDRESS_CLAMP;
    font_sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	
    device->CreateSamplerState(&font_sampler_desc, &font_sampler);
}

// Vertex And Pixel Shaders
ID3D11VertexShader* font_vshader;
ID3D11PixelShader* font_pshader;
{
	ID3DBlob *vblob, *pblob;
    D3DCompileFromFile(L"../font.hlsl", nullptr, nullptr, "vs", "vs_5_0", 0, 0, &vblob, nullptr);
    device->CreateVertexShader(vblob->GetBufferPointer(), vblob->GetBufferSize(), nullptr, &font_vshader);
    D3DCompileFromFile(L"../font.hlsl", nullptr, nullptr, "ps", "ps_5_0", 0, 0, &pblob, nullptr);
	device->CreatePixelShader(pblob->GetBufferPointer(), pblob->GetBufferSize(), nullptr, &font_pshader);
	pblob->Release(); 
	vblob->Release(); 
}

Sprite* sprite_batch = (Sprite*)(HeapAlloc(GetProcessHeap(), 0, MAX_SPRITES * sizeof(Sprite)));



//~
// Render Text
//

int sprite_count = 0;
Sprite sprite = { 100, 100, ATLAS_WIDTH / CHARACTER_COUNT, ATLAS_HEIGHT }; // screen_pos, size
char *text = (char *)"";
while (*text)
{
	sprite.atlas_pos.x = (*text++ - ' ') * sprite.size.x;
	sprite_batch[sprite_count++] = sprite;
	sprite.screen_pos.x += sprite.size.x + 1;
}

D3D11_MAPPED_SUBRESOURCE sprite_mapped;
context->Map(sprite_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &sprite_mapped);
memcpy(sprite_mapped.pData, sprite_batch, sprite_count * sizeof(Sprite));
context->Unmap(sprite_buffer, 0);

context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP); // so we can render sprite quad using 4 vertices

context->VSSetShader(font_vshader, NULL, 0);
context->VSSetShaderResources(0, 1, &sprite_resource_view);
context->VSSetConstantBuffers(0, 1, &font_constant_buffer);

context->RSSetViewports(1, &viewport);

context->PSSetShader(font_pshader, NULL, 0);
context->PSSetShaderResources(1, 1, &atlas_resource_view);
context->PSSetSamplers(0, 1, &font_sampler);

context->OMSetRenderTargets(1, &frame_buffer_view, NULL);
context->DrawInstanced(4, sprite_count, 0, 0); // 4 vertices per instance, each instance is a sprite


font_pshader->Release(); 
font_vshader->Release(); 
font_sampler->Release(); 
font_constant_buffer->Release(); 
atlas_resource_view->Release();
sprite_resource_view->Release(); 
sprite_buffer->Release(); 

*/
