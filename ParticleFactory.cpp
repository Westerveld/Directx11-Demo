#include "ParticleFactory.h"

ParticleFactory::ParticleFactory(ID3D11Device* device, ID3D11DeviceContext* deviceContext, LightManager* lights)
{
	m_pImmediateContext = deviceContext;
	m_pD3DDevice = device;
	m_lights = lights;

	m_x = 0.0f;
	m_y = 0.0f;
	m_z = 0.0f;

	m_timePrevious = float(timeGetTime()) / 1000.0f;
	m_untilParticle = 1.0f;
	m_isActive = true;

	m_xAngle = 0.0f;
	m_yAngle = 0.0f;
	m_zAngle = 0.0f;
	srand(time(NULL));
	m_scale = 1.0f;
	m_defScale = m_scale;

	for (int i = 0; i < 100; i++)
	{
		m_free.push_back(new Particle);
	}
}


ParticleFactory::~ParticleFactory()
{
	
	for (int i = 0; i < m_free.size(); i++)
	{
		delete m_free[i];
		m_free[i] = nullptr;
		m_free.pop_front();
	}
	for (int i = 0; i < m_active.size(); i++)
	{
		delete m_active[i];
		m_active[i] = nullptr;
		m_active.pop_front();
	}


	if (m_pTexture) m_pTexture->Release();
	if (m_pSampler) m_pSampler->Release();
	if (m_pInputLayout) m_pInputLayout->Release();
	if (m_pPShader) m_pPShader->Release();
	if (m_pVShader) m_pVShader->Release();
	if (m_pConstantBuffer) m_pConstantBuffer->Release();
	if (m_pD3DDevice) m_pD3DDevice->Release();
	if (m_pImmediateContext) m_pImmediateContext->Release();
	
}

int ParticleFactory::CreateParticle()
{
	HRESULT hr = S_OK;

	XMFLOAT3 vertices[8] =
	{
		XMFLOAT3(-1.0f, -1.0f, 0.0f),
		XMFLOAT3(1.0f, 1.0f, 0.0f),
		XMFLOAT3(-1.0f,1.0f,0.0f),
		XMFLOAT3(-1.0f, -1.0f, 0.0f),
		XMFLOAT3(1.0f, -1.0f, 0.0f),
		XMFLOAT3(1.0f, 1.0f, 0.0f),
	};

	//Set up rasterizer
	D3D11_RASTERIZER_DESC raster_desc;
	ZeroMemory(&raster_desc, sizeof(raster_desc));
	raster_desc.FillMode = D3D11_FILL_SOLID;
	raster_desc.CullMode = D3D11_CULL_NONE;

	hr = m_pD3DDevice->CreateRasterizerState(&raster_desc, &m_pRasterSolid);

	raster_desc.CullMode = D3D11_CULL_BACK;

	hr = m_pD3DDevice->CreateRasterizerState(&raster_desc, &m_pRasterParticle);

	//Create vert buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(XMFLOAT3) * 6; //Vertex count
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = m_pD3DDevice->CreateBuffer(&bufferDesc, NULL, &m_pVertexBuffer);

	if (FAILED(hr))
	{
		return 0;
	}

	D3D11_MAPPED_SUBRESOURCE ms;

	m_pImmediateContext->Map(m_pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, vertices, sizeof(vertices));
	
	m_pImmediateContext->Unmap(m_pVertexBuffer, NULL);

	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDesc.ByteWidth = 80;
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = m_pD3DDevice->CreateBuffer(&constantBufferDesc, NULL, &m_pConstantBuffer);

	LoadDefaultShaders();

	return 0;

}

float ParticleFactory::RandomNegOneToPosOne()
{
	//rand number between 0-200
	int number = rand() % 200 + 0;
	//now a number between -100 and 100
	number -= 100;
	//divide by 100 to get a value between -1 and 1;
	float randNum = number / 100.0f;

	return randNum;
}

float ParticleFactory::RandomZeroToOne()
{
	int number = rand() % 100 + 0;
	
	float randNum = number / 100.0f;
	return randNum;
}
HRESULT ParticleFactory::AddTexture(char* filename)
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


HRESULT ParticleFactory::LoadDefaultShaders()
{
	HRESULT hr = S_OK;
	ID3DBlob *VS, *PS, *error;

	hr = D3DX11CompileFromFile("particle_shader.hlsl", 0, 0, "VShader", "vs_4_0", 0, 0, 0, &VS, &error, 0);

	if (error != 0)//Check for shader compilation error
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if (FAILED(hr))//Don't fail if error is just a warning
		{
			return hr;
		}
	}

	hr = D3DX11CompileFromFile("particle_shader.hlsl", 0, 0, "PShader", "ps_4_0", 0, 0, 0, &PS, &error, 0);

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
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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
HRESULT ParticleFactory::LoadCustomShader(char* fileName, char* vertexShaderFunction, char* pixelShaderFunction)
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

