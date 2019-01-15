#include "GameManager.h"
#include "Player.h"
#include "Enemy.h"
#include "Movable.h"
#include "MiniMap.h"

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
	m_placementMutliplier = 6.0f;
}

//Game Clean Up
GameManager::~GameManager()
{
	if (m_pRootNode)			delete m_pRootNode;
	if (m_pEnemy)				delete m_pEnemy;
	if (m_pPlayer)				delete m_pPlayer;
	if (m_pParticles)			delete m_pParticles;
	if (m_pReflectModel)		delete m_pReflectModel;
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

	//Create our sampler
	D3D11_SAMPLER_DESC sampler_desc;
	ZeroMemory(&sampler_desc, sizeof(sampler_desc));
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = m_pD3DDevice->CreateSamplerState(&sampler_desc, &m_pSampler0);

	if (FAILED(hr))
	{
		return hr;
	}

	//Set up all the textures for the models
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

	//Create the cameras for the game
	m_pCam = new Camera(0.0, 0.0, -0.5, 0.0, 1.0f, 45.0f);
	m_pCam->ChangeCameraType(CameraType::FirstPerson);
	m_pUICam = new Camera(0.0, 0.0, -0.5, 0.0, 1.0f, 45.0f);
	m_pUICam->ChangeCameraType(CameraType::TopDown);

	//Create the text object
	m_pText = new Text2D("assets/font2.bmp", m_pD3DDevice, m_pImmediateContext);

	//Create the skybox
	m_pSkyBox = new SkyBox(m_pD3DDevice, m_pImmediateContext);
	m_pSkyBox->CreateSkybox(m_pTextureSkyBox);

	//Set up all the models for the game
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
	m_pDissolveModel->SetCollisionType(CollisionType::Sphere);
	m_pDissolveModel->ChangeModelType(ModelType::Dissolve);
#pragma endregion

#pragma region Reflection Model Setup
	m_pReflectModel = new Model(m_pD3DDevice, m_pImmediateContext, m_pLights);
	hr = m_pReflectModel->LoadObjModel("assets/sphere.obj");
	if (FAILED(hr))
	{
		return hr;
	}

	hr = m_pReflectModel->LoadCustomShader("reflect_shader.hlsl", "ModelVS", "ModelPS");
	if (FAILED(hr))
	{
		return hr;
	}

	m_pReflectModel->SetSampler(m_pSampler0);
	m_pReflectModel->SetTexture(m_pTextureBrick);
	m_pReflectModel->SetSkyboxTexture(m_pTextureSkyBox);
	m_pReflectModel->ChangeModelType(ModelType::Shiny);
	m_pReflectModel->SetCollisionType(CollisionType::Sphere);

#pragma endregion

#pragma region Knight Model Setup
	m_pKnightModel = new Model(m_pD3DDevice, m_pImmediateContext, m_pLights);
	hr = m_pKnightModel->LoadObjModel("assets/knight.obj");
	if (FAILED(hr))
	{
		return hr;
	}

	hr = m_pKnightModel->LoadDefaultShaders();
	if (FAILED(hr))
	{
		return hr;
	}

	m_pKnightModel->SetSampler(m_pSampler0);
	m_pKnightModel->SetTexture(m_pTextureBrick);
	m_pKnightModel->ChangeModelType(ModelType::Normal);
	m_pKnightModel->SetCollisionType(CollisionType::Box);

#pragma endregion

	//Particle set up
	m_pParticles = new ParticleFactory(m_pD3DDevice, m_pImmediateContext, m_pLights);
	m_pParticles->CreateParticle();
	m_pParticles->SetActive(true);

	//Mini map set up
	m_pMinimap = new MiniMap(m_pScreenHeight, m_pScreenHeight);
	hr = m_pMinimap->SetUpMiniMap(m_pD3DDevice, m_pImmediateContext);
	m_pMinimap->SetCamera(m_pUICam);
	return hr;
}

