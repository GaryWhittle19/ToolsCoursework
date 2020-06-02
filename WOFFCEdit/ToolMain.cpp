#include "ToolMain.h"
#include "resource.h"
#include <vector>
#include <sstream>


//
//ToolMain Class
ToolMain::ToolMain()
{
	m_currentChunk = 0;		// Default value
	m_selectedObject = 0;	// Initial selection ID
	m_sceneGraph.clear();	// Clear the vector for the scenegraph
	m_databaseConnection = NULL;

	// Zero the input commands
	m_toolInputCommands.forward = false;
	m_toolInputCommands.back = false;
	m_toolInputCommands.left = false;
	m_toolInputCommands.right = false;
	m_toolInputCommands.up = false;
	m_toolInputCommands.down = false;
	//
	m_toolInputCommands.mouseLeft = false;
	m_toolInputCommands.mouseRight = false;
	//
	m_toolInputCommands.sprint = false;
	m_toolInputCommands.wireframe_toggle = false;
	m_toolInputCommands.ray_visualize = false;
	//
	m_toolInputCommands.brush_control_int = 0;
	m_toolInputCommands.decrease = false;
	m_toolInputCommands.increase = false;
	//
	m_toolInputCommands.x = 0;
	m_toolInputCommands.y = 0;
}

ToolMain::~ToolMain()
{
	sqlite3_close(m_databaseConnection);		//close the database connection
}

int ToolMain::getCurrentSelectionID()
{
	return m_selectedObject;
}

void ToolMain::onActionInitialise(HWND handle, int width, int height)
{
	//window size, handle etc for directX
	m_width = width;
	m_height = height;

	m_d3dRenderer.Initialize(handle, m_width, m_height);
	// Create references to display chunk object and DirectX device resources for later use 
	m_displayList = &m_d3dRenderer.GetDisplayList();
	m_display_chunk = &m_d3dRenderer.GetDisplayChunk();
	m_deviceResources = m_d3dRenderer.GetDeviceResourcesRef();
	// Pass reference to editor camera object
	m_d3dRenderer.SetToolCamera(&m_Camera);

	//database connection establish
	int rc;
	rc = sqlite3_open_v2("database/test.db", &m_databaseConnection, SQLITE_OPEN_READWRITE, NULL);

	if (rc)
	{
		TRACE("Can't open database");
		//if the database cant open. Perhaps a more catastrophic error would be better here
	}
	else
	{
		TRACE("Opened database successfully");
	}

	onActionLoad();
}

