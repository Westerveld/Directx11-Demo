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
	XMVECTOR m_dirLightPos;
	XMVECTOR m_dirLightCol;
	XMVECTOR m_ambLightCol;
	XMVECTOR m_ambLightDir;
public:
	LightManager();
	~LightManager();

	XMVECTOR GetDirLightPos(void) { return m_dirLightPos; }
	void SetDirLightPos(XMVECTOR newPos) { m_dirLightPos = newPos; }
	void IncDirLightPos(XMVECTOR inc) { m_dirLightPos = XMVectorSet(m_dirLightPos.x + inc.x, m_dirLightPos.y + inc.y, m_dirLightPos.z + inc.z, 0); }

	XMVECTOR GetDirLightCol(void) { return m_dirLightCol; }
	void SetDirLightCol(XMVECTOR newCol) { m_dirLightCol = newCol; }
	void IncDirLightCol(XMVECTOR inc) { m_dirLightCol = XMVectorSet(m_dirLightCol.x + inc.x, m_dirLightCol.y + inc.y, m_dirLightCol.z + inc.z, 0); }

	XMVECTOR GetAmbLightCol(void) { return m_ambLightCol; }
	void SetAmbLightCol(XMVECTOR newCol) { m_ambLightCol = newCol; }
	void IncAmbLightCol(XMVECTOR inc) { m_ambLightCol = XMVectorSet(m_ambLightCol.x + inc.x, m_ambLightCol.y + inc.y, m_ambLightCol.z + inc.z, 0); }

	XMVECTOR GetAmbLightDir(void) { return m_ambLightDir; }
	void SetAmbLightDir(XMVECTOR newPos) { m_ambLightDir = newPos; }

};

