#pragma once
#include "objfilemodel.h"
#include "LightManager.h"
#include <list>


struct SHINY_CONSTANT_BUFFER
{
	XMMATRIX WorldView;
};

struct SHINYWORLD_CONSTANT_BUFFER
{
	XMMATRIX WorldViewProjection; //64 bytes
	XMVECTOR dirLightCol; //16 bytes
	XMVECTOR dirLightPos; //16 bytes
	XMVECTOR ambLightCol; //16 bytes
};

class ReflectModel
{
private:
	ID3D11Device*				m_pD3DDevice;
	ID3D11DeviceContext*		m_pImmediateContext;

	ObjFileModel*				m_pObject;
	ID3D11VertexShader*			m_pVShader;
	ID3D11PixelShader*			m_pPShader;
	ID3D11InputLayout*			m_pInputLayout;
	ID3D11Buffer*				m_pShinyBuffer;
	ID3D11Buffer*				m_pWorldBuffer;

	ID3D11ShaderResourceView*	m_pTexture;
	ID3D11SamplerState*			m_pSampler;

	LightManager*				m_lights;

	float						m_x, m_y, m_z;
	float						m_xAngle, m_yAngle, m_zAngle;
	float						m_scale, m_defScale;

	float						m_boundingSphereCentreX, m_boundingSphereCentreY, m_boundingSphereCentreZ, m_boundingSphereRadius, m_defRadius;

	void						CalculateModelCentrePoint();
	void						CalculateBoudingSphereRadius();

	XMMATRIX					GetWorldMatrix();
public:
	ReflectModel(ID3D11Device* device, ID3D11DeviceContext* deviceContext, LightManager* lights);
	~ReflectModel();
	HRESULT LoadObjModel(char* filename, char* textureName);
	void Draw(XMMATRIX* view, XMMATRIX* projection);

	//Texture creation
	HRESULT AddTexture(char* filename);

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

	//Custom Shader
	HRESULT LoadDefaultShaders();
	HRESULT LoadCustomShader(char* fileName, char* vertexShaderFunction, char* pixelShaderFunction);

	//Look at a point
	void LookAt_XZ(float x, float z);
	void LookAt_XYZ(float x, float y, float z);

	void MoveForward(float distance);
	void MoveForwardXYZ(float distance);

	XMVECTOR GetBoundingSphereWorldSpacePosition();

	float GetBoundingSphereRadius();

	bool CheckCollision(std::vector<ReflectModel*> other);
};

