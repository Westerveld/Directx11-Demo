#include "MiniMap.h"

MiniMap::MiniMap(float height, float width)
{
	m_screenHeight = height;
	m_screenWidth = width;
}


MiniMap::~MiniMap()
{
}

HRESULT MiniMap::SetUpMiniMap(ID3D11Device* device, ID3D11DeviceContext* context)
{
	HRESULT hr;
	POS_COL_TEX_NORM_VERTEX vertices[] =
	{
		//Front Face
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f),	XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	XMFLOAT2(0.0f, 1.0f),	XMFLOAT3(0.0f,0.0f,-1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f),		XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	XMFLOAT2(0.0f, 0.0f),	XMFLOAT3(0.0f,0.0f,-1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f),		XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	XMFLOAT2(1.0f, 0.0f),	XMFLOAT3(0.0f,0.0f,-1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f),	XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	XMFLOAT2(0.0f, 1.0f),	XMFLOAT3(0.0f,0.0f,-1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f),		XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	XMFLOAT2(1.0f, 0.0f),	XMFLOAT3(0.0f,0.0f,-1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f),		XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	XMFLOAT2(1.0f, 1.0f),	XMFLOAT3(0.0f,0.0f,-1.0f) },

	};
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = device->CreateSamplerState(&samplerDesc, &m_pSampler0);
	if (FAILED(hr))
	{
		return hr;
	}
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = device->CreateBuffer(&bufferDesc, NULL, &m_pMiniMapVertexBuffer);
	if (FAILED(hr))
	{
		return hr;
	}
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	//Map will be a small square in the top right o fthe screen, we will use an eigth of the screen for it
	textureDesc.Width = (UINT)(m_screenWidth / 4);
	textureDesc.Height = (UINT)(m_screenHeight / 4);
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	hr = device->CreateTexture2D(&textureDesc, NULL, &m_pMiniMapTexture);
	if (FAILED(hr))
	{
		return hr;
	}
	//Minimap render target
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetDesc;
	ZeroMemory(&renderTargetDesc, sizeof(renderTargetDesc));
	renderTargetDesc.Format = textureDesc.Format;
	renderTargetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetDesc.Texture2D.MipSlice = 0;

	hr = device->CreateRenderTargetView(m_pMiniMapTexture, &renderTargetDesc, &m_pMiniMapRenderTarget);

	//Minimap shader resource
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
	ZeroMemory(&shaderDesc, sizeof(shaderDesc));
	shaderDesc.Format = textureDesc.Format;
	shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderDesc.Texture2D.MostDetailedMip = 0;
	shaderDesc.Texture2D.MipLevels = 1;

	hr = device->CreateShaderResourceView(m_pMiniMapTexture, &shaderDesc, &m_pMiniMapShaderResource);
	if (FAILED(hr))
	{
		return hr;
	}
	

	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));

	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT; //Can use UpdateSubresources() to update
	constantBufferDesc.ByteWidth = 64; //MUST be a multiple of 16, calculate CB struct
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; //Use as a constant buffer

	hr = device->CreateBuffer(&constantBufferDesc, NULL, &m_pConstantBuffer);
	if (FAILED(hr))
	{
		return hr;
	}

	ID3DBlob *PS, *error;

	hr = D3DX11CompileFromFile("minimap_shader.hlsl", 0, 0, "MiniMapPS", "ps_4_0", 0, 0, 0, &PS, &error, 0);

	if (error != 0)//Check for shader compilation error
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if (FAILED(hr))//Don't fail if error is just a warning
		{
			return hr;
		}
	}
	hr = device->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_pPShader);
	if (FAILED(hr))
	{
		return hr;
	}

	//Set the shaders
	context->PSSetShader(m_pPShader, 0, 0);

	D3D11_INPUT_ELEMENT_DESC iedesc[] =
	{
		//Be very careful setting the correct dxgi format and D3D version
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//NOTE the spelling of COLOR. Again, be careful setting the correct dxgi format (using A32) and correct D3D version
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	if (FAILED(hr))
	{
		return hr;
	}

	return S_OK;
}

void MiniMap::AddToMap(Scene_Node* rootNode, ID3D11DeviceContext* context, ID3D11DepthStencilView*	zBuffer)
{
	float clearCol[4] = { 0.0f, 0.1f, 0.1f, 1.0f };
	context->OMSetRenderTargets(1, &m_pMiniMapRenderTarget, zBuffer);
	context->ClearRenderTargetView(m_pMiniMapRenderTarget, clearCol);

	XMMATRIX world = XMMatrixIdentity();
	XMMATRIX view = m_pCam->GetViewMatrix();
	XMMATRIX proj = XMMatrixOrthographicLH(512, 512, 1.0f, 250.0f);
	world = XMMatrixTranspose(world);
	rootNode->Execute(&world, &view, &proj, m_pCam);
}

void MiniMap::RenderMap(ID3D11DeviceContext* context, ID3D11DepthStencilView* zBuffer)
{

	context->PSSetShader(m_pPShader, 0, 0);

	UINT stride = sizeof(POS_COL_TEX_NORM_VERTEX);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_pMiniMapVertexBuffer, &stride, &offset);

	MINIMAP_CONSTANT_BUFFER m_buffer;

	XMMATRIX world = XMMatrixIdentity();
	world = XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(0.5f, -0.5f, 0.0f);
	m_buffer.WorldViewProjection = XMMatrixTranspose(world);
	context->UpdateSubresource(m_pConstantBuffer, 0, NULL, &m_buffer, 0, 0);
	context->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	context->PSSetShaderResources(0, 1, &m_pMiniMapShaderResource);
	context->PSSetSamplers(0, 1, &m_pSampler0);
	context->DrawIndexed(6,0, 0);
}