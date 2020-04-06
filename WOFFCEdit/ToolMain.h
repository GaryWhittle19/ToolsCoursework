#pragma once

#include <afxext.h>
#include "pch.h"
#include "Game.h"
#include "sqlite3.h"
#include "SceneObject.h"
#include "InputCommands.h"
#include "Picking.h"
#include <vector>

class ToolMain
{
public: //methods
	ToolMain();
	~ToolMain();

	//onAction - These are the interface to MFC
	int		getCurrentSelectionID();										// Returns the selection number of currently selected object so that It can be displayed.
	void	onActionInitialise(HWND handle, int width, int height);			// Passes through handle and hieght and width and initialises DirectX renderer and SQL LITE
	// void	onActionFocusCamera();											   
	void	onActionLoad();													// Load the current chunk
	afx_msg	void	onActionSave();											// Save the current chunk
	afx_msg void	onActionSaveTerrain();									// Save chunk geometry

	void	Tick(MSG *msg);
	void	UpdateInput(MSG *msg);

public:	//variables
	std::vector<SceneObject>    m_sceneGraph;	// Our scenegraph storing all the objects in the current chunk
	ChunkObject					m_chunk;		// Our landscape chunk
	int m_selectedObject;						// ID of current Selection

private:	//methods
	// void	onContentAdded();


		
private:	//variables
	HWND	m_toolHandle;					// Handle to the  window
	Game	m_d3dRenderer;					// Instance of D3D rendering system for our tool

	InputCommands m_toolInputCommands;		// Input commands that we want to use and possibly pass over to the renderer
	InputProcessor m_toolInputProcessor;	// Input processor to (surprise) process inputs!

	CRect	WindowRECT;						// Window area rectangle. 
	
	int xPos, yPos;							// Mouse x and y position
	
	sqlite3 *m_databaseConnection;			// sqldatabase handle

	int m_width;							// Dimensions passed to directX
	int m_height;
	int m_currentChunk;						// The current chunk of thedatabase that we are operating on.  Dictates loading and saving. 
	
	PickingHandler m_pickingHandler;
	DirectX::XMVECTOR m_pickingVector;
};