#include "GameManager.h"



GameManager::GameManager(float height, float width, HWND* hWnd, HINSTANCE* hInst)
{
	m_pScreenHeight = height;
	m_pScreenWidth = width;
	m_pLights = new LightManager();
	m_pTimer = new TimeHandler();
	m_phWnd = hWnd;
	m_phInst = hInst;
	m_pInput = new InputHandler(hWnd, hInst);
	m_pInput->InitialiseKeyboardInput();
}


GameManager::~GameManager()
{
	if (m_pParticles)			delete m_pParticles;
	if (m_pWallModel)			delete m_pWallModel;
	if (m_pSphereModel)			delete m_pSphereModel;
	if (m_pPlaneModel)			delete m_pPlaneModel;
	if (m_pCubeModel)			delete m_pCubeModel;
	if (m_pSkyBox)				delete m_pSkyBox;
	if (m_pText)				delete m_pText;
	if (m_pCam)					delete m_pCam;
	if (m_pTexture0)			m_pTexture0->Release();
	if (m_pTextureSkyBox)		m_pTextureSkyBox->Release();
	if (m_pTextureBrick)		m_pTextureBrick->Release();
	if (m_pTextureFloor)		m_pTextureFloor->Release();
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
#pragma endregion

	m_pCam = new Camera(0.0, 0.0, -0.5, 0.0);
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

	m_pParticles = new ParticleFactory(m_pD3DDevice, m_pImmediateContext, m_pLights);
	m_pParticles->CreateParticle();
	m_pParticles->SetActive(true);

	return hr;
}

void GameManager::LoadLevel (char* textFile)
{
	m_pRootNode = new Scene_Node("Root");
	m_pWall = new Scene_Node("Walls_Root");
	m_pFloor = new Scene_Node("Floor");


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
					wall->SetXPos(j);
					wall->SetZPos(i);
					m_pWall->AddChildNode(wall);
					m_pWalls.push_back(wall);
				}
				break;
				//Camera
				case 'C':
				{
					m_pCameraNode = new Scene_Node("Camera");
					m_pCameraNode->SetModel(m_pCubeModel);
					m_pCameraNode->SetScale(0.3f);
					m_pCam->SetPosition(j, 0, i);
					UpdateCameraNode();
					m_pRootNode->AddChildNode(m_pCameraNode);
				}
				break;
			}
		}
	}
	m_pRootNode->AddChildNode(m_pFloor);
	m_pRootNode->AddChildNode(m_pWall);
	m_pRootNode->AddChildNode(m_pCameraNode);
}


//Returns the frame time
void GameManager::Update()
{
	//m_pTimer->UpdateTimer();
	m_pCam->Update();
	//double deltaTime = m_pTimer->GetFrameTime();
}

void GameManager::Render()
{
	float clearCol[4] = { 0.0f,0.1f,0.1f,1.0f };
	m_pImmediateContext->ClearRenderTargetView(m_pBackBufferRTView, clearCol);
	m_pImmediateContext->ClearDepthStencilView(m_pZBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	XMMATRIX world, view, projection;

	world = XMMatrixIdentity();
	view = m_pCam->GetViewMatrix();
	projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0), m_pScreenWidth / m_pScreenHeight, 1.0, 250.0);
	m_pSkyBox->RenderSkyBox(&view, &projection, m_pCam);


	m_pRootNode->Execute(&world, &view, &projection);

	m_pSwapChain->Present(0, 0);
}

void GameManager::CheckInputs()
{
	m_pInput->ReadInputStates();

	if (m_pInput->IsKeyPressed(DIK_ESCAPE))
	{
		DestroyWindow((*m_phWnd));
	}

	if (m_pInput->IsKeyPressed(DIK_W))
	{
		m_pCam->Forward(0.01f);

		UpdateCameraNode();
		XMMATRIX identity = XMMatrixIdentity();

		m_pRootNode->UpdateCollisionTree(&identity, 1.0f);

		if (m_pCameraNode->CheckCollision(m_pRootNode))
		{
			m_pCam->Forward(-0.01f);
			UpdateCameraNode();
		}
	}

	if (m_pInput->IsKeyPressed(DIK_S))
	{
		m_pCam->Forward(-0.01f);
		UpdateCameraNode();
		XMMATRIX identity = XMMatrixIdentity();

		m_pRootNode->UpdateCollisionTree(&identity, 1.0f);

		if (m_pCameraNode->CheckCollision(m_pRootNode))
		{
			m_pCam->Forward(0.01f);
			UpdateCameraNode();
		}
	}
	
	if (m_pInput->IsKeyPressed(DIK_A))
	{
		m_pCam->Strafe(-0.01f);
		UpdateCameraNode();
		XMMATRIX identity = XMMatrixIdentity();

		m_pRootNode->UpdateCollisionTree(&identity, 1.0f);

		if (m_pCameraNode->CheckCollision(m_pRootNode))
		{
			m_pCam->Strafe(0.01f);
			UpdateCameraNode();
		}
	}
	
	if (m_pInput->IsKeyPressed(DIK_D))
	{
		m_pCam->Strafe(0.01f);
		UpdateCameraNode();
		XMMATRIX identity = XMMatrixIdentity();

		m_pRootNode->UpdateCollisionTree(&identity, 1.0f);

		if (m_pCameraNode->CheckCollision(m_pRootNode))
		{
			m_pCam->Strafe(-0.01f);
			UpdateCameraNode();
		}
	}

	 
	m_pCam->RotateCamera(m_pInput->GetMouseX() * 0.1f, m_pInput->GetMouseY() * 0.1f);
}

void GameManager::UpdateCameraNode()
{
	m_pCameraNode->SetXPos(m_pCam->GetX());
	m_pCameraNode->SetYPos(m_pCam->GetY());
	m_pCameraNode->SetZPos(m_pCam->GetZ());
}