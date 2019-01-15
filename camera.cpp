#include "Camera.h"

Camera::Camera(float x, float y, float z, float camera_rotation, float minDistance, float maxDistance)
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

	//Set up our distance to target min and max
	m_minDistance = minDistance;
	m_maxDistance = maxDistance;


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
	m_camera_rotation_yaw += (yawDegrees); 
	m_camera_rotation_pitch += (pitchDegrees); 
	
	if (m_camera_rotation_yaw > 360.0f)
		m_camera_rotation_yaw = -360.0f;
	if (m_camera_rotation_yaw < -360.0f)
		m_camera_rotation_yaw = 360.0f;
	
	switch (m_camType)
	{
		case FirstPerson:
			//Clamp the pitch 
			if (m_camera_rotation_pitch < -55.0f)
				m_camera_rotation_pitch = -55.0f;
			if (m_camera_rotation_pitch > 55.0f )
				m_camera_rotation_pitch = 55.0f;
			break;
		case FreeLook:
			//Allow the camera to fully rotate 360 degrees by changing the value to a positive if it reaches the limits. This helps prevent gimble lock
			if (m_camera_rotation_pitch < -180.0f )
				m_camera_rotation_pitch = 179.9f ;
			if (m_camera_rotation_pitch > 180.0f)
				m_camera_rotation_pitch = -179.9f;
			break;
		case ThirdPerson:
			//Clamp the camera
			if (m_camera_rotation_pitch < -75.0f)
				m_camera_rotation_pitch = -75.0f;
			if (m_camera_rotation_pitch > 75.0f)
				m_camera_rotation_pitch = 75.0f;
			break;
		case TopDown:
			//Prevent pitch rotation
			m_camera_rotation_pitch = 0;
			break;
	}
}


//Move the camera in the direction of distance
void Camera::Forward(float distance)
{
	switch (m_camType)
	{
	case FirstPerson:
		m_x += distance * m_dx;
		m_z += distance * m_dz;
		break;
	case FreeLook:
		m_x += (XMVectorGetX(m_forward) * distance);
		m_y += (XMVectorGetY(m_forward) * distance);
		m_z += (XMVectorGetZ(m_forward) * distance);
		break;
	case ThirdPerson:
		//Handled in player class
		break;
	case TopDown:
		//Handled in player class
		break;
	}
}

//Move the camera up on the y axis
void Camera::Up(float distance)
{
	switch (m_camType)
	{
	case FirstPerson:
		m_y += distance;
		break;
	case FreeLook:
		m_x += (XMVectorGetX(m_up) * distance);	// new x
		m_y += (XMVectorGetY(m_up) * distance);	// new y 
		m_z += (XMVectorGetZ(m_up) * distance);	// new z
		break;
	case ThirdPerson:
		//Handled in player class
		break;
	case TopDown:
		//Handled in player class
		break;
	}
}

//Strafes the camera left or right
void Camera::Strafe(float distance)
{
	switch (m_camType)
	{
	case FirstPerson:
		m_x += XMVectorGetX(m_right) * -distance;
		m_z += XMVectorGetZ(m_right) * -distance;
		break;
	case FreeLook:
		m_x += (XMVectorGetX(m_right) * distance);
		m_y += (XMVectorGetY(m_right) * distance);
		m_z += (XMVectorGetZ(m_right) * distance);
		break;
	case ThirdPerson:
		//Handled in player class
		break;
	case TopDown:
		//Handled in player class
		break;
	}
}

//Set up the view matrix using the position, look at and up
XMMATRIX Camera::GetViewMatrix()
{
	if (m_camType == TopDown)
		return XMMatrixLookAtLH(m_position, m_target, m_forward);

	return XMMatrixLookAtLH(m_position, m_target, m_up);
}

