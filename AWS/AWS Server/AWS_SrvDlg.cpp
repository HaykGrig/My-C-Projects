#include "stdafx.h"
#include "AWS_Srv.h"
#include "AWS_SrvDlg.h"
#include "afxdialogex.h"
#include "AWS_SrvExitDlg.h"
#include "io.h"

#define MAX_SEM_COUNT 1

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HANDLE ghSemaphore;
DWORD WINAPI ThreadProc();
char *g_Port = DEFAULT_PORT;
BOOL g_bEndServer = FALSE;			// set to TRUE on CTRL-C
BOOL g_bRestart = TRUE;				// set to TRUE to CTRL-BRK
BOOL g_bVerbose = FALSE;
DWORD g_dwThreadCount = 0;		//worker thread count
HANDLE g_hIOCP = INVALID_HANDLE_VALUE;
SOCKET g_sdListen = INVALID_SOCKET;
HANDLE g_ThreadHandles[1000];//MAX_WORKER_THREAD
PPER_SOCKET_CONTEXT g_pCtxtList = NULL;		


CRITICAL_SECTION g_CriticalSection;		

//


BOOL ValidOptions(int argc, char *argv[]);

BOOL WINAPI CtrlHandler(
    DWORD dwEvent
    );

BOOL CreateListenSocket(void);

BOOL CreateAcceptSocket(
    BOOL fUpdateIOCP
    );

DWORD WINAPI WorkerThread (
    LPVOID WorkContext
    );

PPER_SOCKET_CONTEXT UpdateCompletionPort(
    SOCKET s,
    IO_OPERATION ClientIo,
    BOOL bAddToList
    );

VOID CloseClient (
    PPER_SOCKET_CONTEXT lpPerSocketContext,
    BOOL bGraceful
    );

PPER_SOCKET_CONTEXT CtxtAllocate(
    SOCKET s, 
    IO_OPERATION ClientIO
    );

VOID CtxtListFree(
    );

VOID CtxtListAddTo (
    PPER_SOCKET_CONTEXT lpPerSocketContext
    );

VOID CtxtListDeleteFrom(
    PPER_SOCKET_CONTEXT lpPerSocketContext
    );
DWORD WINAPI ThreadProc(char *);


void server ();
int myprintf(const char *lpFormat, ...);
int printip();
void printbuff(char *);
int loger(char *);
int ClientErrorloger(char *);
int IDgenerator();
bool Tester(char *,char*);
int GenerateID();

//
int WeekDay,Day,Month,Year,rDay,rMonth,rYear,rWeekdate;
double Weekdate;
int f_switch(char*,char*);
int Delete (int);
int Write (int,char*);
int Overwrite(int,char*);
void DayDate(int,int);
int Search(char *);
int Form();
void DataUpdater(int,char *,int);
int GetLastDate(char *);
int SearchEmployeebyDate(int,int,int,int);
void Employee(int);
void MonthDate(int);
void WeekDate(int);
void YearDate(int);
time_t t;
bool logdetected;
tm* timePtr;
bool FileExists(const char *);

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();


	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);  


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






CAWS_SrvDlg::CAWS_SrvDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAWS_SrvDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAWS_SrvDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAWS_SrvDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_TRAY_NOTIFY, OnTrayNotify)
	ON_BN_CLICKED(IDC_BUTTON1, &CAWS_SrvDlg::BT_Tray)
	ON_BN_CLICKED(IDCANCEL, &CAWS_SrvDlg::OnBnClickedCancel)
	ON_WM_CLOSE ()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_Statistic, &CAWS_SrvDlg::OnBnClickedStatistic)
END_MESSAGE_MAP()



BOOL CAWS_SrvDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetWindowText(L"AWS Server");

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

	if(FileExists("Log.txt"))
	{
		rYear=GetLastDate("<Year");
		rMonth=GetLastDate("<Month");
		rWeekdate=GetLastDate("<Week");
		rDay=GetLastDate("<Day");
	}
	else
	{
		rYear=-1;
		rMonth=-1;
		rWeekdate=-1;
		rDay=-1;
	}

	ghSemaphore = CreateSemaphore(NULL,MAX_SEM_COUNT,MAX_SEM_COUNT,NULL);

	SetTimer(1,1000,0);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)server, this, 0, &id_srv);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAWS_SrvDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}


void CAWS_SrvDlg::OnPaint()
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
HCURSOR CAWS_SrvDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAWS_SrvDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent)
	{
	case 1:
		{
			time_t t = time(NULL);
			tm* timePtr = localtime(&t);
			Day=timePtr->tm_mday;
			Month=timePtr->tm_mon+1;
			Year=timePtr->tm_year+1900;

			Weekdate=double(Day)/7;
			if((int)Weekdate!=Weekdate)
			{
				Weekdate=(int)Weekdate+1;
			}
			if(timePtr->tm_wday==0)
			{
				WeekDay=7;
			}
			else
			{
				WeekDay=timePtr->tm_wday;
			}

			if(rYear!=Year)
			{
				rYear=Year;
				rMonth=Month;
				rWeekdate=Weekdate;
				rDay=Day;
				YearDate(Year);
			}
			else
			if(rMonth!=Month)
			{
				rMonth=Month;
				rWeekdate=Weekdate;
				rDay=Day;
				MonthDate(Month);
			}
			else
			if(rWeekdate!=Weekdate)
			{
				rWeekdate=Weekdate;
				rDay=Day;
				WeekDate(Weekdate);
			}
			else
			if(rDay!=Day)
			{
				rDay=Day;
				DayDate(Day,WeekDay);
			}
		}
		break;
	case 2:
		{
			//22.3.2015 22:4
			//DataUpdater(23411,"23:3:2015:18:6:",29);

		}
		break;
	}

}

