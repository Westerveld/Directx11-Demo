#include "Enemy.h"



Enemy::Enemy(Scene_Node* myNode, float speed) : Entity(myNode)
{
	m_currentWaypoint = 0;
	m_moveSpeed = speed;
}


Enemy::~Enemy()
{
	if (m_waypoints.size() > 0)
	{
		m_waypoints.clear();
	}
}

void Enemy::Update(Scene_Node* rootNode, float delta)
{
	if (AtWaypoint())
	{
		NextWaypoint();
	}
	else
	{
		if (!m_node->CheckCollision(rootNode))
		{
			Move(delta);
		}

		XMMATRIX identity = XMMatrixIdentity();
		rootNode->UpdateCollisionTree(&identity, 1.0f);

		if (m_node->CheckCollision(rootNode))
		{
			Move(-delta);
		}
	}
	
}

void Enemy::AddWaypoint(float x, float y, float z)
{
	xyz newWaypoint = maths::SetXYZ(x, y, z);
	m_waypoints.push_back(newWaypoint);
}

void Enemy::NextWaypoint()
{
	m_currentWaypoint++;
	if (m_currentWaypoint >= m_waypoints.size())
	{
		m_currentWaypoint = 0;
	}
}

bool Enemy::AtWaypoint()
{
	float xCompare, zCompare;
	xCompare = m_position.x - m_waypoints[m_currentWaypoint].x;
	zCompare = m_position.z - m_waypoints[m_currentWaypoint].z;
	if (xCompare < 0.05f && xCompare > -0.05f && zCompare < 0.05f && zCompare > -0.05f)
	{
		return true;
	}
	return false;
}

void Enemy::Move(float delta)
{
	xyz dir = maths::SubtractXYZ(&m_waypoints[m_currentWaypoint], &m_position);
	
	dir = maths::NormaliseXYZ(&dir);

	m_position.x += dir.x * m_moveSpeed * delta;
	m_position.z += dir.z * m_moveSpeed * delta;

	UpdateNodePosition();
}