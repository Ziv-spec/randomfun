#define UNICODE
#define COBJMACROS
#define _CRT_SECURE_NO_DEPRECATE
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>

#include <CommCtrl.h>
#include <stdio.h>

#define _USE_MATH_DEFINES
#include <math.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define APP_TITLE L"GRAPHIC"

#define Assert(expr) do { if (!expr) __debugbreak(); } while (0)
#define AssertHR(hr) Assert(SUCCEEDED(hr))
#define ArrayLength(arr) (sizeof(arr) / sizeof(arr[0]))

static int window_width = 800;
static int window_height = 600;

static HWND g_window;
static HWND g_groupbox1;
static HWND g_groupbox2;
static HWND g_button;

static void FatalError(const char* message)
{
    MessageBoxA(NULL, message, "Error", MB_ICONEXCLAMATION);
    ExitProcess(0);
}
static LRESULT CALLBACK WinProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
	switch (message)
    {
		case WM_DESTROY: {
			PostQuitMessage(0);
			return 0;
		} break;

/* 		
		case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(window, &ps);
			
            // All painting occurs here, between BeginPaint and EndPaint.
			
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
			
            EndPaint(window, &ps);
			return 0;
		}
		 */

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
		.hCursor = LoadCursor(NULL, IDC_ARROW),
		.lpszClassName = L"wcap_window_class",
	};
	
	ATOM atom = RegisterClassExW(&window_class);
	Assert(atom && "Could not register class"); 
	
	g_window = CreateWindowExW(0, // optional window styles 
							   window_class.lpszClassName,
							   APP_TITLE, 
							   WS_OVERLAPPEDWINDOW, // window styles
							   CW_USEDEFAULT, CW_USEDEFAULT, window_width, window_height, 
							   NULL, NULL, window_class.hInstance, NULL);
    Assert(g_window && "Failed to create window");
	

/* 	
	g_groupbox1 = CreateWindowEx(0, WC_BUTTON, L"GroupBox 1", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 10, 10, 305, 460, g_window, NULL, NULL, NULL);
	g_groupbox2 = CreateWindowEx(0, WC_BUTTON, L"", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 325, 10, 305, 460, g_window, NULL, NULL, NULL);
	g_button    = CreateWindowEx(0, WC_BUTTON, L"Click Me", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,30, 30, 100, 20, g_window, NULL, NULL, NULL);
	 */

	
	
    HRESULT hr;
    ID3D11Device* device;         // represents the adapter and used to create resources
    ID3D11DeviceContext* context; // also represents the adapter but used to send commands to render and configure the rendering pipline
	IDXGISwapChain *swap_chain;   // represents the swap chain used to swich between the back and front buffers 
	
	// create device & context & swap chain
	{
		
		UINT flags = 0; 
		
#if _DEBUG
		// this enables VERY USEFUL debug messages in debugger output
		flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif 
		
		DXGI_SWAP_CHAIN_DESC scd = {
			.BufferDesc = { 
				// default 0 value for width & height means to get it from HWND automatically
				.Width = 0,
				.Height = 0,
				
				// when set to 0, it defaults to the screents refresh rate
				.RefreshRate = { 
					.Numerator = 0,
					.Denominator = 0,
				},
				
				// or use DXGI_FORMAT_R8G8B8A8_UNORM_SRGB for storing sRGB
				.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
				.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
				.Scaling = DXGI_MODE_SCALING_UNSPECIFIED // we don't scale right now 
			},
			
			// FLIP presentation model does not allow MSAA framebuffer
			// if you want MSAA then you'll need to render offscreen and manually
			// resolve to non-MSAA framebuffer
			.SampleDesc = { // used for AA 
				.Count = 1, 
				.Quality = 0,
			},
			
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = 1, // this says that there is one back buffer
			.OutputWindow = g_window,
			.Windowed = TRUE,
			
			// use more efficient FLIP presentation model
			// Windows 10 allows to use DXGI_SWAP_EFFECT_FLIP_DISCARD
			// for Windows 8 compatibility use DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL
			// for Windows 7 compatibility use DXGI_SWAP_EFFECT_DISCARD
			.SwapEffect = DXGI_SWAP_EFFECT_DISCARD,
			
			.Flags = 0
		};
		
		hr = D3D11CreateDeviceAndSwapChain(NULL,
										   D3D_DRIVER_TYPE_HARDWARE,
										   NULL, flags,
										   NULL, 0,
										   D3D11_SDK_VERSION, // the type of d3d software required from the user to install 
										   &scd,
										   &swap_chain,
										   &device,
										   NULL,
										   &context // execute commands immediately or defer it
										   );
		AssertHR(hr);
	}
	
	
