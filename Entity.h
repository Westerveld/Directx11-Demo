#pragma once
#include "main.h"
class Entity
{
protected:
	xyz			m_gravity, m_position, m_rotation, m_velocity, m_drag;

	bool		m_touchingGround;
	Scene_Node* m_node;

	void		UpdateNodePosition();
	void		UpdateNodeRotation();
public:
	Entity(Scene_Node* myNode);
	~Entity();

	void Update(Scene_Node* rootNode, float delta);
	void Jump(float jumpValue);
	void Move(float x, float z);

#pragma region Getter and Setters
	xyz GetPosition() { return m_position; }
	void SetPosition(float x, float y, float z);

	xyz GetGravity() { return m_gravity; }
	void SetGravity(float x, float y,  float z);

	xyz GetRotation() { return m_rotation; }
	void SetRotation(float x, float y, float z);

	void SetTouchingGround(bool val) { m_touchingGround = val; }
	bool GetTouchingGround() { return m_touchingGround; }

	xyz GetVeloicty() { return m_velocity; }
	void SetVelocity(float x, float y, float z);

	xyz GetDrag() { return m_drag; }
	void SetDrag(float x, float y, float z);
#pragma endregion
	
};

