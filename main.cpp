//The #include order is important
#include <d3d11.h>
#include <dxgi.h>
#include <d3dx11.h>
#include <windows.h>
#include <dxerr.h>
#include <xnamath.h>

#include <list>
#include <string>
#include "camera.h"
#include "text2D.h"
#include "Model.h"
#include "InputHandler.h"
#include "SkyBox.h"
#include "ParticleFactory.h"
#include "Scene_Node.h"
#include "maths.h"

//////////////////////////////////////////////////////////////////////////////////////
//	Global Variables
//////////////////////////////////////////////////////////////////////////////////////
HINSTANCE	g_hInst = NULL;
HWND		g_hWnd = NULL;

// Rename for each tutorial – This will appear in the title bar of the window
char		g_TutorialName[100] = "EB AE02\0";

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
ID3D11DepthStencilView* g_pZBuffer; //Tutorial 06-01b
ID3D11ShaderResourceView* g_pBrickTexture; //Tutorial 08-01
ID3D11ShaderResourceView* g_pSkyBoxTexture;
ID3D11SamplerState*		g_pSampler0; //Tutorial 08-01


Camera*					g_cam;
maths*					g_maths;
POINT					g_mousePos;
Text2D*					g_2DText;
LightManager*			g_lights;
Model*					g_Sphere;
Model*					g_brickSphere;
Model*					g_Plane;
Model*					g_Cube;
SkyBox*					g_SkyBox;
ParticleFactory*		g_Particles;

Scene_Node*				g_rootNode;
Scene_Node*				g_node1;
Scene_Node*				g_node2;
Scene_Node*				g_node3;
Scene_Node*				g_camNode;

InputHandler*			g_Input;

//Define vertex structure
struct POS_COL_TEX_NORM_VERTEX //This will be added to and renamed in future tutorials
{
	XMFLOAT3 pos;
	XMFLOAT4 Col;
	XMFLOAT2 Texture0;
	XMFLOAT3 Normal;
};

//Buffer for the cameras position
struct CONSTANT_BUFFER0
{
	XMMATRIX WorldViewProjection; //64 bytes
	XMVECTOR dirLightVector; //16 bytes
	XMVECTOR dirLightCol; // 16 bytes
	XMVECTOR ambLightCol; // 16 bytes
	
};

float verticalMove; 
float horizontalMove; 
float screenWidth; 
float screenHeight;
float degrees;
CONSTANT_BUFFER0 cb0_values;

//Lighting
XMVECTOR g_dirLightLocation;
XMVECTOR g_dirLightColor;
XMVECTOR g_ambLightColor;

//MovementVars;

