// $Id: //depot/OPENDEV/sipXphone/src/pinger/wnt/PhoneGuiDlg.cpp#3 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// PhoneGuiDlg.cpp : implementation file
//

#include <pinger/wnt/stdafx.h>
#include <pinger/wnt/PhoneGui.h>
#include <pinger/wnt/PhoneGuiDlg.h>
#include <pinger/Pinger.h>
#include <ps/PsButtonTask.h>
#include <ps/PsHookswTask.h>
#include <os/OsLock.h>
#include <ps/wnt/PsHookswDevWnt.h>
#include <net/NameValueTokenizer.h>
#include <cp/CallManager.h>
#include <os/ostream>
#include <string.h>
#include <stdio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_LINE_LENGTH 1024
#define MAX_STDOUT_WINDOW_SIZE 24000
#define MAX_STDOUT_BUFFER_SIZE 100000

// Quick hack to catch cout stuff
class OsStreamBuf : public streambuf
{
    int bufChars;
    int bufSize;
    char* buffer;

public:
    OsStreamBuf(int size);
    int sync();
    virtual int overflow(int ch);
    virtual int underflow();
};

OsStreamBuf::OsStreamBuf(int size)
{
    buffer = new char[size + 1];
    bufChars = 0;
    bufSize = size;
}


int OsStreamBuf::sync()
{
    //osPrintf("sync\n");
    buffer[bufChars] ='\0';
    osPrintf("%s", buffer);
    bufChars = 0;
    return(0);
}

int OsStreamBuf::overflow(int ch)
{
    //osPrintf("overflow: %d\n", ch);
    buffer[bufChars] = ch;
    bufChars++;
    if(bufChars == bufSize ||
        ch == '\n') sync();
    return(0);
}
int OsStreamBuf::underflow()
{
    osPrintf("OsStreamBuf::underflow\n");
    return(0);
}

// Create a output buffer for cout
OsStreamBuf osOutBuf(1000);
   
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPhoneGuiDlg dialog

CPhoneGuiDlg::CPhoneGuiDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPhoneGuiDlg::IDD, pParent),
	displayMutex(OsMutex::Q_FIFO),
	guiStdoutMutex(OsMutex::Q_FIFO),
    stdoutBuffer((size_t)MAX_STDOUT_BUFFER_SIZE)
{
	//{{AFX_DATA_INIT(CPhoneGuiDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	displayEnabled = TRUE;
    windowStdout = TRUE;
    mSipLoggingEnabled = FALSE;
}

void CPhoneGuiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPhoneGuiDlg)
	DDX_Control(pDX, IDC_DIAL_URL_FIELD, m_DialUrlField);
	DDX_Control(pDX, IDC_OFF_HOLD_CALL, m_offHoldCallId);
	DDX_Control(pDX, IDC_EDIT_DISPLAY, m_edit_display);
	DDX_Control(pDX, IDC_STATUS_LINE, m_Status);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPhoneGuiDlg, CDialog)
	//{{AFX_MSG_MAP(CPhoneGuiDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_KEY_0, OnKey0)
	ON_BN_CLICKED(IDC_KEY_1, OnKey1)
	ON_BN_CLICKED(IDC_KEY_2, OnKey2)
	ON_BN_CLICKED(IDC_KEY_3, OnKey3)
	ON_BN_CLICKED(IDC_KEY_4, OnKey4)
	ON_BN_CLICKED(IDC_KEY_5, OnKey5)
	ON_BN_CLICKED(IDC_KEY_6, OnKey6)
	ON_BN_CLICKED(IDC_KEY_7, OnKey7)
	ON_BN_CLICKED(IDC_KEY_8, OnKey8)
	ON_BN_CLICKED(IDC_KEY_9, OnKey9)
	ON_BN_CLICKED(IDC_KEY_POUND, OnKeyPound)
	ON_BN_CLICKED(IDC_KEY_STAR, OnKeyStar)
	ON_BN_CLICKED(IDC_OFFHOOK, OnOffhook)
	ON_BN_CLICKED(IDC_ONHOOK, OnOnhook)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_CHAR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_STDOUT_ON, OnStdoutOn)
	ON_BN_CLICKED(IDC_STDOUT_OFF, OnStdoutOff)
	ON_BN_CLICKED(IDC_HOLD, OnHold)
	ON_BN_CLICKED(IDC_OFF_HOLD, OnOffHold)
	ON_BN_CLICKED(IDC_DIAL_URL, OnDialUrl)
	ON_BN_CLICKED(IDC_SIP_CALL_TYPE, OnSipCallType)
	ON_BN_CLICKED(IDC_MGCP_CALL_TYPE, OnMgcpCallType)
	ON_BN_CLICKED(IDC_DUMP_SIP_LOG, OnDumpSipLog)
	ON_BN_CLICKED(IDC_TOGGLE_SIP_LOGGING, OnToggleSipLogging)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int CALLBACK myFontEnum(const LOGFONT* lpelf, 
						const TEXTMETRIC* lpntm, 
						DWORD FontType, 
						LPARAM lParam )
{
	CPhoneGuiDlg* gui = CPhoneGuiApp::getGui();
	gui->guiPrintStdout(lpelf->lfFaceName);
	gui->guiPrintStdout("\r\n");
	OsTask::delay(500);
	return(1);
}
/////////////////////////////////////////////////////////////////////////////
// CPhoneGuiDlg message handlers

