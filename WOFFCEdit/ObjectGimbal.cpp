#include "ObjectGimbal.h"
#include "DirectXHelpers.h"


ObjectGimbal::ObjectGimbal() {
	x.Extents = DirectX::XMFLOAT3(10.0f, 1.0f, 1.0f);
	x.Center = position + DirectX::XMFLOAT3(5.0f, 0.0f, 0.0f);

	y.Extents = DirectX::XMFLOAT3(1.0f, 10.0f, 1.0f);
	y.Center = position + DirectX::XMFLOAT3(0.0f, 5.0f, 0.0f);

	z.Extents = DirectX::XMFLOAT3(1.0f, 1.0f, 10.0f);
	z.Center = position + DirectX::XMFLOAT3(0.0f, 0.0f, 5.0f);
};

void ObjectGimbal::SetPosition(DirectX::SimpleMath::Vector3 newpos)
{
	x.Center = newpos + DirectX::XMFLOAT3(10.0f, 0.0f, 0.0f);
	y.Center = newpos + DirectX::XMFLOAT3(0.0f, 10.0f, 0.0f);
	z.Center = newpos + DirectX::XMFLOAT3(0.0f, 0.0f, 10.0f);
}

bool ObjectGimbal::CheckCollision(DirectX::SimpleMath::Ray raycast)
{
	return 0;
}

void ObjectGimbal::Draw()
{
	
}

void ObjectGimbal::SetAxisBoundingBoxRefs(DirectX::BoundingBox &x_ref, DirectX::BoundingBox &y_ref, DirectX::BoundingBox &z_ref)
{
	x_ref = x;
	y_ref = y;
	z_ref = z;
}