//////////////////////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitialiseD3D();
void ShutdownD3D();
void RenderFrame(void);
HRESULT InitialiseGraphics(void);
void CheckInputs();
void SetUpScene();
void UpdateCameraPosition();



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
	g_Input = new InputHandler(&g_hWnd, &g_hInst);
	if (FAILED(g_Input->InitialiseKeyboardInput()))
	{
		DXTRACE_MSG("Failed to initialise input");
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
	degrees = 0;
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

			CheckInputs();

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

			g_pZBuffer->Release();

			HRESULT hr;
			//Preserve the existing buffer count and format.
			//Automatically choose the width and height to match the client rect
			hr = g_pSwapChain->ResizeBuffers(0, LOWORD(lParam), HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);

			// Get buffer and create new render-target-view
			ID3D11Texture2D* pBuffer;
			hr = g_pSwapChain->GetBuffer(0, _uuidof(ID3D11Texture2D), (void**)&pBuffer);

			hr = g_pD3DDevice->CreateRenderTargetView(pBuffer, NULL, &g_pBackBufferRTView);
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
			hr = g_pD3DDevice->CreateTexture2D(&tex2dDesc, NULL, &pZBufferTexture);
			if (FAILED(hr)) return hr;

			// create the z buffer
			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
			ZeroMemory(&dsvDesc, sizeof(dsvDesc));
			dsvDesc.Format = tex2dDesc.Format;
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

			g_pD3DDevice->CreateDepthStencilView(pZBufferTexture, &dsvDesc, &g_pZBuffer);
			pZBufferTexture->Release();


			g_pImmediateContext->OMSetRenderTargets(1, &g_pBackBufferRTView, g_pZBuffer);


			//Set up viewport
			D3D11_VIEWPORT vp;
			vp.Width = LOWORD(lParam);
			vp.Height = HIWORD(lParam);
			screenHeight = HIWORD(lParam);
			screenWidth = LOWORD(lParam);
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;

			g_pImmediateContext->RSSetViewports(1, &vp);
		}
		return 1;
	case WM_KEYDOWN:
		return 0;

	case WM_LBUTTONDOWN:
		g_mousePos.x = LOWORD(lParam);
		g_mousePos.y = HIWORD(lParam);
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

	screenWidth = width;
	screenHeight = height;
	
	g_pImmediateContext->RSSetViewports(1, &viewport);
	g_2DText = new Text2D("assets/font1.bmp", g_pD3DDevice, g_pImmediateContext);
	
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////
// Clean up D3D objects
//////////////////////////////////////////////////////////////////////////////////////
void ShutdownD3D()
{
	if (g_rootNode) delete g_rootNode;
	if (g_Particles) delete g_Particles;
	if (g_SkyBox) delete g_SkyBox;
	if (g_Sphere) delete g_Sphere;
	if (g_Plane) delete g_Plane;
	if (g_2DText) delete g_2DText;
	if (g_Input) delete g_Input;
	if (g_cam) delete g_cam;
	if (g_pBrickTexture) g_pBrickTexture->Release();
	if (g_pSampler0) g_pSampler0->Release();
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

	D3D11_SAMPLER_DESC sampler_desc;
	ZeroMemory(&sampler_desc, sizeof(sampler_desc));
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	g_pD3DDevice->CreateSamplerState(&sampler_desc, &g_pSampler0);
	
	//Set up constant buffer
	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
	
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT; //Can use UpdateSubresources() to update
	constantBufferDesc.ByteWidth = 112; //MUST be a multiple of 16, calculate CB struct
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; //Use as a constant buffer

	hr = g_pD3DDevice->CreateBuffer(&constantBufferDesc, NULL, &g_pConstantBuffer0);
	
	if (FAILED(hr))
	{
		return hr;

	}	

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
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//NOTE the spelling of COLOR. Again, be careful setting the correct dxgi format (using A32) and correct D3D version
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = g_pD3DDevice->CreateInputLayout(iedesc, ARRAYSIZE(iedesc), VS->GetBufferPointer(), VS->GetBufferSize(), &g_pInputLayout);

	if (FAILED(hr))
	{
		return hr;
	}

	g_pImmediateContext->IASetInputLayout(g_pInputLayout);

	hr = D3DX11CreateShaderResourceViewFromFile(g_pD3DDevice,
		"assets/brick.png", NULL, NULL,
		&g_pBrickTexture, NULL);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = D3DX11CreateShaderResourceViewFromFile(g_pD3DDevice,
		"assets/skybox_mountain.dds", NULL, NULL,
		&g_pSkyBoxTexture, NULL);

	if (FAILED(hr))
	{
		return hr;
	}


	g_cam = new Camera(0.0, 0.0, -0.5, 0.0);
	g_lights = new LightManager();

	g_maths = new maths();

	g_Sphere = new Model(g_pD3DDevice, g_pImmediateContext, g_lights);
	g_Plane = new Model(g_pD3DDevice, g_pImmediateContext, g_lights);
	g_Cube = new Model(g_pD3DDevice, g_pImmediateContext, g_lights);
	
	hr = g_Sphere->LoadObjModel((char*)"assets/Sphere.obj");
	g_Sphere->LoadCustomShader((char*)"reflect_shader.hlsl", (char*)"ModelVS", (char*)"ModelPS");
	g_Sphere->ChangeModelType(ModelType::Shiny);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = g_Plane->LoadObjModel((char*)"assets/plane.obj");
	if (FAILED(hr))
	{
		return hr;
	}
	g_Plane->LoadDefaultShaders();

	hr = g_Cube->LoadObjModel((char*)"assets/cube.obj");
	if (FAILED(hr))
	{
		return hr;
	}
	

	g_Cube->LoadDefaultShaders();

	g_brickSphere = new Model(g_pD3DDevice, g_pImmediateContext, g_lights);
	g_brickSphere->LoadObjModel((char*)"assets/Sphere.obj");
	g_brickSphere->LoadDefaultShaders();
	g_brickSphere->SetSampler(g_pSampler0);
	g_brickSphere->SetTexture(g_pBrickTexture);

	g_Plane->SetSampler(g_pSampler0);
	g_Plane->SetTexture(g_pBrickTexture);
	g_Sphere->SetSampler(g_pSampler0);
	g_Sphere->SetTexture(g_pSkyBoxTexture);
	g_Cube->SetSampler(g_pSampler0);
	g_Cube->SetTexture(g_pBrickTexture);
	

	g_SkyBox = new SkyBox(g_pD3DDevice, g_pImmediateContext);
	hr = g_SkyBox->CreateSkybox("assets/brick.png");
	if (FAILED(hr))
	{
		return hr;
	}

	
	g_Particles = new ParticleFactory(g_pD3DDevice, g_pImmediateContext, g_lights);
	g_Particles->CreateParticle();
	g_Particles->SetActive(true);


	SetUpScene();
	return S_OK;
}
/////////////////////////////////////////////////////////////////////////////////////////////

