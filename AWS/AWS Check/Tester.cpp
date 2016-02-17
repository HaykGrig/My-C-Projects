
// Tester.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Tester.h"
#include "TesterDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTesterApp

BEGIN_MESSAGE_MAP(CTesterApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CTesterApp construction

CTesterApp::CTesterApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CTesterApp object

CTesterApp theApp;


// CTesterApp initialization

BOOL CTesterApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);

	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	CShellManager *pShellManager = new CShellManager;

	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CTesterDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();

	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	return FALSE;
}

