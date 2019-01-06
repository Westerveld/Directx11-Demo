#pragma once
#include "main.h"
class Entity
{
protected:
	xyz			m_gravity, m_position, m_rotation;

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
	void SetRotation(xyz newRot) { m_rotation = newRot; }

	void SetTouchingGround(bool val) { m_touchingGround = val; }
	bool GetTouchingGround() { return m_touchingGround; }
#pragma endregion
	
};

