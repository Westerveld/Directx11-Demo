#pragma once
#include "main.h"


class GameManager
{
private:
	ID3D11Device*					m_pD3DDevice = NULL;
	ID3D11DeviceContext*			m_pImmediateContext = NULL;
	IDXGISwapChain*					m_pSwapChain = NULL;
	ID3D11RenderTargetView*			m_pBackBufferRTView = NULL;
	ID3D11DepthStencilView*			m_pZBuffer;

	ID3D11ShaderResourceView*		m_pTexture0;
	ID3D11ShaderResourceView*		m_pTextureBrick;
	ID3D11ShaderResourceView*		m_pTextureSkyBox;
	ID3D11ShaderResourceView*		m_pTextureFloor;
	ID3D11ShaderResourceView*		m_pTextureDissolve;

	ID3D11SamplerState*				m_pSampler0;
	ID3D11BlendState*				m_pTransparencyBlend;

	LightManager*					m_pLights;
	Camera*							m_pCam;
	Camera*							m_pUICam;

	SkyBox*							m_pSkyBox;
	Text2D*							m_pText;
	ParticleFactory*				m_pParticles;
	InputHandler*					m_pInput;
	class Player*					m_pPlayer;
	class Enemy*					m_pEnemy;
	class Movable*					m_pMovable;
	class MiniMap*					m_pMinimap;
	HINSTANCE*						m_phInst;
	HWND*							m_phWnd;

#pragma region Models
	Model*							m_pSphereModel;
	Model*							m_pPlaneModel;
	Model*							m_pCubeModel;
	Model*							m_pWallModel;
	Model*							m_pPushableModel;
	Model*							m_pDissolveModel;
	Model*							m_pReflectModel;
	Model*							m_pKnightModel;
#pragma endregion

#pragma region Level Objects
	Scene_Node*						m_pRootNode;
	Scene_Node*						m_pFloor;
	Scene_Node*						m_pWallRoot;
	Scene_Node*						m_pSphere;
	Scene_Node*						m_pPlayerNode;
	Scene_Node*						m_pCameraNode;
	Scene_Node*						m_pEnemyNode;
	Scene_Node*						m_pMovableNode;
	Scene_Node*						m_pDissolveNode;
	Scene_Node*						m_pParticleNode;
	Scene_Node*						m_pReflectionNode;
	Scene_Node*						m_pKnightNode;
#pragma endregion
	float							m_pScreenHeight, m_pScreenWidth;

	void							UpdateCameraNode(void);
	void							UpdateText(void);
	
	std::vector<std::string>		m_pLevel;

	bool							m_enableAlpha;
	float							m_placementMutliplier;
public:
	class TimeHandler				*m_pTimer;

	GameManager(float height, float width, HWND* hWnd, HINSTANCE* hInst);
	~GameManager();
	void LoadLevel(char* textFile);
	void Update(void);
	void Render(void);
	HRESULT InitialiseGraphics(void);
	void	CheckInputs(void);

#pragma region Getters and Setters
	void					SetScreenHeight(float height) { m_pScreenHeight = height; }
	void					SetScreenWidth(float width) { m_pScreenWidth = width; }

	IDXGISwapChain*			GetSwapChain(void) { return m_pSwapChain; }
	void					SetSwapChain(IDXGISwapChain* swapChain) { m_pSwapChain = swapChain; }

	ID3D11DeviceContext*	GetImmediateContext(void) { return m_pImmediateContext; }
	void					SetImmediateContext(ID3D11DeviceContext* context) { m_pImmediateContext = context; }

	ID3D11Device*			GetDevice(void) { return m_pD3DDevice; }
	void					SetDevice(ID3D11Device* device) { m_pD3DDevice = device; }

	ID3D11RenderTargetView*	GetRenderTarget(void) { return m_pBackBufferRTView; }
	void					SetRenderTarget(ID3D11RenderTargetView* target) { m_pBackBufferRTView = target; }
	void					ReleaseRenderTarget(void) { if (m_pBackBufferRTView) m_pBackBufferRTView->Release(); }

	ID3D11DepthStencilView*	GetZBuffer(void) { return m_pZBuffer; }
	void					SetZBuffer(ID3D11DepthStencilView* zBuffer) { m_pZBuffer = zBuffer; }
	void					ReleaseZBuffer(void) { if(m_pZBuffer) m_pZBuffer->Release(); }
	
#pragma endregion

	HRESULT					SetUpDirectX(void);
};