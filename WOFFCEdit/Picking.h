#include "DisplayObject.h"
#include "DisplayChunk.h"
#include "ObjectGimbal.h"
#include "Toolbox.h"

#pragma once
class PickingHandler
{
public:
	// GIMBAL PICKING // ----------------------------------
	DirectX::SimpleMath::Ray PerformGimbalPicking(
		ObjectGimbal* m_ToolGimble, 
		int window_x, int window_y, 
		int mouse_x, int mouse_y,
		DirectX::SimpleMath::Matrix world,
		DirectX::SimpleMath::Matrix projection, 
		DirectX::SimpleMath::Matrix view, 
		float min_depth, float max_depth, 
		DirectX::SimpleMath::Vector3 campos);
	// OBJECT PICKING // ----------------------------------
	DirectX::SimpleMath::Ray PerformObjectPicking(	
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
	DirectX::SimpleMath::Ray PerformTerrainPicking(
		int window_x, int window_y,
		int mouse_x, int mouse_y,
		DirectX::SimpleMath::Matrix world,
		DirectX::SimpleMath::Matrix projection,
		DirectX::SimpleMath::Matrix view,
		float min_depth, float max_depth,
		DisplayChunk& chunk, 
		DirectX::SimpleMath::Vector3 campos);		// Arguments for terrain
};