// Render frame
void RenderFrame(void)
{
	g_2DText->AddText("count to null", -1.0, 1.0, .05);

	// Clear the back buffer - choose a colour you like
	float rgba_clear_colour[4] = { 0.0f, 0.1f, 0.1f, 1.0f };
	g_pImmediateContext->ClearRenderTargetView(g_pBackBufferRTView, rgba_clear_colour);
	g_pImmediateContext->ClearDepthStencilView(g_pZBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	//Render Here

	//Lighting set up
	g_dirLightLocation = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
	g_dirLightColor = XMVectorSet(0.75f, 0.75f, 0.75f, 0.0f); //green
	g_ambLightColor = XMVectorSet(0.5f, 0.5f, 0.5f, 1.0f); //dark grey ambient
	
	XMMATRIX projection, view, world;
	
	projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0), screenWidth / screenHeight, 1.0, 250.0);
	view = g_cam->GetViewMatrix();
	world = XMMatrixIdentity();

	g_SkyBox->RenderSkyBox(&view, &projection, g_cam);


	g_rootNode->Execute(&world, &view, &projection);
	

	//g_Particles->Draw(&view, &projection, &g_cam->GetPosition());



	g_2DText->RenderText();


	//Display what has just been rendered
	g_pSwapChain->Present(0, 0);
}

