// $Id: //depot/OPENDEV/sipXphone/src/pinger/wnt/PhoneGui.cpp#2 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// PhoneGui.cpp : Defines the class behaviors for the application.
//

#include <pinger/wnt/stdafx.h>
#include <pinger/wnt/PhoneGui.h>
#include <pinger/wnt/PhoneGuiDlg.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPhoneGuiApp

BEGIN_MESSAGE_MAP(CPhoneGuiApp, CWinApp)
	//{{AFX_MSG_MAP(CPhoneGuiApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPhoneGuiApp construction

CPhoneGuiApp::CPhoneGuiApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPhoneGuiApp object

CPhoneGuiApp theApp;



CPhoneGuiDlg* CPhoneGuiApp::getGui()
{
	return((CPhoneGuiDlg*)theApp.m_pMainWnd);
}

/////////////////////////////////////////////////////////////////////////////
// CPhoneGuiApp initialization

BOOL CPhoneGuiApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CPhoneGuiDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}



extern "C" void osPrintf(char* format, ...)
{
	va_list args;
	va_start(args, format);

	char* formatPtr = format;
	char* formatEnd = formatPtr + strlen(format);
	char* nextArg;
	const char* charPtr;
	int width;
	int precision;
	int prefixType;
	int type32;
	double type64;
	UtlString printString;
	UtlBoolean is32;
	char* dynaBuffer = NULL;
	char buffer[128];
	int dataSize = 100;
	CPhoneGuiDlg* gui = CPhoneGuiApp::getGui();

	while(formatPtr < formatEnd)
	{
		width = 0;
		precision = 0;
		prefixType = 0;
		is32 = FALSE;

		nextArg = strchr(formatPtr + 1, '%');
		if(!nextArg)
		{
			nextArg = formatPtr + strlen(formatPtr);
		}
		printString.remove(0);
		printString.append(formatPtr, nextArg - formatPtr);
		if(!printString.isNull())
		{
			charPtr = printString.data();
			if(*charPtr == '%')
			{
				charPtr++;

				// Ignore flags
				switch(*charPtr)
				{
				case '-':
				case '+':
				case '0':
				case ' ':
				case '#':
					charPtr++;
					break;
				default:
					break;
				}

				// Deal with the width
				while(isdigit(*charPtr))
				{
					charPtr++;
				}
				if(*charPtr == '*')
				{
					width = va_arg(args, int);
					charPtr++;
				}

				// Deal with precision
				if(*charPtr == '.')
				{
					charPtr++;
					while(isdigit(*charPtr))
					{
						charPtr++;
					}
					if(*charPtr == '*')
					{
						precision = va_arg(args, int);
						charPtr++;
					}
				}

				// Deal with the type prefix
				switch(*charPtr)
				{
				case 'h':
				case 'l':
				case 'L':
					prefixType = *charPtr;
					charPtr++;
					break;

				case 'I':
					prefixType = *charPtr;
					charPtr += 3;
					break;

				default:
					break;
				}

				// Finally deal with the type
				switch(*charPtr)
				{
					// 32 bit things

				case 'd':
				case 'i':
				case 'o':
				case 'u':
				case 'x':
				case 'X':
					dataSize = 100; // big enough for a long
					if(prefixType == 'l' && 
						sizeof(int) != sizeof(long))
					{
						type64 = va_arg(args, double);
					}
					else
					{
						type32 = va_arg(args, int);
						is32 = TRUE;
					}
					break;

				case 'c':
				case 'C':
				case 'n':
				case 'p':
					dataSize = 100; // big enough for a pointer
					type32 = va_arg(args, int);
					is32 = TRUE;
					break;

				case 's':
				case 'S':
					type32 = va_arg(args, int);
					dataSize = strlen((char*) type32);
					is32 = TRUE;
					break;

				case 'e':
				case 'E':
				case 'f':
				case 'g':
				case 'G':
					dataSize = 100;  // big enough for char rep of any double
					type64 = va_arg(args, double);


					break;

				default:
					sprintf(buffer, "osPrintf: unknown type: %c", *charPtr);
					gui->guiPrintStdout(buffer);
					break;
				}

				// Room for format string and data element
				dynaBuffer = new char[strlen(printString.data()) + dataSize];
				if(is32)
				{
					if(width && precision)
					{
						sprintf(dynaBuffer, printString.data(), width, precision, type32);
						gui->guiPrintStdout(dynaBuffer);
					}
					else if(width)
					{
						sprintf(dynaBuffer, printString.data(), width, type32);
						gui->guiPrintStdout(dynaBuffer);
					}
					else if(precision)
					{
						sprintf(dynaBuffer, printString.data(), precision, type32);
						gui->guiPrintStdout(dynaBuffer);
					}
					else
					{
						sprintf(dynaBuffer, printString.data(), type32);
						gui->guiPrintStdout(dynaBuffer);
					}
				}
				else // assume 64 bits
				{
					if(width && precision)
					{
						sprintf(dynaBuffer, printString.data(), width, precision, type64);
						gui->guiPrintStdout(dynaBuffer);
					}
					else if(width)
					{
						sprintf(dynaBuffer, printString.data(), width, type64);
						gui->guiPrintStdout(dynaBuffer);
					}
					else if(precision)
					{
						sprintf(dynaBuffer, printString.data(), precision, type64);
						gui->guiPrintStdout(dynaBuffer);
					}
					else
					{
						sprintf(dynaBuffer, printString.data(), type64);
						gui->guiPrintStdout(dynaBuffer);
					}
				}
				if(dynaBuffer)
				{
					delete dynaBuffer;
					dynaBuffer = NULL;
				}

			}
			else
			{
				gui->guiPrintStdout(printString.data());
			}
		}

		formatPtr = nextArg;
	}

	va_end(args);
}
