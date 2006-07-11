// $Id: //depot/OPENDEV/sipXphone/include/pinger/wnt/PhoneGuiDlg.h#2 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////// PhoneGuiDlg.h : header file
//

#include <os/OsDefs.h>
#include <os/OsMutex.h>

#if !defined(AFX_PHONEGUIDLG_H__C85989A7_5A8F_11D2_8472_00104B68C34C__INCLUDED_)
#define AFX_PHONEGUIDLG_H__C85989A7_5A8F_11D2_8472_00104B68C34C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CPhoneGuiDlg dialog

class CPhoneGuiDlg : public CDialog
{
// Construction
public:
	CPhoneGuiDlg(CWnd* pParent = NULL);	// standard constructor

	void displayReplaceText(int row, int col, const char* text);
	void displayGetText(UtlString* text);
	void displayGetTextLine(int row, UtlString* text);
	void displaySetTextLine(int row, const char* text);
	UtlBoolean isOnHook();
	void guiPrintStdout(const char* text);

// Dialog Data
	//{{AFX_DATA(CPhoneGuiDlg)
	enum { IDD = IDD_PHONEGUI_DIALOG };
	CEdit	m_DialUrlField;
	CEdit	m_offHoldCallId;
	CEdit	m_edit_display;
	CEdit	m_Status;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPhoneGuiDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CPhoneGuiDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnKey0();
	afx_msg void OnKey1();
	afx_msg void OnKey2();
	afx_msg void OnKey3();
	afx_msg void OnKey4();
	afx_msg void OnKey5();
	afx_msg void OnKey6();
	afx_msg void OnKey7();
	afx_msg void OnKey8();
	afx_msg void OnKey9();
	afx_msg void OnKeyPound();
	afx_msg void OnKeyStar();
	afx_msg void OnOffhook();
	afx_msg void OnOnhook();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSetfocusDisplay2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void toggleStdoutWindow();
	afx_msg void OnRadio2();
	afx_msg void OnStdoutOn();
	afx_msg void OnSdtoutOff();
	afx_msg void OnStdoutOff();
	afx_msg void OnHold();
	afx_msg void OnOffHold();
	afx_msg void OnDialUrl();
	afx_msg void OnSipCallType();
	afx_msg void OnMgcpCallType();
	afx_msg void OnDumpSipLog();
	afx_msg void OnLogSipMsgs();
	afx_msg void OnToggleSipLogging();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CBitmapButton m_Key0;
	CBitmapButton m_Key1;
	CBitmapButton m_Key2;
	CBitmapButton m_Key3;
	CBitmapButton m_Key4;
	CBitmapButton m_Key5;
	CBitmapButton m_Key6;
	CBitmapButton m_Key7;
	CBitmapButton m_Key8;
	CBitmapButton m_Key9;
	CBitmapButton m_KeyStar;
	CBitmapButton m_KeyPound;

	void handleKeyDown(char key);
	void handleKeyClick(char key);

	OsMutex displayMutex;
	OsMutex guiStdoutMutex;
	UtlBoolean displayEnabled;
    UtlBoolean windowStdout;
    UtlString stdoutBuffer;
    UtlBoolean mSipLoggingEnabled;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHONEGUIDLG_H__C85989A7_5A8F_11D2_8472_00104B68C34C__INCLUDED_)
