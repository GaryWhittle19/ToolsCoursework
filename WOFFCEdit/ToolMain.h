#pragma once

#include <afxext.h>
#include "pch.h"
#include "Game.h"
#include "DisplayChunk.h"
#include "sqlite3.h"
#include "SceneObject.h"
#include "InputCommands.h"
#include "Picking.h"
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

	bool IsMoving();
	
public:	// Variables
	std::vector<SceneObject>    m_sceneGraph;		// Our scenegraph storing all the objects in the current chunk
	ChunkObject					m_chunk;			// Our chunk data
	int m_selectedObject;							// ID of current Selection

private: // Methods
	// void	onContentAdded();

private: // Variables
	HWND	m_toolHandle;					// Handle to the  window
	Game	m_d3dRenderer;					// Instance of D3D rendering system for our tool

	InputCommands m_toolInputCommands;		// Input commands that we want to use and possibly pass over to the renderer
	InputProcessor m_toolInputProcessor;	// Input processor to (surprise) process inputs!

	CRect	WindowRECT;						// Window area rectangle. 
	
	sqlite3 *m_databaseConnection;			// sqldatabase handle

	int m_width;							// Dimensions passed to directX
	int m_height;
	int m_currentChunk;						// The current chunk of thedatabase that we are operating on.  Dictates loading and saving. 
	
	int m_pickingMode = 1;					// Current cursor picking mode; 1 = objects; 2 = terrain sculpt; 3 = terrain paint.
	PickingHandler m_pickingHandler;		// Handles our picking of objects/terrain

	DisplayChunk* m_display_chunk;			// Will store reference to our renderer's DisplayChunk object

	DirectX::SimpleMath::Matrix* m_world;						// Matrix references
	DirectX::SimpleMath::Matrix* m_view;
	DirectX::SimpleMath::Matrix* m_projection;
	DirectX::SimpleMath::Vector3* m_cameraPosition;				// Reference to camera position vector
	std::shared_ptr<DX::DeviceResources>	m_deviceResources;	// Reference to device resources
};