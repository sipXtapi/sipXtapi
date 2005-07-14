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

// QuickConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "QuickConfig.h"
#include "QuickConfigDlg.h"
#include "NetMd5Codec.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQuickConfigDlg dialog

CQuickConfigDlg::CQuickConfigDlg(const char* szDataDir, CWnd* pParent /*=NULL*/)
	: CDialog(CQuickConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQuickConfigDlg)
	m_cstrOtherSettings = _T("");
	m_cstrPassword = _T("");
	m_cstrSipServer = _T("");
	m_cstrStunServer = _T("");
	m_cstrUsername = _T("");
	m_cstrDisplayName = _T("");
	//}}AFX_DATA_INIT

    m_szDataDir = szDataDir ;    
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CQuickConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQuickConfigDlg)
	DDX_Text(pDX, IDC_OTHER_SETTINGS, m_cstrOtherSettings);
	DDX_Text(pDX, IDC_PASSWORD, m_cstrPassword);
	DDX_Text(pDX, IDC_SIP_SERVER, m_cstrSipServer);
	DDX_Text(pDX, IDC_STUN_SERVER, m_cstrStunServer);
	DDX_Text(pDX, IDC_USERNAME, m_cstrUsername);
	DDX_Text(pDX, IDC_DISPLAYNAME, m_cstrDisplayName);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CQuickConfigDlg, CDialog)
	//{{AFX_MSG_MAP(CQuickConfigDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define MAX_CONFIG_LINE 256
BOOL CQuickConfigDlg::parseConfigFile() 
{
    BOOL bSuccess = FALSE ;
    char cLine[MAX_CONFIG_LINE] ;
    char cPath[_MAX_PATH] ;
    strcpy(cPath, m_szDataDir) ;    
    strcat(cPath, "quickconfig") ;

    FILE *fp = fopen(cPath, "r") ;
    if (fp)
    {
        while (fgets(cLine, MAX_CONFIG_LINE, fp))
        {
            if ((cLine[0] != '#') && strlen(cLine))
            {
                CString cstrKey = strtok(cLine, ":") ;
                cstrKey.TrimLeft() ;
                cstrKey.TrimRight() ;
                cstrKey.MakeUpper() ;
                CString cstrValue = strtok(NULL, "\r\n") ;
                cstrValue.TrimLeft() ;
                cstrValue.TrimRight() ;

                if (cstrKey == "TITLE")
                {
                    SetWindowText(cstrValue) ;
                }
                if (cstrKey == "HEADER")
                {
                    GetDlgItem(IDC_HEADER)->SetWindowText(cstrValue) ;
                }
                else if (cstrKey == "NAME")
                {
                    m_cstrDisplayName = cstrValue ;
                }
                else if (cstrKey == "USERID")
                {
                    m_cstrUsername = cstrValue ;
                }
                else if (cstrKey == "USERNAME")
                {
                    m_cstrUsername = cstrValue ;
                }
                else if (cstrKey == "PASSWORD") 
                {
                    m_cstrPassword = cstrValue ;
                }
                else if (cstrKey == "SIP_SERVER")
                {
                    m_cstrSipServer = cstrValue ;
                }
                else if (cstrKey == "STUN_SERVER")
                {
                    m_cstrStunServer = cstrValue ;
                }
                else if (cstrKey == "OTHER")
                {
                    m_cstrOtherSettings += cstrValue ;
                    m_cstrOtherSettings += "\r\n" ;
                }   
                else if (cstrKey == "LOCK")
                {
                    if (cstrValue == "USERNAME")
                    {
                        ((CEdit*) GetDlgItem(IDC_USERNAME))->SetReadOnly(TRUE) ;
                    }
                    else if (cstrValue == "PASSWORD")
                    {
                        ((CEdit*) GetDlgItem(IDC_PASSWORD))->SetReadOnly(TRUE) ;
                    }
                    else if (cstrValue == "SIP_SERVER")
                    {
                        ((CEdit*) GetDlgItem(IDC_SIP_SERVER))->SetReadOnly(TRUE) ;
                    }
                    else if (cstrValue == "STUN_SERVER")
                    {
                        ((CEdit*) GetDlgItem(IDC_STUN_SERVER))->SetReadOnly(TRUE) ;
                    }
                    else if (cstrValue == "OTHER")
                    {
                        ((CEdit*) GetDlgItem(IDC_OTHER_SETTINGS))->SetReadOnly(TRUE) ;
                    }
                    else
                    {
                        printf("Unknown LOCK configuration param: %s\n", cstrValue) ;
                    }

                }                    
                else if (cstrKey.GetLength())
                {
                    printf("Unknown configuration param: %s\n", cstrKey) ;
                }
            }
        }
        fclose(fp) ;
        bSuccess = TRUE ;
    }

    return bSuccess ;
}

/////////////////////////////////////////////////////////////////////////////
// CQuickConfigDlg message handlers

BOOL CQuickConfigDlg::OnInitDialog()
{
	CDialog::OnInitDialog();    

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

    if (!parseConfigFile())
    {
        AfxMessageBox(IDS_ERROR_LOADING_CONFIGURATION) ;
        OnCancel() ;
    }
    UpdateData(FALSE) ;
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CQuickConfigDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
		CDialog::OnPaint();
	}
}

