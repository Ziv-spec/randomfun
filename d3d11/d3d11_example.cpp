#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <d3d11_1.h>
#include <dxgi1_3.h>
#include <dxgidebug.h>
#include <d3dcompiler.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "d3dcompiler")

#define APP_TITLE "D3D11 application!!!"

// default starting width and height for the window
static int window_width  = 800;
static int window_height = 600;

#define MAX_SPRITES_COUNT 0x1000
#define MAX_QUAD_COUNT    0x100

#define WIDGETS_COUNT 0x100
#define MAX_WIDGET_STACK_SIZE 5

#define Assert(expr) do { if (!(expr)) __debugbreak(); } while (0)
#define AssertHR(hr) Assert(SUCCEEDED(hr))
#define ArrayLength(arr) (sizeof(arr) / sizeof(arr[0]))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ABS(a)    ((a) < (0) ? (-a) : (a))

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
typedef int     b32;

typedef struct { int x, y; } int2;
typedef struct { float x, y; } float2;
typedef struct { float minx, miny, maxx, maxy; } Rect;
typedef struct { float r, g, b, a; } Color;

// Resources to look at:
// https://bgolus.medium.com/the-quest-for-very-wide-outlines-ba82ed442cd9 - the quest for wide outlines
// https://ameye.dev/notes/rendering-outlines/                       - 5 ways to draw an outline
// https://wwwtyro.net/2019/11/18/instanced-lines.html               - instanced line rendering
// https://w3.impa.br/~diego/projects/GanEtAl14/                     - massively parallel vector graphics (paper)
// https://www.3dgep.com/understanding-quaternions/                  - understanding quarternions
// https://lxjk.github.io/2016/10/29/A-Different-Way-to-Understand-Quaternion-and-Rotation.html
// https://www.youtube.com/watch?v=Jhopq2lkzMQ&list=PLplnkTzzqsZS3R5DjmCQsqupu43oS9CFN&index=1
// https://handmade.network/forums/t/8704-getting_a_window_message_from_outside_the_window_procedure_but_within_the_same_executable#29347
// https://gist.github.com/mmozeiko/c136c1cfce9fe4267f3c8f7b90f8e4d4
// https://gist.github.com/mmozeiko/b8ccc54037a5eaf35432396feabbe435

//==============================================================================
// # Understanding the Graphics Pipeline[8]: 
//
// Pipline is somewhat like the following: 
// APP -> API (d3d11) -> UMD (dll) -> OS/Driver Graphics Scheduler -> 
// OS/Driver KMD (Command Buffer) -> PCIe lane -> Command Processor(Command Buffer) -> 
// CPU/GPU Synchronization 
//
// API is how you access the GPU (manages resources, tracks state, may or may not validate shader code) 
// UMD User Mode Driver (driver per application to compile shaders, memory management (high level), send commands to KMU)
// Graphics Scheduler (slices access to hardware to support multi-app hardware access)
// KMU Kernal Mode Driver (There is only one KMU, deals with hardware, it manages the command buffer, 
// The bus (usually a PCIe lane unless the GPU and CPU are on the same die, slow)
// Command Processor (reads Command buffer, buffers whatever it can as much as possible)
//
// # About Memory
//
// The GPU was built to handle large parallel tasks. For that reason many 
// trade-offs were made to support this assumption. Memory has large latency 
// but great throughput when compared to CPU memory. Random memory reads/writes
// are bad for performance in general but on the GPU it seems to take a whole 
// new level.
//
// Even slower is the bus. Signals travel from the CPU across the motherboard 
// until they reach the GPU. This process is extremely slow since it takes time 
// for these signals to travel the comparatively large distance the bus takes.
// And for modern hardware that operates in the GHz range signal latency is 
// large. ( I believe there is a good image to showcase the difference in speed
// but I don't the link to it now )
//
// Now we know there are two places to store graphics resources. You can store 
// them in local video memory on the GPU or on the main system memory. The 
// decision for where to store information has a couple solutions each with 
// their own trade-offs. You can use a couple address bits to specify where 
// to store the information or you can use something called the MMU ( Memory 
// Management Unit ). Specifically the MMU allows for copy-free defragmentation 
// of memory when the GPU runs out of local video memory since it virtualizes
// memory. https://www.farbrausch.de/~fg/gpu/gpu_memory.jpg
//
// There is also a DMA Direct Memory Access engine that allows the copying of 
// memory around without the need to use the GPU 3d hardware/shader cores. It 
// allows the copying of memory from main memory to video memory and the
// reverse along with video memory to video memory. This piece of hardware 
// is important when you consider the face that you need to constantly move 
// memory into the GPU and sometimes out of the GPU to draw. This would in 
// effect make the cost of doing such data movements at least not effect your 
// CPU/GPU performance by much.
//
// # Pushing commands to GPU
//
// Let's take a look at how the CPU provides the command buffer to the GPU. 
// Our CPU has the prepared command buffer ready. We use host PCIe interface 
// to send the GPU commands or alternatively store the commands inside the 
// video memory. For the latter case the KMU sets up a DMA transfer of the 
// command buffer to the GPU so neither the CPU or the GPU shader cores have 
// to worry about it. Then we take the data from video memory through our memory
// subsystem. Now the commands are ready for processing and execution.
//
// # Command Processor 
// https://www.farbrausch.de/~fg/gpu/command_processor.jpg
//
// The command processor first buffers the command buffer. Essentially 
// prefetches commands and adds them to a fat buffer to avoid hiccups. This is 
// done since there is only one command processor that eats these commands. 
//
// From that buffer it goes into the command processor front-end. This is
// basically a state machine that decodes the commands and tells the type of 
// command 2D or 3D or state command. It does that since the GPU hardware 
// supports 2D commands although never explicitly seen through the API and had 
// a distinct separation for them. State commands are commands that change the 
// state of some global variable. Of course as we know from parallel programming
// you can't just change the state of some variable and hope things work out
// as they quite often don't. So there are popular methods for dealing with 
// variable changes. Part 2 of the blog[8] documents that. But the main point 
// is that this is none-trivial and has hardware support for making sure things 
// stay fast. 
//
// # Synchronization 
//
// There are many ways to synchronize the CPU and the GPU. It is important to 
// take note of a method that allows the CPU to track the rendering progress
// inside of the GPU. It is commonly used and of course very powerful for 
// the very reasons I am about to show. 
// 
// The tracking happens in registers inside of the GPU. The CPU sends commands 
// and tells in them: "When you encounter command number X write to register 0
// the value of the current command". This intern, allows the CPU when reading 
// this special register 0 to know at which stage the GPU is in rendering the 
// command buffer. Now that the CPU has this knowledge the KMU can reclaim 
// resources that have been acquired by the GPU and do whatever it wants with 
// them. Whether updating or releasing these resources.
//
// With this we also can think of new capabilities that the GPU might have. You
// can tell it: "Once you reach this command, do something something". Once 
// all shaders have finished reading their resources allow CPU to reclaim them. 
// Or even once you have finished all rendering commands do something. These 
// commands are called a "fences". And as you can see they allow us to do 
// operations that require some prior conditions before they can be executed.
//
// One more thing is that the CPU also can report back to the GPU if it has 
// some state which it is now in and the GPU can execute something. This is 
// useful when you have buffers locked by the CPU (multi-threaded code) and you
// want for performance reasons to already send the command and have the GPU 
// execute once it finds that the buffers are not locked. 
//
// # D3D11 Pipeline
//
// IA — Input Assembler. Reads index and vertex data.
// VS — Vertex shader. Gets input vertex data, writes out processed vertex data for the next stage.
// PA — Primitive Assembly. Reads the vertices that make up a primitive and passes them on.
// HS — Hull shader; accepts patch primitives, writes transformed (or not) patch control points, inputs for the domain shader, plus some extra data that drives tessellation.
// TS — Tessellator stage. Creates vertices and connectivity for tessellated lines or triangles.
// DS — Domain shader; takes shaded control points, extra data from HS and tessellated positions from TS and turns them into vertices again.
// GS — Geometry shader; inputs primitives, optionally with adjacency information, then outputs different primitives. Also the primary hub for…
// SO — Stream-out. Writes GS output (i.e. transformed primitives) to a buffer in memory.
// RS — Rasterizer. Rasterizes primitives.
// PS — Pixel shader. Gets interpolated vertex data, outputs pixel colors. Can also write to UAVs (unordered access views).
// OM — Output merger. Gets shaded pixels from PS, does alpha blending and writes them back to the backbuffer.
// CS — Compute shader. In its own pipeline all by itself. Only input is constant buffers+thread ID; can write to buffers and UAVs.
//
// # Vertex Shader NOTES:
// 
// In the past, on graphics units there were special cores that handled vertex
// shading. These were simple cores that took vertices and transformed them down
// to the next stage which was usually the pixel shader stage. Now at some point
// hardware manufacturers decided to unify the PS and VS cores. This under this
// unification, one type of core of course had to go. Vertex shader cores had 
// to process upto let's say 1M vertices while pixel shader cores processed 
// 1920*1080 pixels so about 2M pixels. It is not hard to guess which type of 
// cores did not make the cut. So we ended up with more cores that had to handle
// different types of workloads, and the FIFO type style of vertex input was no
// longer fitting. So now there is a more multi-threaded friendly version that 
// runs on modern hardware.
//
// # Texture shading
// 
// Texturing a triangle is on the pixel shader seems to have trade-offs of 
// anything that has state that needs to change (look at synchronization stage) 
// along with how much information is required by it in each and every pipline 
// stage. So thinking a little about texturing is required TODO(ziv): (reread 
// the post to write more about what I need to think about)
//
// # Rasterization 
//
// When writing my own rasterizer, I used the scan-line technique which 
// modern hardware doesn't use. A more hardware optimized technique utilizes 
// signed distance field of a triangle and checking whether the processing 
// core is inside it along with hierarchical masking in order to allow for 
// truly parallel processing of rasterization. This is interesting information 
// since I was not familiar with this technique.
//
// # Transformations In Engines
//
// As this is a engine it should be noted that I take into account all of the 
// transformations needed in order to draw some pixels onto the screen. Here 
// is a chart to show all of the types of transformations required: 
//
// Model -> World -> View -> Clip -> NDC(Normalized Cords) -> Screen/Viewport
//
// Model  - the space the model is in when imported
// World  - places the model inside the world           (Model Transform)
// View   - takes the camera view into account          (View/Camera Transform)
// Clip   - places the entire view frustum inside a box (Projection Transform)
// NDC    - Normalized Device Coordinates is the clip normalized (Divide Z)
// Screen - viewport coordinates 0-width, 0-height      (Width/Height Transform)
//
// All of these transformations are linear and inverse-able. 
// Also note that I have commented my camera till death, so I have a explanation 
// on all of the math behind the Camera implementation you see here. 
// 
//
// [1] https://learn.microsoft.com/en-us/windows/
// [2] https://learn.microsoft.com/en-us/windows/win32/direct3d9/coordinate-systems?redirectedfrom=MSDN
// [3] https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-per-component-math#matrix-ordering
// [4] https://learnopengl.com/Getting-started/Camera
// [5] https://gist.github.com/vurtun/d41914c00b6608da3f6a73373b9533e5
// [6] https://www.rfleury.com/p/ui-part-1-the-interaction-medium
// [7] https://floooh.github.io/2018/06/17/handles-vs-pointers.html
// [8] https://fgiesen.wordpress.com/2011/07/09/a-trip-through-the-graphics-pipeline-2011-index/
//
//==============================================================================

static void FatalError(const char* message) {
    MessageBoxA(NULL, message, "Error", MB_ICONEXCLAMATION);
    ExitProcess(0);
}

//~
// Math
//

struct matrix { float m[4][4]; };
struct float3 { float x, y, z; };
struct float4 { float x, y, z, w; };


inline static float3 
operator+= (const float3 v1, const float3 v2) { 
	return float3{ v1.x+v2.x, v1.y+v2.y, v1.z+v2.z }; 
}
inline static float3 operator+(const float3 v1, const float3 v2) { return float3{ v1.x+v2.x, v1.y+v2.y, v1.z+v2.z }; }
inline static float3 operator-(const float3 v1, const float3 v2) { return float3{ v1.x-v2.x, v1.y-v2.y, v1.z-v2.z }; }
inline static float3 operator*(const float3 v1, const float3 v2) { return float3{ v1.x*v2.x, v1.y*v2.y, v1.z*v2.z }; }
inline static float3 operator*(const float3 v, const float c)    { return float3{ v.x*c, v.y*c, v.z*c }; }
inline static float3 operator*(const float c, const float3 v)    { return float3{ v.x*c, v.y*c, v.z*c }; }

inline static float lerp(float start, float end, float t) { return start + (end-start)*t; }

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

inline static float4
operator*(const float4 &v, const matrix& m)
{
	return {
        m.m[0][0]*v.x + m.m[1][0]*v.y + m.m[2][0]*v.z + m.m[3][0]*v.w,
        m.m[0][1]*v.x + m.m[1][1]*v.y + m.m[2][1]*v.z + m.m[3][1]*v.w,
        m.m[0][2]*v.x + m.m[1][2]*v.y + m.m[2][2]*v.z + m.m[3][2]*v.w,
        m.m[0][3]*v.x + m.m[1][3]*v.y + m.m[2][3]*v.z + m.m[3][3]*v.w,
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
// Memory Allocator
//

typedef struct {
	size_t size; 
	size_t idx;
	void *data;
} Arena;

static bool 
MemArenaInit(Arena *arena, size_t size) {
	void *data = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
	arena->data = data;
	arena->size = size; 
	arena->idx = 0; 
	
	return data != NULL;
}

static void *
MemArenaAlloc(Arena *arena, size_t size) {
	Assert(arena->idx < arena->size && arena->data); 
	
	size_t bytes_free = arena->size - arena->idx; 
	if (bytes_free > size) {
		void *result = (char *)arena->data + arena->idx;
		arena->idx += size;
		return result;   
	}
	return NULL;
}

//~
// Linked List
// 

#define DLLPushBack_NP(f, l, n, next, prev) ((f)==0?\
((f)=(l)=(n), (n)->next=(n)->prev=0):\
((n)->prev=(l), (l)->next=(n), (l)=(n), (n)->next=0))

#define DLLRemove_NP(f,l,n, next, prev) (((f)==(n)?\
((f)=(f)->next,(f)->prev=0):\
(l)==(n)?\
((l)=(l)->prev,(l)->next=0):\
((n)->next->prev=(n)->prev,\
(n)->prev->next=(n)->next)))


#define DLLPushBack(f, l, n) DLLPushBack(f, l, n, next, prev)
#define DLLPushFront(f, l, n) DLLPushBack(f, l, n, prev, next)
#define DLLRemove(f, l, n) DLLRemove_NP(f, l, n, next, prev)

//~
// String8
// 

typedef struct {
	size_t size;
	const char *data;
} String8;

inline static String8
Str8Lit(const char *str) {
	Assert(str);
	
	const char *temp = str;
	size_t i = 0;
	while (*temp++) i++;
	
	return String8{ i, str };
}

inline static String8
Str8Copy(char *dest, String8 text) {
	memcpy(dest, text.data, text.size); 
	String8 result = { text.size, dest };
	return result;
}

inline static bool
Str8Compare(String8 s1, String8 s2) {
	if (s1.size != s2.size) return 0; 
	for (int i = 0; i < s1.size; i++) {
		if (s1.data[i] != s2.data[i]) {
			return 0;
		}
	}
	return 1;
}

static String8
Str8FormatString(char *buf, const char *fmt, va_list args) {
	Assert(buf && fmt);
	vsprintf(buf, fmt, args);
	return Str8Lit(buf); 
}

static s64 
Str8GetHash(String8 str, s64 seed) {
	s64 hash = seed;
	for (int i = 0; i < str.size; i++) {
		hash <<= str.data[i] & 0x1f;
		hash ^= str.data[i]*seed;
		hash >>= 2;
	}
	return hash;
}

typedef struct {
	size_t size;
	wchar_t *data;
} String16;

static String16
Str8ToStr16(Arena *arena, String8 str) {
	Assert(arena && str.data && str.size > 0); 
	
	size_t size = (str.size+1)*sizeof(wchar_t);
	wchar_t *data  = (wchar_t *)MemArenaAlloc(arena, size); 
	for (int i = 0; i < size; i++) {
		data[i] = str.data[i];
	}
	
	String16 result = { size, data };
	return result;
}


//
// Font
//

#define CHARACTER_COUNT   95

#define ATLAS_WIDTH      475
#define ATLAS_HEIGHT       7

