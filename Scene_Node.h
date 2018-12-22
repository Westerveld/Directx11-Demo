#pragma once
#include "Model.h"

class Scene_Node
{
private:
	Model*					m_model;
	vector<Scene_Node*>		m_children;

	float					m_x, m_y, m_z;
	float					m_Xangle, m_Yangle, m_Zangle;
	float					m_scale, m_worldScale;
	float					m_worldCentreX, m_worldCentreY, m_worldCentreZ;
	XMMATRIX				m_localWorldMatrix;
public:
	Scene_Node();
	~Scene_Node();

	void AddChildNode(Scene_Node* node);
	bool DetachNode(Scene_Node* node);

	void Execute(XMMATRIX* world, XMMATRIX* view, XMMATRIX* projection);

	void UpdateCollisionTree(XMMATRIX* world, float scale);

	bool CheckCollision(Scene_Node* compareTree);
	bool CheckCollision(Scene_Node* compareTree, Scene_Node* objectTreeRoot);


	bool MoveForward(float distance, Scene_Node* rootNode);
	bool MoveForwardXYZ(float distance, Scene_Node* rootNode);

	void LookAt_XZ(float x, float z);
	void LookAt_XYZ(float x, float y, float z);

#pragma region Getters and Setters
	void SetXPos(float value) { m_x = value; }
	float GetXPos(void) { return m_x; }
	bool IncXPos(float value, Scene_Node* rootNode);

	void SetYPos(float value) { m_y = value; }
	float GetYPos(void) { return m_y; }
	bool IncYPos(float value, Scene_Node* rootNode);

	void SetZPos(float value) { m_z = value; }
	float GetZPos(void) { return m_z; }
	bool IncZPos(float value, Scene_Node* rootNode);

	void SetXAngle(float value) { m_Xangle = value; }
	float GetXAngle(void) { return m_Xangle; }

	void SetYAngle(float value) { m_Yangle = value; }
	float GetYAngle(void) { return m_Yangle; }

	void SetZAngle(float value) { m_Zangle = value; }
	float GetZAngle(void) { return m_Zangle; }

	void SetScale(float value) { m_scale = value; }
	float GetScale(void) { return m_scale; }

	void SetModel(Model* model) { m_model = model; }
	

	XMVECTOR GetWorldCentre(void);
#pragma endregion 
};