void CheckInputs(void)
{
	g_Input->ReadInputStates();

	if (g_Input->IsKeyPressed(DIK_ESCAPE))
		DestroyWindow(g_hWnd);

	if (g_Input->IsKeyPressed(DIK_E))
	{
		g_cam->Up(-0.010f);
		UpdateCameraPosition();

		XMMATRIX identity = XMMatrixIdentity();

		g_rootNode->UpdateCollisionTree(&identity, 1.0f);

		if (g_camNode->CheckCollision(g_rootNode))
		{
			g_cam->Up(0.010f);
			UpdateCameraPosition();
		}
	}

	if (g_Input->IsKeyPressed(DIK_Q))
	{
		g_cam->Up(0.010f);
		UpdateCameraPosition();

		XMMATRIX identity = XMMatrixIdentity();

		g_rootNode->UpdateCollisionTree(&identity, 1.0f);

		if (g_camNode->CheckCollision(g_rootNode))
		{
			g_cam->Up(-0.010f);
			UpdateCameraPosition();
		}
	}

	if (g_Input->IsKeyPressed(DIK_W))
	{
		g_cam->Forward(0.010f);
		UpdateCameraPosition();

		XMMATRIX identity = XMMatrixIdentity();

		g_rootNode->UpdateCollisionTree(&identity, 1.0f);

		if (g_camNode->CheckCollision(g_rootNode))
		{
			g_cam->Forward(-0.010f);
			UpdateCameraPosition();
		}
	}

	if (g_Input->IsKeyPressed(DIK_S))
	{
		g_cam->Forward(-0.010f);
		UpdateCameraPosition();

		XMMATRIX identity = XMMatrixIdentity();

		g_rootNode->UpdateCollisionTree(&identity, 1.0f);

		if (g_camNode->CheckCollision(g_rootNode))
		{
			g_cam->Forward(0.010f);
			UpdateCameraPosition();
		}
	}
	if (g_Input->IsKeyPressed(DIK_A))
	{
		g_cam->Strafe(-0.010f);
		UpdateCameraPosition();

		XMMATRIX identity = XMMatrixIdentity();

		g_rootNode->UpdateCollisionTree(&identity, 1.0f);

		if (g_camNode->CheckCollision(g_rootNode))
		{
			g_cam->Strafe(0.010f);
			UpdateCameraPosition();
		}
	}

	if (g_Input->IsKeyPressed(DIK_D))
	{
		g_cam->Strafe(0.010f);
		UpdateCameraPosition();

		XMMATRIX identity = XMMatrixIdentity();

		g_rootNode->UpdateCollisionTree(&identity, 1.0f);

		if (g_camNode->CheckCollision(g_rootNode))
		{
			g_cam->Strafe(-0.010f);
			UpdateCameraPosition();
		}
	}

	if (g_Input->IsKeyPressed(DIK_LEFT))
		

	if (g_Input->IsKeyPressed(DIK_RIGHT))
		g_cam->RotateCamera(0.010f, 0.0f);

	if (g_Input->IsKeyPressed(DIK_1))
		g_cam->ChangeCameraType(CameraType::FirstPerson);

	if (g_Input->IsKeyPressed(DIK_2))
		g_cam->ChangeCameraType(CameraType::FreeLook);


	if (g_Input->IsKeyPressed(DIK_6))
		g_Particles->SwitchParticleType(ParticleType::Explosion);

	if (g_Input->IsKeyPressed(DIK_5))
		g_Particles->SwitchParticleType(ParticleType::Fountain);

	if (g_Input->IsKeyPressed(DIK_F))
	{
		g_Particles->SetActive(!g_Particles->GetActive());
	}

	if (g_Input->GetMouseButtonDown(0))
	{
		g_cam->Forward(0.001f);
	}
	if (g_Input->GetMouseButtonDown(1))
	{
		g_cam->Forward(-0.001f);
	}

	if (g_Input->IsKeyPressed(DIK_J))
		g_node3->MoveForward(0.01f, g_rootNode);

	if (g_Input->IsKeyPressed(DIK_O))
		g_node2->MoveForward(0.01f, g_rootNode);

	if (g_Input->IsKeyPressed(DIK_I))
		g_node2->LookAt_XZ(g_cam->GetX(), g_cam->GetZ());
	
	g_cam->RotateCamera(g_Input->GetMouseX() * 0.1f, g_Input->GetMouseY() * 0.1f);
}

void SetUpScene()
{
	g_rootNode = new Scene_Node();
	//g_node1 = new Scene_Node();
	g_node2 = new Scene_Node();
	g_node3 = new Scene_Node();
	g_camNode = new Scene_Node();

	//g_node1->SetModel(g_Plane);
	//g_node1->SetYPos(-3.0f);
	g_node2->SetModel(g_Sphere);
	g_node2->SetXPos(5.0f);
	g_node2->SetZPos(10.0f);
	g_node3->SetModel(g_brickSphere);
	g_node3->SetXPos(-5.0f);
	g_node3->SetXPos(10.0f);
	g_camNode->SetModel(g_Cube);
	g_camNode->SetScale(0.1f);

	//g_rootNode->AddChildNode(g_node1);
	g_rootNode->AddChildNode(g_camNode);
	g_rootNode->AddChildNode(g_node2);
	g_rootNode->AddChildNode(g_node3);

	UpdateCameraPosition();
}

void UpdateCameraPosition()
{
	g_camNode->SetXPos(g_cam->GetX());
	g_camNode->SetYPos(g_cam->GetY());
	g_camNode->SetZPos(g_cam->GetZ());
}