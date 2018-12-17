#include "SkyBox.h"



SkyBox::SkyBox(ID3D11Device* device, ID3D11DeviceContext* context)
{
	m_pD3DDevice = device;
	m_pImmediateContext = context;
}


SkyBox::~SkyBox()
{
	if (m_pTexture) m_pTexture->Release();
	if (m_pSampler) m_pSampler->Release();
	if (m_pInputLayout) m_pInputLayout->Release();
	if (m_pPShader) m_pPShader->Release();
	if (m_pVShader) m_pVShader->Release();
	if (m_pVertexBuffer) m_pVertexBuffer->Release();
	if(m_pD3DDevice) m_pD3DDevice->Release();
	if (m_pImmediateContext) m_pImmediateContext->Release();
}

HRESULT SkyBox::CreateSkybox(char* texture)
{
	HRESULT hr = S_OK;

	//Define vertices of a Cube
	POS_COL_TEX_NORM_VERTEX vertices[] =
	{
		//Back Face
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f),		XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	XMFLOAT2(0.0f, 0.0f),	XMFLOAT3(0.0f,0.0f,1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f),		XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	XMFLOAT2(0.0f, 1.0f),	XMFLOAT3(0.0f,0.0f,1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f),		XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	XMFLOAT2(1.0f, 0.0f),	XMFLOAT3(0.0f,0.0f,1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f),		XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	XMFLOAT2(1.0f, 0.0f),	XMFLOAT3(0.0f,0.0f,1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f),		XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	XMFLOAT2(0.0f, 1.0f),	XMFLOAT3(0.0f,0.0f,1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f),		XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	XMFLOAT2(1.0f, 1.0f),	XMFLOAT3(0.0f,0.0f,1.0f) },

		//Front Face
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f),	XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	XMFLOAT2(0.0f, 1.0f),	XMFLOAT3(0.0f,0.0f,-1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f),		XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	XMFLOAT2(0.0f, 0.0f),	XMFLOAT3(0.0f,0.0f,-1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f),		XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	XMFLOAT2(1.0f, 0.0f),	XMFLOAT3(0.0f,0.0f,-1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f),	XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	XMFLOAT2(0.0f, 1.0f),	XMFLOAT3(0.0f,0.0f,-1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f),		XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	XMFLOAT2(1.0f, 0.0f),	XMFLOAT3(0.0f,0.0f,-1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f),		XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	XMFLOAT2(1.0f, 1.0f),	XMFLOAT3(0.0f,0.0f,-1.0f) },

		//Left Face
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f),	XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),	XMFLOAT2(0.0f, 1.0f),	XMFLOAT3(-1.0f,0.0f,0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f),		XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),	XMFLOAT2(0.0f, 0.0f),	XMFLOAT3(-1.0f,0.0f,0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f),		XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),	XMFLOAT2(1.0f, 1.0f),	XMFLOAT3(-1.0f,0.0f,0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f),		XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),	XMFLOAT2(0.0f, 0.0f),	XMFLOAT3(-1.0f,0.0f,0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f),		XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),	XMFLOAT2(1.0f, 0.0f),	XMFLOAT3(-1.0f,0.0f,0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f),		XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),	XMFLOAT2(1.0f, 1.0f),	XMFLOAT3(-1.0f,0.0f,0.0f) },

		//Right Face
		{ XMFLOAT3(1.0f, -1.0f, 1.0f),		XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),	XMFLOAT2(0.0f, 0.0f),	XMFLOAT3(1.0f,0.0f,0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f),		XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),	XMFLOAT2(0.0f, 1.0f),	XMFLOAT3(1.0f,0.0f,0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f),		XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),	XMFLOAT2(1.0f, 1.0f),	XMFLOAT3(1.0f,0.0f,0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f),		XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),	XMFLOAT2(1.0f, 0.0f),	XMFLOAT3(1.0f,0.0f,0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f),		XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),	XMFLOAT2(0.0f, 0.0f),	XMFLOAT3(1.0f,0.0f,0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f),		XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),	XMFLOAT2(1.0f, 1.0f),	XMFLOAT3(1.0f,0.0f,0.0f) },

		//Bottom Face
		{ XMFLOAT3(1.0f, -1.0f, -1.0f),		XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),	XMFLOAT2(1.0f, 1.0f),	XMFLOAT3(0.0f,-1.0f,0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f),		XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),	XMFLOAT2(1.0f, 0.0f),	XMFLOAT3(0.0f,-1.0f,0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f),	XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),	XMFLOAT2(0.0f, 1.0f),	XMFLOAT3(0.0f,-1.0f,0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f),		XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),	XMFLOAT2(1.0f, 0.0f),	XMFLOAT3(0.0f,-1.0f,0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f),		XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),	XMFLOAT2(0.0f, 0.0f),	XMFLOAT3(0.0f,-1.0f,0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f),	XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),	XMFLOAT2(0.0f, 1.0f),	XMFLOAT3(0.0f,-1.0f,0.0f) },

		//Top Face
		{ XMFLOAT3(1.0f, 1.0f, 1.0f),		XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),	XMFLOAT2(1.0f, 0.0f),	XMFLOAT3(0.0f,1.0f,0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f),		XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),	XMFLOAT2(1.0f, 1.0f),	XMFLOAT3(0.0f,1.0f,0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f),		XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),	XMFLOAT2(0.0f, 1.0f),	XMFLOAT3(0.0f,1.0f,0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f),		XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),	XMFLOAT2(0.0f, 0.0f),	XMFLOAT3(0.0f,1.0f,0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f),		XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),	XMFLOAT2(1.0f, 0.0f),	XMFLOAT3(0.0f,1.0f,0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f),		XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),	XMFLOAT2(0.0f, 1.0f),	XMFLOAT3(0.0f,1.0f,0.0f) },

	};

	D3D11_SAMPLER_DESC sampler_desc;
	ZeroMemory(&sampler_desc, sizeof(sampler_desc));
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	m_pD3DDevice->CreateSamplerState(&sampler_desc, &m_pSampler);

	//Set up and create vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;										//Allows use by CPU and GPU
	bufferDesc.ByteWidth = sizeof(vertices);									//Set the total size of the buffer (in this case, 3 vertices)
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;							//Set the type of buffer to vertex buffer
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;							//Allow access by the CPU
	hr = m_pD3DDevice->CreateBuffer(&bufferDesc, NULL, &m_pVertexBuffer);		//Create the buffer

	if (FAILED(hr))//Return an error code if failed
	{
		return hr;
	}

	D3D11_RASTERIZER_DESC rasterizer_desc;
	ZeroMemory(&rasterizer_desc, sizeof(rasterizer_desc));

	rasterizer_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_desc.CullMode = D3D11_CULL_BACK;
	hr = m_pD3DDevice->CreateRasterizerState(&rasterizer_desc, &m_pRasterSolid);

	rasterizer_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_desc.CullMode = D3D11_CULL_FRONT;
	hr = m_pD3DDevice->CreateRasterizerState(&rasterizer_desc, &m_pRasterSkyBox);

	D3D11_DEPTH_STENCIL_DESC DSDecsc;
	ZeroMemory(&DSDecsc, sizeof(DSDecsc));
	DSDecsc.DepthEnable = true;
	DSDecsc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DSDecsc.DepthFunc = D3D11_COMPARISON_LESS;
	hr = m_pD3DDevice->CreateDepthStencilState(&DSDecsc, &m_pDepthWriteSolid);
	DSDecsc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	hr = m_pD3DDevice->CreateDepthStencilState(&DSDecsc, &m_pDepthWriteSkyBox);


	//Set up constant buffer
	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));

	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT; //Can use UpdateSubresources() to update
	constantBufferDesc.ByteWidth = 64; //MUST be a multiple of 16, calculate CB struct
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; //Use as a constant buffer

	hr = m_pD3DDevice->CreateBuffer(&constantBufferDesc, NULL, &m_pConstantBuffer);

	if (FAILED(hr))
		return hr;



	//Copy the vertices into the buffer
	D3D11_MAPPED_SUBRESOURCE ms;

	//Lock the buffer to allow writing
	m_pImmediateContext->Map(m_pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);

	//Copy the data
	memcpy(ms.pData, vertices, sizeof(vertices));

	//Unlock the buffer
	m_pImmediateContext->Unmap(m_pVertexBuffer, NULL);


	ID3DBlob *VS, *PS, *error;
	hr = D3DX11CompileFromFile("skybox_shader.hlsl", 0, 0, "SkyboxVS", "vs_4_0", 0, 0, 0, &VS, &error, 0);

	if (error != 0)//Check for shader compilation error
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if (FAILED(hr))//Don't fail if error is just a warning
		{
			return hr;
		}
	}

	hr = D3DX11CompileFromFile("skybox_shader.hlsl", 0, 0, "SkyboxPS", "ps_4_0", 0, 0, 0, &PS, &error, 0);

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
	hr = m_pD3DDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &m_pVShader);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = m_pD3DDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_pPShader);
	if (FAILED(hr))
	{
		return hr;
	}

	//Set the shader objects as active
	m_pImmediateContext->VSSetShader(m_pVShader, 0, 0);
	m_pImmediateContext->PSSetShader(m_pPShader, 0, 0);

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

	hr = m_pD3DDevice->CreateInputLayout(iedesc, ARRAYSIZE(iedesc), VS->GetBufferPointer(), VS->GetBufferSize(), &m_pInputLayout);

	if (FAILED(hr))
	{
		return hr;
	}

	m_pImmediateContext->IASetInputLayout(m_pInputLayout);

	hr = D3DX11CreateShaderResourceViewFromFile(m_pD3DDevice,
		"assets/skybox_mountain.dds", NULL, NULL,
		&m_pTexture, NULL);
	if (FAILED(hr))
		return hr;
}



