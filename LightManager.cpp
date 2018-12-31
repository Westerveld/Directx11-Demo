#include "LightManager.h"



LightManager::LightManager()
{
	ambLightCol = XMVectorSet(0.15f, 0.15f, 0.15f, 0.0f);
	dirLightCol = XMVectorSet(0.95f, 0.95f, 0.95f, 0.0f);
	dirLightPos = XMVectorSet(10.0f, 2.0f, -5.0f, 0.0f);

}


LightManager::~LightManager()
{
}