void server () {
	
	SYSTEM_INFO systemInfo;
	WSADATA wsaData;
	SOCKET sdAccept = INVALID_SOCKET;
	PPER_SOCKET_CONTEXT lpPerSocketContext = NULL;
	DWORD dwRecvNumBytes = 0;     
	DWORD dwFlags = 0;       
	char* file_name = "ServerError.log";
	FILE *file = fopen( file_name, "a" ); 
	int nRet = 0;

	for( int i = 0; i < MAX_WORKER_THREAD; i++ ) {
		g_ThreadHandles[i] = INVALID_HANDLE_VALUE;
	}


	if( !SetConsoleCtrlHandler(CtrlHandler, TRUE) ) {
		fputs("SetConsoleCtrlHandler() failed to install console handler\n", file );
		fclose(file);
		return;
	}

	GetSystemInfo(&systemInfo);
	g_dwThreadCount = systemInfo.dwNumberOfProcessors * 2;

	if( (nRet = WSAStartup(MAKEWORD(2,2), &wsaData)) != 0 ) {
		fputs("WSAStartup() failed: %d\n\n", file );
		fclose(file);
		SetConsoleCtrlHandler(CtrlHandler, FALSE);
		return;
	}

	__try
    {
        InitializeCriticalSection(&g_CriticalSection);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
		fputs("InitializeCriticalSection raised exception.\n", file );
		fclose(file);
        return;

    }

	while( g_bRestart ) {
		g_bRestart = FALSE;
		g_bEndServer = FALSE;

		__try {
			g_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
			if( g_hIOCP == NULL ) {
				fputs("CreateIoCompletionPort() failed to create I/O completion port\n", file );
				fclose(file);
				__leave;
			}

			for( DWORD dwCPU = 0; dwCPU < g_dwThreadCount; dwCPU++ ) {

				HANDLE hThread = INVALID_HANDLE_VALUE;
				DWORD dwThreadId = 0;

				hThread = CreateThread(NULL, 0, WorkerThread, g_hIOCP, 0, &dwThreadId);
				if( hThread == NULL ) {
					fputs("CreateThread() failed to create worker thread\n", file );
					fclose(file);
					__leave;
				}
				g_ThreadHandles[dwCPU] = hThread;
				hThread = INVALID_HANDLE_VALUE;
			}

			if( !CreateListenSocket() )
			{
				fclose(file);
				__leave;
			}

			while( TRUE ) {

				sdAccept = WSAAccept(g_sdListen, NULL, NULL, NULL, 0);
				if( sdAccept == SOCKET_ERROR ) {

					fputs("WSAAccept() failed\n", file );
					fclose(file);
					__leave;
				}

				lpPerSocketContext = UpdateCompletionPort(sdAccept, ClientIoRead, TRUE);
				if( lpPerSocketContext == NULL )
				{
					fclose(file);
					__leave;
				}

				if( g_bEndServer )
				{
					fclose(file);
					break;
				}

				nRet = WSARecv(sdAccept, &(lpPerSocketContext->pIOContext->wsabuf), 
							   1, &dwRecvNumBytes, &dwFlags,
							   &(lpPerSocketContext->pIOContext->Overlapped), NULL);
				if( nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError()) ) {
					fputs("WSARecv() Failed\n", file );
					fclose(file);
					CloseClient(lpPerSocketContext, FALSE);
				}
			} //while
		}

		__finally   {

			g_bEndServer = TRUE;

            //
			// Cause worker threads to exit
			//
			if( g_hIOCP ) {
				for( DWORD i = 0; i < g_dwThreadCount; i++ )
					PostQueuedCompletionStatus(g_hIOCP, 0, 0, NULL);
			}
            
			//
			//Make sure worker threads exits.
            //
			if( WAIT_OBJECT_0 != WaitForMultipleObjects( g_dwThreadCount,  g_ThreadHandles, TRUE, 1000) )
			{
				fputs("WaitForMultipleObjects() failed\n", file );
				fclose(file);
			}
			else
				for( DWORD i = 0; i < g_dwThreadCount; i++ ) {
					if( g_ThreadHandles[i] != INVALID_HANDLE_VALUE ) CloseHandle(g_ThreadHandles[i]);
					g_ThreadHandles[i] = INVALID_HANDLE_VALUE;
				}

			CtxtListFree();

			if( g_hIOCP ) {
				fclose(file);
				CloseHandle(g_hIOCP);
				g_hIOCP = NULL;
			}

			if( g_sdListen != INVALID_SOCKET ) {
				fclose(file);
				closesocket(g_sdListen); 
				g_sdListen = INVALID_SOCKET;
			}

			if( sdAccept != INVALID_SOCKET ) {
				fclose(file);
				closesocket(sdAccept); 
				sdAccept = INVALID_SOCKET;
			}

		} //finally

		if( g_bRestart ) {
			fputs("AWS-Server is restarting...\n", file );
			fclose(file);
		} else
		{
			fputs("AWS-Server is exiting...\n", file );
			fclose(file);
			exit(1);
		}

	} //while (g_bRestart)
	fclose(file);
	DeleteCriticalSection(&g_CriticalSection);
	WSACleanup();
	SetConsoleCtrlHandler(CtrlHandler, FALSE);
} //main      


BOOL ValidOptions(int argc, char *argv[]) {

	BOOL bRet = TRUE;

	for( int i = 1; i < argc; i++ ) {
		if( (argv[i][0] =='-') || (argv[i][0] == '/') ) {
			switch( tolower(argv[i][1]) ) {
			case 'e':
				if( strlen(argv[i]) > 3 )
					g_Port = &argv[i][3];
				break;

			case 'v':
				g_bVerbose = TRUE;
				break;

			case '?':
				myprintf("Usage:\n  iocpserver [-p:port] [-v] [-?]\n");
				myprintf("  -e:port\tSpecify echoing port number\n");        
				myprintf("  -v\t\tVerbose\n");        
				myprintf("  -?\t\tDisplay this help\n");
				bRet = FALSE;
				break;

			default:
				myprintf("Unknown options flag %s\n", argv[i]);
				bRet = FALSE;
				break;
			}
		}
	}   

	return(bRet);
}

//
//  Intercept CTRL-C or CTRL-BRK events and cause the server to initiate shutdown.
//  CTRL-BRK resets the restart flag, and after cleanup the server restarts.
//
BOOL WINAPI CtrlHandler (DWORD dwEvent) {

	SOCKET sockTemp = INVALID_SOCKET;

	switch( dwEvent ) {
	case CTRL_BREAK_EVENT: 
		g_bRestart = TRUE;
	case CTRL_C_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
	case CTRL_CLOSE_EVENT:
		if( g_bVerbose )
			myprintf("CtrlHandler: closing listening socket\n");
		//
		// cause the accept in the main thread loop to fail
		//

		sockTemp = g_sdListen;
		g_sdListen = INVALID_SOCKET;
		g_bEndServer = TRUE;
		closesocket(sockTemp);
		sockTemp = INVALID_SOCKET;
		break;

	default:
		// unknown type--better pass it on.
		return(FALSE);
	}
	return(TRUE);
}

