#include "Model.h"

Model::Model(ID3D11Device* device, ID3D11DeviceContext* deviceContext, LightManager* lights)
{
	m_pImmediateContext = deviceContext;
	m_pD3DDevice = device;
	m_lights = lights;


	m_pTexture = NULL;
	m_pSampler = NULL;
}


Model::~Model()
{
	if (m_pTexture) m_pTexture->Release();
	if (m_pSampler) m_pSampler->Release();
	if (m_pInputLayout) m_pInputLayout->Release();
	if (m_pPShader) m_pPShader->Release();
	if (m_pVShader) m_pVShader->Release();
	if (m_pShinyBuffer) m_pShinyBuffer->Release();
	if (m_pConstantBuffer) m_pConstantBuffer->Release();
	if (m_pD3DDevice) m_pD3DDevice->Release();
	if (m_pImmediateContext) m_pImmediateContext->Release();
	if (m_pObject)delete m_pObject;
}

HRESULT Model::LoadObjModel(char* filename)
{
	HRESULT hr = S_OK;
	m_pObject = new ObjFileModel(filename, m_pD3DDevice, m_pImmediateContext);

	if (m_pObject->filename == "FILE NOT LOADED") return S_FALSE;

	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));

	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT; //Can use UpdateSubresources() to update
	constantBufferDesc.ByteWidth = 112; //MUST be a multiple of 16, calculate CB struct
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; //Use as a constant buffer

	hr = m_pD3DDevice->CreateBuffer(&constantBufferDesc, NULL, &m_pConstantBuffer);

	if (FAILED(hr))
	{
		return hr;
	}

	ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));

	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT; //Can use UpdateSubresources() to update
	constantBufferDesc.ByteWidth = 64; //MUST be a multiple of 16, calculate CB struct
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; //Use as a constant buffer

	hr = m_pD3DDevice->CreateBuffer(&constantBufferDesc, NULL, &m_pShinyBuffer);

	if (FAILED(hr))
	{
		return hr;
	}

	ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));

	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDesc.ByteWidth = 16;
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = m_pD3DDevice->CreateBuffer(&constantBufferDesc, NULL, &m_pDissolveBuffer);


	//Define beldning
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));

	D3D11_RENDER_TARGET_BLEND_DESC renderBlend;

	renderBlend.BlendEnable = true;
	renderBlend.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	renderBlend.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	renderBlend.BlendOp = D3D11_BLEND_OP_ADD;
	renderBlend.SrcBlendAlpha = D3D11_BLEND_ONE;
	renderBlend.DestBlendAlpha = D3D11_BLEND_ZERO;
	renderBlend.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	renderBlend.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0] = renderBlend;

	m_pD3DDevice->CreateBlendState(&blendDesc, &m_pTransparencyBlend);

	D3D11_SAMPLER_DESC sampler_desc;
	ZeroMemory(&sampler_desc, sizeof(sampler_desc));
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = m_pD3DDevice->CreateSamplerState(&sampler_desc, &m_pAlphaSampler);

	///CalculateModelCentrePoint();
	//CalculateBoudingSphereRadius();

	return S_OK;
}

HRESULT Model::LoadDefaultShaders()
{
	HRESULT hr = S_OK;
	ID3DBlob *VS, *PS, *error;

	hr = D3DX11CompileFromFile("model_shader.hlsl", 0, 0, "ModelVS", "vs_4_0", 0, 0, 0, &VS, &error, 0);

	if (error != 0)//Check for shader compilation error
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if (FAILED(hr))//Don't fail if error is just a warning
		{
			return hr;
		}
	}

	hr = D3DX11CompileFromFile("model_shader.hlsl", 0, 0, "ModelPS", "ps_4_0", 0, 0, 0, &PS, &error, 0);

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

	//Set the shaders
	m_pImmediateContext->VSSetShader(m_pVShader, 0, 0);
	m_pImmediateContext->PSSetShader(m_pPShader, 0, 0);

	D3D11_INPUT_ELEMENT_DESC iedesc[] =
	{
		//Be very careful setting the correct dxgi format and D3D version
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//NOTE the spelling of COLOR. Again, be careful setting the correct dxgi format (using A32) and correct D3D version
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = m_pD3DDevice->CreateInputLayout(iedesc, ARRAYSIZE(iedesc), VS->GetBufferPointer(), VS->GetBufferSize(), &m_pInputLayout);

	if (FAILED(hr))
	{
		return hr;
	}

	m_pImmediateContext->IASetInputLayout(m_pInputLayout);

	return S_OK;
}

