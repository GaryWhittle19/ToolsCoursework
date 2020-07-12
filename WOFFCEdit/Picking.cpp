//
// Picking.cpp
//

#include "pch.h"
#include "Picking.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

DirectX::SimpleMath::Ray PickingHandler::PerformGimbalPicking(
	ObjectGimbal* m_ToolGimbal,
	int window_x, int window_y,
	int mouse_x, int mouse_y,
	DirectX::SimpleMath::Matrix world,
	DirectX::SimpleMath::Matrix projection,
	DirectX::SimpleMath::Matrix view,
	float min_depth, float max_depth,
	DirectX::SimpleMath::Vector3 campos
	)
{
	//// Set up near and far planes of frustum with mouse X and mouse Y passed down from Toolmain. 
	//// They may look the same but note, the difference in Z
	const XMVECTOR nearSource = XMVectorSet(mouse_x, mouse_y, 0.0f, 1.0f);
	const XMVECTOR farSource = XMVectorSet(mouse_x, mouse_y, 1.0f, 1.0f);

	//// PERFORM GIMBAL PICK
	// For storing selected gimbal handle
	DirectX::BoundingBox* selected_axis = nullptr;

	//// Picked and minimum distance will allow us to sort for the nearest gimbal handle
	float pickedDistance;

	XMVECTOR PickingVector;
	// Check all of the x, y, z bounding boxes
	DirectX::BoundingBox gimbal_boxes[3];
	m_ToolGimbal->SetAxisBoundingBoxRefs(gimbal_boxes[0], gimbal_boxes[1], gimbal_boxes[2]);

	for (DirectX::BoundingBox boundingBox : gimbal_boxes)
	{
		// Gimbal is in world space - use default scale, rotate, translate
		DirectX::XMVECTOR scale = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		DirectX::XMVECTOR rotate = Quaternion::Identity;
		DirectX::XMVECTOR translate = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		
		// Create set the matrix of the selected object in the world based on the translation, scale and rotation.
		XMMATRIX local = world * XMMatrixTransformation(g_XMZero, Quaternion::Identity, scale, g_XMZero, rotate, translate);

		// Unproject the points on the near and far plane, with respect to the matrix we just created.
		XMVECTOR nearPoint = XMVector3Unproject(nearSource, 0.0f, 0.0f, window_x, window_y, min_depth, max_depth, projection, view, local);
		XMVECTOR farPoint = XMVector3Unproject(farSource, 0.0f, 0.0f, window_x, window_y, min_depth, max_depth, projection, view, local);

		// Turn the transformed points into our picking vector. 
		PickingVector = farPoint - nearPoint;
		PickingVector = XMVector3Normalize(PickingVector);

		// Check collision
		if (boundingBox.Intersects(nearPoint, PickingVector, pickedDistance)) 
		{
			selected_axis = &boundingBox;
			break;
		}
	}

	// Check if any of the axes were hit
	if (selected_axis) {
		if (selected_axis->Extents.x == 4.0f) {
			Toolbox::LogOutput("x");
			m_ToolGimbal->SetChosenAxis('x');
		}
		else if (selected_axis->Extents.y == 4.0f) {
			Toolbox::LogOutput("y");
			m_ToolGimbal->SetChosenAxis('y');
		}
		else if (selected_axis->Extents.z == 4.0f) {
			Toolbox::LogOutput("z");
			m_ToolGimbal->SetChosenAxis('z');
		}
	}

	// Set PickingRay position and direction correctly to be used in object selection/terrain manipulation
	DirectX::SimpleMath::Ray PickingRay;
	PickingRay.position = campos;
	PickingRay.direction = PickingVector;

	// Return the ray
	return PickingRay;
}

