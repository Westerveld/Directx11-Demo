#include "Movable.h"



Movable::Movable(Scene_Node* myNode) : Entity(myNode)
{
}


Movable::~Movable()
{
}


void Movable::Update(Scene_Node* rootNode, float delta)
{
	Move(rootNode, delta);
	if (m_pushed)
	{
		if (m_pushTimer < 0.2f)
		{
			m_pushTimer += delta;
			m_position.x -= m_velocity.x;
			m_position.z -= m_velocity.z;

			UpdateNodePosition();

			XMMATRIX identity = XMMatrixIdentity();
			rootNode->UpdateCollisionTree(&identity, 1.0f);

			if (CheckWallCollision(rootNode, delta))
			{
				m_position.x -= m_velocity.x;
				m_position.z -= m_velocity.z;
			}
		}
		else
		{
			m_pushed = false;
		}
	}
}

void Movable::Move(Scene_Node* rootNode, float delta)
{
	if (m_node->CheckCollisionSN(rootNode) != NULL && !m_pushed)
	{
		Scene_Node* collidingNode = m_node->CheckCollisionSN(rootNode);

		if (collidingNode->GetName() == "Player")
		{
			xyz otherPos = maths::SetXYZ(collidingNode->GetXPos(), collidingNode->GetYPos(), collidingNode->GetZPos());
			xyz dir = maths::SubtractXYZ(&otherPos, &m_position);

			m_velocity.x = dir.x * delta * 10.0f;
			m_velocity.z = dir.z * delta * 10.0f;

			m_pushed = true;
			m_pushTimer = 0.0f;
			/*if (m_node->CheckCollision(rootNode))
			{
				m_position.x -= dir.x * delta;
				m_position.z -= dir.z * delta;
				UpdateNodePosition();
			}*/
		}
		
	}
}

bool Movable::CheckWallCollision(Scene_Node* rootNode, float delta)
{
	if (m_node->CheckCollisionSN(rootNode) != NULL)
	{
		Scene_Node* collidingNode = m_node->CheckCollisionSN(rootNode);
		if (collidingNode->GetName() == "Wall")
		{
			xyz otherPos = maths::SetXYZ(collidingNode->GetXPos(), collidingNode->GetYPos(), collidingNode->GetZPos());
			xyz dir = maths::SubtractXYZ(&otherPos, &m_position);

			m_velocity.x = dir.x * delta * 10.0f;
			m_velocity.z = dir.z * delta * 10.0f;
			return true;
		}
	}
	return false;
}