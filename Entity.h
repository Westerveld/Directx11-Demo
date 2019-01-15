#pragma once
#include "main.h"
class Entity
{
protected:
	xyz			m_gravity, m_position, m_rotation, m_velocity, m_drag;

	bool		m_touchingGround;
	Scene_Node* m_node;

	void		UpdateNodePosition(void);
	void		UpdateNodeRotation(void);
public:
	Entity(Scene_Node* myNode);
	~Entity();


#pragma region Getter and Setters
	xyz GetPosition(void) { return m_position; }
	void SetPosition(float x, float y, float z);

	xyz GetGravity(void) { return m_gravity; }
	void SetGravity(float x, float y,  float z);

	xyz GetRotation(void) { return m_rotation; }
	void SetRotation(float x, float y, float z);

	void SetTouchingGround(bool val) { m_touchingGround = val; }
	bool GetTouchingGround(void) { return m_touchingGround; }

	xyz GetVeloicty(void) { return m_velocity; }
	void SetVelocity(float x, float y, float z);

	xyz GetDrag(void) { return m_drag; }
	void SetDrag(float x, float y, float z);
#pragma endregion
	
};