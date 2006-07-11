// $Id$
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <afxwin.h>
#include <windowsx.h>
#include <shellapi.h>
#include <process.h>
// APPLICATION INCLUDES
#include "resource.h"
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
//uncomment next line to enable tray icon support
//#define ENABLE_TRAY_ICON    1    
#define WM_TRAY_ICON_ID         5001 
#define WM_TRAY_ICON_MESSAGE    (WM_USER+1997)
// STATIC VARIABLE INITIALIZATIONS

// GLOBAL VARIABLES
STARTUPINFO StartupInfo;
PROCESS_INFORMATION ProcessInformation;
HANDLE hStartupEvent;
bool bAskedToExit = false;
LRESULT CALLBACK main_WindowProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
HWND hWnd = NULL;
HINSTANCE g_hInstance = NULL;
HANDLE hMessageThread = NULL;
RECT desktopRect;
DWORD windowStartX;
DWORD windowStartY;
BITMAP bm;
HBITMAP hbmp;
char cmdLineCopy[1024];

// FUNCTION DECLARATIONS
void AskThreadToClose();



// puts the SIP softphone icon in the Windows system tray
void AddTrayIcon()
{
    char *tipText = "SIP Softphone";

    if (hWnd)
    {
        // add the program's icon to the tray
        NOTIFYICONDATA info;
        memset(&info,0,sizeof(info));
        info.cbSize          =sizeof(info) ;
        info.hWnd            =hWnd; // form handle gets notification message
        info.uID             =WM_TRAY_ICON_ID; // id of icon - passed back in wParam of message
        info.uCallbackMessage=WM_TRAY_ICON_MESSAGE; // our notification message
        info.hIcon           =LoadIcon(g_hInstance,MAKEINTRESOURCE(IDI_ICON1)); // icon to display
        strncpy (info.szTip,tipText,sizeof(info.szTip)); // set tool tip text
        info.szTip[sizeof(info.szTip)-1]='0';
        info.uFlags          =NIF_MESSAGE | NIF_ICON | NIF_TIP ; // indicate modifications
        Shell_NotifyIcon(NIM_ADD,&info) ; // add it
    }

}

// removes the SIP softphone icon from the Windows system tray
void RemoveTrayIcon()
{
    char *tipText = "SIP Softphone";

    if (hWnd)
    {
        // add the program's icon to the tray
        NOTIFYICONDATA info;
        memset(&info,0,sizeof(info));
        info.cbSize          =sizeof(info) ;
        info.hWnd            =hWnd; // form handle gets notification message
        info.uID             =WM_TRAY_ICON_ID; // id of icon - passed back in wParam of message
        info.uCallbackMessage=WM_TRAY_ICON_MESSAGE; // our notification message
        info.hIcon           =LoadIcon(g_hInstance,MAKEINTRESOURCE(IDI_ICON1)); // icon to display
        strncpy (info.szTip,tipText,sizeof(info.szTip)); // set tool tip text
        info.szTip[sizeof(info.szTip)-1]='0';
        info.uFlags          =NIF_MESSAGE | NIF_ICON | NIF_TIP ; // indicate modifications
        Shell_NotifyIcon(NIM_DELETE,&info) ; // add it
    }

}

