//The #include order is important
#include <d3d11.h>
#include <dxgi.h>
#include <d3dx11.h>
#include <windows.h>
#include <dxerr.h>
#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
#include <xnamath.h>
#include "Camera.h"

//////////////////////////////////////////////////////////////////////////////////////
//	Global Variables
//////////////////////////////////////////////////////////////////////////////////////
HINSTANCE	g_hInst = NULL;
HWND		g_hWnd = NULL;

// Rename for each tutorial – This will appear in the title bar of the window
char		g_TutorialName[100] = "EB Tutorial 07 Exercise 01\0";

D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pD3DDevice = NULL;
ID3D11DeviceContext*    g_pImmediateContext = NULL;
IDXGISwapChain*         g_pSwapChain = NULL;
ID3D11RenderTargetView* g_pBackBufferRTView = NULL;

ID3D11Buffer*			g_pVertexBuffer;
ID3D11VertexShader*		g_pVertexShader;
ID3D11PixelShader*		g_pPixelShader;
ID3D11InputLayout*		g_pInputLayout;
ID3D11Buffer*			g_pConstantBuffer0; //Tutorial 04-01
ID3D11Buffer*			g_pConstantBufferWVP; //Tutorial 05-01
ID3D11DepthStencilView* g_pZBuffer; //Tutorial 06-01b
Camera*					g_cam;
POINT					g_mousePos;


//Define vertex structure
struct POS_COL_VERTEX //This will be added to and renamed in future tutorials
{
	XMFLOAT3 pos;
	XMFLOAT4 Col;
};

struct CONSTANT_BUFFER0
{
	float RedAmount; // 4 bytes
	float GreenAmount; // 4 bytes
	float BlueAmount; // 4 bytes
	float degrees; // 4 bytes
};

//Buffer for the cameras position
struct CONSTANT_BUFFER1
{
	XMMATRIX WorldViewProjection; //64 bytes
	float vertical; //4 bytes
	float horizontal; //4 bytes
	float width; // 4 bytes
	float height; // 4 bytes
};


CONSTANT_BUFFER0 cb0_values;
CONSTANT_BUFFER1 cbWVP_values;

//////////////////////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitialiseD3D();
void ShutdownD3D();
void RenderFrame(void);
HRESULT InitialiseGraphics(void);



//////////////////////////////////////////////////////////////////////////////////////
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//////////////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitialiseWindow(hInstance, nCmdShow)))
	{
		DXTRACE_MSG("Failed to create Window");
		return 0;
	}

	if (FAILED(InitialiseD3D()))
	{
		DXTRACE_MSG("Failed to create Device");
		return 0;
	}

	if (FAILED(InitialiseGraphics()))
	{
		DXTRACE_MSG("Failed to initialise graphics");
		return 0;
	}


	// Main message loop
	MSG msg = { 0 };

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			
			// do something
			g_cam->Update();
			RenderFrame();
		}
	}

	ShutdownD3D();
	return (int)msg.wParam;
}