void GameManager::LoadLevel (char* textFile)
{
	//Create our root nodes
	m_pRootNode = new Scene_Node("Root");
	m_pWallRoot = new Scene_Node("Walls_Root");


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
	//Cycle through the level array creating objects that correspond to the correct letters
	for (size_t i = 0; i < m_pLevel.size(); i++)
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
					wall->SetXPos((float)j * m_placementMutliplier);
					wall->SetYPos(0.0f);
					wall->SetZPos((float)i * m_placementMutliplier);
					m_pWallRoot->AddChildNode(wall);
				}
				break;
				//Player
				case 'P':
				{
					m_pPlayerNode = new Scene_Node("Player");
					m_pPlayerNode->SetModel(m_pCubeModel);
					m_pPlayer = new Player(m_pPlayerNode, m_pCam, 10.0f, 1.0f);

					m_pPlayer->SetPosition(((float)j * m_placementMutliplier), 1.0f, ((float)i * m_placementMutliplier));
					m_pPlayer->SetGravity(0.0f, -9.81f, 0.0f);
					//UpdatePlayerNode();

					m_pCameraNode = new Scene_Node("Camera");
					m_pCam->SetPosition((float)j * m_placementMutliplier, 0, (float)i * m_placementMutliplier);

					m_pCam->SetTarget(m_pPlayerNode);
					m_pCam->ChangeCameraType(ThirdPerson);
					m_pCam->SetMinFollow(2.5f);
					m_pCam->SetMaxFollow(40.0f);
					m_pCam->SetFollowDistance(15.0f);
					
					UpdateCameraNode();

					m_pUICam->SetTarget(m_pPlayerNode);
					m_pUICam->SetMinFollow(5.0f);
					m_pUICam->SetMaxFollow(80.0f);
					m_pUICam->SetFollowDistance(20.0f);
				}
				break;
				//Enemy
				case 'E':
				{
					m_pEnemyNode = new Scene_Node("Enemy");
					m_pEnemyNode->SetModel(m_pSphereModel);
					m_pEnemyNode->SetScale(0.5f);
					m_pEnemy = new Enemy(m_pEnemyNode, 5.0f);
					m_pEnemy->SetPosition((j * m_placementMutliplier), 1.5f, (i * m_placementMutliplier));
					m_pEnemy->AddWaypoint((j * m_placementMutliplier), 1.5f, (i * m_placementMutliplier));
					
				}
				break;
				//Enemy Waypoint
				case '*':
				{
					m_pEnemy->AddWaypoint((j * m_placementMutliplier), 1.5f, (i * m_placementMutliplier));
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
					m_pMovable->SetPosition((j * m_placementMutliplier), 0.1f, (i * m_placementMutliplier));
				}
				break;
				//Fountain Particles
				case 'F':
				{
					m_pParticles = new ParticleFactory(m_pD3DDevice, m_pImmediateContext, m_pLights);
					m_pParticles->CreateParticle();
					m_pParticles->SwitchParticleType(ParticleType::Fountain);
					m_pParticles->SetActive(true);

					m_pParticleNode = new Scene_Node("Particle");
					m_pParticleNode->SetParticle(m_pParticles);
					m_pParticleNode->SetXPos((float)j);
					m_pParticleNode->SetYPos(3.0f);
					m_pParticleNode->SetZPos((float)i);
				}
				break;
				//Disoolve Model
				case 'D':
				{
					m_pDissolveNode = new Scene_Node("Dissolve");
					m_pDissolveNode->SetModel(m_pDissolveModel);
					m_pDissolveNode->SetXPos((float)j * m_placementMutliplier);
					m_pDissolveNode->SetYPos(3.0f);
					m_pDissolveNode->SetZPos((float)i * m_placementMutliplier);
					m_pDissolveNode->SetTrigger(true);
				}
				break;
				//Reflection Model
				case 'R':
				{
					m_pReflectionNode = new Scene_Node("Reflection");
					m_pReflectionNode->SetModel(m_pReflectModel);
					m_pReflectionNode->SetScale(0.5f);
					m_pReflectionNode->SetXPos((float)j * m_placementMutliplier);
					m_pReflectionNode->SetYPos(1.5f);
					m_pReflectionNode->SetZPos((float)i * m_placementMutliplier);
				}
				break;
				//Knight Model
				case 'K':
				{
					m_pKnightNode = new Scene_Node("Knight");
					m_pKnightNode->SetModel(m_pKnightModel);
					m_pKnightNode->SetScale(0.1f);
					m_pKnightNode->SetXPos((float)j * m_placementMutliplier);
					m_pKnightNode->SetYPos(0.0f);
					m_pKnightNode->SetZPos((float)i * m_placementMutliplier); 
					m_pLights->SetSpotLightPos(XMVectorSet((float)j * m_placementMutliplier, 5.0f, (float)i * m_placementMutliplier, 0.0f));
				}

			}
		}
	}

	//Create the floor node
	m_pFloor = new Scene_Node("Floor");
	m_pFloor->SetModel(m_pPlaneModel);
	m_pFloor->SetYPos(0.0f);
	m_pFloor->SetXPos(m_pLevel.size() * 0.5f);
	m_pFloor->SetZPos(m_pLevel.size() * 0.5f);
	
	m_pRootNode->AddChildNode(m_pFloor);
	m_pRootNode->AddChildNode(m_pWallRoot);
	m_pRootNode->AddChildNode(m_pCameraNode);
	m_pRootNode->AddChildNode(m_pPlayerNode);
	m_pRootNode->AddChildNode(m_pEnemyNode);
	m_pRootNode->AddChildNode(m_pMovableNode);
	m_pRootNode->AddChildNode(m_pDissolveNode);
	m_pRootNode->AddChildNode(m_pReflectionNode);
	m_pRootNode->AddChildNode(m_pKnightNode);
	m_pRootNode->AddChildNode(m_pParticleNode);
}