// Waits for the thread that launched the phone 
// to signal us.  Looks at the exit code from the process that was invoked,
// and either restarts or quits.
unsigned int __stdcall WaitForExitThread(LPVOID Unused)
{
   DWORD ExitCode = STILL_ACTIVE;
   int TimeoutCount = 0;
	DWORD retCode;
   int processRetCode = 0;
   bool bUseSleep = false;

   //wait for the app to exit
	do
	{
   	GetExitCodeProcess(ProcessInformation.hProcess,     // handle to the process
			                 &ExitCode   // address to receive termination status
			               );
      if (bUseSleep)
         Sleep(500);
      else
		   retCode = WaitForSingleObject(hStartupEvent, 500); 

      if (retCode == WAIT_TIMEOUT)
      {

/* Don't post close in new system, because user can close this form the tray
          TimeoutCount++;
          if (TimeoutCount > 120) //60 secs
          {
              //ok we made it here because we kept waiting and waiting but still no event
              //(Whats up with that!) So we will tell launch ix to exit.
              ExitCode = 0;  //force Exit
              PostMessage(hWnd,WM_QUIT,0,0);
          }
*/
      }
      else
		if (retCode == WAIT_OBJECT_0) //did java signal us?
		{
			//hide our startup splash window
			//reset event so we continue to wait 500 ms even after 
			//we were told to hide
			ResetEvent(hStartupEvent); 
			ShowWindow(hWnd,SW_HIDE);

			UpdateWindow(hWnd);
         TimeoutCount = 0; //reset timeout.
         retCode = 0;
         bUseSleep = true;
		}
		
		if (ExitCode == 2) //Java says we should restart
		{

			do
			{
				processRetCode = CreateProcess(
					NULL,  // name of executable module
					cmdLineCopy,       // command line string
					NULL, 
					NULL, 
					FALSE,       // handle inheritance flag
					CREATE_NEW_CONSOLE,      // creation flags
					NULL,       // new environment block
					NULL, // current directory name
					&StartupInfo, 
					&ProcessInformation 
					);
	
			} while (processRetCode <= 0);
			
			if (processRetCode <= 0)
			{
				MessageBox(NULL,"Error spawning process!","ERROR",MB_OK);
				return 3;
			}
			else
			{
				ShowWindow(hWnd,SW_SHOW);
				BringWindowToTop(hWnd);
				UpdateWindow(hWnd);
            TimeoutCount = 0; //reset timeout.
            bUseSleep = false;
			}
		}
        
        if (bAskedToExit)
            break; //out of while

     //wait until spawned app closes or exit code is 2 (reboot) or alt-F4 app was asked to close   
	} while (ExitCode == STILL_ACTIVE || ExitCode == 2 );

   PostMessage(hWnd,WM_QUIT,0,0);
   
	return 0;
}


// Windows Message processing loop.
void MessageLoop()
{
	BOOL bRet;
	MSG msg;

	while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
	{ 
        if (bAskedToExit)
            break;

		TranslateMessage(&msg); 
		DispatchMessage(&msg); 
	}

}


// Window class registration.
void main_Register(HINSTANCE hInstance)
{
  WNDCLASS wc = 
  {
    CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW,
    main_WindowProc,
    0,0,
    hInstance,
    NULL,
    LoadCursor(NULL,IDC_ARROW),
    (HBRUSH)CreateSolidBrush(RGB(215,148,9)),//COLOR_WINDOW+1),
    NULL,
    "MAINFRAME"
  };

  RegisterClass(&wc);
}

