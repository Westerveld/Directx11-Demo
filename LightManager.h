#pragma once
#include <d3d11.h>
#include <d3dx11.h>
#include <dxerr.h>
#include <xnamath.h>
#include <stdio.h>
#include <string>
#include <vector>

class LightManager
{
private:
	//Directional Light
	XMVECTOR m_dirLightPos;
	XMVECTOR m_dirLightCol;

	//Ambient Light
	XMVECTOR m_ambLightCol;
	XMVECTOR m_ambLightDir;

	//Point Light;
	XMVECTOR m_pointLightPos;
	float	m_pointLightRange;
	XMVECTOR m_pointLightCol;

	//Spot Light
	XMVECTOR m_spotLightPos;
	XMVECTOR m_spotLightCol;
	XMVECTOR m_spotLightDir;
	float m_spotLightRange;
	float m_spotLightOuterCone;
	float m_spotLightInnerCone;
public:
	LightManager();
	~LightManager();

#pragma region Directional Getter and Setters
	XMVECTOR GetDirLightPos(void) { return m_dirLightPos; }
	void SetDirLightPos(XMVECTOR newPos) { m_dirLightPos = newPos; }
	
	XMVECTOR GetDirLightCol(void) { return m_dirLightCol; }
	void SetDirLightCol(XMVECTOR newCol) { m_dirLightCol = newCol; }
#pragma endregion

#pragma region Ambient Getter and Setters
	XMVECTOR GetAmbLightCol(void) { return m_ambLightCol; }
	void SetAmbLightCol(XMVECTOR newCol) { m_ambLightCol = newCol; }

	XMVECTOR GetAmbLightDir(void) { return m_ambLightDir; }
	void SetAmbLightDir(XMVECTOR newPos) { m_ambLightDir = newPos; }
#pragma endregion

#pragma region Point Getter and Setters
	XMVECTOR GetPointLightPos(void) { return m_pointLightPos; }
	void SetPointLightPos(XMVECTOR newPos) { m_pointLightPos = newPos; }
	
	XMVECTOR GetPointLightCol(void) { return m_pointLightCol;}
	void SetPointLightCol(XMVECTOR newCol) { m_pointLightCol = newCol; }

	float GetPointLightRange(void) { return m_pointLightRange; }
	void SetPointLightRange(float newVal) { m_pointLightRange = newVal; }
#pragma endregion

#pragma region Spot Getter and Setters
	XMVECTOR GetSpotLightPos(void) { return m_spotLightPos; }
	void SetSpotLightPos(XMVECTOR newPos) { m_spotLightPos = newPos; }

	XMVECTOR GetSpotLightDir(void) { return m_spotLightDir; }
	void SetSpotLightDir(XMVECTOR newDir) { m_spotLightDir = newDir; }

	XMVECTOR GetSpotLightCol(void) { return m_spotLightCol; }
	void SetSpotLightCol(XMVECTOR newCol) { m_spotLightCol = newCol; }

	float GetSpotLightRange(void) { return m_spotLightRange; }
	void SetSpotLightRange(float newRange) { m_spotLightRange = newRange; }

	float GetSpotLightOuterCone(void) { return m_spotLightOuterCone; }
	void SetSpotLightOuterCone(float newCone) { m_spotLightOuterCone = newCone; }

	float GetSpotLightInnerCone(void) { return m_spotLightInnerCone; }
	void SetSpotLightInnerCone(float newCone) { m_spotLightInnerCone = newCone; }
#pragma endregion
};

