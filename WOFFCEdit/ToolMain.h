#pragma once

#include <afxext.h>
#include "pch.h"
#include "Camera.h"
#include "DisplayChunk.h"
#include "Game.h"
#include "InputCommands.h"
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
	int		getCurrentSelectionID();										// Returns the selection number of currently selected object so that It can be displayed.
	void	onActionInitialise(HWND handle, int width, int height);			// Passes through handle and hieght and width and initialises DirectX renderer and SQL LITE
	// void	onActionFocusCamera();											   
	void			onActionLoad();											// Load the current chunk
	afx_msg	void	onActionSave();											// Save the current chunk
	afx_msg void	onActionSaveTerrain();									// Save chunk geometry
	afx_msg void	onActionChangeMode(int mode);							// Change editor mode (objects, terrain...)
	afx_msg void	onActionToggleWireframe();								// Toggle wireframe editing
	afx_msg void	onActionToggleRayVisualization();						// Toggle ray renderering 

	// Standard tick and update functions
	void	Tick(MSG *msg);
	void	UpdateInput(MSG *msg);
	
public:	// Variables
	std::vector<SceneObject>    m_sceneGraph;					// Our scenegraph storing all the objects in the current chunk
	ChunkObject					m_chunk;						// Our chunk data
	int m_selectedObject;										// ID of current Selection

private: // Methods
	void UpdateToolCamera();									// Update the editor's camera
	void UpdatePicking();										// Update mouse picking
	void UpdateSculptSettings();								// Change brush settings
	// void	onContentAdded();

private: // Variables
	HWND	m_toolHandle;										// Handle to the  window
	Game	m_d3dRenderer;										// Instance of D3D rendering system for our tool
	
	InputCommands m_toolInputCommands;							// Input commands that we want to use and possibly pass over to the renderer
	InputProcessor m_toolInputProcessor;						// Input processor to (surprise) process inputs!

	PickingHandler m_pickingHandler;							// Handles our picking of objects/terrain
	int m_pickingMode = 1;										// Current cursor picking mode; 1 = objects; 2 = terrain sculpt; 3 = terrain paint.
	
	float brush_size = 50;										// Brush size and intensity for sculpting/painting terrain
	float brush_intensity = 5;
	
	CRect	WindowRECT;											// Window area rectangle. 
	
	sqlite3 *m_databaseConnection;								// sqldatabase handle
	
	int 									m_width;			// Dimensions passed to directX
	int 									m_height;
	int 									m_currentChunk;		// The current chunk of thedatabase that we are operating on.  Dictates loading and saving. 
	
	std::vector<DisplayObject>*				m_displayList;		// Reference to renderer's DisplayList (list of objects in world)
	DisplayChunk*							m_display_chunk;	// Reference to renderer's DisplayChunk object
	std::shared_ptr<DX::DeviceResources>	m_deviceResources;	// Reference to device resources

	DirectX::SimpleMath::Matrix				m_world;			// Matrices for picking calculations
	DirectX::SimpleMath::Matrix				m_view;
	DirectX::SimpleMath::Matrix				m_projection;
	DirectX::SimpleMath::Vector3			m_cameraPosition;	// Camera position for picking calculations
	
	Camera m_Camera;											// Editor camera

	int									previous_mouse_x = 0;	// Previous mouse values for calculating delta x/y
	int									previous_mouse_y = 0;
};