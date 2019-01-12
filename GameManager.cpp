#include "GameManager.h"
#include "Player.h"
#include "Enemy.h"
#include "Movable.h"



D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;


GameManager::GameManager(float height, float width, HWND* hWnd, HINSTANCE* hInst)
{
	m_pScreenHeight = height;
	m_pScreenWidth = width;
	m_pLights = new LightManager();
	m_pTimer = new TimeHandler();
	m_pTimer->StartTimer();
	m_phWnd = hWnd;
	m_phInst = hInst;
	m_pInput = new InputHandler(hWnd, hInst);
	m_pInput->InitialiseKeyboardInput();
	m_enableAlpha = false;
}

//Game Clean Up
GameManager::~GameManager()
{
	if (m_pEnemy)				delete m_pEnemy;
	if (m_pPlayer)				delete m_pPlayer;
	if (m_pRootNode)			delete m_pRootNode;
	if (m_pParticles)			delete m_pParticles;
	if (m_pDissolveModel)		delete m_pDissolveModel;
	if (m_pPushableModel)		delete m_pPushableModel;
	if (m_pWallModel)			delete m_pWallModel;
	if (m_pSphereModel)			delete m_pSphereModel;
	if (m_pPlaneModel)			delete m_pPlaneModel;
	if (m_pCubeModel)			delete m_pCubeModel;
	if (m_pSkyBox)				delete m_pSkyBox;
	if (m_pText)				delete m_pText;
	if (m_pCam)					delete m_pCam;
	if (m_pTextureDissolve)		m_pTextureDissolve->Release();
	if (m_pTextureSkyBox)		m_pTextureSkyBox->Release();
	if (m_pTextureBrick)		m_pTextureBrick->Release();
	if (m_pTextureFloor)		m_pTextureFloor->Release();
	if (m_pTexture0)			m_pTexture0->Release();
	if (m_pSampler0)			m_pSampler0->Release();
	if (m_pZBuffer)				m_pZBuffer->Release();
	if (m_pBackBufferRTView)	m_pBackBufferRTView->Release();
	if (m_pSwapChain)			m_pSwapChain->Release();
	if (m_pImmediateContext)	m_pImmediateContext->Release();
	if (m_pD3DDevice)			m_pD3DDevice->Release();
	if (m_pInput)				delete m_pInput;
	if (m_pTimer)				delete m_pTimer;
	if (m_pLights)				delete m_pLights;
}

