#include "Scene_Node.h"



Scene_Node::Scene_Node()
{
	m_model = NULL;

	m_x = 0.0f;
	m_y = 0.0f;
	m_z = 0.0f;

	m_Xangle = 0.0f;
	m_Yangle = 0.0f;
	m_Zangle = 0.0f;
	
	m_scale = 1.0f;
}


Scene_Node::~Scene_Node()
{
	if (m_children.size() > 0)
	{
		for (std::vector<Scene_Node*>::iterator itr = m_children.begin(); itr != m_children.end(); ++itr)
		{
			delete (*itr);
		}
		m_children.clear();
	}

}

void Scene_Node::AddChildNode(Scene_Node* node)
{
	m_children.push_back(node);
}

bool Scene_Node::DetachNode(Scene_Node* node)
{
	for (int i = 0; i < m_children.size(); i++)
	{
		if (node == m_children[i])
		{
			m_children.erase(m_children.begin() + i);
			return true;
		}
		if (m_children[i]->DetachNode(node) == true)return true;
	}
	return false;
}

void Scene_Node::Execute(XMMATRIX* world, XMMATRIX* view, XMMATRIX* projection)
{
	XMMATRIX local_world;

	local_world = XMMatrixIdentity();

	local_world = XMMatrixRotationX(XMConvertToRadians(m_Xangle));
	local_world *= XMMatrixRotationY(XMConvertToRadians(m_Yangle));
	local_world *= XMMatrixRotationZ(XMConvertToRadians(m_Zangle));

	local_world *= XMMatrixScaling(m_scale, m_scale, m_scale);
	
	local_world *= XMMatrixTranslation(m_x, m_y, m_z);

	local_world *= *world;

	if (m_model)
	{
		m_model->Draw(&local_world, view, projection);
	}


	for (int i = 0; i < m_children.size(); i++)
	{
		m_children[i]->Execute(&local_world, view, projection);
	}
}

void Scene_Node::UpdateCollisionTree(XMMATRIX* world, float scale)
{
	XMMATRIX local_world;

	local_world = XMMatrixIdentity();

	local_world = XMMatrixRotationX(XMConvertToRadians(m_Xangle));
	local_world *= XMMatrixRotationY(XMConvertToRadians(m_Yangle));
	local_world *= XMMatrixRotationZ(XMConvertToRadians(m_Zangle));

	local_world *= XMMatrixScaling(m_scale, m_scale, m_scale);

	local_world *= XMMatrixTranslation(m_x, m_y, m_z);

	local_world *= *world;

	m_worldScale = scale * m_scale;

	XMVECTOR v;

	if (m_model)
	{
		v = XMVectorSet(m_model->GetBoundingSphereX(), 
			m_model->GetBoundingSphereY(), 
			m_model->GetBoundingSphereZ(), 0.0);
	}
	else
	{
		v = XMVectorSet(0, 0, 0, 0);
	}

	v = XMVector3Transform(v, local_world);
	m_worldCentreX = XMVectorGetX(v);
	m_worldCentreY = XMVectorGetY(v);
	m_worldCentreZ = XMVectorGetZ(v);

	for (int i = 0; i < m_children.size(); i++)
	{
		m_children[i]->UpdateCollisionTree(&local_world, m_worldScale);
	}
}

bool Scene_Node::CheckCollision(Scene_Node* compareTree)
{
	return CheckCollision(compareTree, this);
}

bool Scene_Node::CheckCollision(Scene_Node* compareTree, Scene_Node* objectTreeRoot)
{
	if (objectTreeRoot == compareTree)
		return false;

	if (m_model && compareTree->m_model)
	{
		XMVECTOR v1 = GetWorldCentre();
		XMVECTOR v2 = compareTree->GetWorldCentre();
		XMVECTOR vDiff = v1 - v2;

		float x1 = XMVectorGetX(v1);
		float y1 = XMVectorGetY(v1);
		float z1 = XMVectorGetZ(v1);
		float x2 = XMVectorGetX(v2);
		float y2 = XMVectorGetY(v2);
		float z2 = XMVectorGetZ(v2);

		float dx = x1 - x2;
		float dy = y1 - y2;
		float dz = z1 - z2;

		if (sqrt(dx*dx + dy * dy + dz * dz) < (compareTree->m_model->GetBoundingSphereRadius() * compareTree->m_worldScale) + (this->m_model->GetBoundingSphereRadius() * m_worldScale))
		{
			return true;
		}
	}

	for (int i = 0; i < compareTree->m_children.size(); i++)
	{
		if (CheckCollision(compareTree->m_children[i], objectTreeRoot) == true)
			return true;
	}

	for (int i = 0; i < m_children.size(); i++)
	{
		if (m_children[i]->CheckCollision(compareTree, objectTreeRoot) == true)
			return true;
	}

	return false;
}