HCURSOR CQuickConfigDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void CQuickConfigDlg::OnOK() 
{
    UpdateData(TRUE) ;

    if (m_cstrDisplayName.GetLength() == 0)
    {
        AfxMessageBox(IDS_ERROR_NAME) ;        
    }
    if (m_cstrUsername.GetLength() == 0)
    {
        AfxMessageBox(IDS_ERROR_USERID) ;        
    }
    else if (m_cstrPassword.GetLength() == 0)
    {
        AfxMessageBox(IDS_ERROR_PASSWORD) ;        
    }
    else if (m_cstrSipServer.GetLength() == 0)
    {
        AfxMessageBox(IDS_ERROR_SIP_SERVER) ;        
    }
    else
    {
        if (!areConfigFilesPresent() || AfxMessageBox(IDS_OVERWRITE_CONFIGURATION, MB_YESNO) == IDYES)
        {            
            if (writePingerConfig() && writeUserConfig())
            {
	            CDialog::OnOK() ;
            }
            else
            {
                AfxMessageBox(IDS_ERROR_SAVING) ;
            }
        }
    }
}


BOOL CQuickConfigDlg::writePingerConfig() 
{
    BOOL bSuccess = FALSE ;

    char cPath[_MAX_PATH] ;
    strcpy(cPath, m_szDataDir) ;    
    strcat(cPath, "pinger-config") ;

    FILE *fp = fopen(cPath, "w") ;
    if (fp)
    {
        fprintf(fp, "PHONESET_DIALPLAN_LENGTH : 12\r\n") ;
        fprintf(fp, "PHONESET_LINE.REGISTRATION : REGISTER\r\n") ;
        fprintf(fp, "PHONESET_LINE.URL : \"%s\" <sip:%s@%s>\r\n", m_cstrDisplayName, m_cstrUsername, m_cstrSipServer) ;
        fprintf(fp, "PHONESET_LINE.ALLOW_FORWARDING : ENABLE\r\n") ;
        fprintf(fp, "PHONESET_LINE.CONTACT_TYPE : NAT_MAPPED\r\n") ;

        CString cstrTemp ;
        cstrTemp = m_cstrUsername + ":" + m_cstrSipServer + ":" + m_cstrPassword ;        
        char cEncodedBuf[33] ;
        memset(cEncodedBuf, 0, sizeof(cEncodedBuf)) ;
        NetMd5Codec::encode(cstrTemp, cEncodedBuf) ;        
                       
        fprintf(fp, "PHONESET_LINE.CREDENTIAL.1.PASSTOKEN : %s\r\n", cEncodedBuf) ;
        fprintf(fp, "PHONESET_LINE.CREDENTIAL.1.REALM : %s\r\n", m_cstrSipServer) ;
        fprintf(fp, "PHONESET_LINE.CREDENTIAL.1.USERID : %s\r\n", m_cstrUsername) ;                
        fprintf(fp, "PHONESET_RTP_PORT_START : 8766\r\n") ;
        fprintf(fp, "SIP_DIRECTORY_SERVERS : %s\r\n", m_cstrSipServer) ;
        fprintf(fp, "SIP_REGISTER_PERIOD : 3600\r\n") ;        
        if (m_cstrStunServer.GetLength())
        {
            fprintf(fp, "SIP_STUN_SERVER : %s\r\n", m_cstrStunServer) ;            
            fprintf(fp, "SIP_STUN_REFRESH_PERIOD : 28\r\n") ;
        }
        fprintf(fp, "SIP_SYMMETRIC_SIGNALING : ENABLE\r\n") ;
        fprintf(fp, "SIP_TCP_PORT : 5060\r\n") ;
        fprintf(fp, "SIP_UDP_PORT : 5060\r\n") ;
        fprintf(fp, "%s\r\n", m_cstrOtherSettings) ;
        fclose(fp) ;

        bSuccess = TRUE ;
    }
    
    return bSuccess ;
}

BOOL CQuickConfigDlg::writeUserConfig() 
{
    BOOL bSuccess = FALSE ;

    char cPath[_MAX_PATH] ;
    strcpy(cPath, m_szDataDir) ;    
    strcat(cPath, "user-config") ;

    FILE *fp = fopen(cPath, "w") ;
    if (fp)
    {
        fprintf(fp, "PHONESET_AVAILABLE_BEHAVIOR : RING\r\n") ;
        fprintf(fp, "PHONESET_BUSY_BEHAVIOR : BUSY\r\n") ;
        fprintf(fp, "PHONESET_CALL_WAITING_BEHAVIOR : ALERT\r\n") ;
        fprintf(fp, "PHONESET_DND_METHOD : FORWARD_ON_BUSY\r\n") ;
        fprintf(fp, "PHONESET_RINGER : BOTH\r\n") ;
        fclose(fp) ;

        bSuccess = TRUE ;
    }
    
    return bSuccess ;
}


BOOL CQuickConfigDlg::areConfigFilesPresent() 
{   
    BOOL bPresent = FALSE ;

    char cPath[_MAX_PATH] ;
    strcpy(cPath, m_szDataDir) ;    
    strcat(cPath, "pinger-config") ;

    FILE *fp = fopen(cPath, "r") ;
    if (fp)
    {
        bPresent = TRUE ;
        fclose(fp) ;
    }
    else
    {
        strcpy(cPath, m_szDataDir) ;    
        strcat(cPath, "user-config") ;

        FILE *fp = fopen(cPath, "r") ;
        if (fp)
        {
            bPresent = TRUE ;
            fclose(fp) ;
        }
    }

    return bPresent ;
}