//////////////////////////////////////////////////////////////////////////////////////
// Register class and create window
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Give your app your own name
	char Name[100] = "Ethan Bruins";

	// Register class
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	//   wcex.hbrBackground = (HBRUSH )( COLOR_WINDOW + 1); // Needed for non-D3D apps
	wcex.lpszClassName = Name;

	if (!RegisterClassEx(&wcex)) return E_FAIL;

	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, 640, 480 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(Name, g_TutorialName, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left,
		rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
	if (!g_hWnd)
		return E_FAIL;

	ShowWindow(g_hWnd, nCmdShow);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////
// Called every time the application receives a message
//////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		if (g_pSwapChain)
		{
			g_pImmediateContext->OMSetRenderTargets(0, 0, 0);
			
			//Release all outstanding references to the swap chain's buffers.
			g_pBackBufferRTView->Release();

			HRESULT hr;
			//Preserve the existing buffer count and format.
			//Automatically choose the width and height to match the client rect
			hr = g_pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

			//Get buffer and create a render-target-view
			ID3D11Texture2D* pBuffer;
			hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBuffer);

			if (FAILED(hr))
				return hr;

			hr = g_pD3DDevice->CreateRenderTargetView(pBuffer, NULL, &g_pBackBufferRTView);

			if (FAILED(hr))
				return hr;
			pBuffer->Release();

			g_pImmediateContext->OMSetRenderTargets(1, &g_pBackBufferRTView, NULL);

			//Set up viewport
			D3D11_VIEWPORT vp;
			vp.Width = LOWORD(lParam);
			vp.Height = HIWORD(lParam);
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;

			g_pImmediateContext->RSSetViewports(1, &vp);
		}
		cbWVP_values.height = HIWORD(lParam);
		cbWVP_values.width = LOWORD(lParam);
		return 1;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			DestroyWindow(g_hWnd);
		//R key
		if (wParam == 0x52)
			cb0_values.RedAmount += 0.1f;
		//G key
		if (wParam == 0x47)
			g_cam->RotateCamera(-1.0f, 0.0f);
		//B key
		if (wParam == 0x42)
			g_cam->RotateCamera(1.0f,0.0f);
		//C key
		if (wParam == 0x43)
			cb0_values.degrees -= 0.1f;
		//E Key
		if (wParam == 0x45)
			g_cam->Up(-1);
		//Q Key
		if (wParam == 0x51)
			g_cam->Up(1);
		//W Key
		if (wParam == 0x57)
			g_cam->Forward(1.0f);
		//S Key
		if (wParam == 0x53)
			g_cam->Forward(-1.0f);
		//D Key
		if (wParam == 0x44)
			g_cam->Strafe(1.0f);
		//A Key
		if (wParam == 0x41)
			g_cam->Strafe(-1.0f);
		if (wParam == VK_UP)
			g_cam->RotateCamera(0.0f, -1.0f);
		if (wParam == VK_DOWN)
			g_cam->RotateCamera(0.0f, 1.0f);
		if (wParam == VK_LEFT)
			g_cam->RotateCamera(-1.0f, 0.0f);
		if (wParam == VK_RIGHT)
			g_cam->RotateCamera(1.0f, 0.0f);
		//1 Key
		if (wParam == 0x31)
			g_cam->ChangeCameraType(CameraType::FirstPerson);
		//2 Key
		if (wParam == 0x32)
			g_cam->ChangeCameraType(CameraType::FreeLook);
		return 0;

	case WM_LBUTTONDOWN:
		g_mousePos.x = LOWORD(lParam);
		g_mousePos.y = HIWORD(lParam);
		if (g_mousePos.x < cbWVP_values.width / 2 && g_mousePos.y < cbWVP_values.height / 2)
			g_cam->RotateCamera(-1.0f, -1.0f);
		else if (g_mousePos.x > cbWVP_values.width / 2 && g_mousePos.y < cbWVP_values.height / 2)
			g_cam->RotateCamera(1.0f, -1.0f);
		else if (g_mousePos.x > cbWVP_values.width / 2 && g_mousePos.y > cbWVP_values.height / 2)
			g_cam->RotateCamera(1.0f, 1.0f);
		else if (g_mousePos.x < cbWVP_values.width / 2 && g_mousePos.y > cbWVP_values.height / 2)
			g_cam->RotateCamera(-1.0f, 1.0f);
		return 0;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
// Create D3D device and swap chain
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseD3D()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;
	
	UINT createDeviceFlags = 0;

//#ifdef _DEBUG
//	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
//#endif
//
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE, // comment out this line if you need to test D3D 11.0 functionality on hardware that doesn't support it
		D3D_DRIVER_TYPE_WARP, // comment this out also to use reference device
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL,
			createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &g_pSwapChain,
			&g_pD3DDevice, &g_featureLevel, &g_pImmediateContext);
		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return hr;

	//Get pointer to back buffer texture
	ID3D11Texture2D *pBackBufferTexture;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBufferTexture);

	if (FAILED(hr))
		return hr;

	//Use the back buffer texture pointer to create the render target
	hr = g_pD3DDevice->CreateRenderTargetView(pBackBufferTexture, NULL, &g_pBackBufferRTView);

	pBackBufferTexture->Release();

	if (FAILED(hr))
		return hr;

	//Create Z Buffer texture
	D3D11_TEXTURE2D_DESC tex2dDesc;
	ZeroMemory(&tex2dDesc, sizeof(tex2dDesc));

	tex2dDesc.Width = width;
	tex2dDesc.Height = height;
	tex2dDesc.ArraySize = 1;
	tex2dDesc.MipLevels = 1;
	tex2dDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tex2dDesc.SampleDesc.Count = sd.SampleDesc.Count;
	tex2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	tex2dDesc.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D* pZBufferTexture;
	hr = g_pD3DDevice->CreateTexture2D(&tex2dDesc, NULL, &pZBufferTexture);
	if (FAILED(hr))
		return hr;

	//Create the Z Buffer;
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));

	dsvDesc.Format = tex2dDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	g_pD3DDevice->CreateDepthStencilView(pZBufferTexture, &dsvDesc, &g_pZBuffer);
	pZBufferTexture->Release();


	//Set the render target view
	g_pImmediateContext->OMSetRenderTargets(1, &g_pBackBufferRTView, g_pZBuffer);

	//Set the viewport
	D3D11_VIEWPORT viewport;

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (FLOAT)width;
	viewport.Height = (FLOAT)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	
	g_pImmediateContext->RSSetViewports(1, &viewport);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////
