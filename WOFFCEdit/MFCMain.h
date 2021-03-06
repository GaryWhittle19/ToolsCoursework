#pragma once

#include <afxwin.h> 
#include <afxext.h>
#include <afx.h>
#include "pch.h"
#include "Game.h"
#include "ToolMain.h"
#include "resource.h"
#include "MFCFrame.h"
#include "SelectDialogue.h"


class MFCMain : public CWinApp 
{
public:
	MFCMain();
	~MFCMain();
	BOOL InitInstance();
	int  Run();

private:
	CMyFrame *m_frame;		// Handle to the frame where all our UI is
	HWND m_toolHandle;		// Handle to the MFC window
	ToolMain m_ToolSystem;	// Instance of Tool System that we interface to. 
	CRect WindowRECT;		// Window area rectangle. 
	
	// For modeless dialogue, declare below
	SelectDialogue m_ToolSelectDialogue;			

	// For modal dialogue, declare below
	CColorDialog m_color_dialogue;	// Color picking dialogue
	BYTE R = 0xFF;
	BYTE G = 0x00;
	BYTE B = 0x00;					// Colors stored as byte values.

	// For storing and height of the DirectX window.
	int m_width;		
	int m_height;
	
	// Interface funtions for menu and toolbar etc requires
	afx_msg void MenuFileQuit();
	afx_msg void MenuFileSaveTerrain();

	afx_msg void MenuEditSelect();
	afx_msg void MenuEditWireframe();
	afx_msg void MenuEditRayVisualize();

	afx_msg void MenuModeSelect();
	afx_msg void MenuModeTerrainSculpt();
	afx_msg void MenuModeTerrainPaint();

	afx_msg	void ToolbarSaveButton();
	afx_msg void ToolbarColorsButton();

	afx_msg void CheckTrackbars();

	DECLARE_MESSAGE_MAP()	// Required macro for message map functionality. One per class.

	// Booleans - wireframe and rays on/off
	bool wireframe_enabled = false;
	bool rays_enabled = false;
};
