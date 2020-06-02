#include "MFCFrame.h"
#include "resource.h"


BEGIN_MESSAGE_MAP(CMyFrame, CFrameWnd)
	
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_TOOL, &CMyFrame::OnUpdatePage)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,
	ID_INDICATOR_TOOL
};
// FRAME CLASS


// Frame initialiser
CMyFrame::CMyFrame()
{
	m_selectionID = 999; // An obviously wrong selection ID,  to verify it's working
}

void CMyFrame::SetCurrentSelectionID(int ID)
{
	m_selectionID = ID;
}

void CMyFrame::OnUpdatePage(CCmdUI * pCmdUI)
{
	pCmdUI->Enable();
	CString strPage;
	strPage.Format(_T("%d"), m_selectionID);
	pCmdUI->SetText(strPage);
}

// OnCreate, called after init but before window is shown. 
int CMyFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1) return -1;

	// Load the menu
	m_menu1.LoadMenuW(IDR_MENU1);
	SetMenu(&m_menu1);
	
	// Load the editor controls dialog box
	if (!tools_dialog_box.Create(this, IDD_DIALOGBAR, CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY, IDD_DIALOGBAR)) {
		TRACE0("Failed to create editor controls dialog box\n");
		return -1;      // Failed to create
	}

	// Load the toolbar within the editor controls dialog box
	if (!m_toolBar.CreateEx(&tools_dialog_box, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT, WS_CHILD | WS_VISIBLE | CBRS_ALIGN_TOP
		| CBRS_FLYBY | CBRS_SIZE_DYNAMIC, CRect(0, 0, 0, 0)) || !m_toolBar.LoadToolBar(IDR_TOOLBAR1))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // Failed to create
	}

	// Reposition the control bars to show the created child toolbar first, and repositon toolbar to appropriate position. 
	tools_dialog_box.RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	m_toolBar.MoveWindow(0, 2, 200, 100, 1);

	// Get references to sliders, set them up appropriately
	m_camspeed_trackbar = (CSliderCtrl*)tools_dialog_box.GetDlgItem(IDC_SLIDER1);
	m_camspeed_trackbar->SetRange(1, 4);

	m_brush_size_trackbar = (CSliderCtrl*)tools_dialog_box.GetDlgItem(IDC_SLIDER2);
	m_brush_size_trackbar->SetRange(1, 100);
	m_brush_size_trackbar->SetPos(25);

	m_brush_intensity_trackbar = (CSliderCtrl*)tools_dialog_box.GetDlgItem(IDC_SLIDER3);
	m_brush_intensity_trackbar->SetRange(-10, 10);
	m_brush_intensity_trackbar->SetPos(5);

	// Create a view to occupy the client area of the frame - this is where DirectX is rendered.
	if (!m_DirXView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW, CRect(3, 30, 1916, 1006), &tools_dialog_box, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}

	// Windows status bar.
	if (m_wndStatusBar.Create(this))
	{
		CRect rect;
		GetClientRect(&rect);
		m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT));
		m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_TOOL, SBPS_NORMAL, rect.Width() - 100); // Set width of status bar panel
	}
	else {
		TRACE0("Failed to create status bar\n");
		return -1;      // Failed to create
	}

	return 0;
}