HRESULT GameManager::InitialiseGraphics()
{
	HRESULT hr = S_OK;

	D3D11_SAMPLER_DESC sampler_desc;
	ZeroMemory(&sampler_desc, sizeof(sampler_desc));
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = m_pD3DDevice->CreateSamplerState(&sampler_desc, &m_pSampler0);

	if (FAILED(hr))
	{
		return hr;
	}

#pragma region Texture Setup
	hr = D3DX11CreateShaderResourceViewFromFile(m_pD3DDevice,
		"assets/marble1.png", NULL, NULL,
		&m_pTexture0, NULL);
	if (FAILED(hr))
	{
		return hr;
	}
	
	hr = D3DX11CreateShaderResourceViewFromFile(m_pD3DDevice,
		"assets/brick.png", NULL, NULL,
		&m_pTextureBrick, NULL);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = D3DX11CreateShaderResourceViewFromFile(m_pD3DDevice,
		"assets/skybox_mountain.dds", NULL, NULL,
		&m_pTextureSkyBox, NULL);

	if (FAILED(hr))
	{
		return hr;
	}
	hr = D3DX11CreateShaderResourceViewFromFile(m_pD3DDevice,
		"assets/floor.jpg", NULL, NULL,
		&m_pTextureFloor, NULL);

	if (FAILED(hr))
	{
		return hr;
	}

	hr = D3DX11CreateShaderResourceViewFromFile(m_pD3DDevice,
		"assets/dissolve.png", NULL, NULL,
		&m_pTextureDissolve, NULL);

	if (FAILED(hr))
	{
		return hr;
	}
#pragma endregion

	m_pCam = new Camera(0.0, 0.0, -0.5, 0.0, 1.0f, 45.0f);
	m_pCam->ChangeCameraType(CameraType::FirstPerson);

	m_pText = new Text2D("assets/font1.bmp", m_pD3DDevice, m_pImmediateContext);

	m_pSkyBox = new SkyBox(m_pD3DDevice, m_pImmediateContext);
	m_pSkyBox->CreateSkybox(m_pTextureSkyBox);


#pragma region Cube Model Setup
	m_pCubeModel = new Model(m_pD3DDevice, m_pImmediateContext, m_pLights);
	hr = m_pCubeModel->LoadObjModel("assets/cube.obj");
	if (FAILED(hr))
	{
		return hr;
	}

	hr = m_pCubeModel->LoadDefaultShaders();
	if (FAILED(hr))
	{
		return hr;
	}

	m_pCubeModel->SetSampler(m_pSampler0);
	m_pCubeModel->SetTexture(m_pTextureFloor);
	m_pCubeModel->SetCollisionType(CollisionType::Box);
#pragma endregion

#pragma region Plane Model Setup
	m_pPlaneModel = new Model(m_pD3DDevice, m_pImmediateContext, m_pLights);
	hr = m_pPlaneModel->LoadObjModel("assets/plane.obj");
	if (FAILED(hr))
	{
		return hr;
	}

	hr = m_pPlaneModel->LoadDefaultShaders();
	if (FAILED(hr))
	{
		return hr;
	}

	m_pPlaneModel->SetSampler(m_pSampler0);
	m_pPlaneModel->SetTexture(m_pTextureFloor);
	m_pPlaneModel->SetCollisionType(CollisionType::Box);
#pragma endregion

#pragma region Sphere Model Setup
	m_pSphereModel = new Model(m_pD3DDevice, m_pImmediateContext, m_pLights);
	hr = m_pSphereModel->LoadObjModel("assets/sphere.obj");
	if (FAILED(hr))
	{
		return hr;
	}

	hr = m_pSphereModel->LoadDefaultShaders();
	if (FAILED(hr))
	{
		return hr;
	}

	m_pSphereModel->SetSampler(m_pSampler0);
	m_pSphereModel->SetTexture(m_pTexture0);
	m_pSphereModel->SetCollisionType(CollisionType::Sphere);
#pragma endregion
	
#pragma region Wall Model Setup
	m_pWallModel = new Model(m_pD3DDevice, m_pImmediateContext, m_pLights);
	hr = m_pWallModel->LoadObjModel("assets/wall.obj");
	if (FAILED(hr))
	{
		return hr;
	}
	
	hr = m_pWallModel->LoadDefaultShaders();
	if (FAILED(hr))
	{
		return hr;
	}

	m_pWallModel->SetSampler(m_pSampler0);
	m_pWallModel->SetTexture(m_pTextureBrick);
	m_pWallModel->SetCollisionType(CollisionType::Box);
#pragma endregion

#pragma region Pushable Model Setup
	m_pPushableModel = new Model(m_pD3DDevice, m_pImmediateContext, m_pLights);
	hr = m_pPushableModel->LoadObjModel("assets/pushable.obj");
	if (FAILED(hr))
	{
		return hr;
	}
	hr = m_pPushableModel->LoadDefaultShaders();
	if (FAILED(hr))
	{
		return hr;
	}

	m_pPushableModel->SetSampler(m_pSampler0);
	m_pPushableModel->SetTexture(m_pTexture0);
	m_pPushableModel->SetCollisionType(CollisionType::Box);

#pragma endregion

#pragma region Dissolve Model Setup
	m_pDissolveModel = new Model(m_pD3DDevice, m_pImmediateContext, m_pLights);
	hr = m_pDissolveModel->LoadObjModel("assets/sphere.obj");
	if (FAILED(hr))
	{
		return hr;
	}

	hr = m_pDissolveModel->LoadCustomShader("dissolve_shader.hlsl", "DissolveVS", "DissolvePS");
	if (FAILED(hr))
	{
		return hr;
	}

	m_pDissolveModel->SetSampler(m_pSampler0);
	m_pDissolveModel->SetTexture(m_pTexture0);
	m_pDissolveModel->SetDissolveTexture(m_pTextureDissolve);
	m_pDissolveModel->SetDissolveAmount(1.0f);
	m_pDissolveModel->SetDissolveColor(0.0f, 0.0f, 1.0f, 1.0f);
	m_pDissolveModel->SetCollisionType(CollisionType::Box);
#pragma endregion


	m_pParticles = new ParticleFactory(m_pD3DDevice, m_pImmediateContext, m_pLights);
	m_pParticles->CreateParticle();
	m_pParticles->SetActive(true);

	return hr;
}

