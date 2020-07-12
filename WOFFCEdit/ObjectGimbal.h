#pragma once

#include "pch.h"
#include "DebugDraw.h"

class ObjectGimbal
{
public:
	ObjectGimbal();

	void SetPosition(DirectX::SimpleMath::Vector3 newpos);
	DirectX::SimpleMath::Vector3 const GetPosition() const { return position; };
	int const GetSize() const { return size; };
	bool CheckCollision(DirectX::SimpleMath::Ray raycast);
	bool const GetActive() const { return active; };
	void SetActive(bool is_active) { active = is_active; };
	void Draw();

	void SetAxisBoundingBoxRefs(DirectX::BoundingBox& x_ref, DirectX::BoundingBox& y_ref, DirectX::BoundingBox& z_ref);
	//~ObjectGimbal() {};

private:
	DirectX::SimpleMath::Vector3 position = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	int size = 2;
	bool active = true;

	DirectX::BoundingBox x;
	DirectX::BoundingBox y;
	DirectX::BoundingBox z;
};