const static UINT atlas[] = {
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

#define FAT_PIXEL_SIZE 2.f

static float
DefaultTextWidth(String8 text) {
	return text.size*(ATLAS_WIDTH / CHARACTER_COUNT + 1)*FAT_PIXEL_SIZE; 
}

static float 
DefaultTextHeight(String8 text) {
	return (float)(ATLAS_HEIGHT)*FAT_PIXEL_SIZE;
}


//~
// Win32 
//

static LRESULT CALLBACK WinProc(HWND, UINT ,WPARAM ,LPARAM);
static HWND g_window;

static HWND
Win32CreateWindow() { 

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

	g_window = window;

	return window;
}

static void 
Win32ShowWindow(HWND window) {
	ShowWindow(window, SW_SHOW);
}


static WINDOWPLACEMENT g_previous_window_placement = { sizeof(g_previous_window_placement) };

static  void 
Win32ToggleFullScreen(HWND window) {
	// NOTE(ziv): Code taken from raymond chen blog post
	// https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353

	DWORD style = GetWindowLong(window, GWL_STYLE);
	if (style & WS_OVERLAPPEDWINDOW) {
		MONITORINFO monitor_info = { sizeof(monitor_info) };
		if (GetWindowPlacement(window, &g_previous_window_placement) &&
			GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &monitor_info)) {
			SetWindowLong(window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(window, HWND_TOP,
				 monitor_info.rcMonitor.left, monitor_info.rcMonitor.top,
				 monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
				 monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
				 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	} else {
		SetWindowLong(window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(window, &g_previous_window_placement);
		SetWindowPos(window, NULL, 0, 0, 0, 0,
			 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
			 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
}

inline FILETIME
Win32GetLastFileWriteTime(const char *file_name) {
	FILETIME last_write_time = {0};
	WIN32_FILE_ATTRIBUTE_DATA data;
	if (GetFileAttributesExA(file_name, GetFileExInfoStandard, &data)) {
		last_write_time = data.ftLastWriteTime;
	}
	return last_write_time;
}



//~
// Time
//

thread_local double g_inv_freq;

static void
TimeInit() {
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	g_inv_freq = 1/(double)freq.QuadPart;
}

static double
Time() {
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	return (double)time.QuadPart * g_inv_freq;
}

static s64
TimeMicroseconds() {
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	return time.QuadPart;
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
	s64 px, py; // position
	s64 dx, dy; // delta
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
    float left_thumbstick_x;
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

// Create a event thingy? 

enum OS_Event_Kind { 
	OS_EVENT_KIND_NULL, 
	OS_EVENT_KIND_QUIT, 
	OS_EVENT_KIND_MOUSEMOVE, 
	OS_EVENT_KIND_RAW_MOUSEMOVE, 
	OS_EVENT_KIND_MOUSE_BUTTON_DOWN, 
	OS_EVENT_KIND_MOUSE_BUTTON_UP,
	OS_EVENT_KIND_MOUSE_BUTTON_PRESSED, 
};

typedef struct {
	OS_Event_Kind kind; 
	int value[2];
} OS_Event;

typedef struct {
	OS_Event *list;
	int idx;
} OS_Events;

#define MAX_EVENTS_SIZE 0x100
static OS_Event g_events[MAX_EVENTS_SIZE];
static int g_events_idx;

static OS_Event
OSTopEvent() {
	if (g_events_idx <= 0) {
		return g_events[g_events_idx];
	}
	return g_events[g_events_idx-1];
}

static void
OSEventPost(OS_Event e) {
	g_events[g_events_idx++] = e;
}

static void
OSEventEat(OS_Event *e) {
	e->kind = OS_EVENT_KIND_NULL; // this is a signal to skip the event
}


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
		
		RAWINPUTDEVICE Rid[2];
        Rid[0].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
        Rid[0].usUsage = 0x02;              // HID_USAGE_GENERIC_MOUSE
        Rid[0].dwFlags = RIDEV_DEVNOTIFY;    // adds mouse and also ignores legacy mouse messages
        Rid[0].hwndTarget = window;
		
		// TODO(ziv): Figure out why this doesn't work
		Rid[1].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
		Rid[1].usUsage = 0x6;               // HID_USAGE_GENERIC_KEYBOARD
		Rid[1].dwFlags = RIDEV_DEVNOTIFY;   // adds mouse and also ignores legacy mouse messages
		Rid[1].hwndTarget = window;
		
        if (RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])) == FALSE) {
            //registration failed. Call GetLastError for the cause of the error.
            FatalError("Rawinput couldn't register a mouse\n");
		}
		
	}
	
}

static void
InputShutdown() {
	
	// Rawinput
	{
		RAWINPUTDEVICE Rid[2];
		Rid[0].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
		Rid[0].usUsage = 0x02;              // HID_USAGE_GENERIC_MOUSE
		Rid[0].dwFlags = RIDEV_REMOVE;      // adds mouse and also ignores legacy mouse messages
		Rid[0].hwndTarget = 0;
		
		Rid[1].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
			Rid[1].usUsage = 0x6;               // HID_USAGE_GENERIC_KEYBOARD
			Rid[1].dwFlags = RIDEV_REMOVE;      // adds mouse and also ignores legacy mouse messages
			Rid[1].hwndTarget = 0;
			
		if (RegisterRawInputDevices(Rid, ArrayLength(Rid), sizeof(Rid[0])) == FALSE)
		{
			//registration failed. Call GetLastError for the cause of the error.
			FatalError("Rawinput couldn't register a mouse\n");
		}
	}
	
}

static int2 mouse_delta; 
static int mouse_wheel_delta; 

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
		
		mouse_delta.x += raw->data.mouse.lLastX; 
		mouse_delta.y += raw->data.mouse.lLastY; 
		
		if (raw->data.mouse.usButtonFlags == RI_MOUSE_WHEEL) {
			mouse_wheel_delta += raw->data.mouse.usButtonData;
		}
		
		static int os_mouse_button[] = { MouseLeftButton, MouseRightButton, MouseMiddleButton };
		static int raw_buttons[] = {
			RI_MOUSE_BUTTON_1_DOWN,RI_MOUSE_BUTTON_1_UP,
			RI_MOUSE_BUTTON_2_DOWN,RI_MOUSE_BUTTON_2_UP,
			RI_MOUSE_BUTTON_3_DOWN,RI_MOUSE_BUTTON_3_UP
		};
		for (int i = 0; i < ArrayLength(raw_buttons); i++) {
			if (raw->data.mouse.ulButtons & raw_buttons[i]) {
				OS_Event button_event = {
					(i & 1) ? OS_EVENT_KIND_MOUSE_BUTTON_UP : OS_EVENT_KIND_MOUSE_BUTTON_DOWN,
					os_mouse_button[i/2]
				};
				
				
				OSEventPost(button_event);
			}
		}
		
	}
	delete[] lpb;
	
	return 1;
}

