#include "Enemy.h"



Enemy::Enemy(Scene_Node* myNode) : Entity(myNode)
{
}


Enemy::~Enemy()
{
}

void Enemy::Update(Scene_Node* rootNode, float delta)
{


	Entity::Update(rootNode, delta);
}