void GameManager::LoadLevel (char* textFile)
{
	m_pRootNode = new Scene_Node("Root");
	m_pWallRoot = new Scene_Node("Walls_Root");
	m_pFloor = new Scene_Node("Floor");
	m_pFloor->SetModel(m_pPlaneModel);
	m_pFloor->SetYPos(0.0f);

	if (textFile == NULL)
	{
		cout << "Invalided file" << endl;
	}
	//Open the file;
	std::ifstream file(textFile);
	if (file.is_open())
	{
		std::string s;
		//cycle through file to get level
		while (!file.eof())
		{
			getline(file, s);
			m_pLevel.push_back(s);
		}

	}

	for (int i = 0; i < m_pLevel.size(); i++)
	{
		std::string s = m_pLevel[i];
		int sLength = s.length();
		for (int j = 0; j < sLength ; j++)
		{
			switch (s.at(j))
			{
				//Walls
				case 'W':
				{
					Scene_Node* wall = new Scene_Node("Wall");
					wall->SetModel(m_pWallModel);
					wall->SetScale(0.2f);
					wall->SetXPos(j*6);
					wall->SetYPos(0.0f);
					wall->SetZPos(i*6);
					m_pWallRoot->AddChildNode(wall);
				}
				break;
				//Player
				case 'P':
				{
					m_pPlayerNode = new Scene_Node("Player");
					m_pPlayerNode->SetModel(m_pCubeModel);
					m_pPlayer = new Player(m_pPlayerNode, m_pCam, 10.0f);

					m_pPlayer->SetPosition((j * 6.0f), 1.0f, (i * 6.0f));
					m_pPlayer->SetGravity(0.0f, -9.81f, 0.0f);
					//UpdatePlayerNode();

					m_pCameraNode = new Scene_Node("Camera");
					m_pCam->SetPosition(j*6, 0, i*6);

					m_pCam->SetTarget(m_pPlayerNode);
					m_pCam->ChangeCameraType(ThirdPerson);
					m_pCam->SetFollowDistance(10.0f);
					UpdateCameraNode();



					XMMATRIX identity = XMMatrixIdentity();

					m_pRootNode->UpdateCollisionTree(&identity, 1.0f);
				}
				break;
				//Enemy
				case 'E':
				{
					m_pEnemyNode = new Scene_Node("Enemy");
					m_pEnemyNode->SetModel(m_pSphereModel);
					m_pEnemyNode->SetScale(0.5f);
					m_pEnemy = new Enemy(m_pEnemyNode, 5.0f);
					m_pEnemy->SetPosition((j * 6.0f), 1.5f, (i * 6.0f));
					m_pEnemy->AddWaypoint((j * 6.0f), 1.5f, (i * 6.0f));
					
				}
				break;
				//Enemy Waypoint
				case '*':
				{
					m_pEnemy->AddWaypoint((j * 6.0f), 1.5f, (i * 6.0f));
				}
				break;
				//Movable Object
				case 'M':
				{
					m_pMovableNode = new Scene_Node("Movable");
					m_pMovableNode->SetModel(m_pPushableModel);
					m_pMovableNode->SetScale(0.1f);
					m_pMovableNode->SetTrigger(true);

					m_pMovable = new Movable(m_pMovableNode);
					m_pMovable->SetPosition((j * 6.0f), 0.1f, (i * 6.0f));
				}
				break;
				//Fountain Particles
				case 'F':
				{
					m_pParticles = new ParticleFactory(m_pD3DDevice, m_pImmediateContext, m_pLights);
					m_pParticles->CreateParticle();
					m_pParticles->SwitchParticleType(ParticleType::Explosion);
					m_pParticles->SetActive(true);
					m_pParticles->SetXPos(j * 6.0f);
					m_pParticles->SetYPos(1.0f);
					m_pParticles->SetZPos(i * 6.0f);
				}
				break;
				case 'D':
				{
					m_pDissolveNode = new Scene_Node("Dissolve");
					m_pDissolveNode->SetModel(m_pDissolveModel);
					m_pDissolveNode->SetScale(0.25f);
					m_pDissolveNode->SetXPos(j * 6.0f);
					m_pDissolveNode->SetYPos(1.0f);
					m_pDissolveNode->SetZPos(i * 6.0f);
				}
				break;
			}
		}
	}
	m_pFloor->SetXPos(m_pLevel.size() * 0.5f);
	m_pFloor->SetZPos(m_pLevel.size() * 0.5f);
	m_pRootNode->AddChildNode(m_pFloor);
	m_pRootNode->AddChildNode(m_pWallRoot);
	m_pRootNode->AddChildNode(m_pCameraNode);
	m_pRootNode->AddChildNode(m_pPlayerNode);
	m_pRootNode->AddChildNode(m_pEnemyNode);
	m_pRootNode->AddChildNode(m_pMovableNode);
	m_pRootNode->AddChildNode(m_pDissolveNode);
}