//
//  Create a listening socket.
//
BOOL CreateListenSocket(void) {

	int nRet = 0;
	int nZero = 0;
	struct addrinfo hints = {0};
	struct addrinfo *addrlocal = NULL;
	FILE *file;
	char* file_name = "ServerError.log";
	file = fopen( file_name, "a" ); 

	//
	// Resolve the interface
	//
    hints.ai_flags  = AI_PASSIVE;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_IP;

	if( getaddrinfo(NULL, g_Port, &hints, &addrlocal) != 0 ) {
		fputs("getaddrinfo() failed with error\n", file );
		fclose(file);
        return(FALSE);
	}

	if( addrlocal == NULL ) {
		fputs("getaddrinfo() failed to resolve/convert the interface\n", file );
		fclose(file);
        return(FALSE);
	}

	g_sdListen = WSASocket(addrlocal->ai_family, addrlocal->ai_socktype, addrlocal->ai_protocol, 
						   NULL, 0, WSA_FLAG_OVERLAPPED); 
	if( g_sdListen == INVALID_SOCKET ) {
		fputs("WSASocket(g_sdListen) failed\n", file );
		fclose(file);
		return(FALSE);
	}

	nRet = bind(g_sdListen, addrlocal->ai_addr, (int) addrlocal->ai_addrlen);
	if( nRet == SOCKET_ERROR ) {
		fputs("bind() failed\n", file );
		fclose(file);
		return(FALSE);
	}

	nRet = listen(g_sdListen, 5);
	if( nRet == SOCKET_ERROR ) {
		fputs("listen() failed\n", file );
		fclose(file);
		return(FALSE);
	}

	nZero = 0;
	nRet = setsockopt(g_sdListen, SOL_SOCKET, SO_SNDBUF, (char *)&nZero, sizeof(nZero));
	if( nRet == SOCKET_ERROR ) {
		fputs("setsockopt(SNDBUF) failed\n", file );
		fclose(file);
		return(FALSE);
	}

	freeaddrinfo(addrlocal);
	fclose(file);
	return(TRUE);
}
DWORD WINAPI WorkerThread (LPVOID WorkThreadContext) {

	FILE *file;
	char* file_name = "ServerError.log";
	file = fopen( file_name, "a" ); 

	HANDLE hIOCP = (HANDLE)WorkThreadContext;
	BOOL bSuccess = FALSE;
	int nRet = 0;
	LPWSAOVERLAPPED lpOverlapped = NULL;
	PPER_SOCKET_CONTEXT lpPerSocketContext = NULL;
	PPER_IO_CONTEXT lpIOContext = NULL; 
	WSABUF buffRecv;
	WSABUF buffSend;
	DWORD dwRecvNumBytes = 0;
	DWORD dwSendNumBytes = 0;
	DWORD dwFlags = 0;
	DWORD dwIoSize = 0;
	while( TRUE ) {

        //
		// continually loop to service io completion packets
		//
		bSuccess = GetQueuedCompletionStatus(hIOCP, &dwIoSize,
											 (PDWORD_PTR)&lpPerSocketContext,
											 (LPOVERLAPPED *)&lpOverlapped, 
											 INFINITE);
		if( !bSuccess )
			fputs("GetQueuedCompletionStatus() failed\n", file );

		if( lpPerSocketContext == NULL ) {

			//
			// CTRL-C handler used PostQueuedCompletionStatus to post an I/O packet with
			// a NULL CompletionKey (or if we get one for any reason).  It is time to exit.
			//
			fclose(file);
			return(0);
		}

		if( g_bEndServer ) {

			//
			// main thread will do all cleanup needed - see finally block
			//
			fclose(file);
			return(0);
		}

		if( !bSuccess || (bSuccess && (dwIoSize == 0)) ) {

			fclose(file);
			CloseClient(lpPerSocketContext, FALSE); 
			continue;
		}


		lpIOContext = (PPER_IO_CONTEXT)lpOverlapped;
		switch( lpIOContext->IOOperation ) {
		case ClientIoRead:


			lpIOContext->IOOperation = ClientIoWrite;
			lpIOContext->nTotalBytes = dwIoSize;
			lpIOContext->nSentBytes  = 0;
			lpIOContext->wsabuf.len  = dwIoSize;
			dwFlags = 0;
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if(lpIOContext->wsabuf.buf[0]=='I'&&lpIOContext->wsabuf.buf[1]=='D'&&lpIOContext->wsabuf.buf[2]=='?')
			{
				lpIOContext->wsabuf.len=10;
				lpIOContext->wsabuf.buf="Error";
				srand(time(0));
				int ID=GenerateID();
				char k[100],c[100];
				if(ID)
				{
					sprintf_s(c, "%d", ID);
					int i=0;
					k[0]='I';
					k[1]='D';
					k[2]='=';
					while(c[i]!='\0')
					{
						k[i+3]=c[i];
						i++;
					}
					lpIOContext->wsabuf.buf=k;
					//std::cout<<"Server:\tNew Client ID has been Registered"<<std::endl;
				}
				fclose(file);
			}
			else if(lpIOContext->wsabuf.buf[0]=='I'&&lpIOContext->wsabuf.buf[1]=='P'&&lpIOContext->wsabuf.buf[2]=='?')
			{
				lpIOContext->wsabuf.buf="IP!";
				fclose(file);
				//std::cout<<"Server:\tNew Client has been Connected"<<std::endl;
			}
			else if(lpIOContext->wsabuf.buf[0]=='!')
			{
				ClientErrorloger(lpIOContext->Buffer);
				fclose(file);
				lpIOContext->wsabuf.buf="ok";
			}
			else
			{
				
				/// logggggeeerrrrr // loger
				ThreadProc(lpIOContext->Buffer);
				lpIOContext->wsabuf.buf="ok";
				fclose(file);
			}
			nRet = WSASend(lpPerSocketContext->Socket, &lpIOContext->wsabuf, 1, 
						   &dwSendNumBytes, dwFlags, &(lpIOContext->Overlapped), NULL);

			//
			//
			if( nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError()) ) {
				fputs("WSASend() failed\n", file );
				fclose(file);
				CloseClient(lpPerSocketContext, FALSE);
			} else if( g_bVerbose ) {
				fclose(file);
				myprintf("WorkerThread %d: Socket(%d) Recv completed (%d bytes), Send posted\n", 
					   GetCurrentThreadId(), lpPerSocketContext->Socket, dwIoSize);
			}
			fclose(file);
			break;

		case ClientIoWrite:

			//
			// a write operation has completed, determine if all the data intended to be
			// sent actually was sent.
			//
			lpIOContext->IOOperation = ClientIoWrite;
			lpIOContext->nSentBytes  += dwIoSize;
			dwFlags = 0;
			if( lpIOContext->nSentBytes < lpIOContext->nTotalBytes ) {

				//
				// the previous write operation didn't send all the data,
				// post another send to complete the operation
				//
				buffSend.buf = lpIOContext->Buffer + lpIOContext->nSentBytes;
				buffSend.len = lpIOContext->nTotalBytes - lpIOContext->nSentBytes;
				nRet = WSASend (lpPerSocketContext->Socket, &buffSend, 1, 
								&dwSendNumBytes, dwFlags, &(lpIOContext->Overlapped), NULL);
				if( nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError()) ) {
					fputs("WSASend() failed\n", file );
					fclose(file);
					CloseClient(lpPerSocketContext, FALSE);
				} else if( g_bVerbose ) {
					fclose(file);
					myprintf("WorkerThread %d: Socket(%d) Send partially completed (%d bytes), Recv posted\n", 
						   GetCurrentThreadId(), lpPerSocketContext->Socket, dwIoSize);
				}
			} else {

				//
				// previous write operation completed for this socket, post another recv
				//
				
				lpIOContext->IOOperation = ClientIoRead; 
				dwRecvNumBytes = 0;
				dwFlags = 0;
				buffRecv.buf = lpIOContext->Buffer,
				buffRecv.len = MAX_BUFF_SIZE;
				nRet = WSARecv(lpPerSocketContext->Socket, &buffRecv, 1, 
							   &dwRecvNumBytes, &dwFlags, &lpIOContext->Overlapped, NULL);
				if( nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError()) ) {
					fputs("WSASend() failed\n", file );
					fclose(file);
					CloseClient(lpPerSocketContext, FALSE);
				} else if( g_bVerbose ) {
					fclose(file);
					myprintf("WorkerThread %d: Socket(%d) Send completed (%d bytes), Recv posted\n", 
						   GetCurrentThreadId(), lpPerSocketContext->Socket, dwIoSize);
				}
			}
			//printbuff(lpIOContext->Buffer);
			//if(log)
			//loger(lpIOContext->Buffer);
			
			fclose(file);
			break;

		} //switch
	} //while
	fclose(file);
	return(0);
} 

