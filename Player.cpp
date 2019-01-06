#include "Player.h"



Player::Player(Scene_Node* myNode, Camera* cam, float speed) : Entity(myNode)
{
	m_Cam = cam;
	m_speed = speed;
}


Player::~Player()
{
}

void Player::Update(Scene_Node* rootNode, float delta)
{
	CalculateForwardVector();
	CalculateRightVector();
	//Entity::Update(rootNode, delta);
}

void Player::MoveForward(float distance)
{
	m_position.x += (m_forward.x * distance) * m_speed;
	m_position.z += (m_forward.z * distance)* m_speed;
	UpdateNodePosition();
}

void Player::MoveRight(float distance)
{
	m_position.x += (m_right.x * distance) * m_speed;
	m_position.z += (m_right.z * distance) * m_speed;

	UpdateNodePosition();
}

void Player::CalculateForwardVector()
{
	xyz camForward = m_Cam->GetForward();
	camForward.y = m_position.y;
	m_forward = maths::SetXYZ(&camForward);
}

void Player::CalculateRightVector()
{
	xyz camRight = m_Cam->GetRight();
	camRight.y = m_position.y;
	m_right = maths::SetXYZ(&camRight);
}