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
	XMVECTOR dirLightPos;
	XMVECTOR dirLightCol;
	XMVECTOR ambLightCol;
public:
	LightManager();
	~LightManager();

	XMVECTOR GetDirLightPos() { return dirLightPos; }
	void SetDirLightPos(XMVECTOR newPos) { dirLightPos = newPos; }
	void IncDirLightPos(XMVECTOR inc) { dirLightPos = XMVectorSet(dirLightPos.x + inc.x, dirLightPos.y + inc.y, dirLightPos.z + inc.z, 0); }

	XMVECTOR GetDirLightCol() { return dirLightCol; }
	void SetDirLightCol(XMVECTOR newCol) { dirLightCol = newCol; }
	void IncDirLightCol(XMVECTOR inc) { dirLightCol = XMVectorSet(dirLightCol.x + inc.x, dirLightCol.y + inc.y, dirLightCol.z + inc.z, 0); }

	XMVECTOR GetAmbLightCol() { return ambLightCol; }
	void SetAmbLightCol(XMVECTOR newCol) { ambLightCol = newCol; }
	void IncAmbLightCol(XMVECTOR inc) { ambLightCol = XMVectorSet(ambLightCol.x + inc.x, ambLightCol.y + inc.y, ambLightCol.z + inc.z, 0); }
};