//
//  Allocate a context structures for the socket and add the socket to the IOCP.  
//  Additionally, add the context structure to the global list of context structures.
//
PPER_SOCKET_CONTEXT UpdateCompletionPort(SOCKET sd, IO_OPERATION ClientIo,
										 BOOL bAddToList) {

	FILE *file;
	char* file_name = "ServerError.log";
	file = fopen( file_name, "a" ); 

	PPER_SOCKET_CONTEXT lpPerSocketContext;

	lpPerSocketContext = CtxtAllocate(sd, ClientIo);
	if( lpPerSocketContext == NULL )
	{
		fclose(file);
		return(NULL);
	}

	g_hIOCP = CreateIoCompletionPort((HANDLE)sd, g_hIOCP, (DWORD_PTR)lpPerSocketContext, 0);
	if( g_hIOCP == NULL ) {
		fputs("CreateIoCompletionPort() failed\n", file );
		fclose(file);
		if( lpPerSocketContext->pIOContext )
			xfree(lpPerSocketContext->pIOContext);
		xfree(lpPerSocketContext);
		return(NULL);
	}

    //
	//The listening socket context (bAddToList is FALSE) is not added to the list.
	//All other socket contexts are added to the list.
	//
	if( bAddToList ) CtxtListAddTo(lpPerSocketContext);

	if( g_bVerbose )
		myprintf("UpdateCompletionPort: Socket(%d) added to IOCP\n", lpPerSocketContext->Socket);
	fclose(file);
	return(lpPerSocketContext);
}

//
//  Close down a connection with a client.  This involves closing the socket (when 
//  initiated as a result of a CTRL-C the socket closure is not graceful).  Additionally, 
//  any context data associated with that socket is free'd.
//
VOID CloseClient (PPER_SOCKET_CONTEXT lpPerSocketContext,
				  BOOL bGraceful) {

	FILE *file;
	char* file_name = "ServerError.log";
	file = fopen( file_name, "a" ); 

    __try
    {
        EnterCriticalSection(&g_CriticalSection);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
		fputs("EnterCriticalSection raised an exception.\n", file );
		fclose(file);
        return;
    }

	if( lpPerSocketContext ) {
		if( g_bVerbose )
			myprintf("CloseClient: Socket(%d) connection closing (graceful=%s)\n",
				   lpPerSocketContext->Socket, (bGraceful?"TRUE":"FALSE"));
		if( !bGraceful ) {

			//
			// force the subsequent closesocket to be abortative.
			//
			LINGER  lingerStruct;

			lingerStruct.l_onoff = 1;
			lingerStruct.l_linger = 0;
			setsockopt(lpPerSocketContext->Socket, SOL_SOCKET, SO_LINGER,
					   (char *)&lingerStruct, sizeof(lingerStruct) );
		}
		closesocket(lpPerSocketContext->Socket);
		lpPerSocketContext->Socket = INVALID_SOCKET;
		CtxtListDeleteFrom(lpPerSocketContext);
		lpPerSocketContext = NULL;
	} else {
		myprintf("CloseClient: lpPerSocketContext is NULL\n");
	}

    LeaveCriticalSection(&g_CriticalSection);
	fclose(file);
	return;    
} 

//
// Allocate a socket context for the new connection.  
//
PPER_SOCKET_CONTEXT CtxtAllocate(SOCKET sd, IO_OPERATION ClientIO) {

	FILE *file;
	char* file_name = "ServerError.log";
	file = fopen( file_name, "a" ); 

	PPER_SOCKET_CONTEXT lpPerSocketContext;

	__try
    {
        EnterCriticalSection(&g_CriticalSection);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
		fputs("EnterCriticalSection raised an exception.\n", file );
		fclose(file);
        return NULL;
    }

	lpPerSocketContext = (PPER_SOCKET_CONTEXT)xmalloc(sizeof(PER_SOCKET_CONTEXT));
	if( lpPerSocketContext ) {
		lpPerSocketContext->pIOContext = (PPER_IO_CONTEXT)xmalloc(sizeof(PER_IO_CONTEXT));
		if( lpPerSocketContext->pIOContext ) {
			lpPerSocketContext->Socket = sd;
			lpPerSocketContext->pCtxtBack = NULL;
			lpPerSocketContext->pCtxtForward = NULL;

			lpPerSocketContext->pIOContext->Overlapped.Internal = 0;
			lpPerSocketContext->pIOContext->Overlapped.InternalHigh = 0;
			lpPerSocketContext->pIOContext->Overlapped.Offset = 0;
			lpPerSocketContext->pIOContext->Overlapped.OffsetHigh = 0;
			lpPerSocketContext->pIOContext->Overlapped.hEvent = NULL;
			lpPerSocketContext->pIOContext->IOOperation = ClientIO;
			lpPerSocketContext->pIOContext->pIOContextForward = NULL;
			lpPerSocketContext->pIOContext->nTotalBytes = 0;
			lpPerSocketContext->pIOContext->nSentBytes  = 0;
			lpPerSocketContext->pIOContext->wsabuf.buf  = lpPerSocketContext->pIOContext->Buffer;
			lpPerSocketContext->pIOContext->wsabuf.len  = sizeof(lpPerSocketContext->pIOContext->Buffer);

			ZeroMemory(lpPerSocketContext->pIOContext->wsabuf.buf, lpPerSocketContext->pIOContext->wsabuf.len);
		} else {
			xfree(lpPerSocketContext);
			fputs("HeapAlloc() PER_IO_CONTEXT failed\n", file );

		}

	} else {
		fputs("HeapAlloc() PER_SOCKET_CONTEXT failed", file );
	}

	LeaveCriticalSection(&g_CriticalSection);
	fclose(file);
	return(lpPerSocketContext);
} 

//
//  Add a client connection context structure to the global list of context structures.
//
VOID CtxtListAddTo (PPER_SOCKET_CONTEXT lpPerSocketContext) {

	FILE *file;
	char* file_name = "ServerError.log";
	file = fopen( file_name, "a" ); 

	PPER_SOCKET_CONTEXT     pTemp;

	__try
    {
        EnterCriticalSection(&g_CriticalSection);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
		fputs("EnterCriticalSection raised an exception.\n", file );
		fclose(file);
        return;
    }

	if( g_pCtxtList == NULL ) {

		//
		// add the first node to the linked list
		//
		lpPerSocketContext->pCtxtBack    = NULL;
		lpPerSocketContext->pCtxtForward = NULL;
		g_pCtxtList = lpPerSocketContext;
	} else {

		//
		// add node to head of list
		//
		pTemp = g_pCtxtList;

		g_pCtxtList = lpPerSocketContext;
		lpPerSocketContext->pCtxtBack    = pTemp;
		lpPerSocketContext->pCtxtForward = NULL; 

		pTemp->pCtxtForward = lpPerSocketContext;
	}

	LeaveCriticalSection(&g_CriticalSection);
	fclose(file);
	return;
}