HRESULT Model::LoadCustomShader(char* fileName, char* vertexShaderFunction, char* pixelShaderFunction)
{
	HRESULT hr = S_OK;

	ID3DBlob *VS, *PS, *error;

	hr = D3DX11CompileFromFile(fileName, 0, 0, vertexShaderFunction, "vs_4_0", 0, 0, 0, &VS, &error, 0);

	if (error != 0)//Check for shader compilation error
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if (FAILED(hr))//Don't fail if error is just a warning
		{
			return hr;
		}
	}

	hr = D3DX11CompileFromFile(fileName, 0, 0, pixelShaderFunction, "ps_4_0", 0, 0, 0, &PS, &error, 0);

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

	//Set the shaders
	m_pImmediateContext->VSSetShader(m_pVShader, 0, 0);
	m_pImmediateContext->PSSetShader(m_pPShader, 0, 0);

	D3D11_INPUT_ELEMENT_DESC iedesc[] =
	{
		//Be very careful setting the correct dxgi format and D3D version
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//NOTE the spelling of COLOR. Again, be careful setting the correct dxgi format (using A32) and correct D3D version
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = m_pD3DDevice->CreateInputLayout(iedesc, ARRAYSIZE(iedesc), VS->GetBufferPointer(), VS->GetBufferSize(), &m_pInputLayout);
	if (FAILED(hr))
	{
		return hr;
	}

	m_pImmediateContext->IASetInputLayout(m_pInputLayout);
	return S_OK;
}

void Model::Draw(XMMATRIX* world, XMMATRIX* view, XMMATRIX* projection)
{
	//Check shaders are created
	if (!m_pVShader || !m_pPShader)
	{
		HRESULT hr = S_OK;
		hr = LoadDefaultShaders();

		if (FAILED(hr))
			return;
	}

	if (m_type == ModelType::Dissolve)
		m_pImmediateContext->OMSetBlendState(m_pTransparencyBlend, 0, 0xffffffff);
	else
		m_pImmediateContext->OMSetBlendState(0, 0, 0xffffffff);
	
	//Set input layout and shaders active
	m_pImmediateContext->VSSetShader(m_pVShader, 0, 0);
	m_pImmediateContext->PSSetShader(m_pPShader, 0, 0);
	m_pImmediateContext->IASetInputLayout(m_pInputLayout);

	//Set constant buffer values
	MODEL_CONSTANT_BUFFER model_cbValues;
	model_cbValues.WorldViewProjection = (*world) * (*view) * (*projection);

	model_cbValues.ambLightCol = m_lights->GetAmbLightCol();
	model_cbValues.dirLightCol = m_lights->GetDirLightCol();;
	XMMATRIX transpose;
	transpose = XMMatrixTranspose((*world));
	//Calculate the directional light position in relation to this model
	model_cbValues.dirLightPos = XMVector3Transform(m_lights->GetDirLightPos(), transpose);
	model_cbValues.dirLightPos = XMVector3Normalize(model_cbValues.dirLightPos);

	SHINYMODEL_CONSTANT_BUFFER sm_cbValue;
	sm_cbValue.WorldView = (*world) * (*view);

	DISSOLVE_CONSTANT_BUFFER d_cbValues;
	d_cbValues.dissolveAmount = m_dissolveAmount;
	d_cbValues.specExp = 0.5f;
	d_cbValues.specularIntensity = m_dissolveAmount;
	//d_cbValues.dissolveColor = m_dissolveColor;

	
	//Upload new values to buffer
	m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, 0, &model_cbValues, 0, 0);

	//Set the constant buffer active
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	m_pImmediateContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	if (m_type == ModelType::Shiny)
	{
		m_pImmediateContext->UpdateSubresource(m_pShinyBuffer, 0, 0, &sm_cbValue, 0,0);
		m_pImmediateContext->VSSetConstantBuffers(1, 1, &m_pShinyBuffer);
		m_pImmediateContext->PSSetConstantBuffers(1, 1, &m_pShinyBuffer);
	}

	if (m_type == ModelType::Dissolve)
	{
		m_pImmediateContext->UpdateSubresource(m_pDissolveBuffer, 0, 0, &d_cbValues, 0, 0);
		m_pImmediateContext->VSSetConstantBuffers(1, 1, &m_pDissolveBuffer);
		m_pImmediateContext->PSSetConstantBuffers(1, 1, &m_pDissolveBuffer);
	}

	if (m_pSampler && m_pTexture)
	{
		//Set texture
		m_pImmediateContext->PSSetSamplers(0, 1, &m_pSampler);
		m_pImmediateContext->PSSetShaderResources(0, 1, &m_pTexture);
	}

	if (m_pSkyboxTexture)
	{
		m_pImmediateContext->PSSetShaderResources(1, 1, &m_pSkyboxTexture);
	}

	if (m_pDissolveTexture)
	{
		m_pImmediateContext->PSSetShaderResources(1, 1, &m_pDissolveTexture);

		if (m_pAlphaSampler)
			m_pImmediateContext->PSSetSamplers(1, 1, &m_pAlphaSampler);
	}

	m_pObject->Draw();
}