DirectX::SimpleMath::Ray PickingHandler::PerformObjectPicking(
	int window_x, int window_y,
	int mouse_x, int mouse_y,
	DirectX::SimpleMath::Matrix world,
	DirectX::SimpleMath::Matrix projection,
	DirectX::SimpleMath::Matrix view,
	float min_depth, float max_depth,
	int& object_ID,
	std::vector<DisplayObject>& display_list, DirectX::SimpleMath::Vector3 campos)
{
	// Set up near and far planes of frustum with mouse X and mouse Y passed down from Toolmain. 
	// They may look the same but note, the difference in Z
	const XMVECTOR nearSource = XMVectorSet(mouse_x, mouse_y, 0.0f, 1.0f);
	const XMVECTOR farSource = XMVectorSet(mouse_x, mouse_y, 1.0f, 1.0f);

	// PERFORM OBJECT PICK
	// For storing selected object's ID 
	int selectedID = -1;

	// Picked and minimum distance will allow us to sort for the nearest object
	float pickedDistance;
	float minimumDistance = std::numeric_limits<float>::max();

	XMVECTOR PickingVector;
	// Loop through entire display list of objects and pick with each in turn. 
	for (int i = 0; i < display_list.size(); i++)
	{
		// Get the scale factor and translation of the object
		const XMVECTORF32 scale = { display_list.at(i).m_scale.x,		display_list.at(i).m_scale.y,		display_list.at(i).m_scale.z };
		const XMVECTORF32 translate = { display_list.at(i).m_position.x,	display_list.at(i).m_position.y,	display_list.at(i).m_position.z };

		// Convert euler angles into a quaternion for the rotation of the object
		XMVECTOR rotate = Quaternion::CreateFromYawPitchRoll(
			display_list.at(i).m_orientation.y * 3.1415 / 180,
			display_list.at(i).m_orientation.x * 3.1415 / 180,
			display_list.at(i).m_orientation.z * 3.1415 / 180);

		// Create set the matrix of the selected object in the world based on the translation, scale and rotation.
		XMMATRIX local = world * XMMatrixTransformation(g_XMZero, Quaternion::Identity, scale, g_XMZero, rotate, translate);

		// Unproject the points on the near and far plane, with respect to the matrix we just created.
		XMVECTOR nearPoint = XMVector3Unproject(nearSource, 0.0f, 0.0f, window_x, window_y, min_depth, max_depth, projection, view, local);
		XMVECTOR farPoint = XMVector3Unproject(farSource, 0.0f, 0.0f, window_x, window_y, min_depth, max_depth, projection, view, local);

		// Turn the transformed points into our picking vector. 
		PickingVector = farPoint - nearPoint;
		PickingVector = XMVector3Normalize(PickingVector);

		// Loop through mesh list for object
		for (int y = 0; y < display_list.at(i).m_model.get()->meshes.size(); y++)
		{
			// Checking for ray intersection
			if (display_list.at(i).m_model.get()->meshes[y]->boundingBox.Intersects(nearPoint, PickingVector, pickedDistance))
			{
				if (pickedDistance < minimumDistance) {
					minimumDistance = pickedDistance;
					selectedID = i;
				}
			}
		}
	}

	// Set object_ID ref from ToolMain.cpp to selectedID
	object_ID = selectedID;

	// Set PickingRay position and direction correctly to be used in object selection/terrain manipulation
	DirectX::SimpleMath::Ray PickingRay;
	PickingRay.position = campos;
	PickingRay.direction = PickingVector;

	// Return the ray
	return PickingRay;
}

DirectX::SimpleMath::Ray PickingHandler::PerformTerrainPicking(
	int window_x, int window_y,
	int mouse_x, int mouse_y,
	DirectX::SimpleMath::Matrix world,
	DirectX::SimpleMath::Matrix projection,
	DirectX::SimpleMath::Matrix view,
	float min_depth, float max_depth,
	DisplayChunk& chunk, DirectX::SimpleMath::Vector3 campos)
{
	// Set up near and far planes of frustum with mouse X and mouse Y passed down from Toolmain. 
	// They may look the same but note, the difference in Z
	const XMVECTOR nearSource = XMVectorSet(mouse_x, mouse_y, 0.0f, 1.0f);
	const XMVECTOR farSource = XMVectorSet(mouse_x, mouse_y, 1.0f, 1.0f);

	// PERFORM TERRAIN PICK
	// Terrain is in centre of screen - use default scale, rotate, translate
	DirectX::XMVECTOR scale = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	DirectX::XMVECTOR rotate = Quaternion::Identity;
	DirectX::XMVECTOR translate = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	// Create set the matrix of the selected object in the world based on the translation, scale and rotation.
	XMMATRIX local = world * XMMatrixTransformation(g_XMZero, Quaternion::Identity, scale, g_XMZero, rotate, translate);

	// Unproject the points on the near and far plane, with respect to the matrix we just created.
	const XMVECTOR nearPoint = XMVector3Unproject(nearSource, 0.0f, 0.0f, window_x, window_y, min_depth, max_depth, projection, view, local);
	const XMVECTOR farPoint = XMVector3Unproject(farSource, 0.0f, 0.0f, window_x, window_y, min_depth, max_depth, projection, view, local);

	// Turn the transformed points into our picking vector. 
	XMVECTOR PickingVector = farPoint - nearPoint;
	PickingVector = XMVector3Normalize(PickingVector);

	// Set PickingRay position and direction correctly to be used in object selection/terrain manipulation
	DirectX::SimpleMath::Ray PickingRay;
	PickingRay.position = campos;
	PickingRay.direction = PickingVector;

	// Return the ray
	return PickingRay;
}
