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
}

void Movable::Move(Scene_Node* rootNode, float delta)
{
	Scene_Node* collidingNode = m_node->CheckCollisionSN(rootNode);

	if (collidingNode->GetName() == "Player")
	{
		xyz otherPos = maths::SetXYZ(collidingNode->GetXPos(), collidingNode->GetYPos(), collidingNode->GetZPos());
		xyz dir = maths::SubtractXYZ(&otherPos, &m_position);
		
		m_position.x += dir.x * delta;
		m_position.z += dir.z * delta;
		UpdateNodePosition();

		XMMATRIX identity = XMMatrixIdentity();
		rootNode->UpdateCollisionTree(&identity, 1.0f);

		if (m_node->CheckCollision(rootNode))
		{
			m_position.x -= dir.x * delta;
			m_position.z -= dir.z * delta;
			UpdateNodePosition();
		}
	}

}