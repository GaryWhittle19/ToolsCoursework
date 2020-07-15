#pragma once

#include "pch.h"
#include "DebugDraw.h"
#include "DisplayObject.h"
#include "Toolbox.h"

class ObjectGimbal
{
public:
	ObjectGimbal();
	// Setters
	void SetPosition(DirectX::SimpleMath::Vector3 newpos);
	void SetActive(bool is_active) { active = is_active; };
	void SetAxisChar(char axis_name) { current_axis = axis_name; };
	// Getters
	int const GetSize() const { return size; };
	DirectX::SimpleMath::Vector3 const GetPosition() const { return position; };
	bool const GetActive() const { return active; };
	DirectX::SimpleMath::Ray const GetChosenAxis() const { return chosen_axis; };
	char const GetAxisChar() const { return current_axis; };
	// Set external references within other classes using this
	void SetAxisBoundingBoxRefs(DirectX::BoundingBox& x_ref, DirectX::BoundingBox& y_ref, DirectX::BoundingBox& z_ref);
	// Used when dragging the gimbal
	void TransformRelevantObject(DisplayObject* selectedObject, DirectX::SimpleMath::Vector2 delta, DirectX::SimpleMath::Vector2 projected_vector, float distance, int gimbal_mode);
	// Used to set the relevant axis depending on which gimbal axis was picked
	void SetChosenAxis(char axis);
	void SetChosenAxis(DirectX::SimpleMath::Ray axis_backup);
private:
	// Position of the gimbal
	DirectX::SimpleMath::Vector3 position = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	// Size of the gimbal centre sphere
	int size = 1;
	// Bool for gimbal on/off
	bool active = false;
	// Bounding boxes for respective axes
	DirectX::BoundingBox x;
	DirectX::BoundingBox y;
	DirectX::BoundingBox z;
	DirectX::SimpleMath::Ray chosen_axis;
	char current_axis;
};

