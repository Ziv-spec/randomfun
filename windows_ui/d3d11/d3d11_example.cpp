/// direct3d 11 example
#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <d3d11_1.h>
#include <dxgi1_3.h>
#include <dxgidebug.h>
#include <d3dcompiler.h>
#include <gameinput.h>

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
#pragma comment(lib, "gameinput")

#define APP_TITLE "Testing In Progress..."

// default starting width and height for the window
static int window_width = 800;
static int window_height = 600;

#define Assert(expr) do { if (!(expr)) __debugbreak(); } while (0)
#define AssertHR(hr) Assert(SUCCEEDED(hr))
#define ArrayLength(arr) (sizeof(arr) / sizeof(arr[0]))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef int64_t s64; 
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t  s8;
typedef int64_t u64; 
typedef int32_t u32;
typedef int16_t u16;
typedef int8_t  u8;
typedef int b32; 


// Resources to look at:
// https://bgolus.medium.com/the-quest-for-very-wide-outlines-ba82ed442cd9 - the quest for wide outlines
// https://ameye.dev/notes/rendering-outlines/                       - 5 ways to draw an outline
// https://ameye.dev/notes/stylized-water-shader/                    - stylized water shader
// https://wwwtyro.net/2019/11/18/instanced-lines.html               - instanced line rendering
// https://w3.impa.br/~diego/projects/GanEtAl14/                     - massively parallel vector graphics (paper)
// Advanced Camera (better than the lookat matrix I have implemented)
// https://www.3dgep.com/understanding-quaternions/                  - understanding quarternions
// https://gist.github.com/vurtun/d41914c00b6608da3f6a73373b9533e5   - camera gist for understanding all about cameras 
// https://www.youtube.com/watch?v=Jhopq2lkzMQ&list=PLplnkTzzqsZS3R5DjmCQsqupu43oS9CFN&index=1


/* 
* TODO(ziv):
* [x] Obj loading (with position, textures, normals)
* [x] Obj dynamic transformation
* [x] Obj dynamic lighting(global illumination + point light)
* [x] Texture mapping
	* [x] Camera
			*   [x] Normal Camera
			*   [ ] Free Camera (The only thing left is free movement for which raw input/direct input needed
* [x] Face Culling
* [x] z-buffer
* [ ] Shadow Mapping
* [ ] Normal Mapping
* [ ] Keyboard Input
* [ ] Obj Outlines
* [ ] Mouse screen to world projection
* [ ] Font Rendering (With real fonts and not a bitmap)
* [ ] General UI (This has many steps which I will detail when I will begin working on it)
* [ ] Update on Resize for fluid screen resize handling
			* [ ] Pixelated look
*/


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



// 
// Math
//

struct matrix { float m[4][4]; };
struct float3 { float x, y, z; };

inline static float3 operator+(const float3& v1, const float3& v2) { return float3{ v1.x+v2.x, v1.y+v2.y, v1.z+v2.z }; }
inline static float3 operator-(const float3& v1, const float3& v2) { return float3{ v1.x-v2.x, v1.y-v2.y, v1.z-v2.z }; }
inline static float3 operator*(const float3& v1, const float3& v2) { return float3{ v1.x*v2.x, v1.y*v2.y, v1.z*v2.z }; }
inline static float3 operator*(const float3& v, const float c)     { return float3{ v.x*c, v.y*c, v.z*c }; }

inline static float3 v3normalize(float3 v) {
	float len = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z); 
	float inv_len = 1/len; 
	return float3{ v.x*inv_len, v.y*inv_len, v.z*inv_len }; 
}

inline static float3 v3cross(const float3& a, const float3& b) {
	return float3{
		a.y*b.z - a.z*b.y, 
		a.z*b.x - a.x*b.z, 
		a.x*b.y - a.y*b.x
	};
}