void GameManager::Update()
{
	//Get the delta time to use for our functions
	double deltaTime = m_pTimer->GetDeltaTime();
	m_pPlayer->Update((float)deltaTime);
	m_pCam->Update();
	UpdateCameraNode();
	m_pUICam->Update();
	m_pEnemy->Update(m_pRootNode, (float)deltaTime);
	m_pMovable->Update(m_pRootNode, (float)deltaTime);
}

void GameManager::Render()
{
	UpdateText();

	float clearCol[4] = { 0.0f,0.1f,0.1f,1.0f };
	m_pImmediateContext->ClearRenderTargetView(m_pBackBufferRTView, clearCol);
	m_pImmediateContext->ClearDepthStencilView(m_pZBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_pImmediateContext->OMSetRenderTargets(1, &m_pBackBufferRTView, m_pZBuffer);

	m_pImmediateContext->OMSetBlendState(0, 0, 0xffffffff);

	//Render the world from the cameras perspective
	XMMATRIX world, view, projection;

	world = XMMatrixIdentity();
	view = m_pCam->GetViewMatrix();
	projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0), m_pScreenWidth / m_pScreenHeight, 1.0, 250.0);
	m_pSkyBox->RenderSkyBox(&view, &projection, m_pCam);


	m_pRootNode->Execute(&world, &view, &projection, m_pCam);

	//Render to minimap
	m_pMinimap->AddToMap(m_pRootNode, m_pImmediateContext, m_pZBuffer);

	m_pImmediateContext->OMSetRenderTargets(1, &m_pBackBufferRTView, m_pZBuffer);


	//Render Text last
	m_pImmediateContext->OMSetBlendState(m_pTransparencyBlend, 0, 0xffffffff);
	m_pText->RenderText();
	m_pMinimap->RenderMap(m_pImmediateContext, m_pZBuffer);
	m_pImmediateContext->OMSetBlendState(0, 0, 0xffffffff);
	m_pSwapChain->Present(0, 0);
}