static OS_Events OSProcessEvents() {
	
	
	g_events_idx = 0;
	mouse_delta.x = 0;
	mouse_delta.y = 0;
	
	MSG msg;
	while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
	
	if (mouse_delta.x != 0 || mouse_delta.y != 0) {
		OS_Event mousemove = { 
			OS_EVENT_KIND_RAW_MOUSEMOVE, 
			mouse_delta.x, mouse_delta.y
		};
		OSEventPost(mousemove);
		
	}
	
	return { g_events, g_events_idx };
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


bool show_free_camera = false;

static bool running = true;

static LRESULT CALLBACK WinProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
	
	switch (message)
    {
		case WM_DESTROY: {
			//OSEventPost({ OS_EVENT_KIND_QUIT });
			PostQuitMessage(0);
			running = false;
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
			
			
			// NOTE(ziv): Implemented currently as alt+enter combination
			// which is the windows default for toggling to fullscreen
			if (wparam == VK_RETURN && (lparam & (1<<29))) {
				Win32ToggleFullScreen(window); 
			}
			
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
			
			if (wparam == VK_TAB && (lparam & (1<<24)) == 0) {
				key_tab_pressed = true;
			}
			
		} break;
		
		case WM_ACTIVATE: {
			if (!wparam) {
				g_raw_input_state.mouse = Mouse{0};
			}
		} break;
		
		case WM_MOUSEMOVE: {
			POINTS p = MAKEPOINTS(lparam);
			OSEventPost({ OS_EVENT_KIND_MOUSEMOVE, p.x, p.y});
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
		
		case WM_INPUT_DEVICE_CHANGE: {
			HANDLE device = (HANDLE)lparam; 
			unsigned int size = sizeof(RID_DEVICE_INFO); 
			RID_DEVICE_INFO device_info;
			b32 success = GetRawInputDeviceInfoA(device, RIDI_DEVICEINFO, &device_info, &size);
			
			if (wparam == GIDC_ARRIVAL) {
				if (device_info.dwType == RIM_TYPEMOUSE) {
				printf("Added mouse device\n");
				}
			}
			else {
				if (device_info.dwType == RIM_TYPEMOUSE) {
				printf("Removed mouse device\n");
			}
			}
			
		} break; 
		
	}
	return DefWindowProcA(window, message, wparam, lparam);
}

//~
// D3D11 Renderer core (uses handles as a medium for resource acquisition[7]) 
//

struct Vertex {
	float pos[3];
	float norm[3];
	float uv[2];
};

typedef struct R_QuadInst { // a draw box that I send to the gpu to draw
	Rect rect; 
	Color color;
	float radius;
	float border;
} R_QuadInst; 

typedef struct R_SpriteInst {
	float2 screen_pos, size; 
	int2 atlas_pos;
} R_SpriteInst;

typedef struct { int val; } PSHandle;
typedef struct { int val; } VSHandle;
typedef struct { int val; } BFHandle;
typedef struct { int val; } TEXHandle;


#define FORMAT_TABLE \
X(R_FORMAT_R16_UINT,            DXGI_FORMAT_R16_UINT),\
X(R_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN), \
X(R_FORMAT_B8G8R8A8_UNORM,      DXGI_FORMAT_B8G8R8A8_UNORM),\
X(R_FORMAT_R32G32_FLOAT,        DXGI_FORMAT_R32G32_FLOAT),\
X(R_FORMAT_R32G32B32_FLOAT,     DXGI_FORMAT_R32G32B32_FLOAT),\
X(R_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB),\
X(R_FORMAT_B8G8R8A8_UNORM_SRGB, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB)

#define X(a, b) a
enum R_Format {
	FORMAT_TABLE
};
#undef X

const static DXGI_FORMAT g_renderer_to_dxgi_format[] = {
#define X(a, b) b
	FORMAT_TABLE
#undef X
};

enum R_InputClassification {
	R_INPUT_PER_VERTEX_DATA   = 0,
	R_INPUT_PER_INSTANCE_DATA = 1
};

typedef struct {
	char *semantic_name;
	R_Format format;
	u32 input_slot; 
	u32 aligned_byte_offset;
	R_InputClassification input_slot_class;
} R_LayoutFormat; 

typedef struct {
	HWND window;
	
	Arena *arena; // permanent arena? (maybe should be perframe scratch buffer idk)
	
	ID3D11Device1 *device;
	ID3D11DeviceContext1 *context;
	IDXGISwapChain1 *swap_chain;
	ID3D11RenderTargetView *frame_buffer_view;
	
	ID3D11DepthStencilState* depth_stencil_state;
	ID3D11DepthStencilView *zbuffer;
	ID3D11Texture2D *zbuffer_texture;
	ID3D11RasterizerState1* rasterizer_cull_back;
	ID3D11RasterizerState1* rasterizer_cull_front;
	ID3D11SamplerState *point_sampler; 
	ID3D11SamplerState *linear_sampler; 

	
	D3D11_VIEWPORT *viewport;
	b32 dirty; // window size changed
	
	
	// Book keeping for dynamic loading of shaders and buffers 
	ID3D11VertexShader *vs_array[0x10]; 
	ID3D11InputLayout *lay_array[0x10]; 
	VSHandle vs_handles[0x10];
	struct {
		const char *file_name;
		const char *entry_point;
		FILETIME last_write_time;
		R_LayoutFormat *format;
		unsigned int format_size;
	} vs_file_names_and_time[0x10];
	int vs_idx;
	
	ID3D11PixelShader *ps_array[0x10]; 
	PSHandle ps_handles[0x10];
	struct {
		const char *file_name;
		const char *entry_point;
		FILETIME last_write_time;
	} ps_file_names_and_time[0x10];
	int ps_idx;
	
	ID3D11Buffer *buffer_array[0x10]; 
	ID3D11ShaderResourceView *srv_array[0x10]; 
	BFHandle buffer_handles[0x10];
	
	
	int buffer_idx;
	
} R_D3D11Context;


static void 
RendererD3D11CreateVSShader(R_D3D11Context *r, const char *file, const char *entry_point, 
							R_LayoutFormat format[], unsigned int format_size,
							ID3D11VertexShader **vshader, ID3D11InputLayout  **layout) {
	// Create Vertex Shader
	UINT flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#ifdef _DEBUG
	flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
	ID3DBlob *error  = NULL;
	ID3DBlob *vblob;
	if (FAILED(D3DCompileFromFile(Str8ToStr16(r->arena, Str8Lit(file)).data, NULL, NULL, entry_point, "vs_5_0", flags, 0, &vblob, &error))) {
		const char *message = (const char *)error->GetBufferPointer();
		OutputDebugStringA(message);
		Assert(!"Failed to compile vertex shader!");
	}
	r->device->CreateVertexShader(vblob->GetBufferPointer(), vblob->GetBufferSize(), NULL, vshader);
	
	if (format && format_size > 0) {
		// Vertex Shader Fromat Descriptor
		D3D11_INPUT_ELEMENT_DESC *vs_format_desc = (D3D11_INPUT_ELEMENT_DESC *)malloc(format_size * sizeof(D3D11_INPUT_ELEMENT_DESC)); 
		
		for (unsigned int i = 0; i < format_size; i++) {
			vs_format_desc[i].SemanticName = format[i].semantic_name; 
			vs_format_desc[i].SemanticIndex = 0;
			vs_format_desc[i].Format = g_renderer_to_dxgi_format[format[i].format];
			vs_format_desc[i].InputSlot = format[i].input_slot;
			vs_format_desc[i].AlignedByteOffset = format[i].aligned_byte_offset;
			vs_format_desc[i].InputSlotClass = (D3D11_INPUT_CLASSIFICATION)format[i].input_slot_class;
			vs_format_desc[i].InstanceDataStepRate = 0;
		};
		
		r->device->CreateInputLayout(vs_format_desc, format_size, vblob->GetBufferPointer(), vblob->GetBufferSize(), layout);
		free(vs_format_desc);
	}
	vblob->Release();
}

static void 
RendererD3D11CreatePSShader(R_D3D11Context *r, const char *file, const char *entry_point, ID3D11PixelShader **pshader) {
	
	{
	// Create Pixel Shader 
	UINT flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#ifdef _DEBUG
	flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
		ID3DBlob *error = NULL;
	ID3DBlob *pblob;
	if (FAILED(D3DCompileFromFile(Str8ToStr16(r->arena, Str8Lit(file)).data, NULL, NULL, entry_point, "ps_5_0", flags, 0, &pblob, &error))) {
		const char *message = (const char *)error->GetBufferPointer();
		OutputDebugStringA(message);
		Assert(!"Failed to compile pixel shader!");
	}
	r->device->CreatePixelShader(pblob->GetBufferPointer(), pblob->GetBufferSize(), NULL, pshader);
	pblob->Release();
	}
	
	
}

static VSHandle
RendererCreateVSShader(R_D3D11Context *r, const char *file, const char *entry_point, 
			R_LayoutFormat *format, unsigned int format_count) {
	
	VSHandle handle = { r->vs_idx };
	r->vs_handles[r->vs_idx] = handle;
	r->vs_file_names_and_time[r->vs_idx].file_name = file;
	r->vs_file_names_and_time[r->vs_idx].entry_point= entry_point;
	
	
	if (format) {
		unsigned int format_size = sizeof(format[0])*format_count;
		R_LayoutFormat *format_copy = (R_LayoutFormat *)MemArenaAlloc(r->arena, format_size);
		memcpy(format_copy, format, format_size); 
	r->vs_file_names_and_time[r->vs_idx].format = format_copy;
	}
	r->vs_file_names_and_time[r->vs_idx].format_size = format_count;
	r->vs_idx++;
	return handle;
}

static PSHandle
RendererCreatePSShader(R_D3D11Context *r, const char *file, const char *entry_point) {
	
	
	PSHandle handle = { r->ps_idx };
	r->ps_handles[r->ps_idx] = handle;
	r->ps_file_names_and_time[r->ps_idx].file_name= file;
	r->ps_file_names_and_time[r->ps_idx].entry_point = entry_point;
	r->ps_idx++;
	
	return handle;
}

// Buffer Creation Flags
#define BUFFER_FLAGS_TABLE \
X(R_BIND_VERTEX_BUFFER, D3D11_BIND_VERTEX_BUFFER),     \
X(R_BIND_INDEX_BUFFER, D3D11_BIND_INDEX_BUFFER),       \
X(R_BIND_CONSTANT_BUFFER, D3D11_BIND_CONSTANT_BUFFER), \
X(R_BIND_SHADER_RESOURCE, D3D11_BIND_SHADER_RESOURCE), \
X(R_BIND_RENDER_TARGET, D3D11_BIND_RENDER_TARGET),     \
X(R_USAGE_DEFAULT, D3D11_USAGE_DEFAULT),               \
X(R_USAGE_IMMUTABLE, D3D11_USAGE_IMMUTABLE),           \
X(R_USAGE_DYNAMIC, D3D11_USAGE_DYNAMIC),               \
X(R_USAGE_STAGING, D3D11_USAGE_STAGING),               \
X(R_CPU_ACCESS_WRITE, D3D11_CPU_ACCESS_WRITE),         \
X(R_CPU_ACCESS_READ, D3D11_CPU_ACCESS_READ),           \
X(R_RESOURCE_MISC_BUFFER_STRUCTURED, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED), \


#define X(a, b) a
enum R_BUFFER_FLAGS {
	BUFFER_FLAGS_TABLE
};
#undef X

const static int g_renderer_to_d3d11_buffer_flags[] = {
#define X(a, b) b
	BUFFER_FLAGS_TABLE
#undef X
};

// Last 2 bits of handle 
#define TYPE_VERTEX_BUFFER          (0)
#define TYPE_INDEX_BUFFER           (1)
#define TYPE_CONSTANT_BUFFER        (2)
#define TYPE_SHADER_RESOURCE_BUFFER (3)

typedef struct {
	u8 usage;
	u8 cpu_access; 
	 u8 misc; 
} R_BufferInfo;

static BFHandle
RendererCreateBuffer(R_D3D11Context *r, void *data, u32 elem_size, u32 elem_count, u8 bind, R_BufferInfo info) {

	ID3D11Buffer *buffer = NULL;
	ID3D11ShaderResourceView* buffer_srv = NULL;
	{
		
		D3D11_BUFFER_DESC desc = {0};

		desc.ByteWidth = (elem_size * elem_count) + 0xf & 0xfffffff0;  // constant buffers must be aligned to 16 boundry
		desc.Usage = info.usage ? (D3D11_USAGE)g_renderer_to_d3d11_buffer_flags[info.usage] : D3D11_USAGE_DEFAULT;
		desc.BindFlags = g_renderer_to_d3d11_buffer_flags[bind];
		desc.CPUAccessFlags = info.cpu_access ? g_renderer_to_d3d11_buffer_flags[info.cpu_access] : 0;
		desc.MiscFlags = info.misc ? g_renderer_to_d3d11_buffer_flags[info.misc] : 0;
		desc.StructureByteStride = elem_size;

		D3D11_SUBRESOURCE_DATA subresource_data = {data};
		r->device->CreateBuffer(&desc, data ? &subresource_data : NULL, &buffer);

		if (info.misc == R_RESOURCE_MISC_BUFFER_STRUCTURED) {
			D3D11_SHADER_RESOURCE_VIEW_DESC rv_desc = {0};
			rv_desc.Format = DXGI_FORMAT_UNKNOWN;
			rv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			rv_desc.Buffer.NumElements = elem_count;
			r->device->CreateShaderResourceView(buffer, &rv_desc, &buffer_srv);
		}
		
	}

	// last 5 bits refer to the type of buffer it is
	BFHandle handle = { r->buffer_idx | (bind << (32-5)) };
	r->buffer_array[r->buffer_idx] = buffer;
	r->srv_array[r->buffer_idx] = buffer_srv;
	r->buffer_handles[r->buffer_idx] = handle;
	r->buffer_idx++; 

	return handle;
}

static void
RendererUpdateBuffer(R_D3D11Context *r, BFHandle handle, void *data, unsigned int size) {
	Assert(r && data); 
	
	int idx = handle.val & ((1 << (32-5))-1);
	ID3D11Resource *buff = r->buffer_array[idx];
	Assert(buff);
	
	
		D3D11_MAPPED_SUBRESOURCE mapped;
		r->context->Map(buff, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		memcpy(mapped.pData, data, size);
		r->context->Unmap(buff, 0);
}

static void
RendererPSSetShader(R_D3D11Context *r, PSHandle handle) {
	if (handle.val == r->ps_handles[handle.val].val)  r->context->PSSetShader(r->ps_array[handle.val], NULL, 0);
}
static void
RendererVSSetShader(R_D3D11Context *r, VSHandle handle) {
	if (handle.val != r->vs_handles[handle.val].val)  return;
		
		r->context->VSSetShader(r->vs_array[handle.val], NULL, 0);
	r->context->IASetInputLayout(r->lay_array[handle.val]);
}

static void
RendererVSSetBuffer(R_D3D11Context *r, BFHandle handle) {
	int idx = handle.val & ((1 << (32-5))-1);
	if (handle.val!= r->buffer_handles[idx].val) return;
	
	int last_5_bits = handle.val >> (32-5);
	if (last_5_bits == TYPE_CONSTANT_BUFFER) {
		// TODO(ziv): start slot, amount of buffers, buffers
		r->context->VSSetConstantBuffers(0, 1, &r->buffer_array[idx]);
	}
	else if (last_5_bits == TYPE_SHADER_RESOURCE_BUFFER) {
		r->context->VSSetShaderResources(0, 1, &r->srv_array[idx]);
	}
	else {
		char buff[100]; 
		sprintf(buff, "Unimplemented Renderer Buffer Type (%d)", __LINE__); 
		FatalError(buff); 
	}
	
}

static void
RendererPSSetBuffer(R_D3D11Context *r, BFHandle handle) {
	int idx = handle.val & ((1 << (32-5))-1);
	if (handle.val!= r->buffer_handles[idx].val) return;
	
	int last_5_bits = handle.val >> (32-5);
	if (last_5_bits == TYPE_CONSTANT_BUFFER) {
		// TODO(ziv): start slot, amount of buffers, buffers
		r->context->PSSetConstantBuffers(0, 1, &r->buffer_array[idx]);
	}
	else if (last_5_bits == TYPE_SHADER_RESOURCE_BUFFER) {
		r->context->PSSetShaderResources(0, 1, &r->srv_array[idx]);
	}
	else {
		char buff[100]; 
		sprintf(buff, "Unimplemented Renderer Buffer Type (%d)", __LINE__); 
		FatalError(buff); 
	}
	
}

static void
RendererDrawInstanced(R_D3D11Context *r, unsigned int vertex_count_per_instance, unsigned int instance_count, 
					  unsigned int start_vertex_location, unsigned int start_instance_location) {
	// TODO(ziv): add support for more topologies
	
	r->context->DrawInstanced(vertex_count_per_instance, instance_count, start_vertex_location, start_instance_location);
}


static void
RendererD3D11GetShaders(R_D3D11Context *r,
						ID3D11VertexShader **vshader, wchar_t *vs_file,
						ID3D11PixelShader **pshader, wchar_t *ps_file,
						ID3D11InputLayout **layout, 
						const D3D11_INPUT_ELEMENT_DESC vs_format_desc[], 
						unsigned int format_desc_length) {
	
	HRESULT hr;
	// Create Vertex And Pixel Shaders
	UINT flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#ifdef _DEBUG
	flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
	ID3DBlob *error;
	ID3DBlob *vblob;
	hr = D3DCompileFromFile(vs_file, NULL, NULL, "vs_main", "vs_5_0", flags, 0, &vblob, &error);
	if (FAILED(hr)) {
		const char *message = (const char *)error->GetBufferPointer();
		OutputDebugStringA(message);
		Assert(!"Failed to compile vertex shader!");
	}
	r->device->CreateVertexShader(vblob->GetBufferPointer(), vblob->GetBufferSize(), NULL, vshader);
	
	if (vs_format_desc && format_desc_length > 0) {
		r->device->CreateInputLayout(vs_format_desc, format_desc_length,
									 vblob->GetBufferPointer(), vblob->GetBufferSize(), layout);
	}
	
	ID3DBlob *pblob;
	hr = D3DCompileFromFile(ps_file, NULL, NULL, "ps_main", "ps_5_0", flags, 0, &pblob, &error);
	if (FAILED(hr)) {
		const char *message = (const char *)error->GetBufferPointer();
		OutputDebugStringA(message);
		Assert(!"Failed to compile pixel shader!");
	}
	r->device->CreatePixelShader(pblob->GetBufferPointer(), pblob->GetBufferSize(), NULL, pshader);
	
	vblob->Release();
	pblob->Release();
}

static ID3D11Buffer **
RendererBFToPointer(R_D3D11Context *r, BFHandle handle) {
	int idx = handle.val & ((1 << (32-5))-1);
	if (handle.val != r->buffer_handles[idx].val) return NULL;
	return &r->buffer_array[idx]; 
}

enum R_Topology {
	R_TOPOLOGY_TRIANGLESTRIP = 0, 
	R_TOPOLOGY_TRIANGLELIST,
	R_TOPOLOGY_COUNT,
};

static D3D11_PRIMITIVE_TOPOLOGY r_topology_to_d3d11_topology[] = { 
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, 
};

static void
RendererInit(R_D3D11Context *r, HWND window) {
	
	// NOTE(ziv): if this fails you have failed to add a memeber to the 'r_topology_to_d3d11_topology' array and you are required to do so
	Assert(ArrayLength(r_topology_to_d3d11_topology) == R_TOPOLOGY_COUNT);
	
	r->arena = (Arena *)malloc(sizeof(Arena));
	MemArenaInit(r->arena, 0x1000); 
	
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
	}
	
	// Create a Rasterizer
	ID3D11RasterizerState1* rasterizer_cull_front;
	{
		D3D11_RASTERIZER_DESC1 rasterizer_desc = {};
		rasterizer_desc.FillMode = D3D11_FILL_SOLID; // D3D11_FILL_WIREFRAME;
		rasterizer_desc.CullMode = D3D11_CULL_FRONT;
		
		device->CreateRasterizerState1(&rasterizer_desc, &rasterizer_cull_front);
	}

	// Create Default Point Sampler
	ID3D11SamplerState* point_sampler;
	{
		D3D11_SAMPLER_DESC sampler_desc = {};
		sampler_desc.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT;
		sampler_desc.AddressU       = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.AddressV       = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.AddressW       = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		
		device->CreateSamplerState(&sampler_desc, &point_sampler);
	}
	// Create Default Linear Sampler
	ID3D11SamplerState* linear_sampler;
	{
		D3D11_SAMPLER_DESC sampler_desc = {};
		sampler_desc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampler_desc.AddressU       = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.AddressV       = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.AddressW       = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		
		device->CreateSamplerState(&sampler_desc, &linear_sampler);
	}
		
	// Set all information into the context
	{
	r->window = window; 
	r->device = device; 
	r->context = context;
	r->swap_chain = swap_chain;
	r->frame_buffer_view = frame_buffer_view;
	r->depth_stencil_state = depth_stencil_state;
	r->zbuffer = zbuffer;
	r->zbuffer_texture = zbuffer_texture;
	r->rasterizer_cull_back = rasterizer_cull_back;
	r->rasterizer_cull_front = rasterizer_cull_front;
	r->point_sampler = point_sampler; 
	r->linear_sampler = linear_sampler;
	}
	
}
static void
RendererDeInit(R_D3D11Context *r) {
	r->device->Release();
	r->context->Release();
	r->swap_chain->Release();
	r->frame_buffer_view->Release();
	r->depth_stencil_state->Release();
	r->zbuffer->Release();
	r->zbuffer_texture->Release();
	r->rasterizer_cull_back->Release();
	
	
	for (int i = 0; i < r->vs_idx; i++) {
		if (r->vs_array[i]) r->vs_array[i]->Release(); 
		if (r->lay_array[i]) r->lay_array[i]->Release(); 
		
	}
	
	for (int i = 0; i < r->ps_idx; i++) {
		if (r->ps_array[i]) r->ps_array[i]->Release(); 
	}
	
	for (int i = 0; i < r->buffer_idx; i++) {
		if (r->buffer_array[i]) r->buffer_array[i]->Release(); 
		if (r->srv_array[i]) r->srv_array[i]->Release(); 
	}
	
}

static void
RendererD3D11Resize(R_D3D11Context *r, UINT width, UINT height) {
	Assert(r);
	
	if (r->frame_buffer_view) {
		r->context->ClearState();
		r->frame_buffer_view->Release();
		r->frame_buffer_view = NULL;
		
		r->zbuffer_texture->Release();
		r->zbuffer->Release();
	}
	
	
	if (width != 0 && height != 0) {
		HRESULT hr;
		hr = r->swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
		if (FAILED(hr)) FatalError("Failed to resize the swap chain!");
		
		// Create a new RenderTarget for the new back buffer texture
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
	
	r->dirty = 1;
	r->viewport->Width = (FLOAT)width;
	r->viewport->Height = (FLOAT)height;
}

static void
RendererD3D11UpdateBuffer(R_D3D11Context *r, ID3D11Buffer *buff, void *data, unsigned int data_size) {
	Assert(r && buff && data); 
	if (data_size > 0) {
		D3D11_MAPPED_SUBRESOURCE mapped;
		r->context->Map((ID3D11Resource *)buff, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		memcpy(mapped.pData, data, data_size);
		r->context->Unmap((ID3D11Resource *)buff, 0);
	}
}

static void
RendererD3D11Present(R_D3D11Context *r) { 
	
	// present the backbuffer to the screen
	bool vsync = TRUE; 
	HRESULT hr = r->swap_chain->Present(vsync, 0); // Using here VSYNC
	if (hr == DXGI_STATUS_OCCLUDED)
	{
		// window is minimized, cannot vsync - instead sleep a bit
		if (vsync)
		{
			Sleep(10);
		}
	}
	else if (FAILED(hr))
	{
		FatalError("Failed to present swap chain! Device lost?");
	}
}

//~ renderer.h

typedef struct {
	char name[0x10]; 
	R_Format format;
	int offset;
} R_Layout;

typedef struct {
	const char *vs; 
	const char *vs_ep; // entry point
	const char *ps; 
	const char *ps_ep; // entry point
	R_Layout layout[0x10];
} R_Shader_Desc;

typedef struct {
	ID3D11VertexShader *vshader; 
	ID3D11InputLayout  *layout;
	ID3D11PixelShader *pshader;
} R_Shader; 

typedef struct {
	void *data;
	int element_count;
	int element_size;
	
	int usage;            // how will the buffer get used (default, immutable, dynamic, staging) 
	int bind_flags;       // type of buffer: constant, vertex, index ...
	int cpu_access_flags; // access the cpu has to the buffer
	int misc;             // additional buffer types and things idk
} R_Buffer_Desc;

typedef struct {
	ID3D11Buffer *buffer;
	ID3D11ShaderResourceView* buffer_srv;
	R_Buffer_Desc *desc; 
} R_Buffer;  

enum R_Sampler_Flag {
	R_SAMPLER_POINT,  // point sampler 
	R_SAMPLER_LINEAR, // linear interpolation sampler
};

typedef struct {
	void *data; 
	int width; 
	int height;
	int format;
	int bind_flags; // RenderTargetView/ShaderResourceView
	int cpu_access_flags; 
	int usage;
	int miplevels;
	// TODO(ziv): consider adding mmsa information 
} R_Texture2D_Desc;

typedef struct { 
	ID3D11ShaderResourceView* srv; // shader input
	ID3D11RenderTargetView *rtv; // shader ouptut
} R_Texture2D; 

typedef struct {
	R_Topology topo; 
	R_Shader shaders; 
	R_Buffer vs_bindings[0x10]; 
	R_Buffer ps_bindings[0x10]; 
	R_Texture2D textures[0x10]; // these are inputs into the ps
	R_Sampler_Flag samplers[0x10];
} R_Pipline_Desc;

typedef struct {
	float x, y, width, height;
	float min_depth, max_depth; // depth of image (usually 0-1)
} R_Viewport;

typedef struct {
	ID3D11BlendState1 *blend;
} R_BlendState;

enum R_Cull_Mode_Flags { 
	R_CULL_MODE_NULL = 0, // remove culling
	R_CULL_MODE_BACK, 
	R_CULL_MODE_FRONT,
};



static R_Buffer
r_create_buffer(R_D3D11Context *r, R_Buffer_Desc *desc) {
	
	ID3D11Buffer *buffer = NULL;
	ID3D11ShaderResourceView* buffer_srv = NULL;
	{
		
		D3D11_BUFFER_DESC d3d11_desc = {0};
		
		d3d11_desc.ByteWidth = (desc->element_size * desc->element_count) + 0xf & 0xfffffff0; // constant buffers must be aligned to 16 boundry
		d3d11_desc.Usage = desc->usage ? (D3D11_USAGE)g_renderer_to_d3d11_buffer_flags[desc->usage] : D3D11_USAGE_DEFAULT;
		d3d11_desc.BindFlags = g_renderer_to_d3d11_buffer_flags[desc->bind_flags];
		d3d11_desc.CPUAccessFlags = desc->cpu_access_flags ? g_renderer_to_d3d11_buffer_flags[desc->cpu_access_flags] : 0;
		d3d11_desc.MiscFlags = desc->misc ? g_renderer_to_d3d11_buffer_flags[desc->misc] : 0;
		d3d11_desc.StructureByteStride = desc->element_size;
		
		D3D11_SUBRESOURCE_DATA subresource_data = {desc->data};
		r->device->CreateBuffer(&d3d11_desc, desc->data ? &subresource_data : NULL, &buffer);
		
		if (desc->misc == R_RESOURCE_MISC_BUFFER_STRUCTURED) {
			D3D11_SHADER_RESOURCE_VIEW_DESC rv_desc = {0};
			rv_desc.Format = DXGI_FORMAT_UNKNOWN;
			rv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			rv_desc.Buffer.NumElements = desc->element_count;
			r->device->CreateShaderResourceView(buffer, &rv_desc, &buffer_srv);
		}
		
	}
	
	R_Buffer result = {
		buffer, buffer_srv, desc
	};
	
	return result; 
}

static R_Texture2D
r_create_texture2d(R_D3D11Context *r, R_Texture2D_Desc *desc) {

		D3D11_TEXTURE2D_DESC d3d_desc = {};
		d3d_desc.Width = (UINT)desc->width;
		d3d_desc.Height = (UINT)desc->height;
		d3d_desc.MipLevels = desc->miplevels;
		d3d_desc.ArraySize = 1;
		d3d_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // TODO(ziv): CHANGE THIS!
		d3d_desc.SampleDesc.Count = 1;
		d3d_desc.SampleDesc.Quality = 0;
		d3d_desc.Usage = desc->usage ? (D3D11_USAGE)g_renderer_to_d3d11_buffer_flags[desc->usage] : D3D11_USAGE_DEFAULT;
		d3d_desc.BindFlags = g_renderer_to_d3d11_buffer_flags[desc->bind_flags];

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem     = desc->data;
		data.SysMemPitch = (UINT)desc->width * sizeof(UINT); // TOOD(ziv): change this to format size

		ID3D11Texture2D* texture2d;
		r->device->CreateTexture2D(&d3d_desc, &data, &texture2d);

		ID3D11ShaderResourceView* texture_resource_view = NULL;
		ID3D11RenderTargetView *texture_render_target_view = NULL;
		if (d3d_desc.BindFlags == D3D11_BIND_SHADER_RESOURCE)
			r->device->CreateShaderResourceView(texture2d, NULL, &texture_resource_view);
	if (d3d_desc.BindFlags == D3D11_BIND_RENDER_TARGET)
			r->device->CreateRenderTargetView(texture2d, NULL, &texture_render_target_view);
		texture2d->Release();

		R_Texture2D result = {
			texture_resource_view, texture_render_target_view
		}; 
		return result;
}



static R_Shader
r_create_shaders(R_D3D11Context *r,  R_Shader_Desc *desc) {
	
	
	ID3D11VertexShader *vshader; 
	ID3D11InputLayout  *layout;
	
		// Create Vertex Shader
	{
		UINT flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#ifdef _DEBUG
		flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
		ID3DBlob *error  = NULL;
		ID3DBlob *vblob;
		if (FAILED(D3DCompileFromFile(Str8ToStr16(r->arena, Str8Lit(desc->vs)).data, NULL, NULL, desc->vs_ep, "vs_5_0", flags, 0, &vblob, &error))) {
			const char *message = (const char *)error->GetBufferPointer();
			OutputDebugStringA(message);
			Assert(!"Failed to compile vertex shader!");
		}
		r->device->CreateVertexShader(vblob->GetBufferPointer(), vblob->GetBufferSize(), NULL, &vshader);
		
		if (desc->layout[0].name != NULL) {
			// Vertex Shader Fromat Descriptor
			D3D11_INPUT_ELEMENT_DESC vs_format_desc[0x10]; 
			
			unsigned int i = 0;
			for (; i < 0x10 && desc->layout[i].name[0] != NULL; i++) {
				vs_format_desc[i].SemanticName = desc->layout[i].name; 
				vs_format_desc[i].SemanticIndex = 0;
				vs_format_desc[i].Format = g_renderer_to_dxgi_format[desc->layout[i].format];
				vs_format_desc[i].InputSlot = 0; 
				vs_format_desc[i].AlignedByteOffset = desc->layout[i].offset; 
				vs_format_desc[i].InputSlotClass = (D3D11_INPUT_CLASSIFICATION)0; // input per vertex data
				vs_format_desc[i].InstanceDataStepRate = 0;
			};
			
			r->device->CreateInputLayout(vs_format_desc, i, vblob->GetBufferPointer(), vblob->GetBufferSize(), &layout);
		}
		vblob->Release();
	}
	
	ID3D11PixelShader *pshader;
	{
		// Create Pixel Shader 
		UINT flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#ifdef _DEBUG
		flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
		ID3DBlob *error = NULL;
		ID3DBlob *pblob;
		if (FAILED(D3DCompileFromFile(Str8ToStr16(r->arena, Str8Lit(desc->ps)).data, NULL, NULL, desc->ps_ep, "ps_5_0", flags, 0, &pblob, &error))) {
			const char *message = (const char *)error->GetBufferPointer();
			OutputDebugStringA(message);
			Assert(!"Failed to compile pixel shader!");
		}
		r->device->CreatePixelShader(pblob->GetBufferPointer(), pblob->GetBufferSize(), NULL, &pshader);
		pblob->Release();
	}
	
	R_Shader result = { 
		vshader, 
		layout, 
		pshader
	};
	
	return result;
}

static void
r_switch_pipline(R_D3D11Context *r, R_Pipline_Desc *desc) {
	
	r->context->IASetPrimitiveTopology(r_topology_to_d3d11_topology[desc->topo]);
	r->context->IASetInputLayout(desc->shaders.layout);
	r->context->VSSetShader(desc->shaders.vshader, NULL, 0);
	r->context->PSSetShader(desc->shaders.pshader, NULL, 0);
	r->context->RSSetViewports(1,r->viewport);
	
	
	 int slot = 0;
	
		// bind all vs bindings
	for (int i = 0; i < 0x10 && desc->vs_bindings[i].buffer != NULL; i++) {
		Assert(desc->vs_bindings[i].desc);  // must be set by 'r_create_buffer'
		
		switch (desc->vs_bindings[i].desc->bind_flags) 
		{
			case R_BIND_VERTEX_BUFFER: {
				// vertex buffer
				const UINT stride = desc->vs_bindings[i].desc->element_size;
				const UINT offset = 0; // this can change and I might have uses for it
				// TODO(ziv): Think of ways to integrate this nicely into my API
				r->context->IASetVertexBuffers(0, 1, &desc->vs_bindings[0].buffer, &stride, &offset);
			} break; 
			
			case R_BIND_CONSTANT_BUFFER: {
				// constant buffer
				
				if (desc->vs_bindings[i].buffer_srv ) {
					r->context->VSSetShaderResources(0, 1, &desc->vs_bindings[i].buffer_srv);
				}
				else {
					r->context->VSSetConstantBuffers(slot++, 1, &desc->vs_bindings[i].buffer);
				}
				
			} break; 
			
			case R_BIND_INDEX_BUFFER: {
				r->context->IASetIndexBuffer(desc->vs_bindings[i].buffer, DXGI_FORMAT_R16_UINT, 0);
				
			} break; 
			
			case R_BIND_SHADER_RESOURCE: {
				r->context->VSSetShaderResources(0, 1, &desc->vs_bindings[i].buffer_srv);
			} break;
			
			
			default: {
				Assert(!"Unhandled case!!! Buffer created without buffer binding type.");
			} break; 
		}
		
		}
	
		// bind all the ps bindings
		for (int i = 0; i < 0x10 && desc->ps_bindings[i].buffer != NULL; i++) {
		Assert(desc->ps_bindings[i].desc);  // must be set by 'r_create_buffer'
		
		switch(desc->ps_bindings[i].desc->bind_flags) {
			
			case R_BIND_CONSTANT_BUFFER: {
				r->context->PSSetConstantBuffers(0, 1, &desc->ps_bindings[i].buffer);
			} break; 
			
			case R_BIND_SHADER_RESOURCE: {
				//r->context->PSSetShaderResources(0, 1, &texture_view);
				Assert(!"Not implemented");
			} break;
			
			default: {
				Assert(!"Tried to bind wrong buffer type to a pixel shader");
			} break;
		} 
	}
	
	
	// TODO(ziv): make this dynamic!!!!!
	
	// set samplers 
	r->context->PSSetSamplers(0, 1, &r->point_sampler);
	
	// set textures
	r->context->PSSetShaderResources(0, 1, &desc->textures[0].srv);
}

static void 
r_draw(R_D3D11Context *r, R_Texture2D target, int vertex_count, int start_vertex_position) {
	r->context->OMSetRenderTargets(1, &target.rtv, NULL);
	r->context->Draw(vertex_count, start_vertex_position);
}

static void 
r_draw_indexed(R_D3D11Context *r, R_Texture2D target, int index_count, 
			   int start_index_position, int start_vertex_position) {
	r->context->OMSetRenderTargets(1, &target.rtv, NULL);
	r->context->DrawIndexed(index_count, start_index_position, start_vertex_position);
}

static void 
r_draw_instanced(R_D3D11Context *r, R_Texture2D target, 
				 int vertex_count_per_instance, int instance_count, 
				 int start_vertex_position, int start_instance_position) {
	r->context->OMSetRenderTargets(1, &target.rtv, NULL);
	r->context->DrawInstanced(vertex_count_per_instance, instance_count, start_vertex_position, start_instance_position);
}

static void
r_fill_texture(R_D3D11Context *r, R_Texture2D texture, float color[4]) {
	r->context->ClearRenderTargetView(texture.rtv, color);
}

static void
r_set_cull_mode(R_D3D11Context *r, R_Cull_Mode_Flags mode) {
	if (mode == R_CULL_MODE_BACK) r->context->RSSetState(r->rasterizer_cull_back);
	else if (mode == R_CULL_MODE_FRONT) r->context->RSSetState(r->rasterizer_cull_front);
	else if (mode == R_CULL_MODE_NULL) r->context->RSSetState(NULL);
	else Assert(!"this cull mode does not exist");
}

static void
r_set_blend_mode(R_D3D11Context *r, R_BlendState *blend) {
	// TODO(ziv): what should i do about the two other variables 
	// I need while setting the blend state?
	r->context->OMSetBlendState(blend->blend, NULL, 0xffffffff); // set default
}

static void
r_set_viewport(R_D3D11Context *r, R_Viewport viewport) {
	// NOTE(ziv): This is a trick, it  can be done since
	// R_Viewport and D3D11_VIEWPORT are identical
	D3D11_VIEWPORT *d3d11_viewport = (D3D11_VIEWPORT *)&viewport;
	r->context->RSSetViewports(1, d3d11_viewport);
}

static void
r_update_gpu_buffer(R_D3D11Context *r, R_Buffer buffer, void *data, int size) {
	D3D11_MAPPED_SUBRESOURCE mapped;
	r->context->Map(buffer.buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, data, size);
	r->context->Unmap(buffer.buffer, 0);
}




//~ 
// Draw
// 

typedef struct {
	R_D3D11Context *r;

	matrix *proj; 
	matrix *view;

	// widgets
	struct {
		ID3D11BlendState1* blend_state_use_alpha;
		R_QuadInst data[MAX_QUAD_COUNT];
		int idx;
	} quads;

	// Font rendering
	struct {
		ID3D11ShaderResourceView *srv[2];
		ID3D11SamplerState* sampler; 
		R_SpriteInst data[MAX_SPRITES_COUNT];
		int idx;
	} font;

	struct { 
		float3 data[2*MAX_QUAD_COUNT]; 
		int idx; 
	} lines; 

	// font information
	PSHandle font_pshader;
	VSHandle font_vshader;
	BFHandle font_constant_buffer;
	BFHandle font_sprite_buffer; 

	// widgets information
	PSHandle widgets_pshader;
	VSHandle widgets_vshader;
	BFHandle widgets_cbuffer;
	BFHandle widgets_quads_buffer; 

	// lines information 
	PSHandle lines_pshader;
	VSHandle lines_vshader;
	BFHandle lines_vbuffer;
	BFHandle lines_cbuffer;
	BFHandle lines_structured_buffer;
} D_Context;

static void
DrawInit(D_Context *d) {
	Assert(d);
	if (!d->r) FatalError("Couldn't initialize drawing since renderer was not passed");

	// font information
	d->font_vshader = RendererCreateVSShader(d->r, "../font.hlsl", "vs_main", NULL, 0);
	d->font_pshader = RendererCreatePSShader(d->r, "../font.hlsl", "ps_main");

	// one-time calc here to make it easier for the shader later (float2 rn_screensize, r_atlassize)
	float font_constant_data[4] = {
		2.0f / window_width,
		-2.0f / window_height,
		1.0f / ATLAS_WIDTH,
		1.0f / ATLAS_HEIGHT
	};

	d->font_constant_buffer = RendererCreateBuffer(d->r, font_constant_data, sizeof(float), ArrayLength(font_constant_data), 
			R_BIND_CONSTANT_BUFFER, { R_USAGE_DYNAMIC, R_CPU_ACCESS_WRITE });
	d->font_sprite_buffer = RendererCreateBuffer(d->r, NULL, sizeof(R_SpriteInst), MAX_SPRITES_COUNT, R_BIND_SHADER_RESOURCE, 
			   { R_USAGE_DYNAMIC, R_CPU_ACCESS_WRITE, R_RESOURCE_MISC_BUFFER_STRUCTURED});

	// widgets information
	d->widgets_vshader = RendererCreateVSShader(d->r, "../widgets.hlsl", "vs_main", NULL, 0);
	d->widgets_pshader = RendererCreatePSShader(d->r, "../widgets.hlsl", "ps_main");

	float widgets_constants[4] = {2.f/(float)window_width,-2.f/(float)window_height,(float)window_height*1.f };
	d->widgets_cbuffer = RendererCreateBuffer(d->r, widgets_constants, sizeof(float), ArrayLength(widgets_constants), 
			R_BIND_CONSTANT_BUFFER, { R_USAGE_DYNAMIC,  R_CPU_ACCESS_WRITE });

	d->widgets_quads_buffer = RendererCreateBuffer(d->r, NULL, sizeof(R_QuadInst), MAX_QUAD_COUNT, 
			R_BIND_SHADER_RESOURCE, { R_USAGE_DYNAMIC, R_CPU_ACCESS_WRITE, R_RESOURCE_MISC_BUFFER_STRUCTURED });


	R_LayoutFormat line_format[] = {
		{ "Position", R_FORMAT_R32G32B32_FLOAT, 0, 0, R_INPUT_PER_VERTEX_DATA }, 
	}; 

	d->lines_vshader = RendererCreateVSShader(d->r, "../lines.hlsl", "vs", line_format, ArrayLength(line_format));
	d->lines_pshader = RendererCreatePSShader(d->r, "../lines.hlsl", "ps");


	d->lines_cbuffer = RendererCreateBuffer(d->r, NULL, sizeof(matrix), 1,
					 R_BIND_CONSTANT_BUFFER, { R_USAGE_DYNAMIC, R_CPU_ACCESS_WRITE });
	d->lines_structured_buffer = 
		RendererCreateBuffer(d->r, NULL, 2*sizeof(float3), MAX_QUAD_COUNT,
													  R_BIND_SHADER_RESOURCE, { 
														  R_USAGE_DYNAMIC, 
														  R_CPU_ACCESS_WRITE, 
														  R_RESOURCE_MISC_BUFFER_STRUCTURED 
													  });
}

static void 
DrawQuad(D_Context *d, Rect rect, Color color, float radius, float border) {
	Assert(d->quads.idx < MAX_QUAD_COUNT);
	
	if (0 <= rect.maxx && rect.minx <= d->r->viewport->Width && 
		0 <= rect.maxy && rect.miny <= d->r->viewport->Height) {
		d->quads.data[d->quads.idx].rect = rect;
		d->quads.data[d->quads.idx].color = color;
		d->quads.data[d->quads.idx].radius = radius;
		d->quads.data[d->quads.idx].border = border;
		d->quads.idx++;
	}
	
}

static void 
DrawLines(D_Context *d, float3 *lines, int line_count) {
	
	for (int i = 0; i < line_count*2; i++) { 
		d->lines.data[d->lines.idx++] = lines[i]; 
	}
}

static void 
DrawLine(D_Context *d, float3 p0, float3 p1) {
	float3 line[] = { p0, p1 }; 
	DrawLines(d, line, 1);
}

static void 
DrawDefaultText(D_Context *d, const char *text, size_t len,  float x, float  y) {
	R_SpriteInst sprite = { (float)x, (float)y, (float)(ATLAS_WIDTH / CHARACTER_COUNT)*FAT_PIXEL_SIZE, (float)ATLAS_HEIGHT*FAT_PIXEL_SIZE }; // screen_pos, size
	for (size_t i = 0; i < len; i++) {
		sprite.atlas_pos.x = (text[i] - ' ') * (int)(ATLAS_WIDTH/CHARACTER_COUNT) ;
		d->font.data[d->font.idx++] = sprite;
		sprite.screen_pos.x += sprite.size.x+ 1*FAT_PIXEL_SIZE;
	}

}

static void
DrawSubmitRenderCommands(D_Context *d) {
	R_D3D11Context *r = d->r;

	// Draw Lines
	if (0){
		const UINT stride = sizeof(float3);
		const UINT offset = 0;

		// Input Assembler
		r->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		//RendererUpdateBuffer(r, d->lines_vbuffer, d->lines.data, d->lines.idx*2*sizeof(float3));

		matrix transform = d->view[0] * d->proj[0];
		RendererUpdateBuffer(r, d->lines_cbuffer, &transform , sizeof(transform));
		RendererUpdateBuffer(r, d->lines_structured_buffer, d->lines.data, d->lines.idx*2*sizeof(float3));

		RendererVSSetBuffer(r, d->lines_cbuffer);
		RendererVSSetBuffer(r, d->lines_structured_buffer);
		RendererVSSetShader(r, d->lines_vshader);

		r->context->RSSetViewports(1, r->viewport);
		RendererPSSetShader(r, d->lines_pshader);
		r->context->OMSetRenderTargets(1, &r->frame_buffer_view, NULL);
		RendererDrawInstanced(r, 2, d->lines.idx, 0, 0);
	}

	// Draw Quads
	{
	
		// TODO(ziv): make sure that if graph has not changed from last time
		// there is no need to upload different data to the gpu. 
		// which would reduce the data transfers to the gpu. 
		if (d->quads.idx > 0) {

			// update buffers
			float quads_constants[4] = {2.f/(float)window_width,-2.f/(float)window_height,(float)window_height*1.f };
			RendererUpdateBuffer(r, d->widgets_quads_buffer, d->quads.data, d->quads.idx*sizeof(d->quads.data[0]));
			RendererUpdateBuffer(r, d->widgets_cbuffer, quads_constants, sizeof(quads_constants));
			// render the quads
			RendererVSSetShader(r, d->widgets_vshader); 
			RendererVSSetBuffer(r, d->widgets_cbuffer);
			RendererVSSetBuffer(r, d->widgets_quads_buffer);

			// TODO(ziv): Figure out how to control the viewport and render-targets
			r->context->RSSetViewports(1, r->viewport);
			RendererPSSetShader(r, d->widgets_pshader); 
			r->context->OMSetRenderTargets(1, &r->frame_buffer_view, NULL);

			UINT sampleMask   = 0xffffffff;
			r->context->OMSetBlendState(d->quads.blend_state_use_alpha, NULL, sampleMask);
			r->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			RendererDrawInstanced(r, 4, d->quads.idx, 0, 0);

		}
	}

	// Draw Font
	{

		if (r->dirty) {
			// update constants buffer
			float font_constant_data[4] = {2.0f / window_width,-2.0f / window_height,1.0f / ATLAS_WIDTH,1.0f / ATLAS_HEIGHT };
			RendererUpdateBuffer(r, d->font_constant_buffer, font_constant_data, sizeof(font_constant_data));
		}
		RendererUpdateBuffer(r, d->font_sprite_buffer, d->font.data, d->font.idx*sizeof(d->font.data[0])); 


		RendererVSSetShader(r, d->font_vshader);
		RendererVSSetBuffer(r, d->font_constant_buffer);
		RendererVSSetBuffer(r, d->font_sprite_buffer);

		r->context->RSSetViewports(1, r->viewport);
		RendererPSSetShader(r, d->font_pshader);
		r->context->PSSetShaderResources(1, 1, &d->font.srv[1]);
		r->context->PSSetSamplers(0, 1, &d->font.sampler);
		r->context->OMSetRenderTargets(1,&r->frame_buffer_view, NULL);
		r->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		RendererDrawInstanced(r, 4, d->font.idx, 0, 0);
	}
}

//~
// UI Core
//

// The UI system works exactly the same as in ryan fleury's ui library[6]:
// Core API on which handles all behaviour, layout, core drawing
// Wrappers around that core API which are the default widgets themselves
// Extensions to the core API that have custom drawing and layout scheme
//
// This might be slightly weird to comprehend but:
// he first build the graph higherarchy
// layouts it
// renders it
// figures out interaction with the graph
//
// The building of the graph and interaction with it happens on the same function 
// yet the interaction with the graph happens on the next frame.
//

typedef enum {
	UI_CLICKABLE  = 1 << 0,
	UI_SLIDERABLE = 1 << 1,
	UI_INPUTABLE  = 1 << 3,
	UI_DRAGGABLE  = 1 << 4,
	UI_FLOAT_X    = 1 << 5,
	UI_FLOAT_Y    = 1 << 6,
	UI_DRAWBOX    = 1 << 7,
	UI_DRAWTEXT   = 1 << 8,
	UI_DRAWBORDER = 1 << 9, 
	UI_ANIMATE_HOT = 1 << 10
} UI_Flags;

enum UI_SizeKind {
	UI_SIZEKIND_NULL,            // no size
	UI_SIZEKIND_PIXELS,          // size in pixels
	UI_SIZEKIND_TEXTCONTENT,     // figure out text size
	UI_SIZEKIND_PERCENTOFPARENT, // take some percentage of parent
	UI_SIZEKIND_CHILDRENSUM,     // use the sum sizing of children
	UI_SIZEKIND_CHILDRENMAX,     // largest children value on axis
};

typedef struct {
	UI_SizeKind kind;
	float value;      // if pixel kind, this decided the pixel count of the axis
	float strictness; // how strict we want the value itself to be
} UI_Size;

enum UI_Axis {
	UI_AXIS2_X,
	UI_AXIS2_Y,
	UI_AXIS2_COUNT
};


typedef struct {
	s32 key, alive;
	String8 value;
} UIID;

typedef struct UI_Widget UI_Widget;
struct UI_Widget {
	u32 flags; 
	
	UIID id;
	String8 text;
	
	UI_Axis axis; // layout axis
	UI_Size semantic_size[UI_AXIS2_COUNT];
	
	UI_Widget *parent;
	UI_Widget *child;
	UI_Widget *next;
	UI_Widget *last;
	
	// hash table
	UI_Widget *hash_next;
	UI_Widget *hash_last;
	
	float computed_rel_pos[UI_AXIS2_COUNT];
	float computed_size[UI_AXIS2_COUNT];
	Rect rect; // final computed rect
	
	// TODO(ziv): consider using radius per corner 
	float radius, border;
	float hot_t, active_t;
};

typedef struct {
	UI_Widget *widget;
	bool activated;
	bool clicked;
	bool hovered;
    float slider_value;
} UI_Output;

typedef int UI_Widget_Handle;

typedef struct UI_Theme {
	Color foreground_color;
	Color hot_color;
	Color active_color;
	float border; // size
} UI_Theme; 

typedef struct {
	OS_Events *events;
	OS_Event *events_to_eat[5];
	
	R_D3D11Context *r;
	D_Context *d;
	
	Arena arena;
	
	b32 used_press;
	float mouse_pos[2];
	
	// UI Graph
    UI_Widget widgets[WIDGETS_COUNT];
	UI_Widget *hashmap[WIDGETS_COUNT];
    
	int widgets_idx;
	
	UI_Widget *window;
	UI_Widget *hot, *active;
	
	UI_Widget *stack[MAX_WIDGET_STACK_SIZE];
	int stack_idx;
	
	b32 equip_child; 
	float equip_child_radius;
	float equip_child_border;
	
	
} UI_Context;

static UI_Context *ui = NULL;

static void
UICorePruneDeadWidgets(UI_Widget *head) {

	if (!head) return; 

	if (!head->id.alive) { // dead


		// remove from graph
		
		if (head->last) {
			// remove from middle
			head->last->next = head->next;
			if (head->next) {
				head->next->last = head->last;
			}
			head->last = NULL; 
		}
		else if (head->next) {
			// remove left node
			if (head->parent) {
				head->parent->child = head->next;
			}
		}
		else if (head->parent) {
			// remove only child
			head->parent->child = NULL; 
		}
		
		head->id = UIID{0}; 
		head->text = { 0, NULL }; 
		
		// remove from hash map
		
		if (head->hash_last) {
			// remove from middle
			head->hash_last->hash_next = head->hash_next;
			if (head->hash_next) {
				head->hash_next->hash_last = head->hash_last;
			}
			head->hash_last = NULL; 
		}
		else if (head->hash_next) {
			// remove left node
			if (ui->hashmap[head->id.key]) {
				ui->hashmap[head->id.key] = head->hash_next;
			}
		}
		else if (ui->hashmap[head->id.key]) {
			// remove only child
			ui->hashmap[head->id.key] = NULL; 
		}
		
		
	}
	head->id.alive = 0; // reset the alive flag
	
	UI_Widget *child = head->child; 
	for (; child; child = child->next) {
		
		UICorePruneDeadWidgets(child);
	}
	
}

static void
UICoreLayoutConstant(UI_Widget *widget, int axis) {
	if (!widget)  return; 
	switch (widget->semantic_size[axis].kind) {
		case UI_SIZEKIND_NULL: break;
		case UI_SIZEKIND_PIXELS: { widget->computed_size[axis] = widget->semantic_size[axis].value; } break;
		case UI_SIZEKIND_TEXTCONTENT: {
			if (widget->text.size) {
				widget->computed_size[axis] = axis == UI_AXIS2_X ? DefaultTextWidth(widget->text) : DefaultTextHeight(widget->text);
				widget->computed_size[axis] += 10;
			}
		} break;
	}
	
	UI_Widget *child = widget->child;
	for (; child; child = child->next) {
		UICoreLayoutConstant(child, axis);
	}
}

static void
UICoreLayoutPreOrderUpwardDependednt(UI_Widget *widget, int axis) {
	if (!widget)  return; 
	switch (widget->semantic_size[axis].kind) {
		case UI_SIZEKIND_PERCENTOFPARENT: {
			Assert(widget->parent);
			if (widget->flags) {
				widget->computed_size[axis] = floorf(widget->semantic_size[axis].value * widget->parent->computed_size[axis]);
			}
			else {
				widget->computed_size[axis] = widget->parent->computed_size[axis];
		}
		} break;
	}
	
	UI_Widget *child = widget->child;
	for (; child; child = child->next) {
		UICoreLayoutPreOrderUpwardDependednt(child, axis);
	}
}

static void
UICoreLayoutPostOrderDownwardsDependent(UI_Widget *widget, int axis) {
	if (!widget)  return; 
	
	UI_Widget *child = widget->child;
	for (; child; child = child->next) {
		UICoreLayoutPostOrderDownwardsDependent(child, axis);
	}
	
	switch (widget->semantic_size[axis].kind) {
		case UI_SIZEKIND_CHILDRENSUM: {
			
			if (!widget->child) {
				widget->computed_size[axis] = 0;
				break;
			}
			
			child = widget->child;
			if (!(child->flags >> 4)) { 
				// NOTE(ziv): if we don't draw this widget 
				// this means that this is a layout widget
				// so we skip it, and get to it's children
				child = child->child;
			}
			
			float sum = 0;
			for (; child; child = child->next) {
				sum += child->computed_size[axis];
			}
			
			widget->computed_size[axis] = sum;
			
		} break;
		
		
		case UI_SIZEKIND_CHILDRENMAX: 
		{
			
			if (!widget->child) {
				widget->computed_size[axis] = 0;
				break;
			}
			
			child = widget->child;
			if (!(child->flags >> 4)) { 
				child = child->child;
			}
			
			float max = 0;
			for (; child; child = child->next) {
				max = MAX(child->computed_size[axis], max);
			}
			
			widget->computed_size[axis] = max;
			
		} break;
	}
	
}
static void
UICoreLayoutRelPos(UI_Widget *widget, int axis) { 
	if (!widget)  return; 
	
	UI_Flags float_axis = (axis == UI_AXIS2_X) ? UI_FLOAT_X : UI_FLOAT_Y;
	if ((widget->flags & float_axis)) goto skip_computing_rel_pos; 
	
	if (widget->last) {
		
		// skip widgets with floating x,y relative positions
		// to get the last non floating widget
		UI_Widget *temp = widget;
		while (temp->last && (temp->last->flags & float_axis)) temp = temp->last;
		
		if (temp->last) {
			// should contain a none floating x,y rel pos
			if (axis == widget->axis) {
				if (widget->last->axis == axis) {
					widget->computed_rel_pos[axis] = temp->last->computed_rel_pos[axis] + temp->last->computed_size[axis];
				}
				else {
					// conflicting layouts 
					
					// seek parent for axis. 
					// if we are on that axis
					// great now we can add it 
					// if we are not on that axis
					// bad. don't do shit

					if (widget->parent->axis == axis) {
						widget->computed_rel_pos[axis] = temp->last->computed_rel_pos[axis] + temp->last->computed_size[axis];
					}
					else {
						widget->computed_rel_pos[axis] = temp->last->computed_rel_pos[axis];
					}
					
				}
			}
			else {
				
				widget->computed_rel_pos[axis] = temp->last->computed_rel_pos[axis] ;
				if (axis == widget->parent->axis) {
					widget->computed_rel_pos[axis] +=  temp->last->computed_size[axis];
				}
				
			}
		}
		else {
		// this is the last widget that doesn't have floating x and y 
		// so I take the parent's size 
			widget->computed_rel_pos[axis] = temp->parent->computed_rel_pos[axis];
		}
			
	}
	else if (widget->parent) {
		widget->computed_rel_pos[axis] = widget->parent->computed_rel_pos[axis];
	}
	
	skip_computing_rel_pos:
	UI_Widget *child = widget->child;
	for (; child; child = child->next) {
		UICoreLayoutRelPos(child, axis);
	}
}

static void
UICoreLayoutResolveConstraints(UI_Widget *widget, int axis) {
	if (!widget)  return; 
	
	// 
	// Go down and if you have a parent, figure out the total size 
	// of his children. then adjust yourself accordingly, and for each 
	// of the children do the same. 
	// (so everybody updates themselves in accordance to the updated scheme) 
	// 
	// I need to know the sizes of everyone and percent from parent's sizes
	//
	
	
	float children_overflow = 0;
	float children_overflowed_count = 0;
	if (widget->axis == axis) {
		
		// clamp widgets size to parent's size to prevent overflow
		if (widget->parent) {
			if (widget->computed_size[axis] > widget->parent->computed_size[axis]) {
				widget->computed_size[axis] = widget->parent->computed_size[axis];
			}
		}
		
		
		// I don't overflow (parent said) 
		// I need to figure out if children overflow
		
		float size_on_axis = 0; 
		int child_count = 0; 
		UI_Widget *child = widget->child;
		while (child) {
			
			if (child->flags & (UI_FLOAT_X | UI_FLOAT_Y)) {
				child = child->next;
				continue; 
			}
			
			size_on_axis += child->computed_size[axis]; 
			
			
			if (child->semantic_size[axis].strictness < 1) {
				child_count++;
			}
			
			child = child->next;
			
		}
		
		if (size_on_axis > widget->computed_size[axis]) {
			float overflow = (size_on_axis - widget->computed_size[axis]); 
			
			for (int i = 0; overflow > 0 && i < 3; i++) {
				UI_Widget *temp = widget->child;
				int children_amount = child_count; 
				while (temp) {
					
					if (temp->semantic_size[axis].strictness >= 1.f) {
						temp = temp->next;
						continue; 
					}
					
					// go over all children and apply
					float diff = overflow / (int)children_amount; 
					float max_removeable_amount = temp->computed_size[axis]*(1-temp->semantic_size[axis].strictness);
					float abs_diff = ABS(diff); 
					float amount_to_apply = MIN(abs_diff, max_removeable_amount);
					temp->computed_size[axis] -= amount_to_apply;
					
					if (temp->child) {
						// parent sizing changed, I need to update children sizing (if they depend on it)
						
						UI_Widget *child = temp->child; 
						while (child) {
						if (child->semantic_size[axis].kind == UI_SIZEKIND_PERCENTOFPARENT) {
							child->computed_size[axis] = temp->computed_size[axis]*child->semantic_size[axis].value; 
							}
							child = child->next; 
						}
						
					}
					
					overflow -= amount_to_apply;
					children_amount--;
						temp = temp->next; 
				}
			}
				
		}
		
	}
	else if (widget->parent) {
		
		// clamp widgets size to parent's size to prevent overflow
		if (widget->computed_size[axis] > widget->parent->computed_size[axis]) {
			widget->computed_size[axis] = widget->parent->computed_size[axis];
		}
		
	}
	
	UI_Widget *child = widget->child;
	for (; child; child = child->next) {
		UICoreLayoutResolveConstraints(child, axis);
	}
}
  
static void
UICoreLayout(UI_Widget *widget, int axis) {
	
	UICoreLayoutConstant(widget, axis);
	UICoreLayoutPreOrderUpwardDependednt(widget, axis); 
	UICoreLayoutPostOrderDownwardsDependent(widget, axis);
	UICoreLayoutResolveConstraints(widget, axis);
	UICoreLayoutRelPos(widget, axis); 
}

static void
UICoreLayoutFinalRect(UI_Context *ui, UI_Widget *head) {
	if (!head) return;

    for (; head; head = head->next) {
		// Compute final rects for all widgets
		head->rect.minx = head->computed_rel_pos[0];
		head->rect.miny = head->computed_rel_pos[1];
		head->rect.maxx = head->rect.minx + head->computed_size[0];
		head->rect.maxy = head->rect.miny + head->computed_size[1];

		//
		// Render widgets
		//

		if (head->flags & UI_DRAWBOX) {

			// update hot/active time
			float dt = 1.f/60.f; // TODO(ziv): move/remove this?
			head->hot_t = lerp(head->hot_t, (head == ui->hot), 1-powf(2.f, -4.f * dt));
			if (!(head->flags & UI_SLIDERABLE)) {
				head->active_t = lerp(head->active_t, (head == ui->hot), 1-powf(2.f, -4.f * dt));
			}


#define GRAY       Color{ 130.f/255.f, 130.f/255.f, 130.f/255.f, 255.f/255.f}
			// get final color 
			Color background = { 0.05f, 0.1f, .2f, 1}; 
			if (head->flags & UI_SLIDERABLE) {
				float darken = 0.5;
				background.r *= darken; background.g *= darken; background.b *= darken; 
			}
			Color hot = { 0.2f, 0.3f, .5f, 1};
			Color active = GRAY;
			Color final = background;
			if (head == ui->active) {
				head->active_t = (head->flags & UI_ANIMATE_HOT) ? head->active_t : 0;
				final.r = lerp(background.r, active.r, head->active_t);
				final.g = lerp(background.g, active.g, head->active_t);
				final.b = lerp(background.b, active.b, head->active_t);
				final.a = 1;
			}
			else if ((head == ui->hot) ) {
				head->hot_t = (head->flags & UI_ANIMATE_HOT) ? head->hot_t : 0;
				final.r = lerp(background.r, hot.r, head->hot_t);
				final.g = lerp(background.g, hot.g, head->hot_t);
				final.b = lerp(background.b, hot.b, head->hot_t);
				final.a = 1;
			}

			// draw the quad to the screen
			DrawQuad(ui->d, head->rect, final, head->radius, head->border); 
		}

		if (head->flags & UI_DRAWTEXT) {
			float pad = 5; 
			String8 txt = head->text;

			float rect_width = head->rect.maxx - head->rect.minx - 2 * pad; 
			float amount_to_show =  rect_width / DefaultTextWidth(txt);
			int display_size = (int)((float)txt.size *  MIN(amount_to_show, 1));

			const char *display_txt = txt.data;
			char dest[0x100]; 
			if (amount_to_show < 1.f) {
				display_txt = strcpy(dest, txt.data); 
				dest[display_size-1] = '.';
				dest[display_size-2] = '.';
				display_txt = dest;
			}

			DrawDefaultText(ui->d, display_txt, display_size, 
							head->rect.minx+pad, head->rect.miny+pad); 
		}


		if (head->child) {
			UICoreLayoutFinalRect(ui, head->child);
		}
	}
}

static void UIPushParent(UI_Widget *parent);
static inline UI_Widget *UIPopParent();

static void
UIInit(UI_Context *ctx, D_Context *d, OS_Events *events) {
	
	memset(ctx, 0, sizeof(UI_Context));
	ctx->events = events;
		ctx->r = d->r;
		ctx->d = d;
	
	ctx->window = &ctx->widgets[WIDGETS_COUNT-1];
	ctx->window->id = { WIDGETS_COUNT-1, 1, { 0, "main_window###0001" } };
	ctx->window->computed_size[UI_AXIS2_X] = (float)window_width;
	ctx->window->computed_size[UI_AXIS2_Y] = (float)window_height;
	
	
	MemArenaInit(&ctx->arena, 0x1000); // allocate page size
	
	
	ui = ctx;
	}


static void
UIBegin(UI_Context *ui, b32 dirty) {
	
	UIPushParent(ui->window);
	
	if (dirty) {
		ui->window->computed_size[UI_AXIS2_X] = (float)window_width;
		ui->window->computed_size[UI_AXIS2_Y] = (float)window_height;
	}
	ui->window->id.alive = 1;
	
	// some standard input setup
	{
		// TODO(ziv): abstract this into a os system
		bool window_not_focused = ui->r->window != GetActiveWindow();
		if (window_not_focused) {
			ui->mouse_pos[0] = -100;
			ui->mouse_pos[1] = -100;
		}
		else {
			POINT cursor_pos;
			GetCursorPos(&cursor_pos);
			ScreenToClient(ui->r->window, &cursor_pos);
			ui->mouse_pos[0] = (float)cursor_pos.x;
			ui->mouse_pos[1] = (float)cursor_pos.y;
		}
		
	}
	
	// Button actions for UI
	{ 
		ui->used_press = false;
		static int mouse_button_pressed = 0; 
		
		for (int i = 0; i < ui->events->idx; i++) {
			OS_Event e = ui->events->list[i];
			if (e.kind == OS_EVENT_KIND_MOUSE_BUTTON_DOWN) {
				mouse_button_pressed |= e.value[0];
			}
			else if (e.kind == OS_EVENT_KIND_MOUSE_BUTTON_UP) {
				mouse_button_pressed &= ~e.value[0];
			}
		}
		
		for (int i = 0; i < 5; i++) { 
			
			if (mouse_button_pressed & (1 << i)) {
				OS_Event e = {  
					OS_EVENT_KIND_MOUSE_BUTTON_PRESSED, 
					 1 << i, 0
				};
				ui->events->list[ui->events->idx++] = e;
				
			}
			
		}
		
		
	}
	
}


static void
UIEnd(UI_Context *ui) {
	Assert(ui);
	UIPopParent(); // pop window

	// prune out all widgets that don't participate in hierarchy
	{
		ui->window->id.alive = 1;
		UICorePruneDeadWidgets(ui->window);
	}

	// layout
	{
		for (int axis = 0; axis < UI_AXIS2_COUNT; axis++) { 
			UICoreLayout(ui->window, axis); 
		}
	}

	// draw
	{
		// kind of this is where I draw 
		UICoreLayoutFinalRect(ui, ui->window);
	}
	
	// eat taken events
	{
		if (ui->used_press) {
			for (int i = 0; i < ui->events->idx; i++) {
				if (ui->events->list[i].kind == OS_EVENT_KIND_MOUSE_BUTTON_PRESSED) {
					ui->events->list[i].kind = OS_EVENT_KIND_NULL;
				}
				
			}
		}
		
	}
	
	
}


static void
UIPushParent(UI_Widget *parent) {

	if (ui->stack_idx < MAX_WIDGET_STACK_SIZE) {
		ui->stack[ui->stack_idx++] = parent;
	}
	else {
		char message[0x100];
		sprintf(message, "%s:(%d) UIPushParent %d stack size is insufficient or UIPopParent not used.", __FILE__, __LINE__, MAX_WIDGET_STACK_SIZE);
		FatalError(message);
	}
	
}

static inline UI_Widget *
UIPopParent() {
	if (ui->stack_idx == 0) {
		FatalError("Poping more times than should be possible");
	}
	return ui->stack[--ui->stack_idx];
}

static inline UI_Widget *
UITopParent() { 
	return ui->stack_idx > 0 ? ui->stack[ui->stack_idx-1] : NULL;
}

static int
UICompareKey(UIID k1, UIID k2) {
	if (k1.key != k2.key) return 0; 
	return Str8Compare(k1.value, k2.value);
}

static UI_Widget *
UIMakeWidget(String8 text, u32 flags) {

	//
	// Get entry from hashmap if exists
	//

	String8 strid = text;
	s32 mask = (WIDGETS_COUNT-1);
	s32 key = (s32)Str8GetHash(strid, 234982374) & mask;
	UIID id = { key, 0,  strid };

	UI_Widget *entry = ui->hashmap[key];

	for (; entry; entry = entry->hash_next) {
		if (UICompareKey(entry->id, id))  break;
	}
	if (entry && UICompareKey(entry->id, id)) {
		entry->id.alive = 1;
		ui->equip_child = 0;
		return entry;
	}

    //
    // Create and add the widget to the graph
    //

    UI_Widget *parent = UITopParent();
	UI_Widget *widget = &ui->widgets[ui->widgets_idx++];

	// add to hashmap
	UI_Widget *temp = ui->hashmap[key];
	if (temp) {
	while (temp->hash_next) temp = temp->hash_next;
		temp->hash_next = widget;
		widget->hash_last = temp;
	}
	else {
		ui->hashmap[key] = widget;
	}

	// TODO(ziv): This should not be allocated in this arena thingy. should actually have 
	// something to manage these strings since the way they are allocated and things is 
	// more malloc/free style (which maybe I should use malloc/free idk). 
	//String8 id_string_copy = Str8Copy((char *)MemArenaAlloc(&ui->arena, strid.size), strid);

	// TODO(ziv): REPLACE MALLOC!!! 
	// OR USE FREE IDK JUST FREE MAN
	String8 id_string_copy = Str8Copy((char *)malloc(strid.size), strid);
	widget->id = { key, 1, id_string_copy  };
	widget->parent = parent;
    widget->child = NULL;
    widget->next = NULL;
    widget->flags = flags;
	if (parent) {
		widget->axis= parent->axis;
		widget->semantic_size[0] = parent->semantic_size[0];
		widget->semantic_size[1] = parent->semantic_size[1];
	}
	
	if (ui->equip_child) {
		widget->radius = ui->equip_child_radius; 
		widget->border = ui->equip_child_border; 
			ui->equip_child = 0;
	}
	else {
		widget->radius = 0; 
		widget->border = 0;
	}
	
	// ignore ### seperation
	String8 display_text = id_string_copy ;
	for (int i = 0; i < strid.size; i++) {
		if (i+3 < strid.size && 
			strid.data[i]   == '#' && 
			strid.data[i+1] == '#' && 
			strid.data[i+2] == '#') {
			
			// TODO(ziv): Don't use an arena allocator 
			// use something smarter to reuse the space
			// currently everytime a node is pruned 
			// and then created it allocated more memory
			// but never frees it.
			//char *data = (char *)MemArenaAlloc(&ui->arena, i+1);
			char *data = (char *)malloc(i+1);
				memcpy(data, strid.data, i); data[i] = '\0';
				
				display_text.data = data; 
				display_text.size = (size_t)i; 
				break;
			}
	}
	
	widget->text = display_text;
	
    // Push widget into hierarchy
    if (parent) {
        // connect to parents children
        if (parent->child && parent->child->id.value.data) {
            
			// see whether the widget exists in it's parent already
			UI_Widget *temp = parent->child;
			while (temp->next && !Str8Compare(temp->next->id.value, widget->id.value)) {
				if (temp == temp->next) {
					// wtf???
					Assert(false);
					break;
				}
				temp = temp->next;
			}
            
			// if it is, replace it
			if (Str8Compare(temp->id.value, widget->id.value)) {
				if (!temp->last && !temp->next) {
					// only child 
					parent->child = widget;
				}

				widget->last = temp->last; 
				widget->next = temp->next; 

				if (widget->last) widget->last->next = widget; 
				if (widget->next) widget->next->last = widget; 
			}
			else { // If not, just add it after last alive node
				// TODO(ziv): Make sure that when parent->child is the only 
				// child and it is not alive, correct behavior ensues

				UI_Widget *last_alive = parent->child;
				while (last_alive->next && last_alive->next->id.alive) { 
					last_alive = last_alive->next;
				}

				widget->last = last_alive;
				widget->next = last_alive->next;
				if (widget->next) widget->next->last = widget; 
				last_alive->next = widget; 
			}

        }
        else {
            widget->last = NULL;
            parent->child = widget;
        }

    }
    else {
		Assert(ui->window);
        FatalError("You pop way too much for sure");
    }

    return widget;
}

static UI_Output
UIInteractWidget(UI_Widget *widget) {
    UI_Output output = {0};
	
	b32 left_mouse_button_pressed = false;
	for (int i = 0; i < ui->events->idx; i++) {
		OS_Event *e = &ui->events->list[i]; 
		if (e->kind == OS_EVENT_KIND_MOUSE_BUTTON_PRESSED && 
			e->value[0] & MouseLeftButton) {
			left_mouse_button_pressed = true; 
			break;
		}
	}
	
	b32 used_press = false; 
	
	Rect rect = widget->rect;
    // Find if mouse is inside UI hit-box
	if (ui->active && (ui->active->flags & UI_DRAGGABLE) && 
		left_mouse_button_pressed ) {
		// TODO(ziv): REALLY?
		
		used_press = true;
	}
    else if (rect.minx <= ui->mouse_pos[0] &&
		ui->mouse_pos[0] <= rect.maxx &&
		rect.miny <= ui->mouse_pos[1] &&
		ui->mouse_pos[1] <= rect.maxy) {

		ui->hot = widget;

        // widget is active?
        if ((widget->flags & UI_CLICKABLE &&
				 left_mouse_button_pressed )) {
            ui->active = widget;
			used_press = true;
            output.activated = 1;
		}
		else  {
			if (widget == ui->active)  output.clicked = 1;
			if (widget->flags & UI_CLICKABLE) {
				ui->active = NULL;
			}
		}


    }
    else if ((ui->hot == widget || ui->active == widget) && 
			 !(widget->flags & (UI_SLIDERABLE|UI_DRAGGABLE))) {
		ui->hot = NULL;
        ui->active = NULL;
		used_press = true;
	}


	if (widget->flags & UI_SLIDERABLE  && ui->hot == widget) {
		if (left_mouse_button_pressed) {
			float width = rect.maxx - rect.minx; 
			float slider_value = (ui->mouse_pos[0]-rect.minx)/width;
			widget->active_t = MIN(slider_value, 1);
			widget->active_t = MAX(widget->active_t, 0);
			used_press = true;
		}
		else {
			ui->hot = NULL;
		}
	}
	
	ui->used_press |= used_press; 
	
	output.slider_value = widget->active_t;
	output.widget = widget;

	return output;
	
}

//~ 
// Helpers (sugar for internal operations)
// 

static inline void UIEquipChildRadius(float radius) { ui->equip_child = 1; ui->equip_child_radius = radius; }
static inline void UIEquipChildBorder(float border) { ui->equip_child = 1; ui->equip_child_border = border; }
static inline void UIEquipWidth(UI_Widget *widget, UI_Size size) { widget->semantic_size[0] = size; }
static inline void UIEquipHeight(UI_Widget *widget, UI_Size size) { widget->semantic_size[1] = size; }
static inline void UIEquipChildAxis(UI_Widget *widget, UI_Axis axis) { widget->axis = axis; }
static inline UI_Size UITextContent(float strictness) { return { UI_SIZEKIND_TEXTCONTENT, 0.f, strictness}; } 
static inline UI_Size UIChildrenSum(float strictness) { return { UI_SIZEKIND_CHILDRENSUM, 0.f, strictness};  }
static inline UI_Size UIPixels(float size, float strictness) { return { UI_SIZEKIND_PIXELS, size, strictness}; }
static inline UI_Size UIParentSize(float percent_of_parent, float strictness)  { return { UI_SIZEKIND_PERCENTOFPARENT, percent_of_parent , strictness}; }
static inline b32 UIIsActive(UI_Widget *widget) { return widget == ui->active; }
static inline b32 UIIsHot(UI_Widget *widget) { return widget == ui->hot; }

//
//~ UI Builder Code
//

static UI_Widget *
UICreateRect(UI_Size width, UI_Size height, int x, int y, const char *text, u32 flags) {
	UI_Widget *widget = UIMakeWidget(Str8Lit(text), flags);
	widget->semantic_size[0] = width;
	widget->semantic_size[1] = height;

	widget->computed_rel_pos[UI_AXIS2_X] = (float)x;
	widget->computed_rel_pos[UI_AXIS2_Y] = (float)y;
	return widget;
}

static UI_Widget *
UILayout(UI_Axis axis, UI_Size size_x, UI_Size size_y, const char *text) {
	UI_Widget *widget = UIMakeWidget(Str8Lit(text), 0);
	widget->axis = axis;
	widget->semantic_size[0] = size_x;
	widget->semantic_size[1] = size_y;
    return widget;
}

static UI_Output
UIButton(String8 text) {
	UI_Widget *widget = UIMakeWidget(text, UI_CLICKABLE | UI_DRAWBOX | UI_DRAWTEXT | UI_DRAWBORDER | UI_ANIMATE_HOT);
	UI_Output output = UIInteractWidget(widget);
	return output;
}

static UI_Output
UILabel(String8 text) {
	UI_Widget *widget = UIMakeWidget(text, UI_DRAWTEXT);
	UI_Output output = UIInteractWidget(widget);
	return output; 
}

static UI_Output
UISlider(String8 text) {
	
	UI_Widget *widget = UIMakeWidget(text, UI_SLIDERABLE | UI_DRAWBOX | UI_DRAWBORDER | UI_ANIMATE_HOT);
	UI_Output output = UIInteractWidget(widget);
	
	// NOTE(ziv): should be fine, I have a internal copy of the string, shouldn't be bad right?
	String8 postfix = Str8Lit("sub_widget");
	char buffer[30];
	memcpy(buffer, text.data, text.size); 
	memcpy(buffer+text.size, postfix.data, postfix.size+1); 
	
	UIPushParent(widget); 
	UI_Widget *sub_widget = UIMakeWidget(Str8Lit(buffer), UI_DRAWBOX);
	UIEquipChildAxis(widget, widget->axis);
	sub_widget->semantic_size[0] = { UI_SIZEKIND_PERCENTOFPARENT, output.slider_value+0.005f, 1.f };
	sub_widget->semantic_size[1] = { UI_SIZEKIND_PERCENTOFPARENT, 1.f, 1.f };
	
	UIPopParent();
	
	return output;
}

static UI_Output
UIPad(String8 text, int flags) {
	
	UI_Widget *widget = UIMakeWidget(text, flags);
	UI_Widget *parent = UITopParent();
	widget->semantic_size[parent->axis] = UIParentSize(1, 0);
	widget->semantic_size[1-parent->axis] = parent->semantic_size[1-parent->axis];
	
	UI_Output result = UIInteractWidget(widget); 
	return result;
}


static void
UIPad(String8 text) {
	
	UI_Widget *widget = UIMakeWidget(text, 0);
	UI_Widget *parent = UITopParent();
	widget->semantic_size[parent->axis] = UIParentSize(1, 0);
	widget->semantic_size[1-parent->axis] = parent->semantic_size[1-parent->axis];
	
	//UI_Output result = UIInteractWidget(widget); 
	//return result;
}


static UI_Output
UILabel(char *fmt, ...) {
	//Temp scratch = scratch_begin(0, 0);
	va_list args;
	va_start(args, fmt);
	char buffer[20]; // TODO(ziv): REMOVE THIS!!! MAKE BETTER DECISIONS IN LIFE  
	String8 text = Str8FormatString(buffer, fmt, args);
	va_end(args);
	UI_Output output = UILabel(text);
	//scratch_end(scratch);
	return output;
}

static UI_Output
UIButton(char *fmt, ...) {
	//Temp scratch = scratch_begin(0, 0);
	va_list args;
	va_start(args, fmt);
	char buffer[20]; // TODO(ziv): REMOVE THIS!!! MAKE BETTER DECISIONS IN LIFE  
	String8 text = Str8FormatString(buffer, fmt, args);
	va_end(args);
	UI_Output output = UIButton(text);
	//scratch_end(scratch);
	return output;
}

static UI_Output
UISlider(char *fmt, ...) {
	//Temp scratch = scratch_begin(0, 0);
	va_list args;
	va_start(args, fmt);
	char buffer[20]; // TODO(ziv): REMOVE THIS!!! MAKE BETTER DECISIONS IN LIFE  
	String8 text = Str8FormatString(buffer, fmt, args);
	va_end(args);
	UI_Output output = UISlider(text);
	//scratch_end(scratch);
	return output;
}



//~
// Camera
//

typedef struct {
	
	/// In
	// camera
	float3 pos;
	float3 off;
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
	
	float3 loc; 
	
	// movement vectors
	float3 right, left;
	float3 up, down;
	float3 forward, backward;
} Camera;

static void
CameraInit(Camera *c) {
	
	c->aspect_ratio = 3.0f/2.0f;
    c->fov = (float)M_PI / 2.0f; // 45 degrees fov
    c->n = 0.5f;
    c->f = 1000;
	
	c->proj = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
	c->view = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
}

static void
CameraBuild(Camera *c) {
	Assert(c);
	
	// NOTE(ziv): D3D11 uses a left-handed coordinate system[2]
	// Along side that it also uses a column major matricies[3]
	
	// The lookat matrix is a matrix created from a 'single' vector
	// pointing in the forward direction of the player looking at the
	// object we want to point the camera towards.
	// In this case I create the forward vector using the pitch and yaw
	// values and from it I compute the space in which the camera lies.
	// The view matrix created from this space is the inverse translation
	// and inverse rotations done by the camera itself [4].
	
	// Build the lookat matrix
	float3 some_up_vector = { 0, 1, 0 };
	
	// this camera is pointing in the inverse direction of it's target
	float3 fv = f3normalize({ cosf(c->yaw)*cosf(c->pitch), sinf(c->pitch), sinf(c->yaw)*cosf(c->pitch) }); // z forward
	float3 rv = f3normalize(f3cross(some_up_vector, fv));                                                  // x right
	float3 uv = f3normalize(f3cross(fv, rv));                                                              // y up
	
	//
	// row major vs column major
	// 
	//     | 1 2 3 |
	// M = | 4 5 6 |
	//     | 7 8 9 |
	//
	// M row major    [1, 2, 3, 4, 5, 6, 7, 8, 9]
	// M column major [1, 4, 7, 2, 5, 8, 3, 6, 9]
	// 
	// matrix multiplication row major
	//     | 1 2 3 |    | 1 |   | 1*1 + 2*2 + 3*3 |
	//     | 4 5 6 | *  | 2 | = | 4*1 + 5*2 + 6*3 | = | 14, 32, 50 |
	//     | 7 8 9 |    | 3 |   | 7*1 + 8*2 + 9*3 |
	// 
	// matrix multiplication column  major
	//     | 1 2 3 |    | 1 |   | 1*1 + 4*2 + 7*3 |
	//     | 4 5 6 | *  | 2 | = | 2*1 + 5*2 + 6*3 | =  | 30, 36, 42 |
	//     | 7 8 9 |    | 3 |   | 3*1 + 8*2 + 9*3 |
	//
	// all computation is row major and you can transpose the end matrix to get column major
	// Adding translation:
	// 
	//     | 1 0 0 t |    | x |   | 1*x + 0*y + 0*z + t*1 |   | t+x |
	//     | 0 1 0 t | *  | y | = | 0*x + 1*y + 0*z + t*1 | = | t+y |
	//     | 0 0 1 t |    | z |   | 0*x + 0*y + 1*z + t*1 |   | t+z |
	//     | 0 0 0 1 |    | 1 |   | 0*x + 0*y + 0*z + 1*1 |   |   1 |
	
	c->view = {
		rv.x, uv.x, fv.x, 0,
		rv.y, uv.y, fv.y, 0,
		rv.z, uv.z, fv.z, 0,
		0,    0,    0,    1
	};
	
	// the translation vector is the camera position projected onto the camera space
	c->view.m[3][0] = -f3dot(c->pos, rv) - c->off.x; 
	c->view.m[3][1] = -f3dot(c->pos, uv) - c->off.y;
	c->view.m[3][2] = -f3dot(c->pos, fv) - c->off.z;
	
	
	c->view_inv = {
		rv.x, rv.y, rv.z, 0,
		uv.x, uv.y, uv.z, 0,
		fv.x, fv.y, fv.z, 0,
		0,    0,    0,    1
	};
	c->view_inv.m[3][0] = f3dot(c->off, rv) + c->pos.x; 
	c->view_inv.m[3][1] = f3dot(c->off, uv) + c->pos.y;
	c->view_inv.m[3][2] = f3dot(c->off, fv) + c->pos.z;
	
	
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
	
	c->loc.x = c->view_inv.m[3][0];
	c->loc.y = c->view_inv.m[3][1];
	c->loc.z = c->view_inv.m[3][2];
	
	
	//
	// In this case the projection matrix which we are building
	// is mapping object's in the players thrustum inside a
	// -1 to 1 cordinate space
	// 
	// Understanding d3d11 projection matrix: 
	// https://learn.microsoft.com/en-us/windows/win32/direct3d9/projection-transform
	// https://learn.microsoft.com/en-us/windows/win32/dxtecharts/the-direct3d-transformation-pipeline
	//
	// To get the correct projection matrix (for d3d11):  
	// https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
	// 
	
	float hfov = 1.0f/tanf(c->fov*0.5f);
	ZeroMemory(&c->proj, sizeof(matrix));
	//cn = 1 and cf = 0:
	c->proj.m[0][0] = 1.0f/(c->aspect_ratio*hfov);
	c->proj.m[1][1] = 1.0f/hfov;
	c->proj.m[2][2] = c->f/(c->f - c->n);
	c->proj.m[3][2] = -(c->f*c->n)/(c->f - c->n);
	c->proj.m[2][3] = 1.0f;
	
	// Since perspective matrices have a fixed layout, it makes sense
	// to calculate the specific perspective inverse instead of relying on a default
	// matrix inverse function. Actually calculating the matrix for any perspective
	// matrix is quite straight forward:
	//   I.......identity matrix
	//   p.......perspective matrix
	//   I(p)....inverse perspective matrix
	// 
	// 1.) Fill a variable inversion matrix and perspective layout matrix into the
	//   inversion formula: I(p) * p = I
	//    |x0  x1  x2  x3 |   |a 0 0 0|   |1 0 0 0|
	//    |x4  x5  x6  x7 | * |0 b 0 0| = |0 1 0 0|
	//    |x8  x9  x10 x11|   |0 0 c d|   |0 0 1 0|
	//    |x12 x13 x14 x15|   |0 0 e 0|   |0 0 0 1|
	//
	// 2.) Multiply inversion matrix times perspective matrix
	//   |x0*a  x1*b  x2*c+x3*e   x2*d |   |1 0 0 0|
	//   |x4*a  x5*b  x6*c+x7*e   x6*d | = |0 1 0 0|
	//   |x8*a  x9*b  x10*c+x11*e x10*d|   |0 0 1 0|
	//   |x12*a x13*b x14*c+x15*e x14*d|   |0 0 0 1|
	//
	// 3.) Finally substitute each x value: e.g: x0*a = 1 => x0 = 1/a so I(p) at column 0, row 0 is 1/a.
	//           |1/a 0 0 0|
	//   I(p) =  |0 1/b 0 0|
	//           |0 0 0 1/e|
	//           |0 0 1/d -c/de|
	
	// Inverse of the Projection Matrix
	ZeroMemory(&c->proj_inv, sizeof(matrix));
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

static void 
CameraScreenToWorld(Camera *c, float width, float height, 
					float3 *res, float screen_x, float screen_y, float camera_z) {
	// Goal:  Viewport => NDC => Clip => View => World
	
	// Viewport => NDC => Clip
	float x = (screen_x / width * 2.0f) - 1.0f;
    float y = (screen_y / height) * 2.0f - 1.0f;
    float z = 2.0f * camera_z - 1.0f;
	
	// Clip => View => World
	// here we use the inverse projection, and inverse view 
	// and just multiply everything together by our point 
	// vector from clip space to transform it into world 
	// space
	
	float ax = c->proj_inv.m[0][0]*x;
    float by = c->proj_inv.m[1][1]*y;
    float dz = c->proj_inv.m[2][3]*z;
    float w = c->proj_inv.m[3][3] + dz;
	
    res->x = c->proj_inv.m[3][2] * c->view_inv.m[2][0];
    res->x += c->proj_inv.m[3][3] * c->view_inv.m[3][0];
    res->x += ax * c->view_inv.m[0][0];
    res->x += by * c->view_inv.m[1][0];
    res->x += dz * c->view_inv.m[3][0];
	
    res->y = c->proj_inv.m[3][2] * c->view_inv.m[2][1];
    res->y += c->proj_inv.m[3][3] * c->view_inv.m[3][1];
    res->y += ax * c->view_inv.m[0][1];
    res->y += by * c->view_inv.m[1][1];
    res->y += dz * c->view_inv.m[3][1];
	
    res->z = c->proj_inv.m[3][2] * c->view_inv.m[2][2];
    res->z += c->proj_inv.m[3][3] * c->view_inv.m[3][2];
    res->z += ax * c->view_inv.m[0][2];
    res->z += by * c->view_inv.m[1][2];
    res->z += dz * c->view_inv.m[3][2];
    res->x /= w; res->y /= w; res->z /= w;
}

static void
CameraPickingRay(Camera *c, float w, float h,
				 float mouse_x, float mouse_y, float3 *orig, float3 *dir)
{
    float3 world; 
    CameraScreenToWorld(c, w, h, &world, mouse_x, mouse_y, 0);
	
	*orig = c->loc;
	*dir = f3normalize(world - c->loc);
}

//~
// Collision
//

static b32 
CollideRayTriangle(float3 orig, float3 dir, float3 *trig) {
	// NOTE(ziv): This is a naive implementation.
	// A performant should be done in the future

	// The Möller–Trumbore intersection algorithm should be a 
	// better solution (along with simd for performance) 
	// That said I don't have the time to implement it so 
	// I will make due with this naive solution: 
	// https://www.cs.cornell.edu/courses/cs465/2003fa/homeworks/raytri.pdf

	// find intersection with plane
	float3 norm = f3cross(trig[1]-trig[0], trig[2]-trig[0]); 
	float t = -f3dot(norm, orig-trig[0]) / f3dot(norm, dir); 
	float3 intersection = orig + t*dir;

	// find whether it is inside triangle
	b32 result = 
		f3dot(f3cross((trig[1] - trig[0]), (intersection - trig[0])), norm) > 0 && 
		f3dot(f3cross((trig[2] - trig[1]), (intersection - trig[1])), norm) > 0 && 
		f3dot(f3cross((trig[0] - trig[2]), (intersection - trig[2])), norm) > 0; 

	return result; 
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
		else if (dec) {
		    num += chr;
		    num *= 0.1f;
		}
		else {
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
		
		Assert(CloseHandle(file)); 
		
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

	#if 1
	// TODO(ziv): make sure that win32createwindow takes in a name parameter and so on...
	HWND window = Win32CreateWindow();

	InputInitialize(window);

	R_D3D11Context renderer = {0}, *r = &renderer; 
	RendererInit(&renderer, window);

	D3D11_VIEWPORT viewport = {0};
	viewport.Width = (FLOAT)window_width;
	viewport.Height = (FLOAT)window_height;
	viewport.MaxDepth = 1;
	r->viewport = &viewport;

#define EXPAND_ARRAY(arr) arr, ArrayLength(arr), sizeof(arr[0])

	// Create model vertex buffer
	size_t verticies_count, indicies_count;
	R_Buffer cube_vbuf, cube_ibuf;
	{
		bool success = ObjLoadFile("../resources/cube.obj", NULL, &verticies_count, NULL, &indicies_count);
		Assert(success && "Failed extracting buffer sizes for vertex and index buffers");

		Vertex *verticies = (Vertex *)malloc(verticies_count*sizeof(Vertex));
		unsigned short *indicies = (unsigned short *)malloc(indicies_count*sizeof(unsigned short));
		success = ObjLoadFile("../resources/cube.obj", verticies, &verticies_count, indicies, &indicies_count);
		Assert(success && "Failed extracting model data");

		R_Buffer_Desc vdesc = { 
			verticies, (int)verticies_count, sizeof(Vertex),
			R_USAGE_IMMUTABLE, R_BIND_VERTEX_BUFFER, 
		};
		cube_vbuf = r_create_buffer(r, &vdesc); 

		R_Buffer_Desc idesc = { 
			indicies, (int)indicies_count, sizeof(u16),
			R_USAGE_IMMUTABLE, R_BIND_INDEX_BUFFER,
		};
		cube_ibuf = r_create_buffer(r, &idesc); 
	}

	struct VSConstantBuffer {
		matrix transform;
		matrix projection;
		matrix normal_transform;
		float3 lightposition;
	};

	struct PSConstantBuffer {
		float3 point_light_position;
		float3 sun_light_direction;
	};

	// Create Constant buffers
	R_Buffer vs_constant_buffer, ps_constant_buffer;
	{
		R_Buffer_Desc desc = {
			NULL, 1, sizeof(VSConstantBuffer), 
			R_USAGE_DYNAMIC, R_BIND_CONSTANT_BUFFER, R_CPU_ACCESS_WRITE
		};
		vs_constant_buffer = r_create_buffer(r, &desc);

		desc.element_size = sizeof(PSConstantBuffer);
		ps_constant_buffer = r_create_buffer(r, &desc);
	}

	// Create The Image To Sample From
	R_Texture2D image;
	{

		// Load Image
		int tex_w, tex_h, tex_num_channels;
		unsigned char* bytes = stbi_load("../resources/test.png", &tex_w, &tex_h, &tex_num_channels, 4);
		Assert(bytes);
		int pitch = 4 * tex_w;

		R_Texture2D_Desc desc = {
			bytes, tex_w, tex_h,
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
			R_BIND_SHADER_RESOURCE, 
			0, 0, 1
		};
		image = r_create_texture2d(r, &desc);
		free(bytes);
	}

	// Create Shaders For Cube Model
	R_Shader cube_shdr;
	{
		R_Shader_Desc desc = {
			"../shaders.hlsl", "vs_main",
			"../shaders.hlsl", "ps_main",
			{
				{ "Position", R_FORMAT_R32G32B32_FLOAT, offsetof(struct Vertex, pos) },
				{ "Normal", R_FORMAT_R32G32_FLOAT, offsetof(struct Vertex, norm) },
				{ "Texture", R_FORMAT_R32G32_FLOAT, offsetof(struct Vertex, uv) } 
			}
		};
		cube_shdr = r_create_shaders(r, &desc);
	}

	R_Texture2D screen = { 0, r->frame_buffer_view };

	Win32ShowWindow(window);

	//~ Game State

	Camera c = {0};
	CameraInit(&c);
	c.aspect_ratio = (float)window_width/(float)window_height;
	c.pos.z -= 5;
	c.yaw = 3.14f/2;
	c.off = {0, 0, 0}; 

	// projection matrix variables
	float3 model_rotation    = { 0.0f, 0.0f, 0.0f };
	float3 model_scale       = { 1, 1, 1 }; // { 1.5f, 1.5f, 1.5f };
	float3 model_translation = { 0.0f, 0.0f, 4.0f };

	// global directional light
	float3 sun_direction = { 0, 0, 1 };
	// point light
	float3 lightposition = {  0, 0, 2 };

	TimeInit(); 
	
	// more things that I need I guess...
	double start_frame = Time(), end_frame;
	float dt = 1/60;

	while (running) {

		// Handle input
		MSG msg;
		while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}

		// Handle window resize
		RECT rect;
		GetClientRect(window, &rect);
		LONG width = rect.right - rect.left;
		LONG height = rect.bottom - rect.top;
		if (width != window_width || height != window_height) {
			window_width = width; window_height = height;
			RendererD3D11Resize(r, width, height);
		screen.rtv = r->frame_buffer_view;
		}
		
		float speed = 10*dt;
		CameraMove(&c, (key_d-key_a)*speed, 0, (key_w-key_s)*speed);
		CameraBuild(&c);
		
		
		// NOTE(ziv): if texture is a render target view as it should be 
		// then I will just use the fill render target view function 
		// provided by the d3d11 lib 


		// Draw Model
		{
			float3 translate_vector = { -c.view.m[3][0], -c.view.m[3][1], -c.view.m[3][2] };
			matrix model_view_matrix = get_model_view_matrix(model_rotation, model_translation, model_scale) * c.view;
			
			// update data in the cpu
			VSConstantBuffer vs_cbuf;
			vs_cbuf.transform        = model_view_matrix;
			vs_cbuf.projection       = c.proj;
			vs_cbuf.normal_transform = matrix_inverse_transpose(model_view_matrix);
			
			PSConstantBuffer ps_cbuf;
			ps_cbuf.point_light_position = lightposition - translate_vector;
			ps_cbuf.sun_light_direction = sun_direction;
			
			// update gpu data
			r_update_gpu_buffer(r, vs_constant_buffer, &vs_cbuf, sizeof(vs_cbuf));
			r_update_gpu_buffer(r, ps_constant_buffer, &ps_cbuf, sizeof(ps_cbuf));
			
			R_Pipline_Desc pipline_desc = {
				R_TOPOLOGY_TRIANGLELIST,
				cube_shdr,// shaders
				{ cube_vbuf, cube_ibuf, vs_constant_buffer }, // vs_bindings 
				{ ps_constant_buffer }, // ps_bindings
				{ image },
				{ } // still need sampler
			};

			float color[4] = { 0, 1, 1, 1 }; 
			r_fill_texture(r, screen, color);
			r_switch_pipline(r, &pipline_desc);
			r_set_cull_mode(r, R_CULL_MODE_BACK); // cull back/front
			r_draw_indexed(r, screen, (int)indicies_count, 0, 0);
		}
		
		

		RendererD3D11Present(r);
		end_frame = Time();
		dt = (float)(end_frame - start_frame);
		start_frame = end_frame; // update time for dt calc
	}
	//release_resources: 

	return 0; 
	
	
	
	
	
	
	
#endif 


	#if 0
	//printf("%f %f\n", (float)(ATLAS_WIDTH / CHARACTER_COUNT)*FAT_PIXEL_SIZE, (float)ATLAS_HEIGHT*FAT_PIXEL_SIZE );
	HWND window = Win32CreateWindow(); 
	InputInitialize(window);

	R_D3D11Context renderer = {0}; 
	R_D3D11Context *r = &renderer; 
	RendererInit(&renderer, window);

	D_Context draw = {r}; 
	D_Context *d = &draw;
	DrawInit(d); 

	OS_Events events;
	UI_Context ui_context;
	UIInit(&ui_context, d, &events);

	Arena arena;
	if (!MemArenaInit(&arena, 0x1000)) {
		FatalError("Couldn't Allocated Memory");
	}

	TimeInit();

	//~


	// 
	// Quad
	//

	
	ID3D11BlendState1 *blend_state_use_alpha = NULL;
	{
		// DestColor = SrcColor*SrcBlend <ColorOp> DestColor*DestBlend
		// DestAlpha = SrcAlpha*SrcBlendAlpha <AlphaOp> DestAlpha*DestBlendAlpha
		D3D11_BLEND_DESC1 blend_state = {0};
		blend_state.RenderTarget[0].BlendEnable = TRUE;
		blend_state.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blend_state.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blend_state.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blend_state.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blend_state.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blend_state.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blend_state.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		r->device->CreateBlendState1(&blend_state, &blend_state_use_alpha);
	}

	d->quads.blend_state_use_alpha = blend_state_use_alpha;
	//
	// Font
	//

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
		r->device->CreateTexture2D(&texture_atlas_desc, &atlas_data, &atlas_texture);
		r->device->CreateShaderResourceView(atlas_texture, NULL, &atlas_resource_view);
		atlas_texture->Release();
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
		
		r->device->CreateSamplerState(&font_sampler_desc, &font_sampler);
	}

	d->font.sampler = font_sampler;
	d->font.srv[1] = atlas_resource_view;


	//~
	// Model Data
	//

	char model_path[] = "../resources/cube.obj";

	size_t verticies_count, indicies_count;
	bool success = ObjLoadFile(model_path,NULL, &verticies_count, NULL, &indicies_count);
	Assert(success && "Failed extracting buffer sizes for vertex and index buffers");

	Vertex *verticies = (Vertex *)malloc(verticies_count*sizeof(Vertex));
	unsigned short *indicies = (unsigned short *)malloc(indicies_count*sizeof(unsigned short));
	success = ObjLoadFile(model_path, verticies, &verticies_count, indicies, &indicies_count);
	Assert(success && "Failed extracting model data");

	BFHandle vertex_buffer = RendererCreateBuffer(r, verticies, sizeof(Vertex), (unsigned int)verticies_count, R_BIND_VERTEX_BUFFER, { R_USAGE_DEFAULT} ); 
	BFHandle index_buffer = RendererCreateBuffer(r, indicies, sizeof(u16), (unsigned int)indicies_count, R_BIND_INDEX_BUFFER, { R_USAGE_DEFAULT });
	
	R_LayoutFormat format[] = {
		{ "Position", R_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct Vertex, pos), R_INPUT_PER_VERTEX_DATA }, 
		{ "Normal",   R_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct Vertex, norm), R_INPUT_PER_VERTEX_DATA }, 
		{ "Texture",  R_FORMAT_R32G32_FLOAT, 0, offsetof(struct Vertex, uv), R_INPUT_PER_VERTEX_DATA }
	};

	VSHandle vshader = RendererCreateVSShader(r, "../shaders.hlsl", "vs_main", format, ArrayLength(format));
	PSHandle pshader = RendererCreatePSShader(r, "../shaders.hlsl", "ps_main");

	struct VSConstantBuffer {
		matrix transform;
		matrix projection;
		matrix normal_transform;
		float3 lightposition;
	};

	struct PSConstantBuffer {
		float3 point_light_position;
		float3 sun_light_direction;
	};

	BFHandle cbuffer = RendererCreateBuffer(r, NULL, sizeof(VSConstantBuffer), 1, R_BIND_CONSTANT_BUFFER, { R_USAGE_DYNAMIC, R_CPU_ACCESS_WRITE });
	BFHandle ps_constant_buffer = RendererCreateBuffer(r, NULL, sizeof(PSConstantBuffer), 1, R_BIND_CONSTANT_BUFFER, { R_USAGE_DYNAMIC, R_CPU_ACCESS_WRITE });
	
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
		
		r->device->CreateSamplerState(&sampler_desc, &sampler_state);
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
		r->device->CreateTexture2D(&texture_desc, &texture_data, &texture);
		r->device->CreateShaderResourceView(texture, NULL, &texture_view);

		texture->Release();
		free(bytes);
	}
	
	
	//~ JFA
	ID3D11ShaderResourceView *jfa_mask_resource;
	ID3D11RenderTargetView *jfa_mask_render_target;
	{
		D3D11_TEXTURE2D_DESC texture_desc = {};
		texture_desc.Width              = window_width;
		texture_desc.Height             = window_height;
		texture_desc.MipLevels          =1;
		texture_desc.ArraySize          = 1;
		texture_desc.Format             = DXGI_FORMAT_R8_UNORM;
		texture_desc.SampleDesc.Count   = 1;
		texture_desc.Usage              = D3D11_USAGE_DEFAULT;
		texture_desc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texture_desc.CPUAccessFlags     = D3D11_CPU_ACCESS_READ;
		
		ID3D11Texture2D* texture;
		r->device->CreateTexture2D(&texture_desc, NULL, &texture);
		r->device->CreateShaderResourceView(texture, NULL, &jfa_mask_resource);
		r->device->CreateRenderTargetView(texture, NULL, &jfa_mask_render_target);
		
		texture->Release();
	}
	
	VSHandle jfa_mask_vshader = RendererCreateVSShader(r, "../jfa_mask.hlsl", "vs", format, ArrayLength(format));
	PSHandle jfa_mask_pshader = RendererCreatePSShader(r, "../jfa_mask.hlsl", "ps"); 
	BFHandle jfa_vs_cbuffer = RendererCreateBuffer(r, 
												   NULL, 
												   sizeof(matrix)*2, 
												   1,
												   R_BIND_CONSTANT_BUFFER, 
												   { 
													   R_USAGE_DYNAMIC, 
													   R_CPU_ACCESS_WRITE, 
												   } );
	
	BFHandle inv_size_cbuffer = RendererCreateBuffer(r, 
												   NULL, 
												   sizeof(float)*2, 
												   1,
												   R_BIND_CONSTANT_BUFFER, 
												   {
													   R_USAGE_DYNAMIC, 
													   R_CPU_ACCESS_WRITE, 
												   } );
	
	
	
	// INIT
	ID3D11ShaderResourceView *jfa_init_resource;
	ID3D11RenderTargetView *jfa_init_render_target;
	{
		D3D11_TEXTURE2D_DESC texture_desc = {};
		texture_desc.Width              = window_width;
		texture_desc.Height             = window_height;
		texture_desc.MipLevels          = 1;
		texture_desc.ArraySize          = 1;
		texture_desc.Format             = DXGI_FORMAT_R16G16_UNORM;
		texture_desc.SampleDesc.Count   = 1;
		texture_desc.Usage              = D3D11_USAGE_DEFAULT;
		texture_desc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texture_desc.CPUAccessFlags     = D3D11_CPU_ACCESS_READ;
		
		ID3D11Texture2D* texture;
		r->device->CreateTexture2D(&texture_desc, NULL, &texture);
		r->device->CreateShaderResourceView(texture, NULL, &jfa_init_resource);
		r->device->CreateRenderTargetView(texture, NULL, &jfa_init_render_target);
		
		texture->Release();
	}
	
	
	// JFA
	ID3D11RenderTargetView *jfa_mask0_render_target;
	ID3D11ShaderResourceView *jfa_mask0_resource_view;
	{
		D3D11_TEXTURE2D_DESC texture_desc = {};
		texture_desc.Width              = window_width;
		texture_desc.Height             = window_height;
		texture_desc.MipLevels          = 1;
		texture_desc.ArraySize          = 1;
		texture_desc.Format             = DXGI_FORMAT_R16G16_UNORM;
		texture_desc.SampleDesc.Count   = 1;
		texture_desc.Usage              = D3D11_USAGE_DEFAULT;
		texture_desc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texture_desc.CPUAccessFlags     = D3D11_CPU_ACCESS_READ;
		
		ID3D11Texture2D* texture;
		r->device->CreateTexture2D(&texture_desc, NULL, &texture);
		r->device->CreateRenderTargetView(texture, NULL, &jfa_mask0_render_target);
		r->device->CreateShaderResourceView(texture, NULL, &jfa_mask0_resource_view);
		
		texture->Release();
	}
	
	VSHandle jfa_init_vshader = RendererCreateVSShader(r, "../jfa_init.hlsl", "vs", NULL, 0);
	PSHandle jfa_init_pshader = RendererCreatePSShader(r, "../jfa_init.hlsl", "ps"); 
	
	VSHandle jfa_vshader = RendererCreateVSShader(r, "../jfa.hlsl", "vs", NULL, 0);
	PSHandle jfa_pshader = RendererCreatePSShader(r, "../jfa.hlsl", "ps"); 
	
	VSHandle jfa_solid_vshader = RendererCreateVSShader(r, "../jfa_solid.hlsl", "vs", NULL, 0);
	PSHandle jfa_solid_pshader = RendererCreatePSShader(r, "../jfa_solid.hlsl", "ps"); 
	
	int down_sample_multiplier = 1;
	
	// down-sampled buffer
	ID3D11ShaderResourceView *down_sampled_resource;
	ID3D11RenderTargetView *down_sampled_rtv;
	{
		D3D11_TEXTURE2D_DESC texture_desc = {};
		texture_desc.Width              = window_width  / down_sample_multiplier;
		texture_desc.Height             = window_height / down_sample_multiplier;
		texture_desc.MipLevels          = 1;
		texture_desc.ArraySize          = 1;
		texture_desc.Format             = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		texture_desc.SampleDesc.Count   = 1;
		texture_desc.Usage              = D3D11_USAGE_DEFAULT;
		texture_desc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texture_desc.CPUAccessFlags     = D3D11_CPU_ACCESS_READ;
		
		ID3D11Texture2D* texture;
		r->device->CreateTexture2D(&texture_desc, NULL, &texture);
		r->device->CreateRenderTargetView(texture, NULL, &down_sampled_rtv);
		r->device->CreateShaderResourceView(texture, NULL, &down_sampled_resource);
		
		texture->Release();
	}
	
	float full_screen_verticies[] = {
		-1,  1, 
		 1,   1, 
		-1, -1, 
		1,  -1,
	};
	BFHandle full_screen_vertex_buffer = RendererCreateBuffer(r, full_screen_verticies, sizeof(float)*2, 4, R_BIND_VERTEX_BUFFER, { R_USAGE_DEFAULT } ); 
	R_LayoutFormat downsample_format[] = {
		{ "Position", R_FORMAT_R32G32_FLOAT, 0, 0, R_INPUT_PER_VERTEX_DATA },
	};
	VSHandle downsample_vshader = RendererCreateVSShader(r, "../downsample.hlsl", "vs", downsample_format, 1);
	PSHandle downsample_pshader = RendererCreatePSShader(r, "../downsample.hlsl", "ps"); 
	
	
	
	D3D11_VIEWPORT viewport = {0};
	viewport.Width = (FLOAT)window_width;
	viewport.Height = (FLOAT)window_height;
	viewport.MaxDepth = 1;
	r->viewport = &viewport;

	// TODO(ziv): MOVE THIS CODE!!!
	RECT rc_clip;           // new area for ClipCursor
	RECT rc_old_clip;        // previous area for ClipCursor
	GetClipCursor(&rc_old_clip);
	GetWindowRect(window, &rc_clip);

	//~
	// Main Game Loop
	//

	// projection matrix variables
	float3 model_rotation    = { 0.0f, 0.0f, 0.0f };
	float3 model_scale       = { 1, 1, 1 }; // { 1.5f, 1.5f, 1.5f };
	float3 model_translation = { 0.0f, 0.0f, 4.0f };

	// global directional light
	float3 sun_direction = { 0, 0, 1 };
	// point light
	float3 lightposition = {  0, 0, 2 };


	// more things that I need I guess...
	double start_frame = Time(), end_frame;

	// Camera

	Camera c = {0};
	CameraInit(&c);
	c.aspect_ratio = (float)window_width/(float)window_height;
	c.pos.z -= 5;
	c.yaw = 3.14f/2;
	c.off = {0, 0, 0}; 

	d->proj = &c.proj;
	d->view = &c.view; 

	//~
	float dt = 1/60;
	ShowWindow(window, SW_SHOW);


	// main loop
	for (;;) {
		start_frame = Time();

		// event loop
		events = OSProcessEvents();
		for (int i = 0; i < events.idx; i++) {
			OS_Event e = events.list[i];
			if (e.kind == OS_EVENT_KIND_QUIT) {
				goto release_resources;
			}
		}

		r->dirty = 0;
		d->quads.idx = 0; 
		d->font.idx = 0 ;

		// Handle window resize
		RECT rect;
		GetClientRect(window, &rect);
		LONG width = rect.right - rect.left;
		LONG height = rect.bottom - rect.top;
		if (width != window_width || height != window_height) {
			window_width = width; window_height = height;
			RendererD3D11Resize(r, width, height);
			c.aspect_ratio = (float)width/(float)height;
		}
		
		// Don't render when minimized
		if (width == 0 && height == 0) {
			Sleep(15); continue;
		}
	


		//
		// Update pixel and vertex shaders when changed
		//
		{
			for (int i = 0; i < r->vs_idx; i++) {
				FILETIME last_write_time = Win32GetLastFileWriteTime(r->vs_file_names_and_time[i].file_name);
				Assert(last_write_time.dwLowDateTime != 0 && last_write_time.dwHighDateTime !=0);
				
				
				if (CompareFileTime(&last_write_time,&r->vs_file_names_and_time[i].last_write_time) != 0) {
					r->vs_file_names_and_time[i].last_write_time = last_write_time;
					ID3D11InputLayout  *layout = NULL;
					ID3D11VertexShader *vshader = NULL;
					RendererD3D11CreateVSShader(r, r->vs_file_names_and_time[i].file_name, r->vs_file_names_and_time[i].entry_point, 
												r->vs_file_names_and_time[i].format, 
												r->vs_file_names_and_time[i].format_size, 
												&vshader, &layout);
					if (r->vs_array[i]) r->vs_array[i]->Release();
					r->vs_array[i] = vshader;
					r->lay_array[i] = layout;
																			
				}
			}

			for (int i = 0; i < r->ps_idx; i++) {
				FILETIME last_write_time = Win32GetLastFileWriteTime(r->ps_file_names_and_time[i].file_name);
				if (CompareFileTime(&last_write_time,&r->ps_file_names_and_time[i].last_write_time) != 0) {
					r->ps_file_names_and_time[i].last_write_time = last_write_time;
					ID3D11PixelShader *pshader = NULL;
					RendererD3D11CreatePSShader(r, r->ps_file_names_and_time[i].file_name, r->ps_file_names_and_time[i].entry_point, 
												&pshader);
					if (r->ps_array[i]) r->ps_array[i]->Release();
					r->ps_array[i] = pshader;
				}
			}
		}



		//
		// Enable/Disable Free Camera Mode
		//

		// Update Mouse Clip area
		GetWindowRect(window, &rc_clip);
		if (key_tab_pressed) {
			show_free_camera = !show_free_camera;

			if (show_free_camera) {
				// Confine the cursor to the application's window.
				ClipCursor(&rc_clip);
				ShowCursor(false);
			}
			else {
				// Restore the cursor to its previous area.
				ClipCursor(&rc_old_clip);
				ShowCursor(true);
			}
		}
		key_tab_pressed = false;

		if (show_free_camera) {
			SetCursorPos(window_width/2, window_height/2);
		}


		//~
		// UI
		//

		static float value = 0; 


		UIBegin(ui, r->dirty);

		static b32 show_top_rectangle = 0; 
		static int show_panel = 1;

		UIEquipChildRadius(5); UIEquipChildBorder(2);
		UIPushParent(UICreateRect(UIPixels(200, 1), UIChildrenSum(1), 0,0, "top_rectangle", UI_DRAWBOX));
		UIPushParent(UILayout(UI_AXIS2_Y, UIPixels(200, 1), UITextContent(1), "top_layout"));
		if (UIButton("Panel").clicked) {
			show_top_rectangle= !show_top_rectangle; // toggle show panel
		}
		if (show_top_rectangle) {
			if (UIButton("Move Up").activated) { c.pos = c.pos + c.up * -0.1f; }
			if (UIButton("Move Down").activated) { c.pos = c.pos + c.up * 0.1f; }

			UIPushParent(UILayout(UI_AXIS2_X, UIParentSize(1, 0), UITextContent(1), "whatever layout")); 
			UIEquipWidth(UILabel("speed:").widget, UITextContent(1)); value = UISlider("MovementSpeed").slider_value; 
			UIPopParent();

			if (UIButton("Show Panel").clicked) {
				show_panel = 1;
			}

		}
		UIPopParent();
		UIPopParent();

		static int x = 150;
		static int y = 100;
		static int relx = 0; 
		static int rely = 0; 
		static b32 collided_with_character = false; 

		if (show_panel) {
			UIEquipChildRadius(10); 
			UI_Widget *panel = UICreateRect(UIPixels(200, 1), UIChildrenSum(1), x, y, "floating_panel", UI_FLOAT_X | UI_FLOAT_Y | UI_DRAWBOX);
			UIPushParent(panel);
			UIPushParent(UILayout(UI_AXIS2_Y, UIPixels(200, 1), UITextContent(1), "floating panel layout"));
			{

				static int show_inside_panel = 0;
				UIPushParent(UILayout(UI_AXIS2_X, UITextContent(1), UITextContent(1), "panel_top_bar_layout"));
				{

					UIEquipChildRadius(10); 
					if (UIButton("v###1").clicked) {
						show_inside_panel = !show_inside_panel;
					}

					UI_Widget *rect1 = UICreateRect(UIParentSize(1, 0), UITextContent(1), x, y, "subpanel", UI_CLICKABLE | UI_DRAGGABLE);
					UIInteractWidget(rect1);
					// TODO(ziv): consider putting this logic in another place
					if (UIIsActive(rect1)) {
						x = (int)ui->mouse_pos[0]-relx; 
						y = (int)ui->mouse_pos[1]-rely; 
					}
					else {
						relx = (int)ui->mouse_pos[0] - (int)panel->computed_rel_pos[0]; 
						rely = (int)ui->mouse_pos[1] - (int)panel->computed_rel_pos[1]; 
					}

					//UIPad(Str8Lit("padding"));

					UIEquipChildRadius(10); 
					if (UIButton("X###1").clicked) {
						show_panel = 0;
					}
				}
				UIPopParent(); 

				if (show_inside_panel) {

					if (collided_with_character) {
						UILabel(Str8Lit("Collided With Box"));
					}
					
					if (UIButton("rotate x").activated) {
						model_rotation.x += 0.1f;
					}
					if (UIButton("rotate y").activated) {
						model_rotation.y += 0.1f;
					}
					if (UIButton("rotate z").activated) {
						model_rotation.z += 0.1f;
					}

					if (UIButton("scale x").activated) {
						model_scale.x += 0.1f;
					}
					if (UIButton("scale y").activated) {
						model_scale.y += 0.1f;
					}
					if (UIButton("scale z").activated) {
						model_scale.z += 0.1f;
					}



				}
			}
			UIPopParent(); 
			UIPopParent(); 
		}

		UIEnd(ui);




		//~
		// Update Game State
		//
		
		static float3 orig, dir; 
		for (int i = 0; i < events.idx; i++) {
			OS_Event e = events.list[i]; 

			static float dx = 0, dy = 0;
			if (e.kind == OS_EVENT_KIND_RAW_MOUSEMOVE) {
				if (show_free_camera) { 
					dx = (float)e.value[0];
					dy = (float)-e.value[1];

					c.yaw   = fmodf(c.yaw - dx/(window_width+1)*2*3.14f, (float)(2*M_PI));
					c.pitch = float_clamp(c.pitch + dy/(window_height+1), -(float)M_PI/2.f, (float)M_PI/2.f);
				}
			}
			else if (e.kind == OS_EVENT_KIND_MOUSE_BUTTON_PRESSED) {
				if (e.value[0] & MouseLeftButton) {
					CameraPickingRay(&c, (float)window_width ,(float)window_height, 
					ui->mouse_pos[0], window_height-ui->mouse_pos[1], &orig, &dir); 
				}
			}

		}

		float speed = (1+value*10)*dt;
		CameraMove(&c, (key_d-key_a)*speed, 0, (key_w-key_s)*speed);
		CameraBuild(&c);


		// TODO(ziv): move this into another location
		d->lines.idx = 0;
		//DrawLine(d, orig+ float3{0, 0, .5}, 10*dir); 

		b32 intersecting = false;
		matrix model_view_matrix = get_model_view_matrix(model_rotation, model_translation, model_scale);
		for (int i = 0; i < indicies_count; i += 3) {
			float3 triangle_to_intersect[3];
			for (int j = 0; j < 3; j++) {
				float4 pos = {0};
				memcpy(&pos, verticies[indicies[i+j]].pos, sizeof(float3));
				pos.w = 1;
				pos = pos * model_view_matrix;
				memcpy(&triangle_to_intersect[j], &pos, sizeof(float3));
			}
			intersecting = CollideRayTriangle(orig, dir, triangle_to_intersect);
			if (intersecting) {
				break; 
			}
		}

		collided_with_character = intersecting;


		float3 translate_vector = { -c.view.m[3][0], -c.view.m[3][1], -c.view.m[3][2] };

		//~
		// Render Game
		//
		
		// Update model-view matrix
		{
			matrix model_view_matrix = get_model_view_matrix(model_rotation, model_translation, model_scale) * c.view;

			// Vertex Contstant Buffer
			VSConstantBuffer vs_cbuf;
			vs_cbuf.transform        = model_view_matrix;
			vs_cbuf.projection       = c.proj;
			vs_cbuf.normal_transform = matrix_inverse_transpose(model_view_matrix);
			RendererUpdateBuffer(r, cbuffer, &vs_cbuf, sizeof(vs_cbuf)); 
			
			float inv_size[] = { 1/(float)window_width, 1/(float)window_height};
			RendererUpdateBuffer(r, inv_size_cbuffer, inv_size, 2*sizeof(float)); 
			
			RendererUpdateBuffer(r, jfa_vs_cbuffer, &vs_cbuf, sizeof(matrix)*2); 
			
			// Pixel Contstant Buffer
			PSConstantBuffer ps_cbuf;
			ps_cbuf.point_light_position = lightposition - translate_vector;
			ps_cbuf.sun_light_direction = sun_direction;
			RendererUpdateBuffer(r, ps_constant_buffer, &ps_cbuf, sizeof(ps_cbuf)); 
											
		}
		
		if (r->dirty) {
			// MASK 
			jfa_mask_resource->Release(); 
			jfa_mask_render_target->Release(); 
			jfa_mask_resource = NULL;
			jfa_mask_render_target = NULL;
			
			{
				D3D11_TEXTURE2D_DESC texture_desc = {};
				texture_desc.Width              = window_width;
				texture_desc.Height             = window_height;
				texture_desc.MipLevels          = 1;
				texture_desc.ArraySize          = 1;
				texture_desc.Format             = DXGI_FORMAT_R8_UNORM;
				texture_desc.SampleDesc.Count   = 1;
				texture_desc.Usage              = D3D11_USAGE_DEFAULT;
				texture_desc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
				texture_desc.CPUAccessFlags     = D3D11_CPU_ACCESS_READ;
				
				ID3D11Texture2D* texture;
				r->device->CreateTexture2D(&texture_desc, NULL, &texture);
				r->device->CreateShaderResourceView(texture, NULL, &jfa_mask_resource);
				r->device->CreateRenderTargetView(texture, NULL, &jfa_mask_render_target);
				
				texture->Release();
			}
			
			
			jfa_init_resource->Release(); 
			jfa_init_render_target->Release(); 
			jfa_init_resource = NULL; 
			jfa_init_render_target = NULL;
			
				// INIT
			{
			D3D11_TEXTURE2D_DESC texture_desc = {};
					texture_desc.Width              = window_width;
					texture_desc.Height             = window_height;
					texture_desc.MipLevels          = 1;
					texture_desc.ArraySize          = 1;
					texture_desc.Format             = DXGI_FORMAT_R16G16_UNORM;
					texture_desc.SampleDesc.Count   = 1;
					texture_desc.Usage              = D3D11_USAGE_DEFAULT;
					texture_desc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
					texture_desc.CPUAccessFlags     = D3D11_CPU_ACCESS_READ;
					
					ID3D11Texture2D* texture;
					r->device->CreateTexture2D(&texture_desc, NULL, &texture);
					r->device->CreateShaderResourceView(texture, NULL, &jfa_init_resource);
					r->device->CreateRenderTargetView(texture, NULL, &jfa_init_render_target);
					texture->Release();
		}
			
			// JFA
			jfa_mask0_render_target->Release();
			jfa_mask0_resource_view->Release(); 
			jfa_mask0_resource_view = NULL; 
			jfa_mask0_render_target = NULL;
			{
				D3D11_TEXTURE2D_DESC texture_desc = {};
				texture_desc.Width              = window_width;
				texture_desc.Height             = window_height;
				texture_desc.MipLevels          = 1;
				texture_desc.ArraySize          = 1;
				texture_desc.Format             = DXGI_FORMAT_R16G16_UNORM;
				texture_desc.SampleDesc.Count   = 1;
				texture_desc.Usage              = D3D11_USAGE_DEFAULT;
				texture_desc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
				texture_desc.CPUAccessFlags     = D3D11_CPU_ACCESS_READ;
				
				ID3D11Texture2D* texture;
				r->device->CreateTexture2D(&texture_desc, NULL, &texture);
				r->device->CreateRenderTargetView(texture, NULL, &jfa_mask0_render_target);
				r->device->CreateShaderResourceView(texture, NULL, &jfa_mask0_resource_view);
				
				texture->Release();
			}
		}
		
		
		// clear background
		FLOAT background_color[4] = { 0.025f, 0.025f, 0.025f, 1.0f };
		r->context->ClearRenderTargetView(r->frame_buffer_view, background_color);
		
		
		// Draw outline
			if (intersecting) { 
			// TODO(ziv): Currently this is using too much of the GPU. 
			// this is the case since I am working on the whole screen a couple of times 
			// one after the other. I need to find how this can be made better. Since 
			// Whether by using lower resolution screen first or any other solution. 
			
			// Create mask of the object
			{
				
				float black[] = { 0, 0, 0, 0 };
				r->context->ClearRenderTargetView(jfa_mask_render_target, black);
				
				r->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				const UINT stride = sizeof(Vertex);
				const UINT offset = 0;
				r->context->IASetVertexBuffers(0, 1, RendererBFToPointer(r, vertex_buffer), &stride, &offset);
					r->context->IASetIndexBuffer(*RendererBFToPointer(r, index_buffer),DXGI_FORMAT_R16_UINT, 0);
				RendererVSSetShader(r, jfa_mask_vshader);
				RendererVSSetBuffer(r, cbuffer);
				
				D3D11_VIEWPORT viewport = {0};
				viewport.Width = (FLOAT)window_width;
				viewport.Height = (FLOAT)window_height;
				viewport.MaxDepth = 1;
				r->viewport = &viewport;
				
				r->context->RSSetViewports(1, &viewport);
				r->context->RSSetState(r->rasterizer_cull_back);
				RendererPSSetShader(r, jfa_mask_pshader);
				r->context->OMSetRenderTargets(1, &jfa_mask_render_target, NULL);
				r->context->OMSetBlendState(NULL, NULL, 0xffffffff); // set default blend state
				
				r->context->DrawIndexed((UINT)indicies_count, 0, 0);
			}
			
			// jfa init
			{
				ID3D11RenderTargetView* nullRTV = NULL;
				r->context->OMSetRenderTargets(1, &nullRTV, NULL); // Unbind from slot 0 to bind the mask
				
				float black[] = { 0, 0, 0, 0 };
				r->context->ClearRenderTargetView(jfa_init_render_target, black);
				
				r->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
				RendererVSSetShader(r, jfa_init_vshader);
				D3D11_VIEWPORT viewport = {0};
				viewport.Width = (FLOAT)window_width;
				viewport.Height = (FLOAT)window_height;
				viewport.MaxDepth = 1;
				r->viewport = &viewport;
				r->context->RSSetViewports(1, &viewport);
				r->context->PSSetShaderResources(0, 1, &jfa_mask_resource);
				r->context->PSSetSamplers(0, 1, &sampler_state);
				RendererPSSetShader(r, jfa_init_pshader);
				RendererPSSetBuffer(r, inv_size_cbuffer);
				r->context->RSSetState(NULL);
				r->context->OMSetRenderTargets(1, &jfa_init_render_target, NULL);
				r->context->DrawInstanced(4, 1, 0, 0);
			}
			
			// jfa
			{
					ID3D11RenderTargetView* nullRTV = NULL;
					r->context->OMSetRenderTargets(1, &nullRTV, NULL);
				
				float black[] = { 0, 0, 0, 0 };
				r->context->ClearRenderTargetView(jfa_mask0_render_target, black);
				
					r->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
					RendererVSSetShader(r, jfa_vshader);
					D3D11_VIEWPORT viewport = {0};
					viewport.Width = (FLOAT)window_width;
					viewport.Height = (FLOAT)window_height;
					viewport.MaxDepth = 1;
					r->viewport = &viewport;
					r->context->RSSetViewports(1, &viewport);
					r->context->PSSetShaderResources(0, 1, &jfa_init_resource);
					r->context->PSSetSamplers(0, 1, &sampler_state);
					RendererPSSetShader(r, jfa_pshader);
				RendererPSSetBuffer(r, inv_size_cbuffer);
				r->context->OMSetRenderTargets(1, &jfa_mask0_render_target, NULL);
					r->context->DrawInstanced(4, 1, 0, 0);
			}
			
			r->context->ClearRenderTargetView(down_sampled_rtv, background_color);
			// using the jfa rt along with the mask to create final image
			{
				
				ID3D11RenderTargetView* nullRTV = NULL;
				r->context->OMSetRenderTargets(1, &nullRTV, NULL);
																			
					r->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
				RendererVSSetShader(r, jfa_solid_vshader);
					D3D11_VIEWPORT viewport = {0};
					viewport.Width = (FLOAT)window_width;
					viewport.Height = (FLOAT)window_height;
					viewport.MaxDepth = 1;
					r->viewport = &viewport;
					r->context->RSSetViewports(1, &viewport);
					r->context->PSSetShaderResources(0, 1, &jfa_mask0_resource_view);
					r->context->PSSetSamplers(0, 1, &sampler_state);
				RendererPSSetShader(r, jfa_solid_pshader);
				RendererPSSetBuffer(r, inv_size_cbuffer);
				r->context->OMSetRenderTargets(1, &down_sampled_rtv, NULL);
					r->context->DrawInstanced(4, 1, 0, 0);
				}
							
			
		}
		
		
		// Draw Entity
		{
			
			
			// Input Assembler
			r->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			const UINT stride = sizeof(Vertex);
			const UINT offset = 0;

			r->context->IASetVertexBuffers(0, 1, RendererBFToPointer(r, vertex_buffer), &stride, &offset);
			r->context->IASetIndexBuffer(*RendererBFToPointer(r, index_buffer),DXGI_FORMAT_R16_UINT, 0);

			// Vertex Shader
			RendererVSSetShader(r, vshader);
			RendererVSSetBuffer(r, cbuffer);
			
			
			D3D11_VIEWPORT viewport = {0};
			viewport.Width = (FLOAT)window_width/(float)down_sample_multiplier;
			viewport.Height = (FLOAT)window_height/(float)down_sample_multiplier ;
			viewport.MaxDepth = 1;
			
			// Rasterizer Stage
			r->context->RSSetViewports(1, &viewport);
			r->context->RSSetState(r->rasterizer_cull_back);

			// Pixel Shader
			RendererPSSetShader(r, pshader);
			RendererPSSetBuffer(r, ps_constant_buffer);
			r->context->PSSetShaderResources(0, 1, &texture_view);
			r->context->PSSetSamplers(0, 1, &sampler_state);

			// Output Merger
			r->context->OMSetRenderTargets(1, &down_sampled_rtv, NULL);

			r->context->DrawIndexed((UINT)indicies_count, 0, 0);
		}
		
		
		// draw downsampled texture onto the frame buffer
		{
			ID3D11RenderTargetView* nullRTV = NULL;
			r->context->OMSetRenderTargets(1, &nullRTV, NULL);
			
			r->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			const UINT stride = sizeof(float)*2;
			const UINT offset = 0;
			r->context->IASetVertexBuffers(0, 1, RendererBFToPointer(r, full_screen_vertex_buffer), &stride, &offset);
			RendererVSSetShader(r, downsample_vshader);
			
			r->context->RSSetViewports(1,r->viewport);
			RendererPSSetShader(r, downsample_pshader);
			r->context->PSSetShaderResources(0, 1, &down_sampled_resource);
			r->context->PSSetSamplers(0, 1, &sampler_state); // point sampler
			
			// Output Merger
			r->context->OMSetRenderTargets(1, &r->frame_buffer_view, NULL);
			r->context->Draw(4, 0);
		}
		
		
		
		
		
		 DrawSubmitRenderCommands(d);
		RendererD3D11Present(r); // present the resulting image to the screen
		end_frame = Time(); 
		dt = (float)(end_frame - start_frame);
		start_frame = end_frame; // update time for dt calc
	}

	release_resources:

	InputShutdown(); // rawinput
	texture_view->Release();
	sampler_state->Release();
	// Draw DeInit
	d->quads.blend_state_use_alpha->Release();
	d->font.sampler->Release();
	d->font.srv[1]->Release();
	RendererDeInit(r);
	return 0;

	#endif 
}
