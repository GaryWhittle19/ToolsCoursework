#include "MFCMain.h"
#include "resource.h"


BEGIN_MESSAGE_MAP(MFCMain, CWinApp)
	// TOOLBAR
	ON_COMMAND(ID_BUTTON_SAVE, &MFCMain::ToolbarSaveButton)
	ON_COMMAND(ID_BUTTON_WIRE, &MFCMain::MenuEditWireframe)
	ON_COMMAND(ID_BUTTON_RAY, &MFCMain::MenuEditRayVisualize)
	ON_COMMAND(ID_BUTTON_COLOR, &MFCMain::ToolbarColorsButton)
	// FILE
	ON_COMMAND(ID_FILE_QUIT, &MFCMain::MenuFileQuit)
	ON_COMMAND(ID_FILE_SAVETERRAIN, &MFCMain::MenuFileSaveTerrain)
	// EDIT
	ON_COMMAND(ID_EDIT_SELECT, &MFCMain::MenuEditSelect)
	ON_COMMAND(ID_EDIT_WIREFRAME, &MFCMain::MenuEditWireframe)
	ON_COMMAND(ID_EDIT_RAY, &MFCMain::MenuEditRayVisualize)
	// MODE
	ON_COMMAND(ID_MODE_SELECT, &MFCMain::MenuModeSelect)
	ON_COMMAND(ID_MODE_TERRAIN_SCULPT, &MFCMain::MenuModeTerrainSculpt)
	ON_COMMAND(ID_MODE_TERRAIN_PAINT, &MFCMain::MenuModeTerrainPaint)
	
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_TOOL, &CMyFrame::OnUpdatePage)
END_MESSAGE_MAP()

BOOL MFCMain::InitInstance()
{
	//instanciate the mfc frame
	m_frame = new CMyFrame();
	m_pMainWnd = m_frame;

	m_frame->Create(NULL,
					_T("World of Flimflamcraft Editor"),
					WS_OVERLAPPEDWINDOW,
					CRect(0, 0, 1024, 768),
					NULL,
					NULL,
					0,
					NULL
				);

	//show and set the window to run and update. 
	m_frame->ShowWindow(SW_SHOW);
	m_frame->UpdateWindow();

	//get the rect from the MFC window so we can get its dimensions
	m_toolHandle = m_frame->m_DirXView.GetSafeHwnd();				//handle of directX child window
	m_frame->m_DirXView.GetClientRect(&WindowRECT);
	m_width		= WindowRECT.Width();
	m_height	= WindowRECT.Height();

	m_ToolSystem.onActionInitialise(m_toolHandle, m_width, m_height);

	return TRUE;
}

int MFCMain::Run()
{
	MSG msg;
	BOOL bGotMsg;

	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

	while (WM_QUIT != msg.message)
	{
		if (true)
		{
			bGotMsg = (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0);
		}
		else
		{
			bGotMsg = (GetMessage(&msg, NULL, 0U, 0U) != 0);
		}

		if (bGotMsg)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			m_ToolSystem.UpdateInput(&msg);
		}
		else
		{	
			int ID = m_ToolSystem.getCurrentSelectionID();
			std::wstring statusString = L"Selected Object: " + std::to_wstring(ID);
			m_ToolSystem.Tick(&msg);

			//send current object ID to status bar in The main frame
			m_frame->m_wndStatusBar.SetPaneText(1, statusString.c_str(), 1);	
		}
	}

	return (int)msg.wParam;
}

void MFCMain::MenuFileQuit()
{
	//will post message to the message thread that will exit the application normally
	PostQuitMessage(0);
}

void MFCMain::MenuFileSaveTerrain()
{
	m_ToolSystem.onActionSaveTerrain();
}

void MFCMain::MenuEditSelect()
{
	//SelectDialogue m_ToolSelectDialogue(NULL, &m_ToolSystem.m_sceneGraph);		//create our dialoguebox //modal constructor
	//m_ToolSelectDialogue.DoModal();	// start it up modal

	// Modeless dialogue must be declared in the class. If we do local it will go out of scope instantly and destroy itself.
	m_ToolSelectDialogue.Create(IDD_DIALOG1);	// Start up modeless dialog
	m_ToolSelectDialogue.ShowWindow(SW_SHOW);	// Show modeless dialog
	m_ToolSelectDialogue.SetObjectData(&m_ToolSystem.m_sceneGraph, &m_ToolSystem.m_selectedObject);
}

void MFCMain::MenuEditWireframe()
{
	m_ToolSystem.onActionToggleWireframe();
}

void MFCMain::MenuEditRayVisualize()
{
	m_ToolSystem.onActionToggleRayVisualization();
}

void MFCMain::MenuModeSelect()
{
	m_ToolSystem.onActionChangeMode(1);
}

void MFCMain::MenuModeTerrainSculpt()
{
	m_ToolSystem.onActionChangeMode(2);
}

void MFCMain::MenuModeTerrainPaint()
{
	m_ToolSystem.onActionChangeMode(3);
}

void MFCMain::ToolbarSaveButton()
{
	m_ToolSystem.onActionSave();
}

void MFCMain::ToolbarColorsButton()
{
	if (m_color_dialogue.DoModal() == IDOK)
	{
		COLORREF color = m_color_dialogue.GetColor();
		R = GetRValue(color);
		G = GetGValue(color);
		B = GetBValue(color);
	}
	m_ToolSystem.onActionSetBrushColor(R, G, B);
}

MFCMain::MFCMain()
{
}

MFCMain::~MFCMain()
{
}