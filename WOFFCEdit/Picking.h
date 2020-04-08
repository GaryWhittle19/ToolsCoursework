#include "DisplayObject.h"
#include "DisplayChunk.h"

#pragma once
class PickingHandler
{
public:
	// OBJECT PICKING // ----------------------------------
	DirectX::SimpleMath::Ray PerformPicking(	
		int window_x, int window_y, 					
		int mouse_x, int mouse_y,
		DirectX::SimpleMath::Matrix world, 
		DirectX::SimpleMath::Matrix projection, 
		DirectX::SimpleMath::Matrix view,
		float min_depth, float max_depth,
		int& object_ID,
		std::vector<DisplayObject>& display_list,
		DirectX::SimpleMath::Vector3 campos);			// Arguments for objects
	// TERRAIN PICKING // ----------------------------------
	DirectX::SimpleMath::Ray PerformPicking(
		int window_x, int window_y,
		int mouse_x, int mouse_y,
		DirectX::SimpleMath::Matrix world,
		DirectX::SimpleMath::Matrix projection,
		DirectX::SimpleMath::Matrix view,
		float min_depth, float max_depth,
		DisplayChunk& chunk, 
		DirectX::SimpleMath::Vector3 campos,
		int brush_size, int brush_intensity,
		DirectX::SimpleMath::Vector3& originRef);			// Arguments for terrain
};