// Loads the sipXphone splash screen bitmap
void LoadIXBitmap()
{
	HINSTANCE hInstance = (HINSTANCE) GetModuleHandle (NULL);

	HANDLE hFileBmp = LoadImage(NULL,"oemsplash.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
	if (hFileBmp)
		hbmp = (HBITMAP)hFileBmp;
	else
		hbmp = LoadBitmap (hInstance, MAKEINTRESOURCE(IDB_BITMAP1));

	GetObject(hbmp, sizeof(BITMAP), &bm);
}

// Event handler for the WM_CLOSE message
int OnClose(HWND hwnd)
{
    AskThreadToClose();
    Sleep(250);
    //try to kill now
    TerminateThread(hMessageThread,0);
    TerminateProcess(ProcessInformation.hProcess,0);
	return TRUE;
}

// Event handler for the WM_TRAY_ICON_MESSAGE message
int OnTrayIconMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{

    POINT pos;
    HMENU hMenu = LoadMenu(g_hInstance,MAKEINTRESOURCE(IDR_POPUPMENU1));
    HMENU hSubMenu = GetSubMenu(hMenu,0);

    switch (lParam)
    {
        case WM_RBUTTONUP:

            // Make chosen menu item the default (bold font)
            ::SetMenuDefaultItem(hSubMenu, 0, 0);

            // Display and track the popup menu
            ::GetCursorPos(&pos);

            SetForegroundWindow(hWnd);  
            ::TrackPopupMenu(hSubMenu, 0, pos.x, pos.y, 0, 
                         hWnd, NULL);

            ::PostMessage(hWnd,WM_NULL, 0, 0);
            ::DestroyMenu(hMenu);

            break;
        case WM_LBUTTONDBLCLK:
            HWND hixWnd = FindWindow(NULL,"SIP Softphone (TM)");
            if (hixWnd)
            {
                ShowWindow(hixWnd,SW_SHOW);
                ShowWindow(hixWnd,SW_RESTORE);
            }
            break;
    }
    return 0;
}


// Event handler for the WM_PAINT message. Displays the splash screen
int OnPaint(HWND hwnd)
{
	TEXTMETRIC tm;
	HDC hMemDC;
   RECT updateRect;
   PAINTSTRUCT ps;

   if (GetUpdateRect(hwnd,       // handle to window
         &updateRect,   // update rectangle coordinates
         FALSE)          // erase state
       != 0)
   {

	   HDC hDC = GetDC(hwnd);
      
      BeginPaint(hwnd,&ps);

	    //paint the bmp onto the screen
	   hMemDC = CreateCompatibleDC(hDC);
	   if (hDC && hMemDC)
	   {

		   HGDIOBJ hOldObject = SelectObject(hMemDC, hbmp);

		   char *szMessage = "Starting SIP Softphone... Please wait...";

		   SetBkColor(hDC,RGB(215,148,9));
		   GetTextMetrics(hDC,&tm);


		   BitBlt(
			   hDC,	//destination dc
			   0,		//X dest
			   0,		//Y dest
			   bm.bmWidth,		//Width 
			   bm.bmHeight,	//Height 
			   hMemDC,
			   0,		//X src
			   0,		//Y src
			   SRCCOPY);

		   SIZE textSize;	
		   GetTextExtentPoint(hDC,szMessage,strlen(szMessage),&textSize);
		   
//		   TextOut(hDC,(bm.bmWidth/2)-(textSize.cx/2),bm.bmHeight-tm.tmHeight*1.5,szMessage,strlen(szMessage));
		   
		   SelectObject(hMemDC, hOldObject);

		   DeleteDC(hMemDC);
		   ReleaseDC(hwnd,hDC);
	      
         EndPaint(hwnd,&ps);
	   }
   }
   
	return TRUE;
}


// Window procedure callback.
LRESULT CALLBACK main_WindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
  switch(uMsg)
  {
	 HANDLE_MSG(hwnd,WM_PAINT,OnPaint);
	 HANDLE_MSG(hwnd,WM_CLOSE,OnClose);

    case WM_TRAY_ICON_MESSAGE:
        return OnTrayIconMessage(hwnd,uMsg,wParam,lParam);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDM_SHOWIX)
        {
            HWND hixWnd = FindWindow(NULL,"SIP Softphone (TM)");
            if (hixWnd)
            {
                HWND hixWnd = FindWindow(NULL,"SIP Softphone (TM)");
                ShowWindow(hixWnd,SW_SHOW);
                ShowWindow(hixWnd,SW_RESTORE);
            }
        }
        else
        if (LOWORD(wParam) == IDM_HIDEIX)
        {
            HWND hixWnd = FindWindow(NULL,"SIP Softphone (TM)");
            if (hixWnd)
            {
                ShowWindow(hixWnd,SW_MINIMIZE);
                ShowWindow(hixWnd,SW_HIDE);
            }
        }
        else
        if (LOWORD(wParam) == IDM_SHUTDOWNIX)
        {
            if (ProcessInformation.hProcess)
                TerminateProcess(ProcessInformation.hProcess,0);
        }


        break;
     default:
          return DefWindowProc(hwnd,uMsg,wParam,lParam);
  }
  return DefWindowProc(hwnd,uMsg,wParam,lParam);
}