void ToolMain::onActionLoad()
{
	//load current chunk and objects into lists
	if (!m_sceneGraph.empty())		//is the vector empty
	{
		m_sceneGraph.clear();		//if not, empty it
	}

	//SQL
	int rc;
	char* sqlCommand;
	char* ErrMSG = 0;
	sqlite3_stmt* pResults;								//results of the query
	sqlite3_stmt* pResultsChunk;

	//OBJECTS IN THE WORLD
	//prepare SQL Text
	sqlCommand = "SELECT * from Objects";				//sql command which will return all records from the objects table.
	//Send Command and fill result object
	rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand, -1, &pResults, 0);

	//loop for each row in results until there are no more rows.  ie for every row in the results. We create and object
	while (sqlite3_step(pResults) == SQLITE_ROW)
	{
		SceneObject newSceneObject;
		newSceneObject.ID = sqlite3_column_int(pResults, 0);
		newSceneObject.chunk_ID = sqlite3_column_int(pResults, 1);
		newSceneObject.model_path = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 2));
		newSceneObject.tex_diffuse_path = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 3));
		newSceneObject.posX = sqlite3_column_double(pResults, 4);
		newSceneObject.posY = sqlite3_column_double(pResults, 5);
		newSceneObject.posZ = sqlite3_column_double(pResults, 6);
		newSceneObject.rotX = sqlite3_column_double(pResults, 7);
		newSceneObject.rotY = sqlite3_column_double(pResults, 8);
		newSceneObject.rotZ = sqlite3_column_double(pResults, 9);
		newSceneObject.scaX = sqlite3_column_double(pResults, 10);
		newSceneObject.scaY = sqlite3_column_double(pResults, 11);
		newSceneObject.scaZ = sqlite3_column_double(pResults, 12);
		newSceneObject.render = sqlite3_column_int(pResults, 13);
		newSceneObject.collision = sqlite3_column_int(pResults, 14);
		newSceneObject.collision_mesh = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 15));
		newSceneObject.collectable = sqlite3_column_int(pResults, 16);
		newSceneObject.destructable = sqlite3_column_int(pResults, 17);
		newSceneObject.health_amount = sqlite3_column_int(pResults, 18);
		newSceneObject.editor_render = sqlite3_column_int(pResults, 19);
		newSceneObject.editor_texture_vis = sqlite3_column_int(pResults, 20);
		newSceneObject.editor_normals_vis = sqlite3_column_int(pResults, 21);
		newSceneObject.editor_collision_vis = sqlite3_column_int(pResults, 22);
		newSceneObject.editor_pivot_vis = sqlite3_column_int(pResults, 23);
		newSceneObject.pivotX = sqlite3_column_double(pResults, 24);
		newSceneObject.pivotY = sqlite3_column_double(pResults, 25);
		newSceneObject.pivotZ = sqlite3_column_double(pResults, 26);
		newSceneObject.snapToGround = sqlite3_column_int(pResults, 27);
		newSceneObject.AINode = sqlite3_column_int(pResults, 28);
		newSceneObject.audio_path = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 29));
		newSceneObject.volume = sqlite3_column_double(pResults, 30);
		newSceneObject.pitch = sqlite3_column_double(pResults, 31);
		newSceneObject.pan = sqlite3_column_int(pResults, 32);
		newSceneObject.one_shot = sqlite3_column_int(pResults, 33);
		newSceneObject.play_on_init = sqlite3_column_int(pResults, 34);
		newSceneObject.play_in_editor = sqlite3_column_int(pResults, 35);
		newSceneObject.min_dist = sqlite3_column_double(pResults, 36);
		newSceneObject.max_dist = sqlite3_column_double(pResults, 37);
		newSceneObject.camera = sqlite3_column_int(pResults, 38);
		newSceneObject.path_node = sqlite3_column_int(pResults, 39);
		newSceneObject.path_node_start = sqlite3_column_int(pResults, 40);
		newSceneObject.path_node_end = sqlite3_column_int(pResults, 41);
		newSceneObject.parent_id = sqlite3_column_int(pResults, 42);
		newSceneObject.editor_wireframe = sqlite3_column_int(pResults, 43);
		newSceneObject.name = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 44));

		newSceneObject.light_type = sqlite3_column_int(pResults, 45);
		newSceneObject.light_diffuse_r = sqlite3_column_double(pResults, 46);
		newSceneObject.light_diffuse_g = sqlite3_column_double(pResults, 47);
		newSceneObject.light_diffuse_b = sqlite3_column_double(pResults, 48);
		newSceneObject.light_specular_r = sqlite3_column_double(pResults, 49);
		newSceneObject.light_specular_g = sqlite3_column_double(pResults, 50);
		newSceneObject.light_specular_b = sqlite3_column_double(pResults, 51);
		newSceneObject.light_spot_cutoff = sqlite3_column_double(pResults, 52);
		newSceneObject.light_constant = sqlite3_column_double(pResults, 53);
		newSceneObject.light_linear = sqlite3_column_double(pResults, 54);
		newSceneObject.light_quadratic = sqlite3_column_double(pResults, 55);


		//send completed object to scenegraph
		m_sceneGraph.push_back(newSceneObject);
	}

	//THE WORLD CHUNK
	//prepare SQL Text
	sqlCommand = "SELECT * from Chunks";				//sql command which will return all records from  chunks table. There is only one tho.
														//Send Command and fill result object
	rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand, -1, &pResultsChunk, 0);


	sqlite3_step(pResultsChunk);
	m_chunk.ID = sqlite3_column_int(pResultsChunk, 0);
	m_chunk.name = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 1));
	m_chunk.chunk_x_size_metres = sqlite3_column_int(pResultsChunk, 2);
	m_chunk.chunk_y_size_metres = sqlite3_column_int(pResultsChunk, 3);
	m_chunk.chunk_base_resolution = sqlite3_column_int(pResultsChunk, 4);
	m_chunk.heightmap_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 5));
	m_chunk.tex_diffuse_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 6));
	m_chunk.tex_splat_alpha_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 7));
	m_chunk.tex_splat_1_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 8));
	m_chunk.tex_splat_2_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 9));
	m_chunk.tex_splat_3_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 10));
	m_chunk.tex_splat_4_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 11));
	m_chunk.render_wireframe = sqlite3_column_int(pResultsChunk, 12);
	m_chunk.render_normals = sqlite3_column_int(pResultsChunk, 13);
	m_chunk.tex_diffuse_tiling = sqlite3_column_int(pResultsChunk, 14);
	m_chunk.tex_splat_1_tiling = sqlite3_column_int(pResultsChunk, 15);
	m_chunk.tex_splat_2_tiling = sqlite3_column_int(pResultsChunk, 16);
	m_chunk.tex_splat_3_tiling = sqlite3_column_int(pResultsChunk, 17);
	m_chunk.tex_splat_4_tiling = sqlite3_column_int(pResultsChunk, 18);


	// Process REsults into renderable
	m_d3dRenderer.BuildDisplayList(&m_sceneGraph);
	// Build the renderable chunk and return a reference to the display chunk
	m_d3dRenderer.BuildDisplayChunk(&m_chunk);
	// Load the vertex colours
	m_display_chunk->LoadVertexColours(m_deviceResources);
}

