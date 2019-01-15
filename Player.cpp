#include "Player.h"

Player::Player(Scene_Node* myNode, Camera* cam, float speed, float defaultY) : Entity(myNode)
{
	m_Cam = cam;
	m_speed = speed;
	m_groundY = defaultY;
}

Player::~Player()
{
}

void Player::Update(float delta)
{
	CalculateForwardVector();
	CalculateRightVector();

	if (m_position.y > m_groundY && !m_jumping)
	{
		m_position.y += m_gravity.y * delta;
	}
	else if (m_position.y < m_groundY && !m_jumping)
	{
		m_position.y = m_groundY;
	}

	if (m_jumping)
	{
		if (m_jumpTimer < 0.25f)
		{
			m_position.y += m_velocity.y *delta;
			m_jumpTimer += delta;
		}
		else
		{
			m_jumping = false;
		}
	}
	UpdateNodePosition();
}

void Player::MoveForward(float distance)
{
	m_position.x += (m_forward.x * (distance * m_speed));
	m_position.z += (m_forward.z * (distance * m_speed));
	UpdateNodePosition();
}

void Player::MoveRight(float distance)
{
	m_position.x += (m_right.x * (distance * m_speed));
	m_position.z += (m_right.z * (distance * m_speed));
	UpdateNodePosition();
}

void Player::Jump()
{
	if (m_position.y == m_groundY)
	{
		m_jumping = true;
		m_jumpTimer = 0.0f;
		m_velocity.y = 10.0f;
	}
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