void SkyBox::RenderSkyBox(XMMATRIX* view, XMMATRIX* projection, Camera* cam)
{
	m_pImmediateContext->RSSetState(m_pRasterSkyBox);
	m_pImmediateContext->OMSetDepthStencilState(m_pDepthWriteSkyBox, 0);
	m_pImmediateContext->VSSetShader(m_pVShader, 0, 0);
	m_pImmediateContext->PSSetShader(m_pPShader, 0, 0);

	m_pImmediateContext->IASetInputLayout(m_pInputLayout); 

	
	UINT stride = sizeof(POS_COL_TEX_NORM_VERTEX);
	UINT offset = 0;
	m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//Select primitive type to use 03-01
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	SKYBOX_CONSTANT_BUFFER skyBuff;

	XMMATRIX world;
	world = XMMatrixIdentity();
	world *= XMMatrixScaling(3.0f, 3.0f, 3.0f);
	world *= XMMatrixTranslation(cam->GetX(), cam->GetY(), cam->GetZ());

	skyBuff.WVP = world * (*view) * (*projection);

	m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, 0, &skyBuff, 0, 0);
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	m_pImmediateContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	m_pImmediateContext->PSSetSamplers(0, 1, &m_pSampler);
	m_pImmediateContext->PSSetShaderResources(0, 1, &m_pTexture);
	m_pImmediateContext->Draw(36, 0);

	m_pImmediateContext->OMSetDepthStencilState(m_pDepthWriteSolid, 0);
	m_pImmediateContext->RSSetState(m_pRasterSolid);
	
}