//
//  Remove a client context structure from the global list of context structures.
//
VOID CtxtListDeleteFrom(PPER_SOCKET_CONTEXT lpPerSocketContext) {

	FILE *file;
	char* file_name = "ServerError.log";
	file = fopen( file_name, "a" ); 

	PPER_SOCKET_CONTEXT pBack;
	PPER_SOCKET_CONTEXT pForward;
	PPER_IO_CONTEXT     pNextIO     = NULL;
	PPER_IO_CONTEXT     pTempIO     = NULL;

	
    __try
    {
        EnterCriticalSection(&g_CriticalSection);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
		fputs("EnterCriticalSection raised an exception.\n", file );
		fclose(file);
        return;
    }

	if( lpPerSocketContext ) {
		pBack       = lpPerSocketContext->pCtxtBack;
		pForward    = lpPerSocketContext->pCtxtForward;


		if( ( pBack == NULL ) && ( pForward == NULL ) ) {

			//
			// This is the only node in the list to delete
			//
			g_pCtxtList = NULL;
		} else if ( ( pBack == NULL ) && ( pForward != NULL ) ) {

			//
			// This is the start node in the list to delete
			//
			pForward->pCtxtBack = NULL;
			g_pCtxtList = pForward;
		} else if ( ( pBack != NULL ) && ( pForward == NULL ) ) {

			//
			// This is the end node in the list to delete
			//
			pBack->pCtxtForward = NULL;
		} else if( pBack && pForward ) {

			//
			// Neither start node nor end node in the list
			//
			pBack->pCtxtForward = pForward;
			pForward->pCtxtBack = pBack;
		}

		//
		// Free all i/o context structures per socket
		//
		pTempIO = (PPER_IO_CONTEXT)(lpPerSocketContext->pIOContext);
		do {
			pNextIO = (PPER_IO_CONTEXT)(pTempIO->pIOContextForward);
			if( pTempIO ) {

				//
				//The overlapped structure is safe to free when only the posted i/o has
				//completed. Here we only need to test those posted but not yet received 
				//by PQCS in the shutdown process.
				//
				if( g_bEndServer )
					while( !HasOverlappedIoCompleted((LPOVERLAPPED)pTempIO) ) Sleep(0);
				xfree(pTempIO);
				pTempIO = NULL;
			}
			pTempIO = pNextIO;
		} while( pNextIO );

		xfree(lpPerSocketContext);
		lpPerSocketContext = NULL;

	} else {
		fclose(file);
		myprintf("CtxtListDeleteFrom: lpPerSocketContext is NULL\n");
	}
	fclose(file);
	LeaveCriticalSection(&g_CriticalSection);
	return;
}

//
//  Free all context structure in the global list of context structures.
//
VOID CtxtListFree() {


	FILE *file;
	char* file_name = "ServerError.log";
	file = fopen( file_name, "a" ); 

	PPER_SOCKET_CONTEXT     pTemp1, pTemp2;

	__try
    {
        EnterCriticalSection(&g_CriticalSection);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
		fputs("EnterCriticalSection raised an exception.\n", file );
		fclose(file);
        return;
    }

	pTemp1 = g_pCtxtList; 
	while( pTemp1 ) {
		pTemp2 = pTemp1->pCtxtBack;
		CloseClient(pTemp1, FALSE);
		pTemp1 = pTemp2;
	}

	LeaveCriticalSection(&g_CriticalSection);
	fclose(file);
	return;
}

//
// Our own printf. This is done because calling printf from multiple
// threads can AV. The standard out for WriteConsole is buffered...
//
int myprintf (const char *lpFormat, ... ) {

	int nLen = 0;
	int nRet = 0;
	char cBuffer[512] ;
	va_list arglist ;
	HANDLE hOut = NULL;
    HRESULT hRet;

	ZeroMemory(cBuffer, sizeof(cBuffer));

	va_start(arglist, lpFormat);

	nLen = strlen( lpFormat ) ;
    hRet = StringCchVPrintfA(cBuffer,512,lpFormat,arglist);
	
	if( nRet >= nLen || GetLastError() == 0 ) {
		hOut = GetStdHandle(STD_OUTPUT_HANDLE) ;
		if( hOut != INVALID_HANDLE_VALUE )
			WriteConsole( hOut, cBuffer, strlen(cBuffer), (LPDWORD)&nLen, NULL ) ;
	}

	return nLen ;
}
int printip()
{
    char ac[80];
    if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) {
       // std::cout << "Error " << WSAGetLastError() <<
            //    " when getting local host name." << std::cout;
        return 1;
    }
    //std::cout << "Host name is " << ac << "." << std::endl;

    struct hostent *phe = gethostbyname(ac);
    if (phe == 0) {
       // std::cout << "Yow! Bad host lookup." << std::endl;
        return 1;
    }

    for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
        struct in_addr addr;
        memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
      //  std::cout << "Address " << ": " << inet_ntoa(addr) << std::endl<<std::endl;
    }
    
    return 0;
}