XMVECTOR Scene_Node::GetWorldCentre(void)
{
	return XMVectorSet(m_worldCentreX,
		m_worldCentreY,
		m_worldCentreZ,
		0);
}

#pragma region Movement and Rotation
bool Scene_Node::IncXPos(float value, Scene_Node* rootNode)
{
	float old_x = m_x;			//Store old value;
	m_x += value;				//Increment the value;

	XMMATRIX identity = XMMatrixIdentity();

	//Update the collision tree
	rootNode->UpdateCollisionTree(&identity, 1.0);

	//Check for a collision
	if (CheckCollision(rootNode) == true)
	{
		//if we collide restore  original value;
		m_x = old_x;
		return true;
	}

	return false;
}

bool Scene_Node::IncYPos(float value, Scene_Node* rootNode)
{
	float old_y = m_y;			//Store old value;
	m_y += value;				//Increment the value;

	XMMATRIX identity = XMMatrixIdentity();

	//Update the collision tree
	rootNode->UpdateCollisionTree(&identity, 1.0);

	//Check for a collision
	if (CheckCollision(rootNode) == true)
	{
		//if we collide restore  original value;
		m_y = old_y;
		return true;
	}

	return false;
}

bool Scene_Node::IncZPos(float value, Scene_Node* rootNode)
{
	float old_z = m_z;			//Store old value;
	m_z += value;				//Increment the value;

	XMMATRIX identity = XMMatrixIdentity();

	//Update the collision tree
	rootNode->UpdateCollisionTree(&identity, 1.0);

	//Check for a collision
	if (CheckCollision(rootNode) == true)
	{
		//if we collide restore  original value;
		m_z = old_z;
		return true;
	}

	return false;
}


bool Scene_Node::MoveForward(float value , Scene_Node* rootNode)
{
	float old_x, old_z;

	//Store old values
	old_x = m_x;
	old_z = m_z;

	//Set  new values;
	m_x += sin(m_Yangle * (XM_PI / 180.0)) * value;
	m_z += cos(m_Yangle * (XM_PI / 180.0)) * value;

	XMMATRIX identity = XMMatrixIdentity();

	rootNode->UpdateCollisionTree(&identity, 1.0);

	if (CheckCollision(rootNode) == true)
	{
		//Restore old values if there was a collision
		m_x = old_x;
		m_z = old_z;

		return true;
	}

	return false;
}

bool Scene_Node::MoveForwardXYZ(float value, Scene_Node* rootNode)
{
	float old_x, old_y, old_z;

	//Store old values
	old_x = m_x;
	old_y = m_y;
	old_z = m_z;

	//Set  new values;
	m_x += sin(m_Yangle * (XM_PI / 180.0)) * value;
	m_y += -sin(m_Xangle * (XM_PI / 180.0)) * value;
	m_z += cos(m_Yangle * (XM_PI / 180.0)) * value;

	XMMATRIX identity = XMMatrixIdentity();

	rootNode->UpdateCollisionTree(&identity, 1.0);

	if (CheckCollision(rootNode) == true)
	{
		//Restore old values if there was a collision
		m_x = old_x;
		m_z = old_z;

		return true;
	}

	return false;
}

void Scene_Node::LookAt_XZ(float x, float z)
{
	float dx, dz;
	dx = x - m_x;
	dz = z - m_z;
	m_Yangle = atan2(dx, dz) * (180.0 / XM_PI);
}

void Scene_Node::LookAt_XYZ(float x, float y, float z)
{
	float dx, dy, dz;

	dx = x - m_x;
	dy = y - m_y;
	dz = z - m_z;

	m_Xangle = -atan2(dy, dx - dz) * (180.0 / XM_PI);
	m_Yangle = atan2(dx, dz) * (180.0 / XM_PI);
}
#pragma endregion