void ToolMain::onActionSave()
{
	//SQL
	int rc;
	char* sqlCommand;
	char* ErrMSG = 0;
	sqlite3_stmt* pResults;								//results of the query


	//OBJECTS IN THE WORLD Delete them all
	//prepare SQL Text
	sqlCommand = "DELETE FROM Objects";	 //will delete the whole object table.   Slightly risky but hey.
	rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand, -1, &pResults, 0);
	sqlite3_step(pResults);

	//Populate with our new objects
	std::wstring sqlCommand2;
	int numObjects = m_sceneGraph.size();	//Loop thru the scengraph.

	for (int i = 0; i < numObjects; i++)
	{
		std::stringstream command;
		command << "INSERT INTO Objects "
			<< "VALUES(" << m_sceneGraph.at(i).ID << ","
			<< m_sceneGraph.at(i).chunk_ID << ","
			<< "'" << m_sceneGraph.at(i).model_path << "'" << ","
			<< "'" << m_sceneGraph.at(i).tex_diffuse_path << "'" << ","
			<< m_sceneGraph.at(i).posX << ","
			<< m_sceneGraph.at(i).posY << ","
			<< m_sceneGraph.at(i).posZ << ","
			<< m_sceneGraph.at(i).rotX << ","
			<< m_sceneGraph.at(i).rotY << ","
			<< m_sceneGraph.at(i).rotZ << ","
			<< m_sceneGraph.at(i).scaX << ","
			<< m_sceneGraph.at(i).scaY << ","
			<< m_sceneGraph.at(i).scaZ << ","
			<< m_sceneGraph.at(i).render << ","
			<< m_sceneGraph.at(i).collision << ","
			<< "'" << m_sceneGraph.at(i).collision_mesh << "'" << ","
			<< m_sceneGraph.at(i).collectable << ","
			<< m_sceneGraph.at(i).destructable << ","
			<< m_sceneGraph.at(i).health_amount << ","
			<< m_sceneGraph.at(i).editor_render << ","
			<< m_sceneGraph.at(i).editor_texture_vis << ","
			<< m_sceneGraph.at(i).editor_normals_vis << ","
			<< m_sceneGraph.at(i).editor_collision_vis << ","
			<< m_sceneGraph.at(i).editor_pivot_vis << ","
			<< m_sceneGraph.at(i).pivotX << ","
			<< m_sceneGraph.at(i).pivotY << ","
			<< m_sceneGraph.at(i).pivotZ << ","
			<< m_sceneGraph.at(i).snapToGround << ","
			<< m_sceneGraph.at(i).AINode << ","
			<< "'" << m_sceneGraph.at(i).audio_path << "'" << ","
			<< m_sceneGraph.at(i).volume << ","
			<< m_sceneGraph.at(i).pitch << ","
			<< m_sceneGraph.at(i).pan << ","
			<< m_sceneGraph.at(i).one_shot << ","
			<< m_sceneGraph.at(i).play_on_init << ","
			<< m_sceneGraph.at(i).play_in_editor << ","
			<< m_sceneGraph.at(i).min_dist << ","
			<< m_sceneGraph.at(i).max_dist << ","
			<< m_sceneGraph.at(i).camera << ","
			<< m_sceneGraph.at(i).path_node << ","
			<< m_sceneGraph.at(i).path_node_start << ","
			<< m_sceneGraph.at(i).path_node_end << ","
			<< m_sceneGraph.at(i).parent_id << ","
			<< m_sceneGraph.at(i).editor_wireframe << ","
			<< "'" << m_sceneGraph.at(i).name << "'" << ","

			<< m_sceneGraph.at(i).light_type << ","
			<< m_sceneGraph.at(i).light_diffuse_r << ","
			<< m_sceneGraph.at(i).light_diffuse_g << ","
			<< m_sceneGraph.at(i).light_diffuse_b << ","
			<< m_sceneGraph.at(i).light_specular_r << ","
			<< m_sceneGraph.at(i).light_specular_g << ","
			<< m_sceneGraph.at(i).light_specular_b << ","
			<< m_sceneGraph.at(i).light_spot_cutoff << ","
			<< m_sceneGraph.at(i).light_constant << ","
			<< m_sceneGraph.at(i).light_linear << ","
			<< m_sceneGraph.at(i).light_quadratic

			<< ")";
		std::string sqlCommand2 = command.str();
		rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand2.c_str(), -1, &pResults, 0);
		sqlite3_step(pResults);
	}
	MessageBox(NULL, L"Objects Saved", L"Notification", MB_OK);
}

