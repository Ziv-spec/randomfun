//#include <gameinput.h>
//#pragma comment(lib, "gameinput")
// #define USE_GAMEINPUT

/* 
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




#ifdef USE_GAMEINPUT
Game_Input input = {0};
poll_gameinput(&input);

for (int i = 0; i < ArrayLength(input.gamepads); i++) {
	Gamepad gamepad = input.gamepads[i];
	if (gamepad.buttons & GamepadA) printf("gamepad button a\n");
}
#endif


#ifdef USE_GAMEINPUT
initialize_input();
#endif


#ifdef USE_GAMEINPUT
shutdown_input();
#endif
 */
