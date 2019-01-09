#pragma once
#include "camera.h"

class MiniMap
{
private:
	ID3D11Device*		m_pD3DDevice;

	Camera*				m_Cam;


public:
	MiniMap();
	~MiniMap();
};