void printbuff(char * Buffer)
{
	int i=0;
		while (Buffer[i] >= 32 || Buffer[i] == '\n' || Buffer[i] == '\r') 
		{
			/*if(Buffer[i]=='\\'&&Buffer[i+1]=='n')
			{
				std::cout<<'\n';
				i=i+2;
				continue;
			}
			else
				if(Buffer[i]=='\\'&&Buffer[i+1]=='t')
			{
				std::cout<<'\t';
				i=i+2;
				continue;
			}
			else
				if(Buffer[i]=='\\'&&Buffer[i+1]=='r')
			{
				std::cout<<'\r';
				i=i+2;
				continue;
			}
				else*/
			//std::cout<<Buffer[i++];
		}	
		printip();
}
int ClientErrorloger(char *Buffer)
{
	FILE *file;
	char* file_name = "ClientErrorlog.log",Err_Buffer[100];
	int j=0;
	file = fopen( file_name, "a" ); 
	if(!file) 
	{
        return 0;
    }
	else
	{
		for(int i=0;Buffer[i];i++)
		{
			if(Buffer[i]=='\n')
			{
				Err_Buffer[j++]='\t';
			}
			else
			if(Buffer[i]=='\\'&&Buffer[i+1]=='t')
			{
				Err_Buffer[j++]=' ';
				i++;
			}
			else
			if(!(Buffer[i]=='\\'&&(Buffer[i+1]=='n'||Buffer[i+1]=='r')))
			{
				Err_Buffer[j++]=Buffer[i];
			}
			else
			{
				i++;
			}
		}
		Err_Buffer[j++]='\n';
		Err_Buffer[j]='\0';
		fputs(Err_Buffer, file );
		fclose(file);
		return 1;
	}
}
int loger(char *Buffer) 
{
	struct form
	{
		char Comp_ID[100];
		char Date[100];
		char LoggedUser[100];
		char ProductName[100];
		char WindowText[100];
		char LastUserInput[100];
		char FileDescription[100];
	}log;
FILE *file;
char* file_name = "file.log";
int j=0;
int k;
bool write=false;

log.Comp_ID[0]='\0';
log.Date[0]='\0';
log.FileDescription[0]='\0';
log.LastUserInput[0]='\0';
log.LoggedUser[0]='\0';
log.ProductName[0]='\0';
log.WindowText[0]='\0';


for(int i=0;(Buffer[i]);i++)
{
	if(Buffer[i]=='\n'||(Buffer[i-1]=='\\'&&(Buffer[i]=='t'||Buffer[i+1]=='r'||Buffer[i+1]=='n')))
	{
		continue;
	}
	if(Buffer[i]=='\\'&&Buffer[i+1]=='t')
	{
		k=0;
		j++;
		i++;
		write=true;
		continue;
	}
	if(j==1&&write&&!(Buffer[i]=='\\'&&(Buffer[i+1]=='n'||Buffer[i+1]=='r')))
	{
		log.WindowText[k++]=Buffer[i];
	}
	else
	{
		if(j==1&&write)
		{
			log.WindowText[k]='\0';
			write=false;
			continue;
		}
	}

	if(j==2&&write&&!(Buffer[i]=='\\'&&(Buffer[i+1]=='n'||Buffer[i+1]=='r')))
	{
		log.LoggedUser[k++]=Buffer[i];
	}
	else
	{
		if(j==2&&write)
		{
			log.LoggedUser[k]='\0';
			write=false;
			continue;
		}
		
	}

	if(j==3&&write&&!(Buffer[i]=='\\'&&(Buffer[i+1]=='n'||Buffer[i+1]=='r')))
	{
		log.FileDescription[k++]=Buffer[i];
	}
	else
	{
		if(j==3&&write)
		{
			log.FileDescription[k]='\0';
			write=false;
			continue;
		}
		
	}

	if(j==4&&write&&!(Buffer[i]=='\\'&&(Buffer[i+1]=='n'||Buffer[i+1]=='r')))
	{
		log.ProductName[k++]=Buffer[i];
	}
	else
	{
		if(j==4&&write)
		{
			log.ProductName[k]='\0';
			write=false;
			continue;
		}
		
	}

		if(j==5&&write&&!(Buffer[i]=='\\'&&(Buffer[i+1]=='n'||Buffer[i+1]=='r')))
	{
		log.LastUserInput[k++]=Buffer[i];
	}
	else
	{
		if(j==5&&write)
		{
			log.LastUserInput[k]='\0';
			write=false;
			continue;
		}
		
	}

	if(j==6&&write&&!(Buffer[i]=='\\'&&(Buffer[i+1]=='n'||Buffer[i+1]=='r')))
	{
		log.Date[k++]=Buffer[i];
	}
	else
	{
		if(j==6&&write)
		{
			log.Date[k]='\0';
			write=false;
			continue;
		}
		
	}
		
	if(j==7&&write&&!(Buffer[i]=='\\'&&(Buffer[i+1]=='n'||Buffer[i+1]=='r')))
	{
		log.Comp_ID[k++]=Buffer[i];
	}
	else
	{
		if(j==7&&write)
		{
			log.Comp_ID[k]='\0';
			write=false;
			continue;
		}
		
	}


	
}

file = fopen( file_name, "a" ); 
if(!file) {
        return 0;
    }

fputs( "<row id=\"", file );
fputs( log.Comp_ID, file );
fputs( "\" time=\"", file );
fputs( log.Date, file );
fputs( "\"><user>", file );
fputs( log.LoggedUser, file );
fputs( "</user><programm name=\"", file );
fputs( log.ProductName, file );
fputs( "\">", file );
fputs( log.WindowText, file );
fputs( "</programm><time>", file );
fputs( log.LastUserInput, file );
fputs( "</time></row>\n", file );
//std::cout<<"Succes...\r\n";
fclose(file);

DataUpdater(atoi(log.Comp_ID),log.Date,atoi(log.LastUserInput));

return 1;
}
int GenerateID()
{
	FILE *file;
	char* file_name = "idlist.txt",c[100];
	int a;
	file = fopen( file_name, "a" );
		if(!file)
	{
       return 0;
    }

	while(true)
	{
		a=IDgenerator();
		sprintf_s(c, "%d", a);
		if(Tester(file_name,c))
		{
			fputs(c,file);
			fputs(",",file);
			fclose(file);
			break;
		}
	}
	fclose(file);
	Employee(a);
	return a;
}

int IDgenerator()
{
	int a=0;
	int i=0;

	while (a<10000||a>100000)
	{
	    a=4*(rand()%100000);
	}
	return a;
}

bool Tester(char *file_name,char *c)
{
	FILE *file;
	file = fopen( file_name, "r" );
	char test[10000];
	fgets(test, 10000, file);
	int j=0;
	for(int i=0;test[i];i++)
	{
		if(j==5)
		{
			fclose(file);
			return 0;
		}
		else
		if(test[i]==',')
		{
			j=0;
		}
		else if(c[j]==test[i])
		{
			j++;
		}
	}
	fclose(file);
	return 1;
}

void CAWS_SrvDlg::BT_Tray()
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

LRESULT CAWS_SrvDlg::OnTrayNotify(UINT nID, LPARAM lEvent)
{
    if (nID==0x1234)
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

void CAWS_SrvDlg::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}

void CAWS_SrvDlg::OnClose()
{
	INT_PTR RetValue;
	AWS_SrvExitDlg Dlg;
	RetValue=Dlg.DoModal();
	switch(RetValue)
	{
	case IDC_Close:
         __super::OnClose();
         break;
	case IDC_Hide:
         BT_Tray();
         break;
	default:
		break;
	}
}

void CAWS_SrvDlg::OnBnClickedStatistic()
{
	system("start "" http://lanar.am");
}


