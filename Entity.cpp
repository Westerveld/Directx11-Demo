#include "Entity.h"



Entity::Entity(Scene_Node* myNode)
{
	m_node = myNode;
}


Entity::~Entity()
{
}

void Entity::Update(Scene_Node* rootNode, float delta)
{
	//Update our position based on gravity if we arent touching the ground

	XMMATRIX identity = XMMatrixIdentity();
	rootNode->UpdateCollisionTree(&identity, 1.0f);

	if (m_node->CheckCollision(rootNode))
	{
		m_touchingGround = true;
	}
	else
	{
		m_position = maths::AddXYZ(&m_position, &maths::ScaleXYZ(&m_gravity, delta * 5));
		UpdateNodePosition();
		m_touchingGround = false;
	}
}

void Entity::Jump(float jumpValue)
{
	if (m_touchingGround)
	{
		m_position.y += jumpValue;
		UpdateNodePosition();
		m_touchingGround = false;
	}
}

void Entity::Move(float x, float z)
{
	m_position.x += x;
	m_position.z += z;
	UpdateNodePosition();
}

void Entity::UpdateNodePosition()
{
	m_node->SetXPos(m_position.x);
	m_node->SetYPos(m_position.y);
	m_node->SetZPos(m_position.z);
}

void Entity::UpdateNodeRotation()
{
	m_node->SetXAngle(m_rotation.x);
	m_node->SetYAngle(m_rotation.y);
	m_node->SetZAngle(m_rotation.z);
}

void Entity::SetPosition(float x, float y, float z)
{
	m_position.x = x;
	m_position.y = y;
	m_position.z = z;
}

void Entity::SetGravity(float x, float y, float z)
{
	m_gravity.x = x;
	m_gravity.y = y;
	m_gravity.z = z;
}