#include "Enemy.h"

Enemy::Enemy(Scene_Node* myNode, float speed) : Entity(myNode)
{
	m_currentWaypoint = 0;
	m_moveSpeed = speed;
}

Enemy::~Enemy()
{
	//Clean up our list of waypoints
	if (m_waypoints.size() > 0)
	{
		m_waypoints.clear();
	}
}

void Enemy::Update(Scene_Node* rootNode, float delta)
{
	//Check if we are at a waypoint
	if (AtWaypoint())
	{
		//If we are, go to the next waypoint
		NextWaypoint();
	}
	else
	{
		//move towards our current waypoint
		Move(delta);

		//Update the collision tree
		XMMATRIX identity = XMMatrixIdentity();
		rootNode->UpdateCollisionTree(&identity, 1.0f);
		//If we are colliding
		if (m_node->CheckCollision(rootNode))
		{
			//Move slightly away from our current waypoint
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
	//If we are over the size of the waypoints list, set the current waypoint to 0;
	if (m_currentWaypoint >= (int)m_waypoints.size())
	{
		m_currentWaypoint = 0;
	}
}

bool Enemy::AtWaypoint()
{
	float xCompare, zCompare;
	//Compare our x and z positions with the waypoint
	xCompare = m_position.x - m_waypoints[m_currentWaypoint].x;
	zCompare = m_position.z - m_waypoints[m_currentWaypoint].z;
	//If we are within the threshold, return true
	if (xCompare < 0.05f && xCompare > -0.05f && zCompare < 0.05f && zCompare > -0.05f)
	{
		return true;
	}
	return false;
}

void Enemy::Move(float delta)
{
	//Calculate the direction by subtracting the vectors
	xyz dir = maths::SubtractXYZ(&m_waypoints[m_currentWaypoint], &m_position);
	
	//Normalize the vector
	dir = maths::NormaliseXYZ(&dir);

	//Add to our position
	m_position.x += dir.x * m_moveSpeed * delta;
	m_position.z += dir.z * m_moveSpeed * delta;

	UpdateNodePosition();
}