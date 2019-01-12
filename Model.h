#pragma once
#include "objfilemodel.h"
#include "LightManager.h"
#include <list>

struct MODEL_CONSTANT_BUFFER
{
	XMMATRIX WorldViewProjection; //64 bytes
	XMVECTOR dirLightCol; //16 bytes
	XMVECTOR dirLightPos; //16 bytes
	XMVECTOR ambLightCol; //16 bytes
};

struct SHINYMODEL_CONSTANT_BUFFER
{
	XMMATRIX WorldView;
};

struct DISSOLVE_CONSTANT_BUFFER
{
	XMVECTOR dissolveColor; //16 bytes
	float dissolveAmount; // 4 bytes
	XMFLOAT3 packing;   //12 bytes

};

enum ModelType{Normal, Shiny, Dissolve};
enum CollisionType{Sphere, Box, Mesh};
class Model
{
private:
	ID3D11Device*				m_pD3DDevice;
	ID3D11DeviceContext*		m_pImmediateContext;

	ObjFileModel*				m_pObject;
	ID3D11VertexShader*			m_pVShader;
	ID3D11PixelShader*			m_pPShader;
	ID3D11InputLayout*			m_pInputLayout;
	ID3D11Buffer*				m_pConstantBuffer;
	ID3D11Buffer*				m_pShinyBuffer;
	ID3D11Buffer*				m_pDissolveBuffer;

	ID3D11ShaderResourceView*	m_pTexture;
	ID3D11SamplerState*			m_pSampler;

	LightManager*				m_lights;

	float						m_boundingSphereCentreX, m_boundingSphereCentreY, m_boundingSphereCentreZ, m_boundingSphereRadius, m_defRadius;
	xyz							m_boundingBoxCentre, m_boundingBoxSize;
	void						CalculateModelCentrePoint();
	void						CalculateBoudingSphereRadius();

	void						CalculateBoundingBox();
	
	ModelType					m_type;
	CollisionType				m_collisionType;

	//Used for a dissolve model
	ID3D11ShaderResourceView*	m_pDissolveTexture;
	float						m_dissolveAmount;
	XMVECTOR					m_dissolveColor;
public:
	Model(ID3D11Device* device, ID3D11DeviceContext* deviceContext, LightManager* lights);
	~Model();
	HRESULT LoadObjModel(char* filename);
	void Draw(XMMATRIX* world, XMMATRIX* view, XMMATRIX* projection);

	//Texture creation
	void SetTexture(ID3D11ShaderResourceView* texture) { m_pTexture = texture; }
	void SetSampler(ID3D11SamplerState* sampler) { m_pSampler = sampler; }

	//Used for dissolve shaders;
	void SetDissolveTexture(ID3D11ShaderResourceView* texture) { m_pDissolveTexture = texture; }
	void SetDissolveColor(float r, float g, float b, float a) { m_dissolveColor = XMVectorSet(r, g, b, a); }
	void SetDissolveAmount(float val);
	float GetDissolveAmount() { return m_dissolveAmount; }

	//Custom Shader
	HRESULT LoadDefaultShaders();
	HRESULT LoadCustomShader(char* fileName, char* vertexShaderFunction, char* pixelShaderFunction);


	float GetBoundingSphereRadius();


	float GetBoundingSphereX(void) { return m_boundingSphereCentreX; }
	float GetBoundingSphereY(void) { return m_boundingSphereCentreY; }
	float GetBoundingSphereZ(void) { return m_boundingSphereCentreZ; }

	void ChangeModelType(ModelType newType) { m_type = newType; }
	ObjFileModel* GetModel(void) { return m_pObject; }

	void SetCollisionType(CollisionType newType);

	CollisionType GetCollisionType() { return m_collisionType; }

	xyz GetBoundingBoxSize() { return m_boundingBoxSize; }
	xyz GetBoundingBoxCentre() { return m_boundingBoxCentre; }

};

