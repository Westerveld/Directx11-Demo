#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include <d3dx11.h>
#include <windows.h>
#include <dxerr.h>
#include <xnamath.h>

#include "camera.h"

struct SKYBOX_CONSTANT_BUFFER
{
	XMMATRIX WVP; // 64 Bytes
};

class SkyBox 
{
private:
	ID3D11Device*				m_pD3DDevice;
	ID3D11DeviceContext*		m_pImmediateContext;

	ID3D11Buffer*				m_pConstantBuffer;
	ID3D11Buffer*				m_pVertexBuffer;
	ID3D11InputLayout*			m_pInputLayout;
	
	ID3D11VertexShader*			m_pVShader;
	ID3D11PixelShader*			m_pPShader;

	ID3D11ShaderResourceView*	m_pTexture;
	ID3D11SamplerState*			m_pSampler;

	ID3D11RasterizerState*		m_pRasterSolid = 0;
	ID3D11RasterizerState*		m_pRasterSkyBox = 0;
	ID3D11DepthStencilState*	m_pDepthWriteSolid = 0;
	ID3D11DepthStencilState*	m_pDepthWriteSkyBox = 0;

public:
	struct POS_COL_TEX_NORM_VERTEX 
	{
		XMFLOAT3 pos;
		XMFLOAT4 Col;
		XMFLOAT2 Texture0;
		XMFLOAT3 Normal;
	};

	SkyBox(ID3D11Device* device, ID3D11DeviceContext* context);
	~SkyBox();

	HRESULT CreateSkybox(char* texture);

	void RenderSkyBox(XMMATRIX* view, XMMATRIX* projection, Camera* cam);

};