#if _DEBUG
	// for debug builds enable VERY USEFUL debug break on API errors
    {
        ID3D11InfoQueue* info;
        ID3D11Device_QueryInterface(device, &IID_ID3D11InfoQueue, (void**)&info);
        ID3D11InfoQueue_SetBreakOnSeverity(info, D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        ID3D11InfoQueue_SetBreakOnSeverity(info, D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
        ID3D11InfoQueue_Release(info);
    }
	
    // enable debug break for DXGI too
    {
        IDXGIInfoQueue* dxgiInfo;
        hr = DXGIGetDebugInterface1(0, &IID_IDXGIInfoQueue, (void**)&dxgiInfo);
        AssertHR(hr);
        IDXGIInfoQueue_SetBreakOnSeverity(dxgiInfo, DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        IDXGIInfoQueue_SetBreakOnSeverity(dxgiInfo, DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
        IDXGIInfoQueue_Release(dxgiInfo);
    }
	
    // after this there's no need to check for any errors on device functions manually
    // so all HRESULT return  values in this code will be ignored
    // debugger will break on errors anyway
#endif
	
	
	// get the target view
	ID3D11RenderTargetView *target_view = NULL;
	{
		// gain access to texture subresource in swap chain (back buffer) 
		ID3D11Resource *backbuffer = NULL; 
		IDXGISwapChain1_GetBuffer(swap_chain, 0, &IID_ID3D11Resource, &backbuffer);
		ID3D11Device_CreateRenderTargetView(device, backbuffer, NULL, &target_view);
		ID3D11Resource_Release(backbuffer);
	}
	
	
	
	// model data for scene construction
	struct Vertex {
		float position[2];
	};
	
	ID3D11Buffer *vbuffer;
		struct Vertex data[] =
		{
			{ -0.0f, +0.5f },
			{ +0.5f, -0.5f },
			{ -0.5f, -0.5f },

			{ 0.5f, 1.0f }, 
			{ 1.0f, 0.5f }, 
			{ 0.5f, 0.5f }
		};
		D3D11_BUFFER_DESC desc = {
			.ByteWidth = sizeof(data),
			.StructureByteStride = sizeof(struct Vertex),
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		};
		D3D11_SUBRESOURCE_DATA initial = { .pSysMem = data };
		
		hr = ID3D11Device_CreateBuffer(device, &desc, &initial, &vbuffer);
		AssertHR(hr);
	
	
	 // vertex & pixel shaders for drawing triangle
	ID3D11VertexShader *vshader = NULL;
	ID3D11PixelShader  *pshader = NULL;
	ID3D11InputLayout *layout = NULL; 
	ID3DBlob *vblob = NULL;
	ID3DBlob *pblob = NULL;
	{
	hr = D3DReadFileToBlob(L"d3d11_vshader.cso", &vblob);
		AssertHR(hr); 
		ID3D11Device_CreateVertexShader(device, 
										ID3D10Blob_GetBufferPointer(vblob), 
										ID3D10Blob_GetBufferSize(vblob), 
										NULL,&vshader);
		
		hr = D3DReadFileToBlob(L"d3d11_pshader.cso", &pblob);
		AssertHR(hr); 
		ID3D11Device_CreatePixelShader(device, 
										ID3D10Blob_GetBufferPointer(pblob), 
										ID3D10Blob_GetBufferSize(pblob), 
										NULL,&pshader);
		
		const D3D11_INPUT_ELEMENT_DESC desc[] = {
			{ "Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		
		ID3D11Device_CreateInputLayout(device,
									   desc, 
									   ArrayLength(desc),
									   ID3D10Blob_GetBufferPointer(vblob), 
									   ID3D10Blob_GetBufferSize(vblob), 
									   &layout); 
		
		ID3D10Blob_Release(vblob); 
		ID3D10Blob_Release(pblob);
		
	}
	
	
	
	Assert(ShowWindow(g_window, SW_SHOW) == 0);
	
	float angle = 0; 
	
	LARGE_INTEGER freq, start, end;
	QueryPerformanceFrequency(&freq);
	
	QueryPerformanceCounter(&start); 
	
	
	for (;;)
	{
		MSG msg;
		if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            continue;
        }
		
		
		D3D11_VIEWPORT viewport =
		{
			.TopLeftX = 0,
			.TopLeftY = 0,
			.Width = (FLOAT)window_width,
			.Height = (FLOAT)window_height,
			.MinDepth = 0,
			.MaxDepth = 1,
		};
		
		
		QueryPerformanceCounter(&end); 
		float delta = (float)((double)(end.QuadPart - start.QuadPart) / freq.QuadPart);
		start = end; 
		
		angle += delta * 2.0f * (float)M_PI / 20.0f; // full rotation in 20 seconds
		angle = fmodf((float)angle, 2.0f * (float)M_PI);
		
		struct ConstantBuffer {
			struct {
				float mat[4][4];
			} transformation;
		};
		// z rotation matrix
		float aspect = (float)window_height / (float)window_width;
		struct ConstantBuffer cb = { 
			{
				aspect*cosf(angle),  sinf(angle), 0.f, 0.f, 
				-aspect*sinf(angle), cosf(angle), 0.f, 0.f, 
				0.f,         0.f,        1.f, 0.f, 
				0.f,         0.f,        0.f, 1.f
			}
		}; 
					 
			ID3D11Buffer *ubuffer; 
			{
				D3D11_BUFFER_DESC desc = {
					// I need space for 4x4 float matrix
					.ByteWidth = sizeof(struct ConstantBuffer), 
					.Usage = D3D11_USAGE_DYNAMIC, 
					.BindFlags = D3D11_BIND_CONSTANT_BUFFER, 
					.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE
				};
							
				D3D11_SUBRESOURCE_DATA srd = { .pSysMem = &cb };
							
				ID3D11Device_CreateBuffer(device, &desc, &srd, &ubuffer); 
			}
					
		
		// Clear screen
		float r = 1, g = 1, b = .5f;
		float color[] = { r, g, b, 1.f};
		ID3D11DeviceContext_ClearRenderTargetView(context, target_view, color); 

		// Input Assembler
		// bind the vertex buffer to the pipline
		ID3D11DeviceContext_IASetInputLayout(context, layout);
		ID3D11DeviceContext_IASetPrimitiveTopology(context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		const UINT stride = sizeof(struct Vertex);
		const UINT offset = 0;
		ID3D11DeviceContext_IASetVertexBuffers(context, 0, 1, &vbuffer, &stride, &offset);
		
		// Vertex Shader
		ID3D11DeviceContext_VSSetShader(context, vshader, NULL, 0); 
		ID3D11DeviceContext_VSSetConstantBuffers(context, 0, 1, &ubuffer);
		ID3D11Buffer_Release(ubuffer);
		
		// Rasterizer Stage
		ID3D11DeviceContext_RSSetViewports(context, 1, &viewport);
		
		// Pixel Shader
		ID3D11DeviceContext_PSSetShader(context, pshader, NULL, 0); 
		
		// Output Merger
		ID3D11DeviceContext_OMSetRenderTargets(context, 1, &target_view, NULL);
		
		// draw verticies
		ID3D11DeviceContext_Draw(context, ArrayLength(data), 0);
		
		// change to FALSE to disable vsync
		BOOL vsync = TRUE;
		hr = IDXGISwapChain1_Present(swap_chain, vsync ? 1 : 0, 0);
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
	 
	// release all allocated resources 
	ID3D11Device_Release(device);
	ID3D11DeviceContext_Release(context); 
	IDXGISwapChain1_Release(swap_chain);
	ID3D11RenderTargetView_Release(target_view);
	ID3D11VertexShader_Release(vshader); 
	ID3D11PixelShader_Release(pshader); 
	ID3D11InputLayout_Release(layout);
	return 0;
}
