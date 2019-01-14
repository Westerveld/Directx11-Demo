#include "LightManager.h"



LightManager::LightManager()
{
	m_dirLightCol = XMVectorSet(0.75f, 0.75f, 0.95f, 0.0f);
	m_dirLightPos = XMVectorSet(10.0f, 2.0f, -5.0f, 0.0f);

	m_ambLightCol = XMVectorSet(0.1f, 0.1f, 0.1f, 0.0f);
	m_ambLightDir = XMVectorSet(0.0f, 0.0f, 4.0f, 0.0f);

	m_pointLightPos = XMVectorSet(16.0f, 1.0f, 16.0f, 0.0f);
	m_pointLightCol = XMVectorSet(0.9f, 0.2f, 0.2f, 1.0f);
	m_pointLightRange = 20.0f;

	m_spotLightPos = XMVectorSet(32.0f, 2.0f, 16.0f, 0.0f);
	m_spotLightDir = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
	m_spotLightCol = XMVectorSet(0.2f, 0.2f, 0.9f, 1.0f);
	m_spotLightRange = 60.0f;
	m_spotLightInnerCone = 3.0f;
	m_spotLightOuterCone = 20.0f;


}


LightManager::~LightManager()
{
}
