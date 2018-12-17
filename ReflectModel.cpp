#include "ReflectModel.h"

ReflectModel::ReflectModel(ID3D11Device* device, ID3D11DeviceContext* deviceContext, LightManager* lights)
{
	m_pImmediateContext = deviceContext;
	m_pD3DDevice = device;
	m_lights = lights;

	m_x = 0.0f;
	m_y = 0.0f;
	m_z = 0.0f;

	m_xAngle = 0.0f;
	m_yAngle = 0.0f;
	m_zAngle = 0.0f;

	m_scale = 1.0f;
	m_defScale = m_scale;
}


ReflectModel::~ReflectModel()
{
	if (m_pTexture) m_pTexture->Release();
	if (m_pSampler) m_pSampler->Release();
	if (m_pInputLayout) m_pInputLayout->Release();
	if (m_pPShader) m_pPShader->Release();
	if (m_pVShader) m_pVShader->Release();
	if (m_pShinyBuffer) m_pShinyBuffer->Release();
	if (m_pD3DDevice) m_pD3DDevice->Release();
	if (m_pImmediateContext) m_pImmediateContext->Release();
	if(m_pObject) delete m_pObject;
}

HRESULT ReflectModel::LoadObjModel(char* filename, char* textureName)
{
	HRESULT hr = S_OK;
	m_pObject = new ObjFileModel(filename, m_pD3DDevice, m_pImmediateContext);

	if (m_pObject->filename == "FILE NOT LOADED") return S_FALSE;

	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));

	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDesc.ByteWidth = 64;
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = m_pD3DDevice->CreateBuffer(&constantBufferDesc, NULL, &m_pShinyBuffer);
	
	if (FAILED(hr))
	{
		return hr;
	}

	D3D11_BUFFER_DESC worldBufferDesc;
	ZeroMemory(&worldBufferDesc, sizeof(worldBufferDesc));
	worldBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	worldBufferDesc.ByteWidth = 112;
	worldBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = m_pD3DDevice->CreateBuffer(&worldBufferDesc, NULL, &m_pWorldBuffer);
	if (FAILED(hr))
	{
		return hr;
	}

	LoadDefaultShaders();
	AddTexture(textureName);
	CalculateModelCentrePoint();
	CalculateBoudingSphereRadius();

	return S_OK;
}

HRESULT ReflectModel::AddTexture(char* filename)
{
	HRESULT hr = S_OK;

	D3D11_SAMPLER_DESC sampler_desc;
	ZeroMemory(&sampler_desc, sizeof(sampler_desc));
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = m_pD3DDevice->CreateSamplerState(&sampler_desc, &m_pSampler);

	if (FAILED(hr))
	{
		return hr;
	}

	hr = D3DX11CreateShaderResourceViewFromFile(m_pD3DDevice,
		filename, NULL, NULL,
		&m_pTexture, NULL);

	if (FAILED(hr))
	{
		return hr;
	}

	return S_OK;
}


HRESULT ReflectModel::LoadDefaultShaders()
{
	HRESULT hr = S_OK;
	ID3DBlob *VS, *PS, *error;

	hr = D3DX11CompileFromFile("reflect_shader.hlsl", 0, 0, "ModelVS", "vs_4_0", 0, 0, 0, &VS, &error, 0);

	if (error != 0)//Check for shader compilation error
	{
		OutputDebugString((char*)error->GetBufferPointer());
		error->Release();
		if (FAILED(hr))//Don't fail if error is just a warning
		{
			return hr;
		}
	}

 	hr = D3DX11CompileFromFile("reflect_shader.hlsl", 0, 0, "ModelPS", "ps_4_0", 0, 0, 0, &PS, &error, 0);

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

	return S_OK;
}

//Doesn't work yet
HRESULT ReflectModel::LoadCustomShader(char* fileName, char* vertexShaderFunction, char* pixelShaderFunction)
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

