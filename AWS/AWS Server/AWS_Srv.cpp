#include "stdafx.h"
#include "AWS_Srv.h"
#include "AWS_SrvDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CAWS_SrvApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


CAWS_SrvApp::CAWS_SrvApp()
{
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}


CAWS_SrvApp theApp;

BOOL CAWS_SrvApp::InitInstance()
{

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();
	CShellManager *pShellManager = new CShellManager;
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CAWS_SrvDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	return FALSE;
}

