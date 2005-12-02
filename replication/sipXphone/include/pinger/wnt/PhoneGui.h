// $Id: //depot/OPENDEV/sipXphone/include/pinger/wnt/PhoneGui.h#2 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// PhoneGui.h : main header file for the PHONEGUI application
//

#if !defined(AFX_PHONEGUI_H__C85989A5_5A8F_11D2_8472_00104B68C34C__INCLUDED_)
#define AFX_PHONEGUI_H__C85989A5_5A8F_11D2_8472_00104B68C34C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

class CPhoneGuiDlg;

/////////////////////////////////////////////////////////////////////////////
// CPhoneGuiApp:
// See PhoneGui.cpp for the implementation of this class
//

class CPhoneGuiApp : public CWinApp
{
public:
	CPhoneGuiApp();
	static CPhoneGuiDlg* getGui();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPhoneGuiApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CPhoneGuiApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHONEGUI_H__C85989A5_5A8F_11D2_8472_00104B68C34C__INCLUDED_)
