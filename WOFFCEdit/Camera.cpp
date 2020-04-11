#include "Camera.h"

Camera::Camera()
{
	// Camera initial values
	m_camMovespeed = 1.0f;
	m_camPosition.x = 0.0f;
	m_camPosition.y = 0.0f;
	m_camPosition.z = 0.0f;
	m_camOrientation.x = 0;
	m_camOrientation.y = 0;
	m_camOrientation.z = 0;
	m_camLookAt.x = 0.0f;
	m_camLookAt.y = 0.0f;
	m_camLookAt.z = 0.0f;
	m_camLookDirection.x = 0.0f;
	m_camLookDirection.y = 0.0f;
	m_camLookDirection.z = 0.0f;
	m_camRight.x = 0.0f;
	m_camRight.y = 0.0f;
	m_camRight.z = 0.0f;
}

Camera::~Camera()
{
}

void Camera::UpdateCameraViewMatrix(InputCommands m_InputCommands, int dx, int dy)
{
	// Process input and update stuff
	HandleInput(m_InputCommands);

	// Camera motion is on a plane so kill the other seven deadly dimensions
	Vector3 planarMotionVector = m_camLookDirection;
	planarMotionVector.y = 0.0;

	// Update the camera orientation based on delta mouse xy
	m_camOrientation.y += dx;
	m_camOrientation.x -= dy;

	// Clamp camera pitch within reasonable values
	m_camOrientation.x = Toolbox::Clamp(m_camOrientation.x, -85.0f, 85.0f);

	// Create look direction from Euler angles in m_camOrientation
	float phi = (m_camOrientation.x) * 3.1415 / 180;
	float theta = (m_camOrientation.y) * 3.1415 / 180;
	m_camLookDirection.x = cos(theta) * cos(phi);
	m_camLookDirection.y = sin(phi);
	m_camLookDirection.z = sin(theta) * cos(phi);
	m_camLookDirection.Normalize();

	// Create right vector from look Direction
	m_camLookDirection.Cross(Vector3::UnitY, m_camRight);

	// Update lookat point
	m_camLookAt = m_camPosition + m_camLookDirection;

	// Apply camera vectors
	m_view = Matrix::CreateLookAt(m_camPosition, m_camLookAt, Vector3::UnitY);
}

void Camera::HandleInput(InputCommands& m_InputCommands)
{
	// Camera sprint
	if (m_InputCommands.sprint) { m_camMovespeed = 2.0f; }
	else m_camMovespeed = 1.0f;
	// Movement in xyz
	if (m_InputCommands.forward) { m_camPosition += m_camLookDirection * m_camMovespeed; }
	if (m_InputCommands.back) { m_camPosition -= m_camLookDirection * m_camMovespeed; }
	if (m_InputCommands.right) { m_camPosition += m_camRight * m_camMovespeed; }
	if (m_InputCommands.left) { m_camPosition -= m_camRight * m_camMovespeed; }
	if (m_InputCommands.up) { m_camPosition.y += m_camMovespeed; }
	if (m_InputCommands.down) { m_camPosition.y -= m_camMovespeed; }
	// Determine if camera is moving
	m_camMoving = (
		m_InputCommands.forward ||
		m_InputCommands.left ||
		m_InputCommands.back ||
		m_InputCommands.right ||
		m_InputCommands.up ||
		m_InputCommands.down);
}