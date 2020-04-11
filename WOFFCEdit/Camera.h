#pragma once

#include "pch.h"
#include "InputCommands.h"
#include "SimpleMath.h"
#include "Toolbox.h"

using namespace DirectX::SimpleMath;

class Camera
{
public: // Members
	Camera();
	~Camera();

	void UpdateCameraViewMatrix(InputCommands m_InputCommands, int dx, int dy);
	Matrix GetCameraViewMatrix() { return m_view; };
	Vector3 GetCameraPosition() { return m_camPosition; };
	bool IsCameraMoving() { return m_camMoving; };

private: // Members
	virtual void HandleInput(InputCommands& m_InputCommands);	// Note: can be overriden for bespoke camera (such as a player camera)

private: // Variables
	float m_camMovespeed;
	DirectX::SimpleMath::Vector3		m_camPosition;
	DirectX::SimpleMath::Vector3		m_camOrientation;
	DirectX::SimpleMath::Vector3		m_camLookAt;
	DirectX::SimpleMath::Vector3		m_camLookDirection;
	DirectX::SimpleMath::Vector3		m_camRight;
	Matrix								m_view;
	bool								m_camMoving;
};