#pragma region Sphere Collision Detection
void Model::CalculateModelCentrePoint()
{
	float minX = 0, minY = 0, minZ = 0, maxX = 0, maxY = 0, maxZ = 0;

	for (unsigned int i = 0; i < m_pObject->numverts; i++)
	{
		if (m_pObject->vertices[i].Pos.x < minX)
			minX = m_pObject->vertices[i].Pos.x;
		if (m_pObject->vertices[i].Pos.x > maxX)
			maxX = m_pObject->vertices[i].Pos.x;

		if (m_pObject->vertices[i].Pos.y < minY)
			minY = m_pObject->vertices[i].Pos.y;
		if (m_pObject->vertices[i].Pos.y > maxY)
			maxY = m_pObject->vertices[i].Pos.y;

		if (m_pObject->vertices[i].Pos.z < minZ)
			minZ = m_pObject->vertices[i].Pos.z;
		if (m_pObject->vertices[i].Pos.z > maxZ)
			maxZ = m_pObject->vertices[i].Pos.z;
	}

	m_boundingSphereCentreX = (minX + maxX) / 2;
	m_boundingSphereCentreY = (minY + maxY) / 2;
	m_boundingSphereCentreZ = (minZ + maxZ) / 2;
}

void Model::CalculateBoudingSphereRadius()
{
	float biggestDistFromCentre = 0;

	for (unsigned int i = 0; i < m_pObject->numverts; i++)
	{
		if ((m_boundingSphereCentreX + m_pObject->vertices[i].Pos.x) > (m_boundingSphereCentreX + biggestDistFromCentre))
			biggestDistFromCentre = m_pObject->vertices[i].Pos.x;

		if ((m_boundingSphereCentreY + m_pObject->vertices[i].Pos.y) > (m_boundingSphereCentreY + biggestDistFromCentre))
			biggestDistFromCentre = m_pObject->vertices[i].Pos.y;

		if ((m_boundingSphereCentreZ + m_pObject->vertices[i].Pos.z) > (m_boundingSphereCentreZ + biggestDistFromCentre))
			biggestDistFromCentre = m_pObject->vertices[i].Pos.z;
	}

	m_boundingSphereRadius = biggestDistFromCentre;

	m_defRadius = m_boundingSphereRadius;

}

void Model::CalculateBoundingBox()
{
	float minX = 0, minY = 0, minZ = 0, maxX = 0, maxY = 0, maxZ = 0;

	for (unsigned int i = 0; i < m_pObject->numverts; i++)
	{
		if (m_pObject->vertices[i].Pos.x < minX)
			minX = m_pObject->vertices[i].Pos.x;
		if (m_pObject->vertices[i].Pos.x > maxX)
			maxX = m_pObject->vertices[i].Pos.x;

		if (m_pObject->vertices[i].Pos.y < minY)
			minY = m_pObject->vertices[i].Pos.y;
		if (m_pObject->vertices[i].Pos.y > maxY)
			maxY = m_pObject->vertices[i].Pos.y;

		if (m_pObject->vertices[i].Pos.z < minZ)
			minZ = m_pObject->vertices[i].Pos.z;
		if (m_pObject->vertices[i].Pos.z > maxZ)
			maxZ = m_pObject->vertices[i].Pos.z;
	}

	m_boundingBoxCentre.x = (minX + maxX) * 0.5;
	m_boundingBoxCentre.y = (minY + maxY) * 0.5;
	m_boundingBoxCentre.z = (minZ + maxZ) * 0.5;

	m_boundingBoxSize.x = (maxX - minX) * 0.5;
	m_boundingBoxSize.y = (maxY - minY) * 0.5;
	m_boundingBoxSize.z = (maxZ - minZ) * 0.5;
}
#pragma endregion

#pragma region Getters and Setters

float Model::GetBoundingSphereRadius()
{
	return m_boundingSphereRadius;
}

void Model::SetCollisionType(CollisionType newType)
{
	switch (newType)
	{
		case CollisionType::Sphere:
			CalculateModelCentrePoint();
			CalculateBoudingSphereRadius();
			break;
		case CollisionType::Box:
			CalculateBoundingBox();
			break;
		case CollisionType::Mesh:
			break;

	}
	m_collisionType = newType;
}

void Model::SetDissolveAmount(float val)
{
	if (val < 0.0f)
	{
		m_dissolveAmount = 1.0f;
	}
	else
	{
		m_dissolveAmount = val;
	}
}
#pragma endregion