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
	ID3D11BlendState*			m_pTransparencyBlend;

	LightManager*				m_lights;

	float						m_xAngle, m_yAngle, m_zAngle;
	float						m_scale, m_defScale;

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
	int CreateParticle(void);

	void Draw(XMMATRIX* world, XMMATRIX* view, XMMATRIX* projection, xyz cameraPosition);
	void DrawOne(Particle* particle, XMMATRIX* view, XMMATRIX* projection, xyz cameraPosition);

	//Texture creation
	HRESULT AddTexture(char* filename);

#pragma region Getters and Setters

	//Scale functions
	void SetScale(float value)					{ m_scale = value; }
	void IncScale(float value)					{ m_scale += value; }
	float GetScale(void)						{ return m_scale; }

	void SetActive(bool newState)				{ m_isActive = newState; }
	bool GetActive(void)						{ return m_isActive; }

#pragma endregion

	//Custom Shader
	HRESULT LoadDefaultShaders(void);
	HRESULT LoadCustomShader(char* fileName, char* vertexShaderFunction, char* pixelShaderFunction);

	//Look at a point
	float LookAt_XZ(Particle* particle, float x, float z);
	XMVECTOR LookAt_XYZ(Particle* particle, float x, float y, float z);

	void SwitchParticleType(ParticleType newType);
	
};