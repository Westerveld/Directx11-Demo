#include "GameManager.h"



GameManager::GameManager(float height, float width)
{
	g_pScreenHeight = height;
	g_pScreenWidth = width;
	g_pLights = new LightManager();
	g_pTimer = new Timer();
}


GameManager::~GameManager()
{
}

HRESULT GameManager::InitialiseGraphics(ID3D11Device* device, ID3D11DeviceContext* context, IDXGISwapChain* swapChain, ID3D11RenderTargetView* backBuffer)
{
	g_pD3DDevice = device;
	g_pImmediateContext = context;
	g_pSwapChain = swapChain;
	g_pBackBufferRTView = backBuffer;


	HRESULT hr = S_OK;

	D3D11_SAMPLER_DESC sampler_desc;
	ZeroMemory(&sampler_desc, sizeof(sampler_desc));
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = g_pD3DDevice->CreateSamplerState(&sampler_desc, &g_pSampler0);

	if (FAILED(hr))
	{
		return hr;
	}

	

	hr = D3DX11CreateShaderResourceViewFromFile(g_pD3DDevice,
		"assets/brick.png", NULL, NULL,
		&g_pTextureBrick, NULL);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = D3DX11CreateShaderResourceViewFromFile(g_pD3DDevice,
		"assets/skybox_mountain.dds", NULL, NULL,
		&g_pTexureSkyBox, NULL);

	if (FAILED(hr))
	{
		return hr;
	}

	g_pCam = new Camera(0.0, 0.0, -0.5, 0.0);
	g_pLights = new LightManager();


	g_pText = new Text2D("assets/font1.bmp", g_pD3DDevice, g_pImmediateContext);

	g_pCube = new Model(g_pD3DDevice, g_pImmediateContext, g_pLights);
	g_pPlane = new Model(g_pD3DDevice, g_pImmediateContext, g_pLights);
	g_pSphere = new Model(g_pD3DDevice, g_pImmediateContext, g_pLights);
	g_pWall = new Model(g_pD3DDevice, g_pImmediateContext, g_pLights);

}

void GameManager::LoadLevel (char* textFile)
{
	if (textFile == NULL)
	{
		cout << "Invalided file" << endl;
	}



}


void GameManager::Update()
{
	g_pTimer->UpdateTimer();

	float deltaTime = g_pTimer->GetFrameTime();
}

void GameManager::Render()
{

	XMMATRIX world, view, projection;

	world = XMMatrixIdentity();
	view = g_pCam->GetViewMatrix();
	projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0), g_pScreenWidth / g_pScreenHeight, 1.0, 250.0);
	g_pRootNode->Execute(&world, &view, &projection);

	g_pSwapChain->Present(0, 0);
}