// Clean up D3D objects
//////////////////////////////////////////////////////////////////////////////////////
void ShutdownD3D()
{
	if (g_cam) g_cam = nullptr;
	if (g_pZBuffer) g_pZBuffer->Release(); //06-01b
	if (g_pConstantBuffer0) g_pConstantBuffer0->Release(); //04-01
	if (g_pVertexBuffer) g_pVertexBuffer->Release(); //03-01
	if (g_pInputLayout) g_pInputLayout->Release(); //03-01
	if (g_pVertexShader) g_pVertexShader->Release(); //03-01
	if (g_pPixelShader) g_pPixelShader->Release(); //03-01
	if (g_pBackBufferRTView) g_pBackBufferRTView->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pD3DDevice) g_pD3DDevice->Release();
}

/////////////////////////////////////////////////////////////////////////////////////////////
//Init graphics - Tutorial 03
/////////////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseGraphics()
{
	HRESULT hr = S_OK;

	//Define vertices of a Cube
	POS_COL_VERTEX vertices[] =
	{
	//Back Face
	{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },

	//Front Face
	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }, 
	{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },

	//Left Face
	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },

	//Right Face
	{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },

	//Bottom Face
	{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
	{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
	{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },

	//Top Face
	{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
	{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
	{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },

	};

	//Set up and create vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;										//Allows use by CPU and GPU
	bufferDesc.ByteWidth = sizeof(vertices);									//Set the total size of the buffer (in this case, 3 vertices)
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;							//Set the type of buffer to vertex buffer
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;							//Allow access by the CPU
	hr = g_pD3DDevice->CreateBuffer(&bufferDesc, NULL, &g_pVertexBuffer);		//Create the buffer

	if (FAILED(hr))//Return an error code if failed
	{
		return hr;
	}

	//Set up constant buffer
	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
	
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT; //Can use UpdateSubresources() to update
	constantBufferDesc.ByteWidth = 16; //MUST be a multiple of 16, calculate CB struct
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; //Use as a constant buffer

	hr = g_pD3DDevice->CreateBuffer(&constantBufferDesc, NULL, &g_pConstantBuffer0);
	
	if (FAILED(hr))
		return hr;

	cb0_values.degrees = 5.0f;
	cb0_values.BlueAmount = 0.5f;
	cb0_values.GreenAmount = 0.5f;
	cb0_values.RedAmount = 0.5f;


	//Set up constantbuffer for WorldViewProjection Tutorial 05-01
	D3D11_BUFFER_DESC constantBufferDesc0;
	ZeroMemory(&constantBufferDesc0, sizeof(constantBufferDesc0));

	constantBufferDesc0.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDesc0.ByteWidth = 80; //Must be equal to the size of the constant buffer we are going to use
	constantBufferDesc0.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = g_pD3DDevice->CreateBuffer(&constantBufferDesc0, NULL, &g_pConstantBufferWVP);

	if (FAILED(hr))
		return hr;
	
	cbWVP_values.vertical = 0.0;
	cbWVP_values.horizontal = 0.0;

	//Copy the vertices into the buffer
	D3D11_MAPPED_SUBRESOURCE ms;

	//Lock the buffer to allow writing
	g_pImmediateContext->Map(g_pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);

	//Copy the data
	memcpy(ms.pData, vertices, sizeof(vertices));

	//Unlock the buffer
	g_pImmediateContext->Unmap(g_pVertexBuffer, NULL);

	//Load and compile the pixel and vertex shaders - use vs_5_0 to target DX11 hardware only
	ID3DBlob *VS, *PS, *error;
	hr = D3DX11CompileFromFile("shaders.hlsl", 0, 0, "VShader", "vs_4_0", 0, 0, 0, &VS, &error, 0);

	if (error != 0)//Check for shader compilation error
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if (FAILED(hr))//Don't fail if error is just a warning
		{
			return hr;
		}
	}

	hr = D3DX11CompileFromFile("shaders.hlsl", 0, 0, "PShader", "ps_4_0", 0, 0, 0, &PS, &error, 0);

	if (error != 0)//Check for shader compilation error
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if (FAILED(hr))//Don't fail if error is just a warning
		{
			return hr;
		}
	}

	//Create shader objects
	hr = g_pD3DDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &g_pVertexShader);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = g_pD3DDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &g_pPixelShader);
	if (FAILED(hr))
	{
		return hr;
	}

	//Set the shader objects as active
	g_pImmediateContext->VSSetShader(g_pVertexShader, 0, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, 0, 0);

	//Create and set the input layout object
	D3D11_INPUT_ELEMENT_DESC iedesc[] =
	{
		//Be very careful setting the correct dxgi format and D3D version
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		//NOTE the spelling of COLOR. Again, be careful setting the correct dxgi format (using A32) and correct D3D version
		{ "COLOR", 0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },
	};

	hr = g_pD3DDevice->CreateInputLayout(iedesc, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &g_pInputLayout);
	if (FAILED(hr))
	{
		return hr;
	}

	g_pImmediateContext->IASetInputLayout(g_pInputLayout);

	g_cam = new Camera(0.0, 0.0, -0.5, 0.0);

	return S_OK;
}
/////////////////////////////////////////////////////////////////////////////////////////////

