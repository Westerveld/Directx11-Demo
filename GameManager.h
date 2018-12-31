#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include <d3dx11.h>
#include <windows.h>
#include <dxerr.h>
#include <xnamath.h>

#include "Scene_Node.h"
#include "camera.h"
#include "maths.h"
#include "Model.h"
#include "SkyBox.h"
#include "text2D.h"
#include "Timer.h"
#include "LightManager.h"
#include "ParticleFactory.h"

#include <iostream>
#include <deque>

class GameManager
{
private:
	ID3D11Device*					g_pD3DDevice;
	ID3D11DeviceContext*			g_pImmediateContext;
	IDXGISwapChain*					g_pSwapChain;
	ID3D11RenderTargetView*			g_pBackBufferRTView;

	ID3D11ShaderResourceView*		g_pTexture0;
	ID3D11ShaderResourceView*		g_pTextureBrick;
	ID3D11ShaderResourceView*		g_pTexureSkyBox;
	ID3D11SamplerState*				g_pSampler0;
	
	
	LightManager*					g_pLights;
	Camera*							g_pCam;

	SkyBox*							g_pSkyBox;
	Timer*							g_pTimer;
	Text2D*							g_pText;

#pragma region Models
	Model*							g_pSphere;
	Model*							g_pPlane;
	Model*							g_pCube;
	Model*							g_pWall;
#pragma endregion

#pragma region Level Objects
	Scene_Node*						g_pRootNode;
	Scene_Node*						g_pFloor;
#pragma endregion

	float							g_pScreenHeight, g_pScreenWidth;
public:
	GameManager(float height, float width);
	~GameManager();
	void LoadLevel(char* textFile);
	void Update();
	void Render();
	HRESULT InitialiseGraphics(ID3D11Device* device, ID3D11DeviceContext* context, IDXGISwapChain* swapChain, ID3D11RenderTargetView* g_pBackBufferRTView);

#pragma region Getters and Setters
	void SetScreenHeight(float height) { g_pScreenHeight = height; }
	void SetScreenWidth(float width) { g_pScreenWidth = width; }
#pragma endregion
};

