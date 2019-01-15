#include "main.h"
//////////////////////////////////////////////////////////////////////////////////////
//	Global Variables
//////////////////////////////////////////////////////////////////////////////////////
HINSTANCE	g_hInst = NULL;
HWND		g_hWnd = NULL;


// Rename for each tutorial – This will appear in the title bar of the window
char		g_TutorialName[100] = "Ethan Bruins AE02\0";

GameManager*			g_pGameManager;



//////////////////////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
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

	if (FAILED(g_pGameManager->SetUpDirectX()))
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
			//Game Loop
			if (g_pGameManager)
			{
				g_pGameManager->m_pTimer->UpdateTimer();
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
	char Name[100] = "Ethan Bruins - AE2";

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

	//Set up the game manager
	g_pGameManager = new GameManager((UINT)(rc.bottom - rc.top), (UINT)(rc.right - rc.left), &g_hWnd, &g_hInst);

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
				g_pGameManager->GetImmediateContext()->OMSetRenderTargets(0, 0, 0);
				//Release buffers
				g_pGameManager->ReleaseRenderTarget();
				g_pGameManager->ReleaseZBuffer();

				ID3D11RenderTargetView* backBufferRTView;
				ID3D11DepthStencilView* zBuffer;

				HRESULT hr;

				hr = g_pGameManager->GetSwapChain()->ResizeBuffers(0, LOWORD(lParam), HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);

				ID3D11Texture2D* pBuffer;
				hr = g_pGameManager->GetSwapChain()->GetBuffer(0, _uuidof(ID3D11Texture2D), (void**)&pBuffer);
				//create new render target
				hr = g_pGameManager->GetDevice()->CreateRenderTargetView(pBuffer, NULL, &backBufferRTView);

				pBuffer->Release();

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
				hr = g_pGameManager->GetDevice()->CreateTexture2D(&tex2dDesc, NULL, &pZBufferTexture);
				if (FAILED(hr)) return hr;

				D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
				ZeroMemory(&dsvDesc, sizeof(dsvDesc));
				dsvDesc.Format = tex2dDesc.Format;
				dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				//Create new z  buffer
				g_pGameManager->GetDevice()->CreateDepthStencilView(pZBufferTexture, &dsvDesc, &zBuffer);
				pZBufferTexture->Release();
				g_pGameManager->SetZBuffer(zBuffer);

				g_pGameManager->GetImmediateContext()->OMSetRenderTargets(1, &backBufferRTView, zBuffer);


				//Set up viewport
				D3D11_VIEWPORT vp;
				vp.Width = LOWORD(lParam);
				vp.Height = HIWORD(lParam);
				vp.MinDepth = 0.0f;
				vp.MaxDepth = 1.0f;
				vp.TopLeftX = 0;
				vp.TopLeftY = 0;

				//Update the game manager
				g_pGameManager->GetImmediateContext()->RSSetViewports(1, &vp);
				g_pGameManager->SetScreenHeight(HIWORD(lParam));
				g_pGameManager->SetScreenWidth(LOWORD(lParam));
				g_pGameManager->SetRenderTarget(backBufferRTView);
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

//Clean up the app
void ShutdownD3D()
{
	if (g_pGameManager) delete g_pGameManager;
}