//Returns the frame time
void GameManager::Update()
{
	float deltaTime = m_pTimer->GetDeltaTime();
	m_pPlayer->Update();
	m_pCam->Update();
	UpdateCameraNode();
	m_pEnemy->Update(m_pRootNode, deltaTime);
	m_pMovable->Update(m_pRootNode, deltaTime);
	m_pDissolveNode->GetModel()->SetDissolveAmount(m_pDissolveNode->GetModel()->GetDissolveAmount() - deltaTime);
	
}

void GameManager::Render()
{
	string s = "Fps ";
	s += std::to_string(m_pTimer->GetFPS());
	m_pText->AddText(s, -1, 1, .05);
	float clearCol[4] = { 0.0f,0.1f,0.1f,1.0f };
	m_pImmediateContext->ClearRenderTargetView(m_pBackBufferRTView, clearCol);
	m_pImmediateContext->ClearDepthStencilView(m_pZBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_pImmediateContext->OMSetBlendState(0, 0, 0xffffffff);
	if (m_enableAlpha)
	{
		float blendFactor[] = { 0.75f, 0.75f,0.75f,1.0f };
		m_pImmediateContext->OMSetBlendState(m_pTransparencyBlend, blendFactor, 0xffffffff);
	}
	XMMATRIX world, view, projection;

	world = XMMatrixIdentity();
	view = m_pCam->GetViewMatrix();
	projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0), m_pScreenWidth / m_pScreenHeight, 1.0, 250.0);
	m_pSkyBox->RenderSkyBox(&view, &projection, m_pCam);


	m_pRootNode->Execute(&world, &view, &projection);


	m_pParticles->Draw(&view, &projection, &XMFLOAT3(m_pCam->GetX(), m_pCam->GetY(), m_pCam->GetZ()));

	//RenderText last
	m_pText->RenderText();
	m_pSwapChain->Present(0, 0);
}

