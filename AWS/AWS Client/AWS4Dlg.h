
// AWS4Dlg.h : header file
//
#define WM_TRAY_NOTIFY WM_USER+567
#define TRAYICON_ID1 0x1234
#pragma once
#include "afxwin.h"
#include "iostream"
#include "time.h"

// CAWS4Dlg dialog
class CAWS4Dlg : public CDialogEx
{
// Construction
public:
	CAWS4Dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_AWS4_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnTrayNotify(UINT nID, LPARAM lEvent);
	DECLARE_MESSAGE_MAP()
	bool GetActiveProcessName(CString& resStr);
	bool EnableDebugPrivilege(BOOL bEnable);
	bool GetFileProperties(const CString pszProcessPath, CString& resStr);
	HANDLE ghSemaphore;
	CString GetProcessUsername(HANDLE *phProcess, BOOL bIncDomain);
	CString ParseLastError();
	void *HandleThread;
    unsigned long IdThread_Requster;
	unsigned long IdThread_Tester;
	static DWORD WINAPI ThreadF(void *arg);
	afx_msg inline int Request(CString);
	CString Client_ID;
	CString Server_IP;
	CString Server_PORT;
	bool m_visible;
	bool IDexists;
	bool IPrequest;
	BOOL InitInstance();
	int Test_id();
	int	Test_IP();
	int RegID(CString);
	int CAWS4Dlg::autostart();
	afx_msg void CAWS4Dlg::OnWindowPosChanging(WINDOWPOS FAR*);
	int change (char*,char*,int);
	int Pop();
	int Size();
	CString Front();
	void Push(CString);

public:
	CString m_LogView;
	afx_msg void OnClickedStart();
	afx_msg void OnClickedStop();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLButtonDblClk(UINT flags,CPoint XY);
	CButton m_StartButton;
	CButton m_StopButton;
	CEdit m_LogVCntrl;
	afx_msg void OnEnChangeserver();
	afx_msg void BT_Tray();
	CString CAWS4Dlg::timestamp(void);
	unsigned long CAWS4Dlg::Reqqueue();
	afx_msg static void Launch();
	afx_msg void CAWS4Dlg::OnClose();
	afx_msg static DWORD FindProcessId(const std::wstring& processName);
};