void DataUpdater(int Id,char * ReqTime,int AcTime)
{
	char* Input="Log.txt";
	FILE* pIn = fopen (Input, "r+");
	char szBuf [256];
	int uDay,uMonth,uYear,uHour,uMin;
	int pStart_h,pStart_m,pEnd_h,pEnd_m,pTime=0;
	int i=0,k=0,p=0,line;
	char c[32],Start_h[32],Start_m[32],End_h[32],End_m[32],Time[32];
		
	while(ReqTime[i]!='\0')
	{
		if(ReqTime[i]==':'||ReqTime[i]=='.')
		{
			c[p]='\0';
			switch(k)
			{
				case 0:
					uDay=atoi(c);
					break;
				case 1:
					uMonth=atoi(c);
					break;
				case 2:
					uYear=atoi(c);
					break;
				case 3:
					uHour=atoi(c);
					break;
				case 4:
					uMin=atoi(c);
					break;
			}
			p=0;
			k++;
		}
		else
		{
			c[p++]=ReqTime[i];
		}
		i++;
	}//While

	line=SearchEmployeebyDate(uDay,uMonth,uYear,Id);
	i=0;
	
	while(fgets(szBuf, sizeof(szBuf), pIn))
	{
		if(i==line)
		{
			break;
		}
		i++;
	}
	i=0;
	k=0;
	p=0;
	int l=0;
	int m=0;
	int n=0;
	int b=0;
	while(szBuf[i])
	{
		if(szBuf[i]=='"'||szBuf[i]==':')
		{
			k++;
		}
		else
			if(k==3)
			{
				Start_h[p++]=szBuf[i];
				if(szBuf[i]=='-1')
				{
					k++;
				}
			}
			else
				if(k==4)
				{
					Start_m[l++]=szBuf[i];
				}
				else
					if(k==6)
					{
						End_h[m++]=szBuf[i];
						if(szBuf[i]=='-1')
						{
							k++;
						}
					}
					else
						if(k==7)
						{
							End_m[n++]=szBuf[i];
						}
						else
							if(k==9)
							{
								Time[b++]=szBuf[i];
							}
		i++;
	}//while
	Start_h[p]='\0';
	Start_m[l]='\0';
	End_h[m]='\0';
	End_m[n]='\0';
	Time[b]='\0';
	
	

	

	pStart_h=atoi(Start_h);
	pStart_m=atoi(Start_m);
	pEnd_h=atoi(End_h);
	pEnd_m=atoi(End_m);
	pTime=atoi(Time);
	
	CString msg=L"";

	if(pStart_h==-1&&AcTime<30)
	{
		pStart_h=uHour;
		pStart_m=uMin;
		
	}
	if(AcTime<30)
	{
		pEnd_h=uHour;
		pEnd_m=uMin;
		sprintf_s(c, "%d",pEnd_m);
		pTime+=30;
		msg=L"<Employee id=\"";
		sprintf_s(c, "%d",Id);
		msg+=CString(c);
		msg+=L"\" start=\"";
		sprintf_s(c, "%d",pStart_h);
		msg+=c;
		msg+=L":";
		sprintf_s(c, "%d",pStart_m);
		msg+=c;
		msg+=L"\" end=\"";
		sprintf_s(c, "%d",pEnd_h);
		msg+=c;
		msg+=L":";
		sprintf_s(c, "%d",pEnd_m);
		msg+=CString(c);
		msg+=L"\" time=\"";
		sprintf_s(c, "%d",pTime);
		msg+=c;
		msg+=L"\" />\r\n";
		
	}
	else
	{
		msg=L"<Employee id=\"";
		sprintf_s(c, "%d",Id);
		msg+=CString(c);
		msg+=L"\" start=\"";
		sprintf_s(c, "%d",pStart_h);
		msg+=c;
		if(pStart_h!=-1)
		{
			msg+=L":";
			sprintf_s(c, "%d",pStart_m);
			msg+=c;
		}
		msg+=L"\" end=\"";
		sprintf_s(c, "%d",pEnd_h);
		msg+=c;
		if(pEnd_h!=-1)
		{
			msg+=L":";
			sprintf_s(c, "%d",pEnd_m);
			msg+=c;
		}
		msg+=L"\" time=\"";
		sprintf_s(c, "%d",pTime);
		msg+=c;
		msg+=L"\" />\r\n";
	}
	
	size_t convertedCharsw = 0;
	size_t newsizew = (msg.GetLength()+1)*2;
	char * c_Msg = new char[newsizew];
	wcstombs_s(&convertedCharsw, c_Msg, newsizew, msg, _TRUNCATE );

	sprintf_s(c, "%d",line);
	fclose(pIn);
	if(AcTime<30)
	{
		Overwrite(line,c_Msg);
	}
	
	delete[] c_Msg;
	
}


int Search(char *tag)
{
	char* Input="Log.txt";
	FILE* pIn = fopen (Input, "r");
	char szBuf [256];
	int iCurStr = 0;
	int j=-1;
	if(!pIn)
	{
		fclose(pIn);
		return j;
	}
	while (fgets(szBuf, sizeof(szBuf), pIn))
	{
	   if (tag[0]==szBuf[0]&&tag[1]==szBuf[1]&&tag[2]==szBuf[2]&&tag[3]==szBuf[3])
	   {
		   j=iCurStr;
	   }
	   iCurStr++;
	}
	fclose(pIn);
	return j;
}

int SearchEmployeebyDate(int day,int month,int year,int ID)
{
	char* Input="Log.txt";
	FILE* pIn = fopen (Input, "r");
	int CurLine=0;
	char szBuf[256];
	char tag[32];
	char date[32];
	int j=-1;
	int i,k,p;
	if(!pIn)
	{
		fclose(pIn);
		return j;
	}
	//
	tag[0]='<';
	tag[1]='Y';
	tag[2]='e';
	tag[3]='a';
	//
	while (fgets(szBuf, sizeof(szBuf), pIn))
	{
		k=0;
		i=0;
		p=0;
	   if (tag[0]==szBuf[0]&&tag[1]==szBuf[1]&&tag[2]==szBuf[2]&&tag[3]==szBuf[3])
	   {
		   while(k!=2)
		   {
			   if(szBuf[i]=='"')
			   {
				   k++;
				   i++;
				   continue;
			   }
			   if(k==1)
			   {
				  date[p]=szBuf[i];
				  p++;
			   }

			   i++;
		   }
		   date[p]='\0';
	   }
	   if(atoi(date)==year)
	   {

		   tag[0]='<';
		   tag[1]='M';
		   tag[2]='o';
		   tag[3]='n';
	   }
	   else
		   if(atoi(date)==month&&tag[1]=='M')
		   {

		   tag[0]='<';
		   tag[1]='D';
		   tag[2]='a';
		   tag[3]='y';
		   }
		   else
			   if(atoi(date)==day&&tag[1]=='D')
			   {
				   tag[0]='<';
				   tag[1]='E';
				   tag[2]='m';
				   tag[3]='p';
			   }
			   else
				   if(atoi(date)==ID&&tag[1]=='E')
				   {
					   fclose(pIn);
					   return CurLine;
				   }
		CurLine++;
	}
	fclose(pIn);
	return j;
}

int GetLastDate(char *tag)
{
	char* Input="Log.txt";
	FILE* pIn = fopen (Input, "r");
	char szBuf [256];
	char date[32];
	int j=-1;
	int i,k,p;
	if(!pIn)
	{
		fclose(pIn);
		return j;
	}
	while (fgets(szBuf, sizeof(szBuf), pIn))
	{
		k=0;
		i=0;
		p=0;
	   if (tag[0]==szBuf[0]&&tag[1]==szBuf[1]&&tag[2]==szBuf[2]&&tag[3]==szBuf[3])
	   {
		   while(k!=2)
		   {
			   if(szBuf[i]=='"')
			   {
				   k++;
				   i++;
				   continue;
			   }
			   if(k==1)
			   {
				  date[p]=szBuf[i];
				  p++;
			   }

			   i++;
		   }
		   date[p]='\0';
	   }
	}
	fclose(pIn);
	return atoi(date);
}

