// 
// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// QuickConfig.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "QuickConfig.h"
#include "QuickConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQuickConfigApp

BEGIN_MESSAGE_MAP(CQuickConfigApp, CWinApp)
	//{{AFX_MSG_MAP(CQuickConfigApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQuickConfigApp construction

CQuickConfigApp::CQuickConfigApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CQuickConfigApp object

CQuickConfigApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CQuickConfigApp initialization

BOOL CQuickConfigApp::InitInstance()
{
	// Standard initialization

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif    
    
	CQuickConfigDlg dlg(m_lpCmdLine) ;
	m_pMainWnd = &dlg ;
	int nResponse = dlg.DoModal() ;
 
	return FALSE ;
}
