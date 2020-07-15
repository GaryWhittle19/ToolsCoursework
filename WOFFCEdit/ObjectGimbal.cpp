#include "ObjectGimbal.h"

ObjectGimbal::ObjectGimbal() {
	// Initialize the gimbal axis handles
	x.Extents = DirectX::XMFLOAT3(4.0f, 0.5f, 0.5f);
	x.Center = position + DirectX::XMFLOAT3(4.0f, 0.0f, 0.0f);
	y.Extents = DirectX::XMFLOAT3(0.5f, 4.0f, 0.5f);
	y.Center = position + DirectX::XMFLOAT3(0.0f, 4.0f, 0.0f);
	z.Extents = DirectX::XMFLOAT3(0.5f, 0.5f, 4.0f);
	z.Center = position + DirectX::XMFLOAT3(0.0f, 0.0f, 4.0f);
};

void ObjectGimbal::SetPosition(DirectX::SimpleMath::Vector3 newpos)
{
	// Set the new position and axis handle positions
	position = newpos;
	x.Center = newpos + DirectX::XMFLOAT3(4.0f, 0.0f, 0.0f);
	y.Center = newpos + DirectX::XMFLOAT3(0.0f, 4.0f, 0.0f);
	z.Center = newpos + DirectX::XMFLOAT3(0.0f, 0.0f, 4.0f);
	chosen_axis.position = position;
}

void ObjectGimbal::SetAxisBoundingBoxRefs(DirectX::BoundingBox &x_ref, DirectX::BoundingBox &y_ref, DirectX::BoundingBox &z_ref)
{
	// Set the external references to gimbal axes bounding boxes
	x_ref = x;
	y_ref = y;
	z_ref = z;
}

void ObjectGimbal::MoveWithObject(DisplayObject* selectedObject, DirectX::SimpleMath::Vector2 delta, DirectX::SimpleMath::Vector2 projected_vector, float distance)
{
	// Create a movement multiplier using the camera to object distance - we only want small translations up close/larger when far away. 
	float movement_factor = Toolbox::MappedClamp(distance, 1.0f, 100.0f, 0.05f, 2.5f);

	// Use the dot product to determine drag direction, translate object using the chosen axis
	if (DirectX::XMVector2Dot(delta, projected_vector).m128_f32[0] < 0.0f) {
		
		selectedObject->m_position -= chosen_axis.direction * movement_factor;
	}
	else if (DirectX::XMVector2Dot(delta, projected_vector).m128_f32[0] > 0.0f) {
		selectedObject->m_position += chosen_axis.direction * movement_factor;
	}

	// Set the position of the gimbal
	SetPosition(selectedObject->m_position);
}

void ObjectGimbal::SetChosenAxis(char axis)
{
	switch (axis) {
	case 'x':
		chosen_axis = DirectX::SimpleMath::Ray(position, DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f));
		break;
	case 'y':
		chosen_axis = DirectX::SimpleMath::Ray(position, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
		break;
	case 'z':
		chosen_axis = DirectX::SimpleMath::Ray(position, DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f));
		break;
	}
}

void ObjectGimbal::SetChosenAxis(DirectX::SimpleMath::Ray axis_backup)
{
	chosen_axis = axis_backup;
}