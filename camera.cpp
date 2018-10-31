#include "camera.h"

Camera::Camera(float x, float y, float z, float camera_rotation)
{
	//Set the position;
	m_x = x;
	m_y = y;
	m_z = z;
	m_position = XMVectorSet(m_x, m_y, m_z, 0.0);
	
	m_dx = sin(camera_rotation);
	m_dy = 0;
	m_dz = cos(camera_rotation);

	m_camera_rotation_yaw = camera_rotation;
	m_camera_rotation_pitch = 0.0;
	m_rotationMatrix = XMMatrixRotationRollPitchYaw(m_camera_rotation_pitch, m_camera_rotation_yaw, 0);


	//Set up the default vectors
	m_dRight = XMVectorSet(1, 0, 0, 0);
	m_dUp = XMVectorSet(0, 1, 0, 0);
	m_dForward = XMVectorSet(0, 0, 1, 0);
	m_up = m_dUp;

	m_camType = CameraType::FirstPerson;
}


Camera::~Camera()
{
}

//Rotate the camera on the y axis to rotate left and right
void Camera::RotateCamera(float yawDegrees, float pitchDegrees)
{
	m_camera_rotation_yaw += (yawDegrees * (XM_PI / 180.0)); //Radify the Angles
	m_camera_rotation_pitch += (pitchDegrees* (XM_PI / 180.0)); //Radify the angles
	
	
	switch (m_camType)
	{
		case CameraType::FirstPerson:
			//Clamp the pitch to stop the camera moving up and down by holding one key
			if (m_camera_rotation_pitch < (-90.0f * (XM_PI / 180.0)))
				m_camera_rotation_pitch = (-90.0f* (XM_PI / 180.0));
			if (m_camera_rotation_pitch > (90.0f * (XM_PI / 180.0)))
				m_camera_rotation_pitch = (90.0f* (XM_PI / 180.0));
			break;
		case CameraType::FreeLook:
			//Allow the camera to fully rotate 360 degrees by changing the value to a positive if it reaches the limits. This helps prevent gimble lock
			if (m_camera_rotation_pitch < (-180.0f * (XM_PI / 180.0)))
				m_camera_rotation_pitch = (180.0f * (XM_PI / 180.0));
			if (m_camera_rotation_pitch > (180.0f * (XM_PI / 180.0)))
				m_camera_rotation_pitch = (-180.0f * (XM_PI / 180.0));
			break;
		case CameraType::ThirdPerson:
			break;
	}
}


//Move the camera in the direction of distance
void Camera::Forward(float distance)
{
	switch (m_camType)
	{
	case CameraType::FirstPerson:
		m_x += distance * m_dx;	
		m_z += distance * m_dz;
		break;
	case CameraType::FreeLook:
		m_position = XMVectorSet(
			XMVectorGetX(m_position) + (XMVectorGetX(m_forward) * distance),	// new x
			XMVectorGetY(m_position) + (XMVectorGetY(m_forward) * distance),	// new y 
			XMVectorGetZ(m_position) + (XMVectorGetZ(m_forward) * distance),	// new z
			0);																	// ignore w
		break;
	case CameraType::ThirdPerson:
		break;
	}
}

//Move the camera up on the y axis
void Camera::Up(float distance)
{
	switch (m_camType)
	{
	case CameraType::FirstPerson:
		m_y += distance;
		break;
	case CameraType::FreeLook:
		m_position = XMVectorSet(
			XMVectorGetX(m_position) + (XMVectorGetX(m_up) * distance),	// new x
			XMVectorGetY(m_position) + (XMVectorGetY(m_up) * distance),	// new y 
			XMVectorGetZ(m_position) + (XMVectorGetZ(m_up) * distance),	// new z
			0);
		break;
	case CameraType::ThirdPerson:
		break;
	}
}

//Strafes the camera left or right
void Camera::Strafe(float distance)
{
	switch (m_camType)
	{
	case CameraType::FirstPerson:
		m_x += XMVectorGetX(m_right) * -distance;
		m_z += XMVectorGetZ(m_right) * -distance;

		break;
	case CameraType::FreeLook:
		m_position = XMVectorSet(
			XMVectorGetX(m_position) + (XMVectorGetX(m_right) * distance),	// new x
			XMVectorGetY(m_position) + (XMVectorGetY(m_right) * distance),	// new y
			XMVectorGetZ(m_position) + (XMVectorGetZ(m_right) * distance),	// new Z
			0);																// w is ignored
		break;
	case CameraType::ThirdPerson:
		break;
	}
}

//Set up the view matrix using the position, look at and up
XMMATRIX Camera::GetViewMatrix()
{
	return XMMatrixLookAtLH(m_position, m_target, m_up);
}

void Camera::Update()
{

	m_rotationMatrix = XMMatrixRotationRollPitchYaw(m_camera_rotation_pitch, m_camera_rotation_yaw, 0);

	m_dx = sin(m_camera_rotation_yaw);
	m_dy = -sin(m_camera_rotation_pitch);
	m_dz = cos(m_camera_rotation_yaw);

	if(m_camType == CameraType::FirstPerson)
	{
		
		m_position = XMVectorSet(m_x, m_y, m_z, 0.0);
		m_target = XMVectorSet(m_x + m_dx, m_y + m_dy, m_z + m_dz, 0);
		m_up = m_dUp;

		m_forward = XMVector3Normalize(m_target - m_position);
		m_right = XMVector3Cross(m_forward, m_up);
	}
	else if (m_camType == CameraType::FreeLook)
	{

		m_target = XMVector3TransformCoord(m_dForward, m_rotationMatrix);
		m_target = XMVector3Normalize(m_target);
		m_right = XMVector3TransformCoord(m_dRight, m_rotationMatrix);
		m_up = XMVector3TransformCoord(m_dUp, m_rotationMatrix);
		m_forward = XMVector3TransformCoord(m_dForward, m_rotationMatrix);
		m_target = m_position + m_target;
	}
	else if (m_camType == CameraType::ThirdPerson)
	{

	}
	

}