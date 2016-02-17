
// AWS_Srv.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CAWS_SrvApp:
// See AWS_Srv.cpp for the implementation of this class
//

class CAWS_SrvApp : public CWinApp
{
public:
	CAWS_SrvApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CAWS_SrvApp theApp;