// Render frame
void RenderFrame(void)
{
	// Clear the back buffer - choose a colour you like
	float rgba_clear_colour[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	g_pImmediateContext->ClearRenderTargetView(g_pBackBufferRTView, rgba_clear_colour);
	g_pImmediateContext->ClearDepthStencilView(g_pZBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//Render Here
	XMMATRIX projection, world, view, world1;
	
	//world = XMMatrixTranslation(cbWVP_values.horizontal, cbWVP_values.vertical, cb0_values.AlphaAmount);
	//world = XMMatrixRotationZ(XMConvertToRadians(15.0f)); // 06-01

	world = XMMatrixRotationRollPitchYaw(0.0f, 1.0f, 10.0f + cb0_values.degrees);

	world *= XMMatrixTranslation(0, 0, 5); // 05-03

	projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0), cbWVP_values.width / cbWVP_values.height, 1.0, 100.0);
	view = g_cam->GetViewMatrix();
	cbWVP_values.WorldViewProjection = world * view * projection;

	//Upload new values to buffer
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer0, 0, 0, &cb0_values, 0, 0);
	g_pImmediateContext->UpdateSubresource(g_pConstantBufferWVP, 0, 0, &cbWVP_values, 0, 0);

	//Set the constant buffer active
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer0);
	g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pConstantBufferWVP);

	g_pImmediateContext->Draw(36, 0);

	//Start of second cube
	world1 = XMMatrixRotationRollPitchYaw(0.0f, 10.0f + cb0_values.degrees, 1.0f);
	world1 *= XMMatrixTranslation(0, 0, 5 + cb0_values.degrees); // 05-03

	cbWVP_values.WorldViewProjection = world1 * view * projection;

	//Upload new values to buffer
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer0, 0, 0, &cb0_values, 0, 0);
	g_pImmediateContext->UpdateSubresource(g_pConstantBufferWVP, 0, 0, &cbWVP_values, 0, 0);

	//Set the constant buffer active
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer0);
	g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pConstantBufferWVP);

	//Draw the vertex buffer to the back buffer 06-01 (now a cube)
	g_pImmediateContext->Draw(36, 0);

	//End of second cube

	//Start of third cube
	world1 = XMMatrixRotationRollPitchYaw(0.0f + cb0_values.degrees, 0.0f, 1.0f);
	world1 *= XMMatrixTranslation(5, 0, 5);

	cbWVP_values.WorldViewProjection = world1 * view * projection;

	//Upload new values to buffer
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer0, 0, 0, &cb0_values, 0, 0);
	g_pImmediateContext->UpdateSubresource(g_pConstantBufferWVP, 0, 0, &cbWVP_values, 0, 0);

	//Set the constant buffer active
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer0);
	g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pConstantBufferWVP);

	//Draw the vertex buffer to the back buffer 06-01 (now a cube)
	g_pImmediateContext->Draw(36, 0);
	//Set vertex buffer 03-01
	UINT stride = sizeof(POS_COL_VERTEX);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	//Select primitive type to use 03-01
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	//Display what has just been rendered
	g_pSwapChain->Present(0, 0);
}