void ToolMain::onActionSaveTerrain()
{
	m_d3dRenderer.SaveDisplayChunk(&m_chunk);
	m_display_chunk->SaveVertexColours(m_deviceResources);
}

void ToolMain::onActionChangeMode(int mode)
{
	m_pickingMode = mode;
	if (mode == 2 || mode == 3) {
		m_toolInputCommands.brush_visualize = true;
	}
	else {
		m_toolInputCommands.brush_visualize = false;
	}
}

bool ToolMain::onActionToggleWireframe()
{
	m_toolInputCommands.wireframe_toggle = !m_toolInputCommands.wireframe_toggle;
	return m_toolInputCommands.wireframe_toggle;
}

bool ToolMain::onActionToggleRayVisualization()
{
	m_toolInputCommands.ray_visualize = !m_toolInputCommands.ray_visualize;
	return m_toolInputCommands.ray_visualize;
}

void ToolMain::onActionSetBrushColor(BYTE R, BYTE G, BYTE B)
{
	brush_color.x = R / 255.f;
	brush_color.y = G / 255.f;
	brush_color.z = B / 255.f;
}

void ToolMain::onActionChangeCameraSpeed(float camera_speed)
{
	m_Camera.SetCameraMovespeed(camera_speed);
}

void ToolMain::onActionChangeBrushSize(float new_brush_size)
{
	brush_size = new_brush_size;
}

void ToolMain::onActionChangeBrushIntensity(float new_brush_intensity)
{
	brush_intensity = new_brush_intensity;
}

void ToolMain::Tick(MSG* msg)
{
	//do we have a selection
	//do we have a mode
	//are we clicking / dragging /releasing
	//has something changed
		//update Scenegraph
		//add to scenegraph
		//resend scenegraph to Direct X renderer

	// Update the camera before ticking the renderer as we will need the view matrix first
	UpdateToolCamera();

	//Renderer Update Call
	m_d3dRenderer.Tick(&m_toolInputCommands);

	// Update the picking for object selection/terrain editing
	UpdatePicking();
}

