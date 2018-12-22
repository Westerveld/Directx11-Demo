#pragma once
#include "d3d11.h"
#include "xnamath.h"
#include "math.h"

enum CameraType{FirstPerson, FreeLook, ThirdPerson};

class Camera
{
private:
	float m_x, m_y, m_z, m_dx, m_dy, m_dz, m_camera_rotation_yaw, m_camera_rotation_pitch;
	XMVECTOR m_position, m_target, m_up, m_forward, m_right, m_dForward, m_dRight, m_dUp;

	XMMATRIX m_rotationMatrix;

	CameraType m_camType;
public:
	Camera(float x, float y, float z, float camera_rotation);
	~Camera();
	void RotateCamera(float yawDegrees, float pitchDegrees);
	void Forward(float distance);
	void Up(float distance);
	void Strafe(float distance);
	void Update(void);
	XMMATRIX GetViewMatrix();
	void ChangeCameraType(CameraType newType) { m_camType = newType; }

	float GetX() { return m_x; }
	float GetY() { return m_y; }
	float GetZ() { return m_z; }

	float GetdX() { return m_dx; }
	float GetdY() { return m_dy; }
	float GetdZ() { return m_dz; }

	XMFLOAT3 GetPosition() { return XMFLOAT3(m_x, m_y, m_z); }

};

