#include "LightManager.h"



LightManager::LightManager()
{
	m_ambLightCol = XMVectorSet(0.15f, 0.15f, 0.15f, 0.0f);
	m_dirLightCol = XMVectorSet(0.95f, 0.95f, 0.95f, 0.0f);
	m_dirLightPos = XMVectorSet(10.0f, 2.0f, -5.0f, 0.0f);
	m_ambLightDir = XMVectorSet(0.0f, 0.0f, 4.0f, 0.0f);
}


LightManager::~LightManager()
{
}
