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

	m_localWorldMatrix = XMMatrixIdentity();

	m_localWorldMatrix = XMMatrixRotationX(XMConvertToRadians(m_Xangle));
	m_localWorldMatrix *= XMMatrixRotationY(XMConvertToRadians(m_Yangle));
	m_localWorldMatrix *= XMMatrixRotationZ(XMConvertToRadians(m_Zangle));

	m_localWorldMatrix *= XMMatrixScaling(m_scale, m_scale, m_scale);

	m_localWorldMatrix *= XMMatrixTranslation(m_x, m_y, m_z);

	m_localWorldMatrix *= *world;

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

	v = XMVector3Transform(v, m_localWorldMatrix);
	m_worldCentreX = XMVectorGetX(v);
	m_worldCentreY = XMVectorGetY(v);
	m_worldCentreZ = XMVectorGetZ(v);

	for (int i = 0; i < m_children.size(); i++)
	{
		m_children[i]->UpdateCollisionTree(&m_localWorldMatrix, m_worldScale);
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

bool Scene_Node::CheckCollisionRay(float x, float y, float z, float rx, float ry, float rz)
{
	if (m_model)
	{
		xyz pos, rayStart, rayEnd;

		//Set the xyz positions for our position, the ray start, direction and end
		pos = maths::SetXYZ(m_x, m_y, m_z);
		rayStart = maths::SetXYZ(x, y, z);
		rayEnd = maths::SetXYZ(x + rx, y + ry, z + rz);
		
		//Calculate the distance between us and the ray start
		float distance = maths::Dot(&pos, &rayStart);
		//Calculate the distance between the rayStart and rayEnd
		float rayDistance = maths::Dot(&rayStart, &rayEnd);
		//Add on the radius of the bounding spehere
		rayDistance += m_model->GetBoundingSphereRadius();

		//If the distance is greater than the ray distance, check through the model for a collision
		if (distance > rayDistance)
		{
			//Get the object model file
			ObjFileModel* pObject = m_model->GetModel();
			//Cycle through the vertices
			for (int i = 0; i < pObject->numverts; i+=3)
			{
				//Store the three vertices
				XMVECTOR p1 = XMVectorSet(pObject->vertices[i].Pos.x,
					pObject->vertices[i].Pos.y,
					pObject->vertices[i].Pos.z,
					0.0f);
				XMVECTOR p2 = XMVectorSet(pObject->vertices[i + 1].Pos.x,
					pObject->vertices[i + 1].Pos.y,
					pObject->vertices[i + 1].Pos.z,
					0.0f);
				XMVECTOR p3 = XMVectorSet(pObject->vertices[i + 2].Pos.x,
					pObject->vertices[i + 2].Pos.y,
					pObject->vertices[i + 2].Pos.z,
					0.0f);

				//Transform them to world space
				p1 = XMVector3Transform(p1, m_localWorldMatrix);
				p2 = XMVector3Transform(p2, m_localWorldMatrix);
				p3 = XMVector3Transform(p3, m_localWorldMatrix);

				//Convert to xyz for maths equations
				xyz t1 = maths::SetXYZ(XMVectorGetX(p1), XMVectorGetY(p1), XMVectorGetZ(p1));
				xyz t2 = maths::SetXYZ(XMVectorGetX(p2), XMVectorGetY(p2), XMVectorGetZ(p2));
				xyz t3 = maths::SetXYZ(XMVectorGetX(p3), XMVectorGetY(p3), XMVectorGetZ(p3));

				//Create a plane for the triangle
				Plane tri = maths::CalcPlane(&t1, &t2, &t3);
				float ray1, ray2;
				//Calculate the plane points to see if they are different
				ray1 = maths::CalcPlanePoint(&tri, &rayStart);
				ray2 = maths::CalcPlanePoint(&tri, &rayEnd);
				//Set the floats using Sign to ensure they are either -1, 0 or 1
				ray1 = maths::Sign(ray1);
				ray2 = maths::Sign(ray2);
				//If they are different values
				if (ray1 != ray2)
				{
					bool check1, check2;
					//Check to see if the points are within the triangles
					check1 = maths::InTriangle(&t1, &t2, &t3, &rayStart);
					check2 = maths::InTriangle(&t1, &t2, &t3, &rayEnd);
					
					//if either point is in the triangle, we have a collision
					if (check1 || check2)
					{
						return true;
					}
				}

			}
		}
	}

	for (int i = 0; i < m_children.size(); i++)
	{
		//Check through the children of the node
		if (m_children[i]->CheckCollisionRay(x, y, z, rx, ry, rz))
		{
			return true;
		}
	}
	return false;

}