void YearDate(int Year)
{
	char c[10];
	sprintf_s(c, "%d", Year);
	CString msg=L"";
	msg=L"<Year date=\""+CString(c)+L"\">\r\n</Year>\r\n";
	size_t convertedCharsw = 0;
	size_t newsizew = (msg.GetLength()+1)*2;
	//From CString to char
	char * c_msg = new char[newsizew];
	wcstombs_s(&convertedCharsw, c_msg, newsizew, msg, _TRUNCATE );
	FILE* pIn = fopen ("Log.txt", "a");
	fputs(c_msg,pIn);
	fclose(pIn);
	delete[] c_msg;
	MonthDate(Month);
}

void MonthDate(int Month)
{
	int line=Search("</Year>");
	char c[10];
	sprintf_s(c, "%d", Month);
	CString msg=L"";
	msg=L"<Month date=\""+CString(c)+L"\">\r\n</Month>\r\n";
	size_t convertedCharsw = 0;
	size_t newsizew = (msg.GetLength()+1)*2;
	//From CString to char
	char * c_msg = new char[newsizew];
	wcstombs_s(&convertedCharsw, c_msg, newsizew, msg, _TRUNCATE );

	Write(line,c_msg);
	delete[] c_msg;
	WeekDate(Weekdate);
}

void WeekDate(int Weekdate)
{
	int line=Search("</Month>");
	char c[10];
	sprintf_s(c, "%d", Weekdate);
	CString msg=L"";
	msg=L"<Week date=\""+CString(c)+L"\">\r\n</Week>\r\n";
	size_t convertedCharsw = 0;
	size_t newsizew = (msg.GetLength()+1)*2;
	//From CString to char
	char * c_msg = new char[newsizew];
	wcstombs_s(&convertedCharsw, c_msg, newsizew, msg, _TRUNCATE );

	Write(line,c_msg);
	delete[] c_msg;
	DayDate(Day,WeekDay);
}

void DayDate(int Day,int WDay)
{
	int line=Search("</Week>");
	char c[10],k[10];
	sprintf_s(c, "%d", WDay);
	sprintf_s(k, "%d", Day);

	CString msg=L"";
	msg=L"<Day date=\""+CString(k)+L"\" Weekday=\""+CString(c)+L"\">\r\n</Day>\r\n";

	size_t convertedCharsw = 0;
	size_t newsizew = (msg.GetLength()+1)*2;
	//From CString to char
	char * c_msg = new char[newsizew];
	wcstombs_s(&convertedCharsw, c_msg, newsizew, msg, _TRUNCATE );

	Write(line,c_msg);

	delete[] c_msg;
	Form();
}

void Employee(int ID)
{
	int line=Search("</Day>");
	char c[10];
	sprintf_s(c, "%d", ID);

	CString msg=L"";
	msg=L"<Employee id=\""+CString(c)+L"\" start=\"-1\" end=\"-1\" time=\"0\"/>\r\n";

	size_t convertedCharsw = 0;
	size_t newsizew = (msg.GetLength()+1)*2;
	//From CString to char
	char * c_msg = new char[newsizew];
	wcstombs_s(&convertedCharsw, c_msg, newsizew, msg, _TRUNCATE );

	Write(line,c_msg);

	delete[] c_msg;
}

int f_switch(char* Input,char* Output)
{
	FILE* pOut = fopen (Input, "w");
				FILE* pIn = fopen (Output, "r");
				if(!pOut||!pIn)
				{
					fclose (pOut);
					return 0;
				}
				char szBuf [1024];
 
			while (fgets(szBuf, sizeof(szBuf), pIn))
			{
				fputs(szBuf,pOut);
			}
 
			fclose (pOut);
			fclose (pIn);
			return 1;
}

int Delete (int iNumStr)
{
	char* Input="Log.txt",*Output="Logout.txt";
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
	return f_switch(Input,Output);
}

int Write (int iNumStr,char* value)
{
	char* Input="Log.txt",*Output="Logout.txt";
	FILE* pIn = fopen (Input, "r");
	FILE* pOut = fopen (Output, "w");
	if(!pOut||!pIn)
	{
		fclose (pOut);
		return 0;
	}
	int iCurStr = 0;
	char szBuf [256];
 
	while (fgets(szBuf, sizeof(szBuf), pIn))
	{
	   if (iCurStr != iNumStr)
	   {
		   fputs (szBuf, pOut);
	   }
	   else
	   {
		   fputs(value,pOut);
		   fputs (szBuf, pOut);
	   }
	   iCurStr++;
	}
	fclose (pOut);
	fclose (pIn);
	return f_switch(Input,Output);
}

int Overwrite(int line,char *value)
{
	char* Input="Log.txt",*Output="Logout.txt";
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

	   if (i&&iCurStr == line)
	   {
		   fputs (value, pOut);
	   }
	   else if(i)
	   {
		   fputs (szBuf, pOut);
	   }
	   iCurStr++;
	}
 
	fclose (pOut);
	fclose (pIn);
	return f_switch(Input,Output);
}

bool FileExists(const char *fname)
{
	return _access(fname, 0) != -1;
}

int Form()
{
	char* Input="idlist.txt";
	FILE* pIn = fopen (Input, "r");
	if(!pIn)
	{
		return 0;
	}
	int i=0,j=0;
	char Buffer[2048];
	char Id[32];
	fgets(Buffer,2048,pIn);
	while(Buffer[i])
	{
		if(Buffer[i]==',')
		{
			Id[j]='\0';
			Employee(atoi(Id));
			j=0;
		}
		else
		{
			Id[j]=Buffer[i];
			j++;
		}
		i++;
	}


	fclose(pIn);
	return 1;
}

DWORD WINAPI ThreadProc(char * Buffer)
{
    DWORD dwWaitResult; 
    BOOL bContinue=TRUE;
    while(bContinue)
    {

        dwWaitResult = WaitForSingleObject(ghSemaphore,0L);

        switch (dwWaitResult) 
        { 
            case WAIT_OBJECT_0: 
                bContinue=FALSE;            

                // Simulate thread spending time on task <-----
				loger(Buffer);
                // Release the semaphore when task is finished

                if (!ReleaseSemaphore( 
                        ghSemaphore,  // handle to semaphore
                        1,            // increase count by one
                        NULL) )       // not interested in previous count
                {

                    //printf("ReleaseSemaphore error: %d\n", GetLastError()); <---- Error
                }
                break; 
            case WAIT_TIMEOUT: 
                //printf("Thread %d: wait timed out\n", GetCurrentThreadId());
                break; 
        }
    }
    return TRUE;
}