void GameManager::CheckInputs()
{
	float deltaTime = m_pTimer->GetDeltaTime();
	m_pInput->ReadInputStates();


	if (m_pInput->IsKeyPressed(DIK_ESCAPE))
	{
		DestroyWindow((*m_phWnd));
	}
	
	//Third Person Movement
	if (m_pCam->GetCameraType() == ThirdPerson || m_pCam->GetCameraType() == TopDown)
	{
		if (m_pInput->IsKeyPressed(DIK_W))
		{
			m_pPlayer->MoveForward(deltaTime);

			XMMATRIX identity = XMMatrixIdentity();

			m_pRootNode->UpdateCollisionTree(&identity, 1.0f);

			if (m_pPlayerNode->CheckCollision(m_pRootNode))
			{
				m_pPlayer->MoveForward(-deltaTime);
			}
		}

		if (m_pInput->IsKeyPressed(DIK_S))
		{
			m_pPlayer->MoveForward(-deltaTime);
			XMMATRIX identity = XMMatrixIdentity();

			m_pRootNode->UpdateCollisionTree(&identity, 1.0f);

			if (m_pPlayerNode->CheckCollision(m_pRootNode))
			{
				m_pPlayer->MoveForward(deltaTime);
			}
		}

		if (m_pInput->IsKeyPressed(DIK_A))
		{
			m_pPlayer->MoveRight(deltaTime);
			XMMATRIX identity = XMMatrixIdentity();

			m_pRootNode->UpdateCollisionTree(&identity, 1.0f);

			if (m_pPlayerNode->CheckCollision(m_pRootNode))
			{
				m_pPlayer->MoveRight(-deltaTime);
			}
		}

		if (m_pInput->IsKeyPressed(DIK_D))
		{
			m_pPlayer->MoveRight(-deltaTime);
			XMMATRIX identity = XMMatrixIdentity();

			m_pRootNode->UpdateCollisionTree(&identity, 1.0f);

			if (m_pPlayerNode->CheckCollision(m_pRootNode))
			{
				m_pPlayer->MoveRight(deltaTime);
			}
		}
	}
	else
	{
		//First person camera movement
		if (m_pInput->IsKeyPressed(DIK_W))
		{
			m_pCam->Forward(deltaTime);
			UpdateCameraNode();
		}
		if (m_pInput->IsKeyPressed(DIK_S))
		{
			m_pCam->Forward(-deltaTime);

		}
		if (m_pInput->IsKeyPressed(DIK_A))
		{
			m_pCam->Strafe(deltaTime);
		}
		if (m_pInput->IsKeyPressed(DIK_D))
		{
			m_pCam->Strafe(-deltaTime);
		}
	}
	if (m_pInput->IsKeyPressed(DIK_SPACE))
	{
		m_pPlayer->Jump(deltaTime * 30.0f);
	}

	if (m_pInput->IsKeyPressed(DIK_8))
	{
		m_pCam->ChangeCameraType(FirstPerson);
		UpdateCameraNode();
	}
	if (m_pInput->IsKeyPressed(DIK_9))
	{
		m_pCam->ChangeCameraType(FreeLook);
		UpdateCameraNode();
	}
	if (m_pInput->IsKeyPressed(DIK_0))
	{
		m_pCam->ChangeCameraType(ThirdPerson);
		UpdateCameraNode();
	}
	if (m_pInput->IsKeyPressed(DIK_MINUS))
	{
		m_pCam->ChangeCameraType(TopDown);
		UpdateCameraNode();
	}

	if (m_pInput->GetMouseButtonDown(1))
	{
		m_enableAlpha = !m_enableAlpha;
	}

	if (m_pInput->GetMouseScroll() > 0)
	{
		m_pCam->SetFollowDistance(m_pCam->GetFollowDistance() + .50f);
	}
	if (m_pInput->GetMouseScroll() < 0)
	{
		m_pCam->SetFollowDistance(m_pCam->GetFollowDistance() - .5f);
	}
	 
	m_pCam->RotateCamera(m_pInput->GetMouseX() * 0.1f, m_pInput->GetMouseY() * 0.1f);
}

void GameManager::UpdateCameraNode()
{
	m_pCameraNode->SetXPos(m_pCam->GetX());
	m_pCameraNode->SetYPos(m_pCam->GetY());
	m_pCameraNode->SetZPos(m_pCam->GetZ());
}