BOOL CPhoneGuiDlg::OnInitDialog()
{
	CDialog::OnInitDialog();



	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

   // Initialize keypad button images
   m_Key0.AutoLoad(IDC_KEY_0, this);
   m_Key1.AutoLoad(IDC_KEY_1, this);
   m_Key2.AutoLoad(IDC_KEY_2, this);
   m_Key3.AutoLoad(IDC_KEY_3, this);
   m_Key4.AutoLoad(IDC_KEY_4, this);
   m_Key5.AutoLoad(IDC_KEY_5, this);
   m_Key6.AutoLoad(IDC_KEY_6, this);
   m_Key7.AutoLoad(IDC_KEY_7, this);
   m_Key8.AutoLoad(IDC_KEY_8, this);
   m_Key9.AutoLoad(IDC_KEY_9, this);
   m_KeyStar.AutoLoad(IDC_KEY_STAR,  this);
   m_KeyPound.AutoLoad(IDC_KEY_POUND, this);
	
   // Initialize the display
   int displayH = 4;
   int displayW = 32;
   CString dispStats("");
   dispStats.Format("Height: %d Width: %d\r\n", displayW, displayH);
   guiPrintStdout(dispStats);
   UtlString paddedLine(' ', displayW);
   UtlString initialText;
   int rowIndex;

   for(rowIndex = 0; rowIndex < displayH; rowIndex++)
   {
	   initialText.append(paddedLine);
	   if(rowIndex < displayH - 1)
		   initialText.append("\r\n");
   }

   // Set up the font and initialize the display text
   CFont* displayFont = new CFont();
   displayFont->CreatePointFont(90, "Courier New");
   m_edit_display.SetFont(displayFont, TRUE);
   m_edit_display.SetWindowText(initialText.data());
   
   // Set up the font for the stdout status window
   displayFont = new CFont();
   displayFont->CreatePointFont(85, "Courier New");
   m_Status.SetFont(displayFont, TRUE);

   // Initialize the hook switch state
   CheckRadioButton(IDC_ONHOOK, IDC_OFFHOOK, IDC_ONHOOK);
   PsHookswTask* hookTask = PsHookswTask::getHookswTask();
   PsHookswDevWnt::setHookState(PsHookswTask::ON_HOOK);

   // Set the stdout on button to be in
   CheckRadioButton(IDC_STDOUT_ON, IDC_STDOUT_OFF, IDC_STDOUT_ON);

   // Set the SIP call type button to be in
   CheckRadioButton(IDC_SIP_CALL_TYPE, IDC_MGCP_CALL_TYPE, IDC_SIP_CALL_TYPE);

    // Set cout & cerr to use osPrintf
    // cout = &osOutBuf;
    // cerr = &osOutBuf;

    //cout << "Hey does this cout thing work?\n";
    //cout.flush();

    //osPrintf("hey does os printf work?\n");

    //cerr << "Hey does this cerr thing work?\n";
    //cerr.flush();

   	// Force the pinger and subordinate tasks to startup
	Pinger::getPingerTask();
	displayEnabled = TRUE;

	// osPrintf("12345678901234567890123456789012345678901234567890123456789012345678901234567890\r\n");
   	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPhoneGuiDlg::displayReplaceText(int row, int col, const char* text)
{
	if(displayEnabled)
	{
	//OsLock lock(displayMutex);
	int len = strlen(text);
	int charIndex = m_edit_display.LineIndex(row);
	int rowLength = m_edit_display.LineLength(row);
	int selectEnd;
#	ifdef TEST
	//osPrintf("CPhoneGuiDlg::displayReplaceText(row: %d, col: %d, text: %s)\r\n", row, col, text);
#	endif

	if(col > rowLength)
	{
		charIndex += rowLength - 1;
		//osPrintf("col: %d rowLen: %d charIndex: %d row: %d\r\n", 
		//	col, rowLength, row, charIndex);
		selectEnd =  charIndex;
		m_edit_display.SetSel(charIndex, selectEnd);
		UtlString padChars(' ', col - rowLength);
		m_edit_display.ReplaceSel(padChars.data());
	}
	else
	{
		charIndex += col;
		if(col + len > rowLength)
		{
			selectEnd = charIndex + rowLength - 1;
			//osPrintf("Selectend: %d charIndex: %d len: %d col: %d\r\n",
			//	selectEnd, charIndex, len, col);
		}
		else
		{
			selectEnd = charIndex + len;
			//osPrintf("CharIndex: %d selectEnd: %d len: %d text: %s\r\n",
			//	charIndex, selectEnd, len, text);

		}
	}

	m_edit_display.SetSel(charIndex, selectEnd);
	m_edit_display.ReplaceSel(text);
	}
}

void CPhoneGuiDlg::displayGetText(UtlString* text)
{
	if(displayEnabled)
	{
	//OsLock lock(displayMutex);
	CString windowText;
	m_edit_display.GetWindowText( windowText );
	text->remove(0);
	text->append(windowText);
	}
}

void CPhoneGuiDlg::displayGetTextLine(int row, UtlString* text)
{
	if(displayEnabled)
	{
	//OsLock lock(displayMutex);
	char winText[MAX_LINE_LENGTH];
	m_edit_display.GetLine(row, winText, MAX_LINE_LENGTH);
	text->remove(0);
	text->append(winText);
	}
}

void CPhoneGuiDlg::displaySetTextLine(int row, const char* text)
{
	if(displayEnabled)
	{
	//OsLock lock(displayMutex);
#	ifdef TEST
	//char buffer [1024];
	//osPrintf("CPhoneGuiDlg::displaySetTextLine(row: %d, text: %s)\r\n", row, text);
	//guiPrintStdout(buffer);
#	endif
	int charIndex = m_edit_display.LineIndex(row);
	int rowLength = m_edit_display.LineLength(row);
	m_edit_display.SetSel(charIndex, charIndex + rowLength - 1);
	m_edit_display.ReplaceSel(text);
	}
}

UtlBoolean CPhoneGuiDlg::isOnHook()
{
	//OsLock lock(displayMutex);
	return(IDC_ONHOOK == GetCheckedRadioButton(IDC_ONHOOK, IDC_OFFHOOK));
}

void CPhoneGuiDlg::guiPrintStdout(const char* text)
{
	//if(displayEnabled)
	//{
	//OsLock lock(guiStdoutMutex);

	// Replace all new lines with carriage return & new line
	UtlString textString;
	char prevChar = 0;
	while(*text)
	{
		if(*text == '\n' && prevChar != '\r')
		{
			textString.append('\r');
		}
		prevChar = *text;
		textString.append(prevChar);
		text++;
	}

	// Reposition at the end
	int lastChar = m_Status.GetWindowTextLength();
	int excessLength = lastChar + textString.length() - MAX_STDOUT_WINDOW_SIZE;
    //if(windowStdout && excessLength > 0)
    //{
    //    windowStdout = FALSE;
    //}

    if(windowStdout)
    {
	    // Trim off the top if the status display is full
	    if(excessLength > 0)
	    {
		    int trimLine = m_Status.LineFromChar(excessLength);
		    int trimChar = m_Status.LineIndex(trimLine + 1);
		    //trimChar += m_Status.LineLength(trimLine);

		    //char buffer[128];
		    //sprintf(buffer, "Trimming %d/%d chars\r\n", trimChar, textLength - 1000);
		    //m_Status.SetSel(lastChar, lastChar);
		    //m_Status.ReplaceSel(buffer);

		    // Delete from the top to make room to add at the end
		    m_Status.SetSel(0, trimChar - 1);
		    m_Status.ReplaceSel("");

		    lastChar = m_Status.GetWindowTextLength();
	    }

	    m_Status.SetSel(lastChar, lastChar);
	    m_Status.ReplaceSel(textString.data());
    }
    /*else
    {
        int bufLen = stdoutBuffer.length();
        if(bufLen < MAX_STDOUT_BUFFER_SIZE)
        {
            if(bufLen + textString.length() > MAX_STDOUT_BUFFER_SIZE))
            {
                stdoutBuffer.append(textString.data(), 
                    (bufLen + textString.length()) - MAX_STDOUT_BUFFER_SIZE);
            }
            else
            {
                stdoutBuffer.append(textString.data());
            }
        }
    }*/
	//lastChar = m_Status.GetWindowTextLength();
	//m_Status.SetSel(lastChar, lastChar);
	//char buffer[128];
	//sprintf(buffer, "%d chars in display\r\n", lastChar);
	//m_Status.ReplaceSel(buffer);
	//}
}


void CPhoneGuiDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPhoneGuiDlg::OnPaint() 
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

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPhoneGuiDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CPhoneGuiDlg::OnKey0() 
{
	// TODO: Add your control notification handler code here
   handleKeyClick('0');
}

void CPhoneGuiDlg::OnKey1() 
{
	// TODO: Add your control notification handler code here
   handleKeyClick('1');
}

void CPhoneGuiDlg::OnKey2() 
{
	// TODO: Add your control notification handler code here
	/*LOGFONT myLogFont;
	CHOOSEFONT lpcf;
	lpcf.lStructSize = sizeof(CHOOSEFONT);
	lpcf.lpLogFont = &myLogFont;
	lpcf.hwndOwner = m_edit_display;
	lpcf.hDC = ::GetDC(m_edit_display);
	lpcf.iPointSize = 80;
	lpcf.nSizeMin = 70;
	lpcf.nSizeMax = 100;
	lpcf.Flags = CF_FIXEDPITCHONLY;

	ChooseFont( &lpcf); */

   handleKeyClick('2');
}

void CPhoneGuiDlg::OnKey3() 
{
	// TODO: Add your control notification handler code here
   handleKeyClick('3');
}

void CPhoneGuiDlg::OnKey4() 
{
	// TODO: Add your control notification handler code here
   handleKeyClick('4');
}

void CPhoneGuiDlg::OnKey5() 
{
	// TODO: Add your control notification handler code here
	//    EnumFonts(::GetDC(m_edit_display), // HDC 
	//	NULL, 
	// myFontEnum, // pointer to callback function 
	//	NULL); 

   handleKeyClick('5');
}

void CPhoneGuiDlg::OnKey6() 
{
	// TODO: Add your control notification handler code here
   handleKeyClick('6');
}

void CPhoneGuiDlg::OnKey7() 
{
	// TODO: Add your control notification handler code here
   handleKeyClick('7');
}

void CPhoneGuiDlg::OnKey8() 
{
	// TODO: Add your control notification handler code here
   handleKeyClick('8');
}

void CPhoneGuiDlg::OnKey9() 
{
	// TODO: Add your control notification handler code here
   handleKeyClick('9');
}

void CPhoneGuiDlg::OnKeyPound() 
{
	// TODO: Add your control notification handler code here
   handleKeyClick('#');
}

void CPhoneGuiDlg::OnKeyStar() 
{
	// TODO: Add your control notification handler code here
   handleKeyClick('*');
}

void CPhoneGuiDlg::OnOffhook() 
{
	// TODO: Add your control notification handler code here
	PsHookswTask* hookTask = PsHookswTask::getHookswTask();
    PsHookswDevWnt::setHookState(PsHookswTask::OFF_HOOK);
	hookTask->postEvent(PsMsg::HOOKSW_STATE, // msg type
                        this,            // source
                        PsHookswTask::OFF_HOOK);


	//osPrintf("Off hook , %d line\r\n", m_edit_display.GetLineCount());
}

void CPhoneGuiDlg::OnOnhook() 
{
	// TODO: Add your control notification handler code here
	PsHookswTask* hookTask = PsHookswTask::getHookswTask();
    PsHookswDevWnt::setHookState(PsHookswTask::ON_HOOK);
	hookTask->postEvent(PsMsg::HOOKSW_STATE, // msg type
                        this,            // source
                        PsHookswTask::ON_HOOK);

	char buffer[1024];
	sprintf(buffer, "On hook , %d line\r\n", m_edit_display.GetLineCount());
	//osPrintf(buffer);
}

void CPhoneGuiDlg::handleKeyClick(char key)
{
	PsButtonTask* buttonTask = PsButtonTask::getButtonTask();
	int keyIndex = buttonTask->getButtonIndex(key);
	buttonTask->postEvent(PsMsg::BUTTON_DOWN, this, keyIndex);

	buttonTask->postEvent(PsMsg::BUTTON_UP, this, keyIndex);

	//osPrintf("Key: %c\r\n", key);
}

void CPhoneGuiDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	PsButtonTask* buttonTask = PsButtonTask::getButtonTask();
	int keyIndex = buttonTask->getButtonIndex(nChar);
	buttonTask->postEvent(PsMsg::BUTTON_DOWN, this, keyIndex);

	//osPrintf("Key Down: %c\r\n", nChar);

	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CPhoneGuiDlg::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	PsButtonTask* buttonTask = PsButtonTask::getButtonTask();
	int keyIndex = buttonTask->getButtonIndex(nChar);
	buttonTask->postEvent(PsMsg::BUTTON_UP, this, keyIndex);

	//osPrintf("Key Up: %c\r\n", nChar);

	CDialog::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CPhoneGuiDlg::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	//osPrintf("On Char: %c\r\n", nChar);

	CDialog::OnChar(nChar, nRepCnt, nFlags);
}

void CPhoneGuiDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	//osPrintf("Left Button Down\r\n");

	CDialog::OnLButtonDown(nFlags, point);
}

void CPhoneGuiDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	//osPrintf("Left Button Up\r\n");
	CDialog::OnLButtonUp(nFlags, point);
}

void CPhoneGuiDlg::OnSetfocusDisplay2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}



void CPhoneGuiDlg::OnStdoutOn() 
{
	// TODO: Add your control notification handler code here
	windowStdout = TRUE;
    //guiPrintStdout(stdoutBuffer.data());
    //stdoutBuffer.remove(0);
}

void CPhoneGuiDlg::OnStdoutOff() 
{
	// TODO: Add your control notification handler code here
	windowStdout = FALSE;
}

void CPhoneGuiDlg::OnHold() 
{
	// TODO: Add your control notification handler code here
	hold();
}

void CPhoneGuiDlg::OnOffHold() 
{
	// TODO: Add your control notification handler code here
    CString windowText;
    m_offHoldCallId.GetWindowText(windowText);
    UtlString trimmedText(windowText);
    NameValueTokenizer::frontBackTrim(&trimmedText, " \t\n\r");
    osPrintf("Taking call: \"%s\" off hold\n", trimmedText.data());
	offhold(trimmedText.data());
}

void CPhoneGuiDlg::OnDialUrl() 
{
	// TODO: Add your control notification handler code here
	CString url;
    m_DialUrlField.GetWindowText(url);
    UtlString trimmedUrl(url);
    osPrintf("Dialing url: \"%s\"\n", trimmedUrl);
    dialUrl(trimmedUrl);
}

void CPhoneGuiDlg::OnSipCallType() 
{
	// TODO: Add your control notification handler code here
    outGoingCallType(CallManager::SIP_CALL);
}

void CPhoneGuiDlg::OnMgcpCallType() 
{
	// TODO: Add your control notification handler code here
    outGoingCallType(CallManager::MGCP_CALL);
}



void CPhoneGuiDlg::OnDumpSipLog() 
{
	// TODO: Add your control notification handler code here
    dumpSipLog();
}

void CPhoneGuiDlg::OnToggleSipLogging() 
{
	// TODO: Add your control notification handler code here
	if(!mSipLoggingEnabled) 
    {
        startSipLog();
        mSipLoggingEnabled = TRUE;
        osPrintf("SIP Log enabled\n");
    }
    else 
    {
        startSipLog();
        mSipLoggingEnabled = FALSE;
        osPrintf("SIP Log disabled\n");
    }
}
