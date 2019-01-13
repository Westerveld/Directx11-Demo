#pragma once
#include "Entity.h"
class Player :	
	public Entity
{
private:
	Camera*			m_Cam;
	float			m_speed;
	xyz				m_forward, m_right, m_input;

	void			CalculateForwardVector(void);
	void			CalculateRightVector(void);

	float			m_groundY;
	float			m_jumpTimer;
	bool			m_jumping;
public:
	Player(Scene_Node* myNode, Camera* cam, float speed, float defaultY);
	~Player();

	void Update(float delta);
	void MoveForward(float distance);
	void MoveRight(float distance);
	void Jump();
};