void ParticleFactory::Draw(XMMATRIX* view, XMMATRIX* projection, XMFLOAT3* cameraPosition)
{
	UINT stride = sizeof(XMFLOAT3);
	UINT offset = 0;
	XMMATRIX world;

	float timeNow = float(timeGetTime()) /1000.0f;
	float deltaTime = timeNow - m_timePrevious;
	m_timePrevious = timeNow;
	m_untilParticle -= deltaTime;

	//Iterator to go through the deque
	std::deque<Particle*>::iterator itr;

	//Set up new particles to draw
	if (m_untilParticle <= 0.0f)
	{
		if (m_isActive)
		{
			itr = m_free.begin();

			if (m_free.size() != NULL)
			{
				switch (m_type)
				{
				case Fountain:
					m_untilParticle = 0.008f;
					m_age = 2.0f;
					(*itr)->color = XMFLOAT4(RandomZeroToOne(), RandomZeroToOne(), RandomZeroToOne(), 1.0f);
					(*itr)->gravity = 4.5f;
					(*itr)->position = XMFLOAT3(0.0f, 1.0f, 3.0f);
					(*itr)->velocity = XMFLOAT3(RandomNegOneToPosOne(), 2.5f, RandomNegOneToPosOne());
					(*itr)->scale = 0.3f;
					break;

				case Explosion:
					m_untilParticle == 0.008f;
					m_age = 3.0f;
					(*itr)->color = XMFLOAT4(1.0f, RandomZeroToOne(), RandomZeroToOne(), 1.0f);
					(*itr)->gravity = 0.0f;
					(*itr)->position = XMFLOAT3(0.0f, 1.0f, 3.0f);
					(*itr)->velocity = XMFLOAT3(RandomNegOneToPosOne() * 5.0f, RandomNegOneToPosOne() * 5.0f, RandomNegOneToPosOne() * 5.0f);
					(*itr)->scale = 0.4f;
					break;

				default:
					break;
				}
				(*itr)->age = 0.0f;
				m_active.push_back(*itr);
				m_free.pop_front();
			}


		}
		else 
			m_untilParticle = 0.001f;
	}


	//Check if there are particles to draw
	if (m_active.size() != NULL)
	{

		itr = m_active.begin();

		while (itr != m_active.end())
		{
			switch (m_type)
			{
			case Fountain:
				(*itr)->age += deltaTime;
				(*itr)->scale -= 0.12f * (deltaTime);
				if ((*itr)->scale < 0.05f)
					(*itr)->scale = 0.05f;

				(*itr)->velocity.y -= (*itr)->gravity*(deltaTime);
				(*itr)->position.x += (*itr)->velocity.x*(deltaTime);
				(*itr)->position.y += (*itr)->velocity.y*(deltaTime);
				(*itr)->position.z += (*itr)->velocity.z * (deltaTime);
				break;

			case Explosion:
				(*itr)->age += deltaTime;
				(*itr)->scale -= 0.10f * (deltaTime);
				if ((*itr)->scale < 0.05f)
					(*itr)->scale = 0.05f;

				(*itr)->position.x += (*itr)->velocity.x * (deltaTime);
				(*itr)->position.y += (*itr)->velocity.y * (deltaTime);
				(*itr)->position.z += (*itr)->velocity.z * (deltaTime);
				break;

			default:
				break;
			}

			world = XMMatrixIdentity();

			switch (m_type)
			{
			case Fountain:
				world = GetWorldMatrix();
				world *= XMMatrixScaling((*itr)->scale, (*itr)->scale, (*itr)->scale);
				world *= XMMatrixRotationY(XMConvertToRadians(LookAt_XZ((*itr), cameraPosition->x,cameraPosition->z)));
				world *= XMMatrixTranslation((*itr)->position.x, (*itr)->position.y, (*itr)->position.z);
				break;
				
			case Explosion:
				world = GetWorldMatrix();
				world *= XMMatrixScaling((*itr)->scale, (*itr)->scale, (*itr)->scale);
				world *= XMMatrixRotationY(XMConvertToRadians(LookAt_XZ((*itr), cameraPosition->x, cameraPosition->z)));
				world *= XMMatrixTranslation((*itr)->position.x, (*itr)->position.y, (*itr)->position.z);
				break;

			default:
				break;
			}

			PARTICLE_CONSTANT_BUFFER particle_cbValues;
			particle_cbValues.WorldViewProjection = world * (*view) * (*projection);
			particle_cbValues.color = (*itr)->color;

			//Set input layout and shaders active
			m_pImmediateContext->VSSetShader(m_pVShader, 0, 0);
			m_pImmediateContext->PSSetShader(m_pPShader, 0, 0);
			m_pImmediateContext->IASetInputLayout(m_pInputLayout);

			UINT offset = 0;
			m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);



			m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, 0, &particle_cbValues, 0, 0);
			m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
			m_pImmediateContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

			//m_pImmediateContext->RSSetState(m_pRasterParticle);

			m_pImmediateContext->RSSetState(m_pRasterSolid); 
			m_pImmediateContext->Draw(6, 0);

			if ((*itr)->age >= m_age)
			{
				itr++;
				m_active.front()->age = m_age;
				m_free.push_back(m_active.front());
				m_active.pop_front();
			}
			else
			{
				itr++;
			}
		}
	}

	//DrawOne(, view, projection, cameraPosition);

}