void ToolMain::UpdateToolCamera()
{
	int dx = 0; int dy = 0;					
	// Camera can only rotate when mouse right is down - dx, dy will remain 0, 0 otherwise
	if (m_toolInputCommands.mouseRight) {	
		dx = m_toolInputCommands.x - previous_mouse_x;
		dy = m_toolInputCommands.y - previous_mouse_y;
	}
	// Update camera view matrix. dx, dy control camera orientation
	m_Camera.UpdateCameraViewMatrix(m_toolInputCommands, dx, dy);	
	// Set previous mouse values for next update
	previous_mouse_x = m_toolInputCommands.x;
	previous_mouse_y = m_toolInputCommands.y;
}

void ToolMain::UpdatePicking()
{
	// Get references to game variables and resources
	m_cameraPosition = m_Camera.GetCameraPosition();
	m_world = m_d3dRenderer.GetWorldMatrix();
	m_view = m_Camera.GetCameraViewMatrix();
	m_projection = m_d3dRenderer.GetProjectionMatrix();
	// Set up the ray and brush center vector
	DirectX::SimpleMath::Ray picking_ray;
	DirectX::SimpleMath::Vector3 brush_center;
	bool DidHit = false;	// Bool for detecting failed intersection test

	if (m_toolInputCommands.mouseLeft) {
		switch (m_pickingMode) {

		case 1:	// Object picking
				// Get picking ray for object
			picking_ray = m_pickingHandler.PerformObjectPicking(
				m_deviceResources->GetScreenViewport().Width, m_deviceResources->GetScreenViewport().Height,
				m_toolInputCommands.x, m_toolInputCommands.y, m_world, m_projection, m_view,
				m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_selectedObject,
				*m_displayList, m_cameraPosition);
			// Set mouseleft back up to perform a *click*
			m_toolInputCommands.mouseLeft = false;
			// Visualize ray if required
			if (m_toolInputCommands.ray_visualize)
				m_d3dRenderer.SetRayForVisualization(picking_ray);
			break;

		case 2:	// Terrain sculpting
				// Get picking ray for terrain
			picking_ray = m_pickingHandler.PerformTerrainPicking(
				m_deviceResources->GetScreenViewport().Width, m_deviceResources->GetScreenViewport().Height,
				m_toolInputCommands.x, m_toolInputCommands.y, m_world, m_projection, m_view,
				m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth,
				*m_display_chunk, m_cameraPosition);
			// Get brush center
			brush_center = m_display_chunk->GetBrushCenter(picking_ray, &DidHit);
			// Perform terrain sculpting and visualize if intersection returned true
			if (DidHit) {
				m_display_chunk->GenerateHeightmap(brush_size, brush_intensity, brush_center);
				// Visualize brush if required
				if (m_toolInputCommands.brush_visualize)
					m_d3dRenderer.SetBrushForVisualization(brush_center, brush_size);
			}
			// Visualize ray if required
			if (m_toolInputCommands.ray_visualize)
				m_d3dRenderer.SetRayForVisualization(picking_ray);
			break;

		case 3:	// Terrain painting
				// Get picking ray for terrain
			picking_ray = m_pickingHandler.PerformTerrainPicking(
				m_deviceResources->GetScreenViewport().Width, m_deviceResources->GetScreenViewport().Height,
				m_toolInputCommands.x, m_toolInputCommands.y, m_world, m_projection, m_view,
				m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth,
				*m_display_chunk, m_cameraPosition);
			// Get brush center
			brush_center = m_display_chunk->GetBrushCenter(picking_ray, &DidHit);
			// Perform terrain painting and visualize if intersection returned true
			if (DidHit) {
				m_display_chunk->PaintTerrain(brush_size, brush_intensity, brush_center, brush_color);
				// Visualize brush if required
				if (m_toolInputCommands.brush_visualize)
					m_d3dRenderer.SetBrushForVisualization(brush_center, brush_size);
			}
			// Visualize ray if required
			if (m_toolInputCommands.ray_visualize)
				m_d3dRenderer.SetRayForVisualization(picking_ray);
			break;
		}
	}

	// If terrain sculpting/painting without left click held, visualize the brush but only when camera is stationary to make performance hit more bearable
	if (m_toolInputCommands.brush_visualize && !m_Camera.IsCameraMoving()) {
		// Get picking ray for terrain
		picking_ray = m_pickingHandler.PerformTerrainPicking(
			m_deviceResources->GetScreenViewport().Width, m_deviceResources->GetScreenViewport().Height,
			m_toolInputCommands.x, m_toolInputCommands.y, m_world, m_projection, m_view,
			m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth,
			*m_display_chunk, m_cameraPosition);
		// Get brush center
		brush_center = m_display_chunk->GetBrushCenter(picking_ray, &DidHit);
		// Set the brush variables for visualization in game
		if (DidHit)
		m_d3dRenderer.SetBrushForVisualization(brush_center, brush_size);
	}
}

