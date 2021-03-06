#pragma once

#include <afxext.h>
#include "pch.h"
#include "Camera.h"
#include "DisplayChunk.h"
#include "Game.h"
#include "InputCommands.h"
#include "ObjectGimbal.h"
#include "Picking.h"
#include "SceneObject.h"
#include "sqlite3.h"
#include "Toolbox.h"
#include <vector>

class ToolMain
{
public: // Methods
	ToolMain();
	~ToolMain();

	// onAction - These are the interface to MFC
	const int		getCurrentSelectionID();								// Returns the selection number of currently selected object so that It can be displayed.
	void			onActionInitialise(HWND handle, int width, int height);	// Passes through handle and height and width and initialises DirectX renderer and SQL LITE				   
	void			onActionLoad();											// Load the current chunk
	afx_msg	void	onActionSave();											// Save the current chunk
	afx_msg void	onActionSaveTerrain();									// Save chunk geometry
	afx_msg void	onActionChangeMode(int mode);							// Change editor mode (objects, terrain...)
	afx_msg bool	onActionToggleWireframe();								// Toggle wireframe editing
	afx_msg bool	onActionToggleRayVisualization();						// Toggle ray renderering 
	afx_msg void	onActionSetBrushColor(BYTE R, BYTE G, BYTE B);			// Set brush color for vertex painting
	afx_msg void	onActionChangeCameraSpeed(float camera_speed);			// Change camera speed
	afx_msg void	onActionChangeBrushSize(float new_size);				// Change size of terrain brush
	afx_msg void	onActionChangeBrushIntensity(float new_intensity);		// Change intensity of terrain brush

	void ChangeToolGimbalMode();

	// Standard tick and update functions
	void	Tick(MSG *msg);
	void	UpdateInput(MSG *msg);
	
public:	// Variables
	std::vector<SceneObject>    m_sceneGraph;								// Our scenegraph storing all the objects in the current chunk
	ChunkObject					m_chunk;									// Our chunk data
	int m_selectedObject;													// ID of current Selection

private: // Methods
	void UpdateDeltaMouse();												// Update the editor's camera
	void UpdateCamera();
	void UpdateGimbalDrag();
	void UpdatePicking();													// Update mouse picking
	void FocusToolCamera();
	void WasAnyMouseReleased(bool& leftMouseReleased, bool& rightMouseReleased);

private: // Variables
	HWND	m_toolHandle;													// Handle to the  window
	Game	m_d3dRenderer;													// Instance of D3D rendering system for our tool
	
	InputCommands m_toolInputCommands;										// Input commands that we want to use and possibly pass over to the renderer
	InputProcessor m_toolInputProcessor;									// Input processor used to process inputs

	PickingHandler m_pickingHandler;										// Handles our picking of objects/terrain
	int m_pickingMode = 1;													// Current cursor picking mode; 1 = objects; 2 = terrain sculpt; 3 = terrain paint.
	
	float									brush_size = 50;				// Brush size and intensity for sculpting/painting terrain, paint colour
	float									brush_intensity = 5;			
	DirectX::XMFLOAT4						brush_color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	
	CRect	WindowRECT;														// Window area rectangle. 
	
	sqlite3 *m_databaseConnection;											// sqldatabase handle
	
	int 									m_width;						// Dimensions passed to directX
	int 									m_height;
	int 									m_currentChunk;					// The current chunk of thedatabase that we are operating on.  Dictates loading and saving. 
	
	std::vector<DisplayObject>*				m_displayList;					// Reference to renderer's DisplayList (list of objects in world)
	DisplayChunk*							m_display_chunk;				// Reference to renderer's DisplayChunk object
	std::shared_ptr<DX::DeviceResources>	m_deviceResources;				// Reference to device resources

	DirectX::SimpleMath::Matrix				m_world;						// Matrices for picking calculations
	DirectX::SimpleMath::Matrix				m_view;
	DirectX::SimpleMath::Matrix				m_projection;

	DirectX::SimpleMath::Vector3			m_cameraPosition;				// Camera position for picking calculations
	
	Camera m_Camera;														// Editor camera
	ObjectGimbal m_Gimbal;													// Editor gimbal

	int										previous_mouse_x = 0;			// Previous mouse values for calculating delta x/y
	int										previous_mouse_y = 0;
	int										dx = 0; 
	int										dy = 0;
	bool									previous_mouseLeft = false;		// Previous mouse button values for calculating left/right mouse released
	bool									previous_mouseRight = false;
	bool									leftMouseReleased = false;
	bool									rightMouseReleased = false;
	bool									m_camMoving = false;			// Store whether cam is moving here
	bool									dragging = false;				// Store whether gimbal is being dragged here
	int										gimbalMode = 1;					// Gimbal mode - translate 1, rotate 2, scale 3
};