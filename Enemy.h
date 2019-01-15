#pragma once
#include "Entity.h"
class Enemy :
	public Entity
{
private:
	std::vector<xyz>	m_waypoints;
	int					m_currentWaypoint;
	float				m_moveSpeed;

	void				Move(float delta);
	void				NextWaypoint(void);
	bool				AtWaypoint(void);
public:
	Enemy(Scene_Node* rootNode, float speed);
	~Enemy();

	void Update(Scene_Node* rootNode, float delta);
	void AddWaypoint(float x, float y, float z);

};