inline static float v3dot(const float3& a, const float3& b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline static matrix operator*(const matrix& m1, const matrix& m2)
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

static matrix matrix_inverse_transpose(matrix A) {
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

inline static matrix matrix_transpose(matrix A) {
	matrix r = {
		A.m[0][0], A.m[1][0], A.m[2][0], A.m[3][0], 
		A.m[0][1], A.m[1][1], A.m[2][1], A.m[3][1], 
		A.m[0][2], A.m[1][2], A.m[2][2], A.m[3][2], 
		A.m[0][3], A.m[1][3], A.m[2][3], A.m[3][3], 
	};
	return r; 
}

static inline float float_clamp(float val, float min, float max) {
	float temp = MIN(val, max);
	return MAX(temp, min);
}

static matrix get_model_view_matrix(float3 rotation, float3 translation, float3 scale) {
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

static float parse_float(char* str, size_t *length) {
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

static unsigned int parse_uint(char *str, size_t *length) {
	unsigned int len = 0, result = 0;
	
	while (str[len] == ' ' && str[len] != '\0') len++; 
	
	while (((unsigned int)str[len] - '0') < 10) {
		result = 10*result + str[len++]-'0';
	}
	
	*length = len;
	return result; 
}

struct Vertex {
	float pos[3];
	float norm[3];
	float uv[2];
};

static bool parse_obj(char *path, Vertex *vdest, size_t *v_cnt, unsigned short *idest, size_t *i_cnt) {
	
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
		Assert(file != INVALID_HANDLE_VALUE);
		
	DWORD file_size = GetFileSize(file, NULL);
		obj_buf = (char *)malloc((file_size+1) * sizeof(char));
	
	DWORD bytes_read_cnt;
	bool success = ReadFile(file, obj_buf, file_size, &bytes_read_cnt, NULL);
		if (!success || file_size != bytes_read_cnt) {
			return false;
		}
		
		obj_buf[file_size] = '\0'; // NULL terminate the string
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
		*i_cnt = indicies_count ; 
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
					pos_buff[pos_idx++] = parse_float(s, &len); s+=len+1;
					pos_buff[pos_idx++] = parse_float(s, &len); s+=len+1;
					pos_buff[pos_idx++] = parse_float(s, &len); s+=len;
				}
				else if (*s == 't') {
					s+=2;
					uv_buff[uv_idx++] = parse_float(s, &len); s+=len+1;
					uv_buff[uv_idx++] = parse_float(s, &len); s+=len;
				}
				else if (*s == 'n') {
					s++; s++;
					normals_buff[norm_idx++] = parse_float(s, &len); s+=len+1;
					normals_buff[norm_idx++] = parse_float(s, &len); s+=len+1;
					normals_buff[norm_idx++] = parse_float(s, &len); s+=len;
				}
				
		}
			else if (*s == 'f') {
				s++;
				
				unsigned int vi, vt, vn;
				for (int i = 0; i < 3; i++) {
					vi = parse_uint(s, &len); s+=len+1;
					vt = parse_uint(s, &len); s+=len+1;
					vn = parse_uint(s, &len); s+=len+1;
					
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

//~

// 
// Application Input
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
} Application_Input;




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

static void poll_gameinput(Application_Input *input) {
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
			printf("Couldn't get any readings from gamepad\n"); 
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
		printf("%d, %d,  %d, %d\n", (int)input->mouse.px, (int)input->mouse.px,
			   (int)mouse_state.wheelX, (int)mouse_state.wheelY);
		#endif 
		reading->Release();
	}
	
	
	
	}

//~

static void FatalError(const char* message)
{
    MessageBoxA(NULL, message, "Error", MB_ICONEXCLAMATION);
    ExitProcess(0);
}

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
		 
		
	}
	return DefWindowProcA(window, message, wparam, lparam);
}

#ifdef _DEBUG
int main()
#else
int WINAPI WinMain(HINSTANCE instance, HINSTANCE previouse, LPSTR CmdLine, int ShowCmd)
#endif
{
	
	//~
	// Typical WIN32 Window creation
	//
	
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
	
	//~
	// D3D11 Initialization
	//
	
	// Create D3D11 Device used for resource creation and Device Context for pipline setup and rendering
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
		
		// TODO(ziv): Make this dynamically resizeable
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
	
	
	//~
	// Model Data
	//
	
	char model_path[] = "../resources/cube.obj";
	
	size_t verticies_count, indicies_count; 
	bool success = parse_obj(model_path,NULL, &verticies_count, NULL, &indicies_count); 
	Assert(success && "Failed extracting buffer sizes for vertex and index buffers");
	
	Vertex *verticies = (Vertex *)malloc(verticies_count*sizeof(Vertex)); 
	unsigned short *_indicies = (unsigned short *)malloc(indicies_count*sizeof(unsigned short));
	success = parse_obj(model_path,
						verticies, &verticies_count, 
						_indicies, &indicies_count); 
	Assert(success && "Failed extracting model data");
	
	Vertex *data = verticies; 
	unsigned short *indicies = _indicies; 

	// Create Vertex And Index Buffers
	ID3D11Buffer *vertex_buffer, *index_buffer;
	{
		D3D11_BUFFER_DESC vertex_descriptor = {}; 
		vertex_descriptor.ByteWidth = (UINT)verticies_count*sizeof(Vertex);
		vertex_descriptor.Usage = D3D11_USAGE_DEFAULT;
		vertex_descriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		D3D11_SUBRESOURCE_DATA vertex_data  = { data };
		device->CreateBuffer(&vertex_descriptor, &vertex_data, &vertex_buffer);
		
		D3D11_BUFFER_DESC index_descriptor = {};
		index_descriptor.ByteWidth = (UINT)indicies_count*sizeof(unsigned short); 
		index_descriptor.StructureByteStride = sizeof(unsigned short); // TODO(ziv): check whether I will need to move to 32 bit
		index_descriptor.Usage = D3D11_USAGE_DEFAULT;
		index_descriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;
		D3D11_SUBRESOURCE_DATA index_data = { indicies };
		device->CreateBuffer(&index_descriptor, &index_data, &index_buffer);
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
		device->CreateVertexShader(vblob->GetBufferPointer(), vblob->GetBufferSize(), NULL, &vshader);
		
		device->CreateInputLayout(vs_input_desc, ARRAYSIZE(vs_input_desc), 
								  vblob->GetBufferPointer(), vblob->GetBufferSize(), &layout);
		
		
		ID3DBlob *pblob;
		hr = D3DCompileFromFile(L"../shaders.hlsl", NULL, NULL, "ps_main", "ps_5_0", flags, 0, &pblob, &error);
		if (FAILED(hr)) {
			const char *message = (const char *)error->GetBufferPointer(); 
			OutputDebugStringA(message); 
			Assert(!"Failed to compile pixel shader!");
		}
		device->CreatePixelShader(pblob->GetBufferPointer(), pblob->GetBufferSize(), NULL, &pshader);
		
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
		device->CreateBuffer(&cbuffer_descriptor, NULL, &cbuffer); 
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
		device->CreateBuffer(&cbuffer_descriptor, NULL, &ps_constant_buffer); 
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
		
        device->CreateSamplerState(&sampler_desc, &sampler_state);
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
			device->CreateTexture2D(&texture_desc, &texture_data, &texture);
			device->CreateShaderResourceView(texture, NULL, &texture_view);
		
		texture->Release();
		free(bytes);
		
	}
	
	
	
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
	
	// camera 
	float3 camera = { 0, 0, 0 };
	float camera_pitch = -3.14f/4;
	float camera_yaw = -3.14f/2; 
	
	
	// Testing free camera option
	//float dx = 0, dy = 0;
	bool show_free_camera = false; 
	
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
	
	initialize_input(); 
	
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
		
		Application_Input input = {0}; 
		poll_gameinput(&input); 
		
		for (int i = 0; i < ArrayLength(input.gamepads); i++) {
			Gamepad gamepad = input.gamepads[i];
			if (gamepad.buttons & GamepadA) printf("gamepad button a\n");
		}
		
		mouse_pos[0] = (int)input.mouse.px; 
		mouse_pos[1] = (int)input.mouse.py; 
		
		//
		// Handle window resize
		//
		
		RECT rect; 
		GetClientRect(window, &rect);
		LONG width = rect.right - rect.left;
		LONG height = rect.bottom - rect.top;
		if (width != (LONG)viewport.Width || height != (LONG)viewport.Height) {
			
			if (frame_buffer_view) {
				context->ClearState(); 
				frame_buffer_view->Release(); 
				frame_buffer_view = NULL;
				
				zbuffer_texture->Release(); 
				zbuffer->Release(); 
			}
			
			if (width != 0 && height != 0) {
				hr = swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
				if (FAILED(hr)) FatalError("Failed to resize the swap chain!");
				
				// Create a new RenderTarget for the new backbuffer texture
				ID3D11Texture2D *backbuffer; 
				swap_chain->GetBuffer(0, __uuidof(ID3D11Resource), (void **)&backbuffer);
				device->CreateRenderTargetView(backbuffer, NULL, &frame_buffer_view);
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
					device->CreateTexture2D(&depth_buffer_desc, NULL, &zbuffer_texture);
					
					D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = {}; 
					depth_stencil_view_desc.Format = DXGI_FORMAT_D32_FLOAT;
					depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
					depth_stencil_view_desc.Texture2D.MipSlice = 0; 
					device->CreateDepthStencilView(zbuffer_texture, &depth_stencil_view_desc, &zbuffer);
			}
			
			viewport.Width = (FLOAT)width; 
			viewport.Height = (FLOAT)height;
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
		
/* 		
		model_rotation.x -= 0.03f;
		model_rotation.y -= 0.03f;
				 */
		
			
		
		// Update Camera
		matrix inv_camera_matrix;
		float3 translate_vector; 
		{
			// https://learnopengl.com/Getting-started/Camera
			
			float dx = (float)(mouse_pos[0] - last_mouse_pos[0]);
			float dy = (float)(last_mouse_pos[1] - mouse_pos[1]); // NOTE(ziv): flipped y axis so up is positive
			
			float speed = 5;
			
			for (int i = 0; i < 4; i++) {
				dx += input.gamepads[i].right_thumbstick_x*speed;
				dy += input.gamepads[i].right_thumbstick_y*speed;
				
			}
			
			camera_yaw   = fmodf(camera_yaw - dx/window_width*2*3.14f, (float)(2*M_PI));
			camera_pitch = float_clamp(camera_pitch + dy/window_height, -(float)M_PI/2.f, (float)M_PI/2.f); 
			
			// this camera is pointing in the inverse direction of it's target
		float3 camera_dir = {
			cosf(camera_yaw) * cosf(camera_pitch), 
			sinf(camera_pitch), 
			sinf(camera_yaw) * cosf(camera_pitch)
		}; 
			
			
		float3 forward_vector = v3normalize(camera_dir); 
		// some vector which is included in the up vector plain
		float3 some_up_vector = { 0, 1, 0 }; 
		// calculate the right vector using a cross product
		float3 right_vector = v3normalize(v3cross(some_up_vector, forward_vector)); 
		float3 up_vector = v3normalize(v3cross(forward_vector, right_vector)); 
		
			if (!show_free_camera) {
			float3 fv, rv;
				fv = float3 { cosf(camera_yaw), 0, sinf(camera_yaw)}; // forward
				rv = v3normalize(v3cross(some_up_vector, fv)); 

				// first person camera movement
				if (key_w) camera = camera+fv*(dt*speed);
				if (key_s) camera = camera-fv*(dt*speed);
				if (key_d) camera = camera+rv*(dt*speed);
				if (key_a) camera = camera-rv*(dt*speed);
				
				for (int i = 0; i < 4; i++) {
					camera = camera + rv*(dt*speed)*input.gamepads[i].left_thumbstick_x;
					camera = camera + fv*(dt*speed)*input.gamepads[i].left_thumbstick_y;
				}
				
			}
			else {
				// free camera movement
				if (key_w) camera = camera+forward_vector*(dt*speed);
		if (key_s) camera = camera-forward_vector*(dt*speed);
		if (key_d) camera = camera+right_vector*(dt*speed);
		if (key_a) camera = camera-right_vector*(dt*speed);
			}

		translate_vector = { v3dot(camera, right_vector), v3dot(camera, up_vector), v3dot(camera, forward_vector) };
		
			matrix camera_matrix = matrix{
			right_vector.x, right_vector.y, right_vector.z, -translate_vector.x, 
			up_vector.x,    up_vector.y,    up_vector.z,    -translate_vector.y, 
			camera_dir.x,   camera_dir.y,   camera_dir.z,   -translate_vector.z, 
			0, 0, 0, 1
													}; 
		
		// inverse the camera matrix to create the inverse transform
		inv_camera_matrix = matrix_transpose(camera_matrix);
		}
		
		// Don't render when minimized
		if (width == 0 && height == 0) {
			Sleep(16); continue;
		}
		
		
		
		
		model_translation.x = 0;
		model_rotation.x = 0;
		// Update model-view matrix
		{
			matrix model_view_matrix = get_model_view_matrix(model_rotation, model_translation, model_scale) * inv_camera_matrix;
			
			// fov
			float w = viewport.Width / viewport.Height; // width (aspect ratio)
			matrix projection = { 2 * n / w, 0, 0, 0, 0, 2 * n / h, 0, 0, 0, 0, f / (f - n), 1, 0, 0, n * f / (n - f), 0  };
			
			
			// Vertex Contstant Buffer
			VSConstantBuffer vs_cbuf; 
			vs_cbuf.transform        = model_view_matrix;  
			vs_cbuf.projection       = projection; 
			vs_cbuf.normal_transform = matrix_inverse_transpose(model_view_matrix);
			
			D3D11_MAPPED_SUBRESOURCE mapped;
			context->Map((ID3D11Resource *)cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			memcpy(mapped.pData, &vs_cbuf, sizeof(vs_cbuf)); 
			context->Unmap((ID3D11Resource *)cbuffer, 0);
			
			// Pixel Contstant Buffer
			PSConstantBuffer ps_cbuf; 
			ps_cbuf.point_light_position = lightposition - translate_vector;
			ps_cbuf.sun_light_direction = sun_direction;
			
			D3D11_MAPPED_SUBRESOURCE ps_mapped;
			context->Map((ID3D11Resource *)ps_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ps_mapped);
			memcpy(ps_mapped.pData, &ps_cbuf, sizeof(ps_cbuf)); 
			context->Unmap((ID3D11Resource *)ps_constant_buffer, 0);
		}
		
		
		context->ClearRenderTargetView(frame_buffer_view, background_color);
		context->ClearDepthStencilView(zbuffer, D3D11_CLEAR_DEPTH, 1.0f, 0);
		
		// Input Assembler
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(layout);
		const UINT stride = sizeof(Vertex); 
		const UINT offset = 0;
        context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
        context->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R16_UINT, 0);
		
		// Vertex Shader
        context->VSSetShader(vshader, NULL, 0);
		context->VSSetConstantBuffers(0, 1, &cbuffer);
		
		// Rasterizer Stage
        context->RSSetViewports(1, &viewport);
		context->RSSetState(rasterizer_cull_back);
		
		// Pixel Shader
        context->PSSetShader(pshader, NULL, 0);
		context->PSSetConstantBuffers(0, 1, &ps_constant_buffer); 
        context->PSSetShaderResources(0, 1, &texture_view);
        context->PSSetSamplers(0, 1, &sampler_state);
		
		// Output Merger
		context->OMSetDepthStencilState(depth_stencil_state, 0);
		context->OMSetRenderTargets(1, &frame_buffer_view, zbuffer);
		
		context->DrawIndexed((UINT)indicies_count, 0, 0); 
		
		
		
		
		model_translation.x = 1;
		model_rotation.x = 1;
		 
		// Update model-view matrix
		{
			matrix rx = { 1, 0, 0, 0, 0, cosf(model_rotation.x), -sinf(model_rotation.x), 0, 0, sinf(model_rotation.x), cosf(model_rotation.x), 0, 0, 0, 0, 1 };
			matrix ry = { cosf(model_rotation.y), 0, sinf(model_rotation.y), 0, 0, 1, 0, 0, -sinf(model_rotation.y), 0, cosf(model_rotation.y), 0, 0, 0, 0, 1 };
			matrix rz = { cosf(model_rotation.z), -sinf(model_rotation.z), 0, 0, sinf(model_rotation.z), cosf(model_rotation.z), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
			matrix scale = { model_scale.x, 0, 0, 0, 0, model_scale.y, 0, 0, 0, 0, model_scale.z, 0, 0, 0, 0, 1 };
			matrix translate = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, model_translation.x, model_translation.y, model_translation.z, 1 };
			
			matrix model_view_matrix = rx * ry * rz * scale * translate * inv_camera_matrix;
			
			
			// fov
			float w = viewport.Width / viewport.Height; // width (aspect ratio)
			matrix projection = { 2 * n / w, 0, 0, 0, 0, 2 * n / h, 0, 0, 0, 0, f / (f - n), 1, 0, 0, n * f / (n - f), 0  };
			
			
			// Send new constant data to the GPU
			VSConstantBuffer vs_cbuf; 
			vs_cbuf.transform        = model_view_matrix;  
			vs_cbuf.projection       = projection; 
			vs_cbuf.normal_transform = matrix_inverse_transpose(model_view_matrix);
			
			D3D11_MAPPED_SUBRESOURCE mapped;
			context->Map((ID3D11Resource *)cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			memcpy(mapped.pData, &vs_cbuf, sizeof(vs_cbuf)); 
			context->Unmap((ID3D11Resource *)cbuffer, 0);
			
			PSConstantBuffer ps_cbuf; 
			ps_cbuf.point_light_position = lightposition - translate_vector;
			ps_cbuf.sun_light_direction = sun_direction;
			
			D3D11_MAPPED_SUBRESOURCE ps_mapped;
			context->Map((ID3D11Resource *)ps_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ps_mapped);
			memcpy(ps_mapped.pData, &ps_cbuf, sizeof(ps_cbuf)); 
			context->Unmap((ID3D11Resource *)ps_constant_buffer, 0);
		}
		
		
		
		//~
		// Render Model
		// 
		
		

/* 		
		context->ClearRenderTargetView(frame_buffer_view, background_color);
		context->ClearDepthStencilView(zbuffer, D3D11_CLEAR_DEPTH, 1.0f, 0);
		 */

		// Input Assembler
		{
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			context->IASetInputLayout(layout);
		const UINT stride = sizeof(Vertex); 
		const UINT offset = 0;
        context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
        context->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R16_UINT, 0);
		}
		// Vertex Shader
        context->VSSetShader(vshader, NULL, 0);
		context->VSSetConstantBuffers(0, 1, &cbuffer);
		
		// Rasterizer Stage
        context->RSSetViewports(1, &viewport);
		context->RSSetState(rasterizer_cull_back);
		
		// Pixel Shader
        context->PSSetShader(pshader, NULL, 0);
		context->PSSetConstantBuffers(0, 1, &ps_constant_buffer); 
        context->PSSetShaderResources(0, 1, &texture_view);
        context->PSSetSamplers(0, 1, &sampler_state);
		
		// Output Merger
		context->DrawIndexed((UINT)indicies_count, 0, 0); 
		
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
		
		// present the backbuffer to the screen
		swap_chain->Present(1, 0);
		
		
		// end of frame
		last_mouse_pos[0] = mouse_pos[0]; last_mouse_pos[1] = mouse_pos[1]; 
		start_frame = end_frame; // update time for dt calc
		}
	
	
	release_resources:
	
	shutdown_input(); 
	
	index_buffer->Release();
	vertex_buffer->Release();
	cbuffer->Release();
	ps_constant_buffer->Release();
	texture_view->Release();
	sampler_state->Release();
	layout->Release();
	pshader->Release();
	vshader->Release();
	
	font_pshader->Release(); 
	font_vshader->Release(); 
	font_sampler->Release(); 
	font_constant_buffer->Release(); 
	atlas_resource_view->Release();
	sprite_resource_view->Release(); 
	sprite_buffer->Release(); 
	
	device->Release(); 
	context->Release();
	swap_chain->Release();
	frame_buffer_view->Release();
	rasterizer_cull_back->Release();
	depth_stencil_state->Release(); 
	zbuffer->Release();
	zbuffer_texture->Release(); 
	
	return 0; 
}