#pragma once
#include "objfilemodel.h"
#include "Camera.h"
#include "main.h"

struct MINIMAP_CONSTANT_BUFFER
{
	XMMATRIX WorldViewProjection; //64 bytes
};

class MiniMap
{
private:
	ID3D11Device*				m_pD3DDevice;
	ID3D11DeviceContext*		m_pImmediateContext;
	ID3D11DepthStencilView*		m_pZBuffer;
	ID3D11SamplerState*			m_pSampler0;

	ID3D11VertexShader*			m_pVShader;
	ID3D11PixelShader*			m_pPShader;
	ID3D11InputLayout*			m_pInputLayout;

	ID3D11Texture2D*			m_pMiniMapTexture;
	ID3D11RenderTargetView*		m_pMiniMapRenderTarget;
	ID3D11ShaderResourceView*	m_pMiniMapShaderResource;
	ID3D11Buffer*				m_pMiniMapVertexBuffer;
	ID3D11Buffer*				m_pConstantBuffer;
	Camera*						m_pCam;
	float						m_screenHeight, m_screenWidth;
public:
	MiniMap(float height, float width);
	~MiniMap();

	HRESULT SetUpMiniMap(ID3D11Device*	device, ID3D11DeviceContext* context);
	void SetCamera(Camera* cam) { m_pCam = cam; }
	void AddToMap(Scene_Node* rootNode, ID3D11DeviceContext* context, ID3D11DepthStencilView*	zBuffer);
	void RenderMap(ID3D11DeviceContext* context, ID3D11DepthStencilView*	zBuffer);
};