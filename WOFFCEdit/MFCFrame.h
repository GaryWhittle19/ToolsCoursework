#pragma once
#include <afxwin.h> 
#include <afxext.h>
#include <afxcmn.h>
#include "MFCRenderFrame.h"
#include "ToolMain.h"



class CMyFrame : public CFrameWnd
{
protected:
//	DECLARE_DYNAMIC(CMainFrame)

public:
	CMenu			m_menu1;					// MENU

	CChildRender	m_DirXView;					// DIRECTX RENDER

	CDialogBar		tools_dialog_box;		// EDITOR CONTROLS DIALOG BAR
	CToolBar		m_toolBar;						// Toolar
	CSliderCtrl		*m_camspeed_trackbar;			// Camera speed slider
	CSliderCtrl		*m_brush_size_trackbar;			// Brush size slider
	CSliderCtrl		*m_brush_intensity_trackbar;	// Brush intensity slider

	CStatusBar		m_wndStatusBar;				// STATUS BAR


public:
	CMyFrame();
	void SetCurrentSelectionID(int ID);
	afx_msg void OnUpdatePage(CCmdUI *pCmdUI);

private:	
	int		m_selectionID;

	// Note the afx_message keyword is linking this method to message map access.
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()	// Required macro for message map functionality. One per class.
};