// Creates the splash window. 
int CreateSplashWindow(HINSTANCE hInstance)
{
	HWND hDesktopWnd = GetDesktopWindow();
	
	GetWindowRect(hDesktopWnd,&desktopRect);   // window coordinates
	
	windowStartX = (desktopRect.right/2)-(bm.bmWidth/2);
	windowStartY = (desktopRect.bottom/2)-(bm.bmHeight/2);

   hWnd = CreateWindowEx( WS_EX_TOPMOST| WS_EX_TOOLWINDOW,
			"MAINFRAME","Starting SIP Softphone, please wait...",
			 WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU,
			windowStartX,windowStartY,bm.bmWidth,bm.bmHeight,
			HWND_DESKTOP,NULL,hInstance,NULL);

    
	ShowWindow(hWnd,SW_SHOW);
	BringWindowToTop(hWnd);
	UpdateWindow(hWnd);

	return 0;
}


// starts up the thread that waits for the launched application to exit.
// not sure why you need two threads to take care of restarting an app???
void StartWaitingThread()
{
   unsigned int dummyThreadID;
   
	hMessageThread = (void *)_beginthreadex(
      NULL,             // pointer to thread security attributes
      16000,            // initial thread stack size, in bytes
      WaitForExitThread,      // pointer to thread function
      (void *) 0,       // argument for new thread
      0, // creation flags
      &dummyThreadID    // pointer to returned thread identifier
   );

   if (hMessageThread == 0)
	{
		MessageBox(NULL,"Error starting WaitForExitThread thread!","ERROR",MB_OK);
	}
}


// creates the splash, starts the message loop
int StartIX(char *lpCmdLine, HINSTANCE hInstC)
{
	bool retval = false;

	
	hStartupEvent = CreateEvent( NULL,TRUE,FALSE,"XpressaStartupComplete");

	LoadIXBitmap();


	//clear out structure
	memset(&StartupInfo,'\0',sizeof(STARTUPINFO));

	StartupInfo.cb = sizeof(STARTUPINFO);
	StartupInfo.lpReserved = NULL;
	StartupInfo.wShowWindow = SW_HIDE;
	StartupInfo.lpDesktop = NULL;
	StartupInfo.lpTitle = NULL;
	int retcode = CreateProcess(
				NULL,  // name of executable module
				lpCmdLine,       // command line string
				NULL, 
				NULL, 
				FALSE,       // handle inheritance flag
				CREATE_NEW_CONSOLE,      // creation flags
				NULL,       // new environment block
				NULL, // current directory name
				&StartupInfo, 
				&ProcessInformation 
				);

	if (retcode > 0) //success
	{
   	
      //show splash screen
	   if (hWnd == NULL)
      {
		   main_Register(hInstC);
			CreateSplashWindow(hInstC);

#ifdef ENABLE_TRAY_ICON    
            AddTrayIcon();
#endif

      }

      StartWaitingThread();
      
      //thread created by CreateSplash will make message loop exit
      MessageLoop();  //pump message and wait for quit

   }
	else
	{
		MessageBox(NULL,"Error spawning process!","ERROR",MB_OK);
		return 3;
	}

	return 0;
}


// Sets a global flag to indicate a request to terminate the thread.
void AskThreadToClose()
{
    bAskedToExit = true; //ask the thread (if still running) to exit
}

// Main entry point for the application.
int WINAPI WinMain(HINSTANCE hInstC, HINSTANCE hInstP, 
                   LPSTR lpCmdLine, int nCmdShow)
{
    atexit(AskThreadToClose);
    
    //save off the hInstance
    g_hInstance = hInstC;

	HANDLE h = CreateEvent(
		NULL,			// EventAttributes,
						// pointer to security attributes
		TRUE,			// flag for manual-reset event
		TRUE, // flag for initial state
		"IXPRESSA2"      // pointer to event-object name
		);

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBox(NULL,"You cannot have more than one instance of SIP Softphone running!","ALREADY RUNNING",MB_OK);
		return 1;
	}
	
	//if no input just return
	if (!strlen(lpCmdLine))
		return 0;
   //make a copy
   strcpy(cmdLineCopy,lpCmdLine);
   
	int retcode = StartIX(lpCmdLine,hInstC); //start the IX process and enter message loop

	DeleteObject (hbmp); /* Delete objects when you're finished with them. */

#ifdef ENABLE_TRAY_ICON    
    RemoveTrayIcon();
#endif

	return retcode; 
}