void ReflectModel::Draw(XMMATRIX* view, XMMATRIX* projection)
{
	//Check shaders are created
	if (!m_pVShader || !m_pPShader)
	{
		HRESULT hr = S_OK;
		hr = LoadDefaultShaders();

		if (FAILED(hr))
			return;
	}

	//Set input layout and shaders active
	m_pImmediateContext->VSSetShader(m_pVShader, 0, 0);
	m_pImmediateContext->PSSetShader(m_pPShader, 0, 0);
	m_pImmediateContext->IASetInputLayout(m_pInputLayout);

	XMMATRIX world, transpose;

	world = GetWorldMatrix();

	//transpose the world matrix for the lighting
	transpose = XMMatrixTranspose(world);

	SHINYWORLD_CONSTANT_BUFFER cb0_values;
	cb0_values.WorldViewProjection = world * (*view) * (*projection);
	cb0_values.ambLightCol = m_lights->GetAmbLightCol();
	cb0_values.dirLightCol = m_lights->GetDirLightCol();
	cb0_values.dirLightPos = XMVector3Transform(m_lights->GetDirLightPos(), transpose);
	cb0_values.dirLightPos = XMVector3Normalize(cb0_values.dirLightPos);

	//Set constant buffer values
	SHINY_CONSTANT_BUFFER shiny_bValues;
	shiny_bValues.WorldView = world * (*view);

	m_pImmediateContext->UpdateSubresource(m_pShinyBuffer, 0, 0, &shiny_bValues, 0, 0);
	m_pImmediateContext->UpdateSubresource(m_pWorldBuffer, 0, 0, &cb0_values, 0, 0);

	//Set the constant buffer active
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pWorldBuffer);
	m_pImmediateContext->VSSetConstantBuffers(1, 1, &m_pShinyBuffer);

	m_pImmediateContext->PSSetConstantBuffers(0, 1, &m_pWorldBuffer);
	m_pImmediateContext->PSSetConstantBuffers(1, 1, &m_pShinyBuffer);


	if (m_pSampler && m_pTexture)
	{
		//Set texture
		m_pImmediateContext->PSSetSamplers(0, 1, &m_pSampler);
		m_pImmediateContext->PSSetShaderResources(0, 1, &m_pTexture);
	}

	m_pObject->Draw();
}

void ReflectModel::LookAt_XZ(float x, float z)
{
	float dx, dz;
	dx = x - m_x;
	dz = z - m_z;
	m_yAngle = atan2(dx, dz) * (180.0 / XM_PI);
}

void ReflectModel::LookAt_XYZ(float x, float y, float z)
{
	float dx, dy, dz;

	dx = x - m_x;
	dy = y - m_y;
	dz = z - m_z;

	m_xAngle = -atan2(dy, dx - dz) * (180.0 / XM_PI);
	m_yAngle = atan2(dx, dz) * (180.0 / XM_PI);

}


void ReflectModel::MoveForward(float distance)
{
	m_x += sin(m_yAngle * (XM_PI / 180.0)) * distance;
	m_z += cos(m_yAngle * (XM_PI / 180.0)) * distance;
}

void ReflectModel::MoveForwardXYZ(float distance)
{
	MoveForward(distance);
	m_y += -sin(m_xAngle * (XM_PI / 180.0)) * distance;
}

#pragma region Sphere Collision Detection
void ReflectModel::CalculateModelCentrePoint()
{
	float minX = 0, minY = 0, minZ = 0, maxX = 0, maxY = 0, maxZ = 0;

	for (int i = 0; i < m_pObject->numverts; i++)
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

void ReflectModel::CalculateBoudingSphereRadius()
{
	float biggestDistFromCentre = 0;

	for (int i = 0; i < m_pObject->numverts; i++)
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

XMVECTOR ReflectModel::GetBoundingSphereWorldSpacePosition()
{
	XMVECTOR offset;
	XMMATRIX world = GetWorldMatrix();

	offset = XMVectorSet(m_boundingSphereCentreX, m_boundingSphereCentreY, m_boundingSphereCentreZ, 0.0f);
	offset = XMVector3Transform(offset, world);

	return offset;
}

bool ReflectModel::CheckCollision(std::vector<ReflectModel*> other)
{
	for (int i = 0; i < other.size(); i++)
	{
		if (other[i] == this)
			continue;

		XMVECTOR myPos;
		XMVECTOR otherPos;
		myPos = this->GetBoundingSphereWorldSpacePosition();
		otherPos = other[i]->GetBoundingSphereWorldSpacePosition();

		float x1, x2, y1, y2, z1, z2;

		x1 = XMVectorGetX(myPos);
		y1 = XMVectorGetY(myPos);
		z1 = XMVectorGetZ(myPos);

		x2 = XMVectorGetX(otherPos);
		y2 = XMVectorGetY(otherPos);
		z2 = XMVectorGetZ(otherPos);

		float distanceSqrd = pow(x1 - x2, 2) + pow(y1 - y2, 2) + pow(z1 - z2, 2);

		if (distanceSqrd < pow(this->GetBoundingSphereRadius() + other[i]->GetBoundingSphereRadius(), 2))
		{
			return true;
		}
	}
	return false;

}
#pragma endregion

#pragma region Getters and Setters
XMMATRIX ReflectModel::GetWorldMatrix()
{
	XMMATRIX world;
	world = XMMatrixIdentity();
	//Set scale
	world *= XMMatrixScaling(m_scale, m_scale, m_scale);

	//Set rotation
	world *= XMMatrixRotationX(XMConvertToRadians(m_xAngle));
	world *= XMMatrixRotationY(XMConvertToRadians(m_yAngle));
	world *= XMMatrixRotationZ(XMConvertToRadians(m_zAngle));

	//Set position
	world *= XMMatrixTranslation(m_x, m_y, m_z);

	return world;
}

float ReflectModel::GetBoundingSphereRadius()
{
	return m_boundingSphereRadius * m_scale;
}
#pragma endregion