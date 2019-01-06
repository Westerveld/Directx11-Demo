#pragma once
#include "Entity.h"
class Player :	
	public Entity
{
private:
	Camera*			m_Cam;
	float			m_speed;
	xyz				m_forward, m_right;

	void			CalculateForwardVector();
	void			CalculateRightVector();
public:
	Player(Scene_Node* myNode, Camera* cam, float speed);
	~Player();

	void Update(Scene_Node* rootNode, float delta);
	void MoveForward(float distance);
	void MoveRight(float distance);
};

