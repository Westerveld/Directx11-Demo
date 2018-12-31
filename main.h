#pragma once
//The #include order is important
#include <d3d11.h>
#include <dxgi.h>
#include <d3dx11.h>
#include <windows.h>
#include <dxerr.h>
#include <xnamath.h>

#include <list>
#include <string>
#include <deque>
#include <iostream>
#include <fstream>

#include "camera.h"
#include "text2D.h"
#include "Model.h"
#include "InputHandler.h"
#include "SkyBox.h"
#include "ParticleFactory.h"
#include "Scene_Node.h"
#include "maths.h"
#include "GameManager.h"
#include "TimeHandler.h"

//Define vertex structure
struct POS_COL_TEX_NORM_VERTEX //This will be added to and renamed in future tutorials
{
	XMFLOAT3 pos;
	XMFLOAT4 Col;
	XMFLOAT2 Texture0;
	XMFLOAT3 Normal;
};
