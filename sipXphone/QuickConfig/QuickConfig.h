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

// QuickConfig.h : main header file for the QUICKCONFIG application
//

#if !defined(AFX_QUICKCONFIG_H__1726A791_1367_4BCD_8D90_EE229771CB66__INCLUDED_)
#define AFX_QUICKCONFIG_H__1726A791_1367_4BCD_8D90_EE229771CB66__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CQuickConfigApp:
// See QuickConfig.cpp for the implementation of this class
//

class CQuickConfigApp : public CWinApp
{
public:
	CQuickConfigApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQuickConfigApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CQuickConfigApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUICKCONFIG_H__1726A791_1367_4BCD_8D90_EE229771CB66__INCLUDED_)
