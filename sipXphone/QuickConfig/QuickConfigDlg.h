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

// QuickConfigDlg.h : header file
//

#if !defined(AFX_QUICKCONFIGDLG_H__28502C57_24EE_4553_8E6D_9EB95D5D5263__INCLUDED_)
#define AFX_QUICKCONFIGDLG_H__28502C57_24EE_4553_8E6D_9EB95D5D5263__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CQuickConfigDlg dialog

class CQuickConfigDlg : public CDialog
{
// Construction
public:
	CQuickConfigDlg(const char* szDataDir, CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CQuickConfigDlg)
	enum { IDD = IDD_CONFIGURE };
	CString	m_cstrOtherSettings;
	CString	m_cstrPassword;
	CString	m_cstrSipServer;
	CString	m_cstrStunServer;
	CString	m_cstrUsername;
	CString	m_cstrDisplayName;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQuickConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
    const char* m_szDataDir ;

	// Generated message map functions
	//{{AFX_MSG(CQuickConfigDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    BOOL parseConfigFile() ;
    BOOL writePingerConfig() ;
    BOOL writeUserConfig() ;
    BOOL areConfigFilesPresent() ;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUICKCONFIGDLG_H__28502C57_24EE_4553_8E6D_9EB95D5D5263__INCLUDED_)
