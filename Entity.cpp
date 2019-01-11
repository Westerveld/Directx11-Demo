#include "Entity.h"



Entity::Entity(Scene_Node* myNode)
{
	m_node = myNode;
	SetVelocity(0.0f, 0.0f, 0.0f);
	SetDrag(0.0f, 0.0f, 0.0f);
}


Entity::~Entity()
{
}

void Entity::Update(Scene_Node* rootNode, float delta)
{
		
}

void Entity::Jump(float jumpValue)
{
	if (m_node->CheckCollisionRay(m_position.x, m_position.y, m_position.z, 0.0f, -1.0f, 0.0f))
	{
		m_velocity.y = jumpValue;
		m_touchingGround = false;
	}
	
}

void Entity::Move(float x, float z)
{
	m_position.x += x;
	m_position.z += z;
	UpdateNodePosition();
}

#pragma region Sets and updates
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
	UpdateNodePosition();
}

void Entity::SetGravity(float x, float y, float z)
{
	m_gravity.x = x;
	m_gravity.y = y;
	m_gravity.z = z;
}

void Entity::SetRotation(float x, float y, float z)
{
	m_rotation.x = x;
	m_rotation.y = y;
	m_rotation.z = z;
	UpdateNodeRotation();
}

void Entity::SetVelocity(float x, float y, float z)
{
	m_velocity.x = x;
	m_velocity.y = y;
	m_velocity.z = z;
}

void Entity::SetDrag(float x, float y, float z)
{
	m_drag.x = x;
	m_drag.y = y;
	m_drag.z = z;
}
#pragma endregion