void Camera::Update()
{

	switch (m_camType)
	{
	case FirstPerson:
		{
			//Calculate the delta to our look at destination
			m_dx = sin(XMConvertToRadians(m_camera_rotation_yaw));
			m_dy = -sin(XMConvertToRadians(m_camera_rotation_pitch));
			m_dz = cos(XMConvertToRadians(m_camera_rotation_yaw));

			//Set our position
			m_position = XMVectorSet(m_x, m_y, m_z, 0.0);
			//Set our target
			m_target = XMVectorSet(m_x + m_dx, m_y + m_dy, m_z + m_dz, 0);
			//Use the default up
			m_up = m_dUp;
			//Calculate our forward and right vectors
			m_forward = XMVector3Normalize(m_target - m_position);
			m_right = XMVector3Cross(m_forward, m_up);
		}
		break;
	case FreeLook:
		{
			//Set up rotation matrix
			m_rotationMatrix = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_camera_rotation_pitch), XMConvertToRadians(m_camera_rotation_yaw), 0);

			//Set camera position
			m_position = XMVectorSet(m_x, m_y, m_z, 0.0);
			//Set cameras target
			m_target = XMVector3TransformCoord(m_dForward, m_rotationMatrix);
			m_target = XMVector3Normalize(m_target);
			//Determine our right vector
			m_right = XMVector3TransformCoord(m_dRight, m_rotationMatrix);
			//Determine our up vector
			m_up = XMVector3TransformCoord(m_dUp, m_rotationMatrix);
			//Calculate the forward
			m_forward = XMVector3TransformCoord(m_dForward, m_rotationMatrix);
			//Finalise our targets position
			m_target = m_position + m_target;

		}
		break;
	case ThirdPerson:
		{
			//Get the targets position
			m_target = XMVectorSet(m_followTarget->GetXPos(), m_followTarget->GetYPos(), m_followTarget->GetZPos(), 0);

			//Move up the model a bit
			m_target = XMVectorSetY(m_target, XMVectorGetY(m_target));
			//Setup rotation matrix
			m_rotationMatrix = XMMatrixRotationRollPitchYaw(XMConvertToRadians(-m_camera_rotation_pitch), XMConvertToRadians(m_camera_rotation_yaw), 0);
			m_position = XMVector3TransformNormal(m_dForward, m_rotationMatrix);
			m_position = XMVector3Normalize(m_position);

			//Set our position of the camera
			m_position = (m_position * m_followDistance) + m_target;

			m_x = XMVectorGetX(m_position);
			m_y = XMVectorGetY(m_position);
			m_z = XMVectorGetZ(m_position);

			//Calculate our foward vector
			m_forward = XMVector3Normalize(m_target - m_position);
			//Set our forward to 0 so it lays on the xz plane
			m_forward = XMVectorSetY(m_forward, 0.0f);
			
			//Normalize the vector
			m_forward = XMVector3Normalize(m_forward);

			//Calculate our right vector
			m_right = XMVectorSet(-XMVectorGetZ(m_forward), 0.0f, XMVectorGetX(m_forward), 0.0f);

			//Calculate our up vector
			m_up = XMVector3Cross(XMVector3Normalize(m_position - m_target), m_right);
		}
		break;
	case TopDown:
		{
			//Get our target
			m_target = XMVectorSet(m_followTarget->GetXPos(), m_followTarget->GetYPos(), m_followTarget->GetZPos(), 0);
			//No rotations for top down
			m_rotationMatrix = XMMatrixRotationY(0);
			
			m_position = XMVector3TransformNormal(m_dForward, m_rotationMatrix);
			m_position = XMVector3Normalize(m_position);
			//Set our position
			m_position = XMVectorSet(XMVectorGetX(m_target), XMVectorGetY(m_target) + m_followDistance, XMVectorGetZ(m_target), 0);
			//Update the local variables
			m_x = XMVectorGetX(m_position);
			m_y = XMVectorGetY(m_position);
			m_z = XMVectorGetZ(m_position);

			//Default forward vector
			m_forward = m_dForward;
			//Default right vector
			m_up = m_dUp;
			//SEt up the right vector
			m_right = XMVectorSet(-XMVectorGetZ(m_forward), 0.0f, XMVectorGetX(m_forward), 0.0f);
		}
		break;
	}
}

void Camera::SetPosition(float x, float y, float z)
{
	m_x = x;
	m_y = y;
	m_z = z;
}

//Used for text display to UI
string Camera::GetCameraTypeString()
{
	string s;
	switch (m_camType)
	{
	case FirstPerson:
		s = "First Person";
		break;
	case FreeLook:
		s = "Free Look";
		break;
	case ThirdPerson:
		s = "Third Person";
		break;
	case TopDown:
		s = "Top Down";
		break;

	}
	return s;
}