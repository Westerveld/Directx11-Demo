#pragma once
#include "objfilemodel.h"
#include "LightManager.h"
#include <deque>
#include <time.h>


struct PARTICLE_CONSTANT_BUFFER
{
	XMMATRIX WorldViewProjection; //64 bytes
	XMFLOAT4 color; //16 bytes
};

struct Particle
{
	float gravity;
	XMFLOAT3 position;
	XMFLOAT3 velocity;
	XMFLOAT4 color;
	float age;
	float scale;
};

enum ParticleType{Fountain, Explosion};

class ParticleFactory
{
private:
	ID3D11Device*				m_pD3DDevice;
	ID3D11DeviceContext*		m_pImmediateContext;

	ID3D11VertexShader*			m_pVShader;
	ID3D11PixelShader*			m_pPShader;
	ID3D11InputLayout*			m_pInputLayout;
	ID3D11Buffer*				m_pConstantBuffer;
	ID3D11Buffer*				m_pVertexBuffer;

	ID3D11ShaderResourceView*	m_pTexture;
	ID3D11SamplerState*			m_pSampler;

	ID3D11RasterizerState*		m_pRasterSolid;
	ID3D11RasterizerState*		m_pRasterParticle;

	LightManager*				m_lights;

	float						m_x, m_y, m_z;
	float						m_xAngle, m_yAngle, m_zAngle;
	float						m_scale, m_defScale;

	float						m_boundingSphereCentreX, m_boundingSphereCentreY, m_boundingSphereCentreZ, m_boundingSphereRadius, m_defRadius;

	/*void						CalculateModelCentrePoint();
	void						CalculateBoudingSphereRadius();*/

	XMMATRIX					GetWorldMatrix();

	float						m_timePrevious;
	float						m_untilParticle;
	float						RandomZeroToOne();
	float						RandomNegOneToPosOne();

	std::deque<Particle*>		m_free;
	std::deque<Particle*>		m_active;

	bool						m_isActive;

	ParticleType				m_type;
	float						m_age;
public:
	ParticleFactory(ID3D11Device* device, ID3D11DeviceContext* deviceContext, LightManager* lights);
	~ParticleFactory();
	int CreateParticle();

	void Draw(XMMATRIX* view, XMMATRIX* projection, XMFLOAT3* cameraPosition);
	void DrawOne(Particle* particle, XMMATRIX* view, XMMATRIX* projection, XMFLOAT3* cameraPosition);

	//Texture creation
	HRESULT AddTexture(char* filename);

#pragma region Getters and Setters
	//m_x functions
	void SetXPos(float value) { m_x = value; }
	void IncXPos(float value) { m_x += value; }
	float GetXPos() { return m_x; }

	//m_y functions
	void SetYPos(float value) { m_y = value; }
	void IncYPos(float value) { m_y += value; }
	float GetYPos() { return m_y; }

	//m_z functions
	void SetZPos(float value) { m_z = value; }
	void IncZPos(float value) { m_z += value; }
	float GetZPos() { return m_z; }

	//xAngle functions
	void SetXAngle(float value) { m_xAngle = value; }
	void IncXAngle(float value) { m_xAngle += value; }
	float GetXAngle() { return m_xAngle; }

	//yAngle functions
	void SetYAngle(float value) { m_yAngle = value; }
	void IncYAngle(float value) { m_yAngle += value; }
	float GetYAngle() { return m_yAngle; }

	//zAngle functions
	void SetZAngle(float value) { m_zAngle = value; }
	void IncZAngle(float value) { m_zAngle += value; }
	float GetZAngle() { return m_zAngle; }

	//Scale functions
	void SetScale(float value) { m_scale = value; }
	void IncScale(float value) { m_scale += value; }
	float GetScale() { return m_scale; }
#pragma endregion

	//Custom Shader
	HRESULT LoadDefaultShaders();
	HRESULT LoadCustomShader(char* fileName, char* vertexShaderFunction, char* pixelShaderFunction);

	//Look at a point
	float LookAt_XZ(Particle* particle, float x, float z);
	XMVECTOR LookAt_XYZ(float x, float y, float z);

	void MoveForward(float distance);
	void MoveForwardXYZ(float distance);

	XMVECTOR GetBoundingSphereWorldSpacePosition();

	float GetBoundingSphereRadius();

	void SwitchParticleType(ParticleType newType);

	void SetActive(bool newState) { m_isActive = newState; }
	bool GetActive() { return m_isActive; }

};

