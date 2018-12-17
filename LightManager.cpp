#include "LightManager.h"



LightManager::LightManager()
{
	ambLightCol = XMVectorSet(0.15f, 0.15f, 0.15f, 0.0f);
	dirLightCol = XMVectorSet(0.75f, 0.75f, 0.75f, 0.0f);
	dirLightPos = XMVectorSet(0.0f, 1.0f, -1.0f, 0.0f);

}


LightManager::~LightManager()
{
}
