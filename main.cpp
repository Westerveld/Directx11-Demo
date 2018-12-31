#include "main.h"

//////////////////////////////////////////////////////////////////////////////////////
//	Global Variables
//////////////////////////////////////////////////////////////////////////////////////
HINSTANCE	g_hInst = NULL;
HWND		g_hWnd = NULL;


// Rename for each tutorial – This will appear in the title bar of the window
char		g_TutorialName[100] = "EB AE02\0";

D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;

GameManager*			g_pGameManager;



//////////////////////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitialiseD3D();
void ShutdownD3D();


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
	if (FAILED(g_pGameManager->InitialiseGraphics()))
	{
		DXTRACE_MSG("Failed to initialise graphics");
		return 0;
	}

	g_pGameManager->LoadLevel("assets/Levels/1.lvl");

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
			if (g_pGameManager)
			{
				g_pGameManager->CheckInputs();
				g_pGameManager->Update();
				g_pGameManager->Render();
			}
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
		if (g_pGameManager != NULL)
		{
			if (g_pGameManager->GetSwapChain())
			{

				ID3D11Device* device = g_pGameManager->GetDevice();
				ID3D11DeviceContext* immediateContext = g_pGameManager->GetImmediateContext();
				ID3D11RenderTargetView* backBuffer = g_pGameManager->GetRenderTarget();
				ID3D11DepthStencilView* zBuffer = g_pGameManager->GetZBuffer();
				IDXGISwapChain* swapChain = g_pGameManager->GetSwapChain();
				immediateContext->OMSetRenderTargets(0, 0, 0);

				//Release all outstanding references to the swap chain's buffers.
				backBuffer->Release();

				zBuffer->Release();

				HRESULT hr;
				//Preserve the existing buffer count and format.
				//Automatically choose the width and height to match the client rect
				hr = swapChain->ResizeBuffers(0, LOWORD(lParam), HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);

				// Get buffer and create new render-target-view
				ID3D11Texture2D* pBuffer;
				hr = swapChain->GetBuffer(0, _uuidof(ID3D11Texture2D), (void**)&pBuffer);

				hr = device->CreateRenderTargetView(pBuffer, NULL, &backBuffer);
				pBuffer->Release();


				// Create Z buffer texture
				D3D11_TEXTURE2D_DESC tex2dDesc;
				ZeroMemory(&tex2dDesc, sizeof(tex2dDesc));

				tex2dDesc.Width = LOWORD(lParam);
				tex2dDesc.Height = HIWORD(lParam);
				tex2dDesc.ArraySize = 1;
				tex2dDesc.MipLevels = 1;
				tex2dDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
				tex2dDesc.SampleDesc.Count = 1;
				tex2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
				tex2dDesc.Usage = D3D11_USAGE_DEFAULT;

				ID3D11Texture2D *pZBufferTexture;
				hr = device->CreateTexture2D(&tex2dDesc, NULL, &pZBufferTexture);
				if (FAILED(hr)) return hr;

				// create the z buffer
				D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
				ZeroMemory(&dsvDesc, sizeof(dsvDesc));
				dsvDesc.Format = tex2dDesc.Format;
				dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

				device->CreateDepthStencilView(pZBufferTexture, &dsvDesc, &zBuffer);
				pZBufferTexture->Release();


				immediateContext->OMSetRenderTargets(1, &backBuffer, zBuffer);


				//Set up viewport
				D3D11_VIEWPORT vp;
				vp.Width = LOWORD(lParam);
				vp.Height = HIWORD(lParam);
				vp.MinDepth = 0.0f;
				vp.MaxDepth = 1.0f;
				vp.TopLeftX = 0;
				vp.TopLeftY = 0;
				if (g_pGameManager)
				{
					g_pGameManager->SetScreenHeight(vp.Height);
					g_pGameManager->SetScreenWidth(vp.Width);
				}
				immediateContext->RSSetViewports(1, &vp);
			}
		}
		return 1;
	case WM_KEYDOWN:
		return 0;

	case WM_LBUTTONDOWN:
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
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swapChain;
	ID3D11RenderTargetView* backBuffer;
	ID3D11DepthStencilView* zBuffer;

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
			D3D11_SDK_VERSION, &sd, &swapChain,
			&device, &g_featureLevel, &context);
		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return hr;

	//Get pointer to back buffer texture
	ID3D11Texture2D *pBackBufferTexture;
	hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBufferTexture);

	if (FAILED(hr))
		return hr;

	//Use the back buffer texture pointer to create the render target
	hr = device->CreateRenderTargetView(pBackBufferTexture, NULL, &backBuffer);

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
	hr = device->CreateTexture2D(&tex2dDesc, NULL, &pZBufferTexture);
	if (FAILED(hr))
		return hr;

	//Create the Z Buffer;
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));

	dsvDesc.Format = tex2dDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	device->CreateDepthStencilView(pZBufferTexture, &dsvDesc, &zBuffer);
	pZBufferTexture->Release();


	//Set the render target view
	context->OMSetRenderTargets(1, &backBuffer, zBuffer);

	//Set the viewport
	D3D11_VIEWPORT viewport;

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (FLOAT)width;
	viewport.Height = (FLOAT)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	
	context->RSSetViewports(1, &viewport);

	g_pGameManager = new GameManager(viewport.Height, viewport.Width, &g_hWnd, &g_hInst);

	g_pGameManager->SetDevice(device);
	g_pGameManager->SetImmediateContext(context);
	g_pGameManager->SetSwapChain(swapChain);
	g_pGameManager->SetZBuffer(zBuffer);
	g_pGameManager->SetRenderTarget(backBuffer);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////
// Clean up D3D objects
//////////////////////////////////////////////////////////////////////////////////////
void ShutdownD3D()
{
	if (g_pGameManager) delete g_pGameManager;
}
