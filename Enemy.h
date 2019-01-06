#pragma once
#include "Entity.h"
class Enemy :
	public Entity
{
public:
	Enemy(Scene_Node* rootNode);
	~Enemy();

	void Update(Scene_Node* rootNode, float delta);
};

