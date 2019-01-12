#pragma once
#include "Entity.h"
class Movable :
	public Entity
{
private:
	bool		m_pushed;
	float		m_pushTimer;

public:
	
	Movable(Scene_Node* myNode);
	~Movable();
	void Update(Scene_Node* rootNode, float delta);
	void Move(Scene_Node* rootNode, float delta);
	bool CheckWallCollision(Scene_Node* rootNode, float delta);
};