void GameManager::CheckInputs()
{
	//Get the deltat ime
	float deltaTime = (float)m_pTimer->GetDeltaTime();
	//Read the input from the input handler class
	m_pInput->ReadInputStates();

	if (m_pInput->IsKeyPressed(DIK_ESCAPE))
	{
		//Quit the game
		DestroyWindow((*m_phWnd));
	}
	
	//Third Person Movement
	if (m_pCam->GetCameraType() == ThirdPerson || m_pCam->GetCameraType() == TopDown)
	{
		//Move forwards
		if (m_pInput->IsKeyPressed(DIK_W))
		{
			m_pPlayer->MoveForward(deltaTime);

			//Update our object position
			XMMATRIX identity = XMMatrixIdentity();
			m_pRootNode->UpdateCollisionTree(&identity, 1.0f);

			//Check if we collided with anything
			if (m_pPlayerNode->CheckCollision(m_pRootNode))
			{
				m_pPlayer->MoveForward(-deltaTime);
			}
		}

		//Move backwards
		if (m_pInput->IsKeyPressed(DIK_S))
		{
			m_pPlayer->MoveForward(-deltaTime);

			//Update our object position
			XMMATRIX identity = XMMatrixIdentity();
			m_pRootNode->UpdateCollisionTree(&identity, 1.0f);

			//Check if we collided with anything
			if (m_pPlayerNode->CheckCollision(m_pRootNode))
			{
				m_pPlayer->MoveForward(deltaTime);
			}
		}

		if (m_pInput->IsKeyPressed(DIK_A))
		{
			m_pPlayer->MoveRight(deltaTime);

			//Update our object position
			XMMATRIX identity = XMMatrixIdentity();
			m_pRootNode->UpdateCollisionTree(&identity, 1.0f);

			//Check if we collided with anything
			if (m_pPlayerNode->CheckCollision(m_pRootNode))
			{
				m_pPlayer->MoveRight(-deltaTime);
			}
		}

		if (m_pInput->IsKeyPressed(DIK_D))
		{
			m_pPlayer->MoveRight(-deltaTime);

			//Update our object position
			XMMATRIX identity = XMMatrixIdentity();
			m_pRootNode->UpdateCollisionTree(&identity, 1.0f);

			//Check if we collided with anything
			if (m_pPlayerNode->CheckCollision(m_pRootNode))
			{
				m_pPlayer->MoveRight(deltaTime);
			}
		}
		if (m_pInput->IsKeyPressed(DIK_SPACE))
		{
			m_pPlayer->Jump();
		}

		//Zooming in and out of the target
		if (m_pInput->GetMouseScroll() > 0)
		{
			m_pCam->SetFollowDistance(m_pCam->GetFollowDistance() + .50f);
		}
		if (m_pInput->GetMouseScroll() < 0)
		{
			m_pCam->SetFollowDistance(m_pCam->GetFollowDistance() - .5f);
		}
	}
	else
	{
		float camSpeed = 10.0f * deltaTime;
		//First person camera movement
		if (m_pInput->IsKeyPressed(DIK_W))
		{
			m_pCam->Forward(camSpeed);
			UpdateCameraNode();
		}
		if (m_pInput->IsKeyPressed(DIK_S))
		{
			m_pCam->Forward(-camSpeed);
			UpdateCameraNode();
		}
		if (m_pInput->IsKeyPressed(DIK_A))
		{
			m_pCam->Strafe(-camSpeed);
			UpdateCameraNode();
		}
		if (m_pInput->IsKeyPressed(DIK_D))
		{
			m_pCam->Strafe(camSpeed);
			UpdateCameraNode();
		}
		if (m_pInput->IsKeyPressed(DIK_Q))
		{
			m_pCam->Up(camSpeed);
			UpdateCameraNode();
		}
		if (m_pInput->IsKeyPressed(DIK_E))
		{
			m_pCam->Up(-camSpeed);
			UpdateCameraNode();
		}
	}

	//Camera changer
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
	//Change particle type
	if (m_pInput->IsKeyPressed(DIK_P))
	{
		m_pParticles->SwitchParticleType(ParticleType::Explosion);
	}
	if (m_pInput->IsKeyPressed(DIK_O))
	{
		m_pParticles->SwitchParticleType(ParticleType::Fountain);
	}

	float mouseSensitivity = 0.1f; 
	m_pCam->RotateCamera(m_pInput->GetMouseX() * mouseSensitivity, m_pInput->GetMouseY() * mouseSensitivity);
}

void GameManager::UpdateCameraNode()
{
	m_pCameraNode->SetXPos(m_pCam->GetX());
	m_pCameraNode->SetYPos(m_pCam->GetY());
	m_pCameraNode->SetZPos(m_pCam->GetZ());
}

//Set up our direct x objects
HRESULT GameManager::SetUpDirectX()
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

	//Define blending
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

//Update the text to display
void GameManager::UpdateText()
{
	string fps = "Fps ";
	fps += std::to_string(m_pTimer->GetFPS());
	m_pText->AddText(fps, -1.0f, 1.0f, .07f);

	string info = "WASD to Move";
	m_pText->AddText(info, -1.0f, -0.9f, .07f);

	string camType;
	camType += m_pCam->GetCameraTypeString();
	m_pText->AddText(camType, 0.0f, -0.9f, 0.07f);
}