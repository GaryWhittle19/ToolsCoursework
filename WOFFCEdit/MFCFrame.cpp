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

	// Create a view to occupy the client area of the frame - this is where DirectX is rendered.
	if (!m_DirXView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW, CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}

	// Load the menu.
	m_menu1.LoadMenuW(IDR_MENU1);
	SetMenu(&m_menu1);
	
	// Load the toolbar. Note third parameter - styles! Useful for customising the MFC window layout. 
	if (!m_toolBar.CreateEx(this, TBSTYLE_TRANSPARENT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, CRect(0, 0, 0, 4)) ||
		!m_toolBar.LoadToolBar(IDR_TOOLBAR1))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // Failed to create
	}
	
	// Camera speed trackbar. Styles: child window (of m_toolBar), initially visible. 
	if (!m_camspeed_trackbar.Create(WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_TOOLTIPS, CRect(844, 0, 944, 25), &m_toolBar, IDC_TRACKBAR_CAMERA))
	{
		TRACE0("Failed to create trackbar\n");
		return -1;      // Failed to create
	}
	else {
		m_camspeed_trackbar.SetRangeMin(1, false);
		m_camspeed_trackbar.SetRangeMax(8, false);
		m_camspeed_trackbar.SetPos(1);
	}

	// Windows status bar.
	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // Failed to create
	}

	CRect rect;
	GetClientRect(&rect);
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT));
	m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_TOOL, SBPS_NORMAL, rect.Width() - 500); // Set width of status bar panel

	return 0;
}