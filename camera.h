#pragma once
#include "d3d11.h"
#include "xnamath.h"
#include "math.h"

#include "Scene_Node.h"

enum CameraType{FirstPerson, FreeLook, ThirdPerson};

class Camera
{
private:
	float m_x, m_y, m_z, m_dx, m_dy, m_dz, m_camera_rotation_yaw, m_camera_rotation_pitch;
	XMVECTOR m_position, m_target, m_up, m_forward, m_right, m_dForward, m_dRight, m_dUp;

	XMMATRIX m_rotationMatrix;

	CameraType m_camType;
	Scene_Node* m_followTarget;
public:
	Camera(float x, float y, float z, float camera_rotation);
	~Camera();
	void RotateCamera(float yawDegrees, float pitchDegrees);
	void Forward(float distance, Scene_Node* root);
	void Up(float distance);
	void Strafe(float distance, Scene_Node* root);
	void Update(void);
	XMMATRIX GetViewMatrix();
	void ChangeCameraType(CameraType newType) { m_camType = newType; }

	//Return positions
	float GetX() { return m_x; }
	float GetY() { return m_y; }
	float GetZ() { return m_z; }

	//Return look at position
	float GetdX() { return XMVectorGetX(m_target); }
	float GetdY() { return XMVectorGetY(m_target); }
	float GetdZ() { return XMVectorGetZ(m_target); }

	XMFLOAT3 GetPosition() { return XMFLOAT3(m_x, m_y, m_z); }

	void SetTarget(Scene_Node* target) { m_followTarget = target; }
};