void ParticleFactory::DrawOne(Particle* particle, XMMATRIX* view, XMMATRIX* projection, XMFLOAT3* cameraPosition)
{
	UINT stride = sizeof(XMFLOAT3);
	UINT offset = 0;
	XMMATRIX world;

	SetScale(1.0f);
	SetYAngle(XM_PI);

	SetXPos(particle->position.x);
	SetYPos(particle->position.y);
	SetZPos(particle->position.z);

	world = GetWorldMatrix();
	

	PARTICLE_CONSTANT_BUFFER particle_cbValues;
	particle_cbValues.WorldViewProjection = world * (*view) * (*projection);
	particle_cbValues.color = particle->color;

	m_pImmediateContext->RSSetState(m_pRasterSolid);
	//Set input layout and shaders active
	m_pImmediateContext->VSSetShader(m_pVShader, 0, 0);
	m_pImmediateContext->PSSetShader(m_pPShader, 0, 0);
	m_pImmediateContext->IASetInputLayout(m_pInputLayout);

	m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	


	m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, 0, &particle_cbValues, 0, 0);
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	m_pImmediateContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	
	m_pImmediateContext->Draw(6, 0);

	m_pImmediateContext->RSSetState(m_pRasterParticle);
}

float ParticleFactory::LookAt_XZ(Particle* particle, float x, float z)
{
	float dx, dz;
	dx = x - particle->position.x;
	dz = z - particle->position.z;
	//m_yAngle = atan2(dx, dz) * (180.0 / XM_PI);
	float value = atan2(dx, dz) * (180.0 / XM_PI);
	return value;
}

XMVECTOR ParticleFactory::LookAt_XYZ(float x, float y, float z)
{
	float dx, dy, dz;
	XMVECTOR angles;
	dx = x - m_x;
	dy = y - m_y;
	dz = z - m_z;

	/*m_xAngle = -atan2(dy, dx - dz) * (180.0 / XM_PI);
	m_yAngle = atan2(dx, dz) * (180.0 / XM_PI);*/

	angles.z = 0;
	angles.x = -atan2(dy, dx - dz) * (180.0 / XM_PI);
	angles.y = atan2(dx, dz) * (180.0 / XM_PI);

	return angles;
}


void ParticleFactory::MoveForward(float distance)
{
	m_x += sin(m_yAngle * (XM_PI / 180.0)) * distance;
	m_z += cos(m_yAngle * (XM_PI / 180.0)) * distance;
}

void ParticleFactory::MoveForwardXYZ(float distance)
{
	MoveForward(distance);
	m_y += -sin(m_xAngle * (XM_PI / 180.0)) * distance;
}

#pragma region Sphere Collision Detection
/*void ParticleFactory::CalculateModelCentrePoint()
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

/*void ParticleFactory::CalculateBoudingSphereRadius()
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

}*/

XMVECTOR ParticleFactory::GetBoundingSphereWorldSpacePosition()
{
	XMVECTOR offset;
	XMMATRIX world = GetWorldMatrix();

	offset = XMVectorSet(m_boundingSphereCentreX, m_boundingSphereCentreY, m_boundingSphereCentreZ, 0.0f);
	offset = XMVector3Transform(offset, world);

	return offset;
}

/*bool ParticleFactory::CheckCollision(std::vector<Model*> other)
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
*/
#pragma region Getters and Setters
XMMATRIX ParticleFactory::GetWorldMatrix()
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

float ParticleFactory::GetBoundingSphereRadius()
{
	return m_boundingSphereRadius * m_scale;
}

void ParticleFactory::SwitchParticleType(ParticleType newType)
{
	if (newType != m_type)
	{

		m_type = newType;
		if (m_active.size() != NULL)
		{
			std::deque<Particle*>::iterator itr;

			itr = m_active.begin();

			while (m_active.size() > 0)
			{
				itr++;
				m_active.front()->age = m_age;
				m_free.push_back(m_active.front());
				m_active.pop_front();
			}
		}
	}

}

#pragma endregion
