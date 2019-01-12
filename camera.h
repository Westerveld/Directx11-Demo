#pragma once
#include "d3d11.h"
#include "xnamath.h"
#include "math.h"

#include "Scene_Node.h"

enum CameraType{FirstPerson, FreeLook, ThirdPerson, TopDown};
enum CameraPerspective {Orthographic, Perspective};
class Camera
{
private:
	float m_x, m_y, m_z, m_dx, m_dy, m_dz, m_camera_rotation_yaw, m_camera_rotation_pitch;

	float m_minDistance, m_maxDistance;
	XMVECTOR m_position, m_target, m_up, m_forward, m_right, m_dForward, m_dRight, m_dUp;

	XMMATRIX m_rotationMatrix;

	CameraType			m_camType;
	CameraPerspective	m_camPers;
	//Third person
	class Scene_Node* m_followTarget;
	float m_followDistance;
public:
	Camera(float x, float y, float z, float camera_rotation, float minDistance, float maxDistance);
	~Camera();
	void RotateCamera(float yawDegrees, float pitchDegrees);
	void Forward(float distance);
	void Up(float distance);
	void Strafe(float distance);
	void Update(void);
	XMMATRIX GetViewMatrix();
	void ChangeCameraType(CameraType newType) { m_camType = newType; }
	CameraType GetCameraType(void) { return m_camType; }
	void ChangeCameraPerspective(CameraPerspective newPers) { m_camPers = newPers; }
	CameraPerspective GetCameraPerspective(void) { return m_camPers; }
	//Return positions
	float GetX(void) { return m_x; }
	float GetY(void) { return m_y; }
	float GetZ(void) { return m_z; }

	//Return look at position
	float GetdX(void) { return XMVectorGetX(m_target); }
	float GetdY(void) { return XMVectorGetY(m_target); }
	float GetdZ(void) { return XMVectorGetZ(m_target); }

	xyz GetPosition(void) { return maths::SetXYZ(XMVectorGetX(m_position), XMVectorGetY(m_position), XMVectorGetZ(m_position)); }
	void SetPosition(float x, float y, float z);

	void SetTarget(Scene_Node* target) { m_followTarget = target; }
	void SetFollowDistance(float value) { if(value > m_minDistance && value < m_maxDistance) m_followDistance = value; }
	float GetFollowDistance(void) { return m_followDistance; }
	void SetMinFollow(float value) { m_minDistance = value; }
	void SetMaxFollow(float value) { m_maxDistance = value; }

	xyz GetForward(void)	{ return maths::SetXYZ(XMVectorGetX(m_forward), XMVectorGetY(m_forward), XMVectorGetZ(m_forward)); }
	xyz GetRight(void)		{ return maths::SetXYZ(XMVectorGetX(m_right), XMVectorGetY(m_right), XMVectorGetZ(m_right)); }
	xyz GetUp(void)			{ return maths::SetXYZ(XMVectorGetX(m_up), XMVectorGetY(m_up), XMVectorGetZ(m_up)); }
	xyz GetTarget(void)		{ return maths::SetXYZ(XMVectorGetX(m_target), XMVectorGetY(m_target), XMVectorGetZ(m_target)); }

};