void ToolMain::UpdateInput(MSG* msg)
{
	m_toolInputProcessor.ResetKeys();
	bool wasDown, isDown;
	// Message handling
	switch (msg->message)
	{
		// Global inputs - mouse position/buttons and keyboard
	case WM_KEYDOWN:
		wasDown = ((msg->lParam & (1 << 30)) != 0);
		isDown = ((msg->lParam & (1 << 31)) == 0);
		m_toolInputProcessor.SetKey(msg->wParam, isDown, false, !wasDown && isDown);
		break;

	case WM_KEYUP:
		wasDown = ((msg->lParam & (1 << 30)) != 0);
		m_toolInputProcessor.SetKey(msg->wParam, false, wasDown, false);
		break;

	case WM_MOUSEMOVE:
		m_toolInputCommands.x = GET_X_LPARAM(msg->lParam);
		m_toolInputCommands.y = GET_Y_LPARAM(msg->lParam);
		break;

	case WM_LBUTTONDOWN:
		m_toolInputCommands.mouseLeft = true;
		break;

	case WM_LBUTTONUP:
		m_toolInputCommands.mouseLeft = false;
		break;

	case WM_RBUTTONDOWN:
		m_toolInputCommands.mouseRight = true;
		break;

	case WM_RBUTTONUP:
		m_toolInputCommands.mouseRight = false;
		break;
	}

	// MOVEMENT
	// Directions
	m_toolInputCommands.forward = m_toolInputProcessor.IsKeyDown('W');
	m_toolInputCommands.left = m_toolInputProcessor.IsKeyDown('A');
	m_toolInputCommands.back = m_toolInputProcessor.IsKeyDown('S');
	m_toolInputCommands.right = m_toolInputProcessor.IsKeyDown('D');
	m_toolInputCommands.up = m_toolInputProcessor.IsKeyDown('E');
	m_toolInputCommands.down = m_toolInputProcessor.IsKeyDown('Q');
	// Camera sprint
	if (GetKeyState(VK_SHIFT) & 0x8000)	// & 0x8000 gives real-time key state
	{
		m_toolInputCommands.sprint = true;
	}
	else m_toolInputCommands.sprint = false;

	// TERRAIN SETTINGS
	// Increase and decrease target control variable
	if (GetKeyState(VK_LEFT) & 0x8000)
	{
		m_toolInputCommands.decrease = true;
	}
	else m_toolInputCommands.decrease = false;
	if (GetKeyState(VK_RIGHT) & 0x8000)
	{
		m_toolInputCommands.increase = true;
	}
	else m_toolInputCommands.increase = false;

	// Change target brush variable being controlled
	if (m_toolInputProcessor.WasKeyReleased('C'))
	{
		m_toolInputCommands.brush_control_int += 1;			// Add one which will result in target variable being changes
		if (m_toolInputCommands.brush_control_int > 1) {	// Ensure that the cycle_mode int won't go rogue
			m_toolInputCommands.brush_control_int = 0;
		}
	}
}