/* This doesnt seem to work at the momenet, have resulted in doing it in main.cpp
HRESULT GameManager::ResizeWindow(LPARAM* lParam)
{
	HRESULT hr;

	m_pImmediateContext->OMSetRenderTargets(0, 0, 0);

	//Release all oustanding references to the swap chains buffers
	m_pBackBufferRTView->Release();
	m_pZBuffer->Release();

	

	//Preserve the existing buffer count and format.
	//Automatically chose the width and height to match the client rect
	hr = m_pSwapChain->ResizeBuffers(0, LOWORD(lParam), HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);

	//Get buffer and new render-target-view
	ID3D11Texture2D* pBuffer;
	
	hr = m_pSwapChain->GetBuffer(0, _uuidof(ID3D11Texture2D), (void**)&pBuffer);

	hr = m_pD3DDevice->CreateRenderTargetView(pBuffer, NULL, &m_pBackBufferRTView);

	pBuffer->Release();

	//Create Z buffer texture;
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
	hr = m_pD3DDevice->CreateTexture2D(&tex2dDesc, NULL, &pZBufferTexture);
	if (FAILED(hr)) return hr;

	// create the z buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = tex2dDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	m_pD3DDevice->CreateDepthStencilView(pZBufferTexture, &dsvDesc, &m_pZBuffer);
	pZBufferTexture->Release();


	m_pImmediateContext->OMSetRenderTargets(1, &m_pBackBufferRTView, m_pZBuffer);


	//Set up viewport
	D3D11_VIEWPORT vp;
	vp.Width = LOWORD(lParam);
	vp.Height = HIWORD(lParam);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	m_pImmediateContext->RSSetViewports(1, &vp);

}
*/
HRESULT GameManager::SetupDirectX()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect((*m_phWnd), &rc);

	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;

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
	sd.OutputWindow = (*m_phWnd);
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL,
			createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &m_pSwapChain,
			&m_pD3DDevice, &g_featureLevel, &m_pImmediateContext);
		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return hr;

	//Get pointer to back buffer texture
	ID3D11Texture2D *pBackBufferTexture;
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBufferTexture);

	if (FAILED(hr))
		return hr;

	//Use the back buffer texture pointer to create the render target
	hr = m_pD3DDevice->CreateRenderTargetView(pBackBufferTexture, NULL, &m_pBackBufferRTView);

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
	hr = m_pD3DDevice->CreateTexture2D(&tex2dDesc, NULL, &pZBufferTexture);
	if (FAILED(hr))
		return hr;

	//Create the Z Buffer;
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));

	dsvDesc.Format = tex2dDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	m_pD3DDevice->CreateDepthStencilView(pZBufferTexture, &dsvDesc, &m_pZBuffer);
	pZBufferTexture->Release();


	//Set the render target view
	m_pImmediateContext->OMSetRenderTargets(1, &m_pBackBufferRTView, m_pZBuffer);

	//Set the viewport
	D3D11_VIEWPORT viewport;

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (FLOAT)width;
	viewport.Height = (FLOAT)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;


	m_pImmediateContext->RSSetViewports(1, &viewport);


	//Define beldning
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));

	D3D11_RENDER_TARGET_BLEND_DESC renderBlend;

	renderBlend.BlendEnable				= true;
	renderBlend.SrcBlend				= D3D11_BLEND_SRC_COLOR;
	renderBlend.DestBlend				= D3D11_BLEND_BLEND_FACTOR;
	renderBlend.BlendOp					= D3D11_BLEND_OP_ADD;
	renderBlend.SrcBlendAlpha			= D3D11_BLEND_ONE;
	renderBlend.DestBlendAlpha			= D3D11_BLEND_ZERO;
	renderBlend.BlendOpAlpha			= D3D11_BLEND_OP_ADD;
	renderBlend.RenderTargetWriteMask	= D3D10_COLOR_WRITE_ENABLE_ALL;

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0] = renderBlend;

	m_pD3DDevice->CreateBlendState(&blendDesc, &m_pTransparencyBlend);


	return S_OK;

}