// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/wnt/Clarisys.cpp#2 $
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
#include <windows.h>
#include <objbase.h>
#include <process.h>

// APPLICATION INCLUDES
#include "mp/dmaTask.h"
#include "pinger/Pinger.h"
#include "os/osConfigDb.h"
#include "ps/psPhoneTask.h"
#include "ps/psButtonTask.h"
#include "ps/psButtonId.h"
#include "ps/PsButtonInfo.h"
#include "ps/PsLampTask.h"
#include "ps/PsTaoComponentGroup.h"
#include "pingerjni/JXAPI.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

//HINSTANCE of pingerjni DLL that was loaded.
extern HINSTANCE g_hDllInstance;


//everything beyond this point is extern "C"
#ifdef __cplusplus
extern "C" {
#endif

// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

//handle to dll
HINSTANCE gClaritelDll = NULL;

//handle to window to receive message from phone
HWND g_ClarisysHiddenWnd = NULL;
HWND g_IxWnd = NULL;

// FORWARD DECLARATIONS
void claritel_RegisterWnd(HINSTANCE hInstance);
HWND createHiddenClaritelWnd();

// CLARITEL FUNCTION POINTERS
// These are pointers to functions that the app will load at runtime
typedef long (*LPCLARITELINITIALIZE)(HWND,BSTR,BSTR);
LPCLARITELINITIALIZE gClaritelInitialize = NULL;

typedef long (*LPCLARITELCLOSE)();
LPCLARITELCLOSE gClaritelClose = NULL;

typedef long (*LPCLARITELSTARTTONE)(long);
LPCLARITELSTARTTONE	gClaritelStartTone = NULL;

typedef long (*LPCLARITELSTOPTONE)();
LPCLARITELSTOPTONE gClaritelStopTone = NULL;

typedef long (*LPCLARITELLOCK)();
LPCLARITELLOCK gClaritelLock = NULL;

typedef long (*LPCLARITELUNLOCK)();
LPCLARITELUNLOCK gClaritelUnlock = NULL;

typedef long (*LPCLARITELGETONOFFSTATE)();
LPCLARITELGETONOFFSTATE gClaritelGetOnOffState = NULL;

typedef long (*LPCLARITELGETMUTESTATE)();
LPCLARITELGETMUTESTATE gClaritelGetMuteState = NULL;

typedef long (*LPCLARITELGETSPSTATE)();
LPCLARITELGETSPSTATE gClaritelGetSpState = NULL;

typedef long (*LPCLARITELSETMUTESTATE)(long STATE);
LPCLARITELSETMUTESTATE gClaritelSetMuteState = NULL;

typedef long (*LPCLARITELSETECHOSUP)(long STATE);
LPCLARITELSETECHOSUP gClaritelSetEchoSup = NULL;

typedef long (*LPCLARITELSETVOLPOPUP)(long STATE);
LPCLARITELSETVOLPOPUP gClaritelSetVolPopup = NULL;

//-----------------------------------------------------------------------//


//here is the list of CLARITEL created messages

#define WINMM_DEVICECHANGE		   0x0000C06F
#define CLARITEL_END_PRESS		   0x000083EA
#define CLARITEL_SEND_PRESS		0x000083EB
#define CLARITEL_MUTE_CHANGE	   0x000083EC
#define CLARITEL_KEYPAD_PRESS	   0x000083ED
#define CLARITEL_ARROW_PRESS	   0x000083EE
#define CLARITEL_ONOFF_HOOK		0x000083EF
#define CLARITEL_SP_CHANGE		   0x000083F0
#define WM_USER_IX_CHANGE_STATE  WM_USER+4001


BOOL gAskClarisysMessageLoopToExit = FALSE;

//handle to message processing thread
HANDLE ghMessageThread = NULL;


//message processing thread func
unsigned int __stdcall clarisysMessageLoopThread(LPVOID Unused)
{

	//register our window
    claritel_RegisterWnd(g_hDllInstance);

    //this creates the hidden window to receive messages
    createHiddenClaritelWnd();
        
    //this starts a thread which will sit receiving and post messages
    //to our Wndproc
    if (g_ClarisysHiddenWnd && gClaritelInitialize)
    {
	    gClaritelInitialize(g_ClarisysHiddenWnd,L"instant xpressa",L"instant xpressa softphone");

      UtlBoolean bIsOffHook = FALSE;
      
      //this next function returns 1 when it is onhook and 0 when it is offhook
      //seems reverse to me...
      if (!gClaritelGetOnOffState())
         bIsOffHook = TRUE;

      //if clarisys is off hook, we need to send a message and make ix that way too...
      if (bIsOffHook && g_ClarisysHiddenWnd != NULL)
         PostMessage(g_ClarisysHiddenWnd,CLARITEL_ONOFF_HOOK,0,0);
      
      //clear the mute light
      gClaritelSetMuteState(false);
    }

    // call DLL function to initialize Claritel				
    
    if (g_ClarisysHiddenWnd)
    {
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0) && !gAskClarisysMessageLoopToExit) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
    }

	return 0;
}

void startClaritelMessageProcessingThread()
{
   unsigned int dummyThreadID;
   
	ghMessageThread = (void *)_beginthreadex(
      NULL,             // pointer to thread security attributes
      32767,            // initial thread stack size, in bytes
      clarisysMessageLoopThread,      // pointer to thread function
      (void *) 0,       // argument for new thread
      0, // creation flags
      &dummyThreadID    // pointer to returned thread identifier
   );
	
}

//sends a button press to phone
//
void createAndExecuteButtonPress(int iLookupId, UtlBoolean bIsDown)
{
   int iButtonIndex;
   int iButtonState;

   PsPhoneTask* pPhoneTask = PsPhoneTask::getPhoneTask();;
   PsButtonTask *pButtonTask = PsButtonTask::getButtonTask();
   PsMsg        pingtelMsg(PsMsg::BUTTON_DOWN, NULL, 0, 0); //the message we are going to create

   if (bIsDown)
	   iButtonState = PsMsg::BUTTON_DOWN;
   else
	   iButtonState = PsMsg::BUTTON_UP;

   iButtonIndex = pButtonTask->getButtonIndex(iLookupId);
   
   pingtelMsg.setParam1(iButtonIndex);

	pingtelMsg.setParam2(iLookupId);
   
   //there was never a string lookup added for backspace
   if (iLookupId == FKEY_BACKSPACE)
   	pingtelMsg.setStringParam1("BACKSPACE");
   else
   {
      if (iButtonIndex != -1)
	      pingtelMsg.setStringParam1(pButtonTask->getButtonInfo(iButtonIndex).getName());
   }

	pingtelMsg.setMsgSubType(iButtonState);
	pPhoneTask->postMessage(pingtelMsg);

}

// here is the message func that will be called with clarisys messages
LRESULT CALLBACK claritel_WindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
  bool bIxIsOffHook = false;
  bool bClarisysOffHook;
  int spkrMode;
  int iLookupId;
  UtlBoolean bIsButtonDown; 
  PsPhoneTask* pPhoneTask = PsPhoneTask::getPhoneTask();;

#ifdef DEBUG_MESSAGES
  static int i = 0;
  char buf[80];
  sprintf(buf,"Message %d = 0x%0X wParam=%d lParam=%d\n",i++,uMsg,wParam,lParam);
  OutputDebugString(buf);
#endif

   //when receiving clarisys button event wParam will denote 
   // button up or down state as follows:
   // wParam = 0 is a button down event
   // wParam = 1 is a button up   
   bIsButtonDown = (wParam == 0);

   switch(uMsg)
   {
  
   //when we get this message, we should tell the message processing thread
   //that it's ok to exit now.
   case WM_QUIT:
     gAskClarisysMessageLoopToExit = true;
     break;

  //This message is received what a multimedia device is inserted or removed.
  //We will pop a message box on the users display alerting them of this fact.
  //We will also start the restart process.
  case WINMM_DEVICECHANGE:
            JXAPI_MessageBox(0,"Error: Change detected","The status of your USB handset has changed.\n"
               "Press Ok to restart your sip softphone now "
               "to assure quality audio in your calls.",TRUE);
             Pinger::getPingerTask()->restart(FALSE, -1, "USB device change detected") ;
         break;
  
   //when receiving CLARITEL_END_PRESS wParam and lParams can be
  // wParam = 0 is a button down event
  // wParam = 1 is a button up   
  case CLARITEL_END_PRESS:
		iLookupId = FKEY_SKEY_B2;
      createAndExecuteButtonPress(iLookupId,bIsButtonDown);
		break;

  //when receiving CLARITEL_SEND_PRESS wParam and lParams can be
  // wParam = 0 is a button down event
  // wParam = 1 is a button up   
  case CLARITEL_SEND_PRESS:
		iLookupId = FKEY_SKEY_B3;
      createAndExecuteButtonPress(iLookupId,bIsButtonDown);
		break;

  //when receiving CLARITEL_ARROW_PRESS wParam and lParams can be
  // wParam = 0 is a button down event
  // wParam = 1 is a button up 
  // lParam = 0 is LEFT arrow
  // lParam = 1 is UP arrow.
  // lParam = 2 is RIGHT arrow.
  // lParam = 3 is DOWN arrow
  case CLARITEL_ARROW_PRESS:
      iLookupId = -1;
		if (lParam == 0) //user pressed the left arrow
		{
			iLookupId = FKEY_BACKSPACE;
 
		}
		else 
		if (lParam == 1) //user pressed clarisys up key
			iLookupId = FKEY_SCROLL_UP;
		else
		if (lParam == 2) //user pressed clarisys right arrow
      {
         //since we don't have a dedicated key which means "move right"
         //I neeed to send the L3 key.  Most of the time, 
         //when processing input, it is a move right key
			iLookupId	= 	FKEY_SKEY_L3;
      }
		else
		if (lParam == 3) //user pressed the clarisys down arrow
			iLookupId = FKEY_SCROLL_DOWN;
		
      if (iLookupId > 0)
         createAndExecuteButtonPress(iLookupId,bIsButtonDown);
		break;

  //when receiving CLARITEL_ONOFF_HOOK wParam and lParams can be
  // wParam = 0 is a button down event
  // wParam = 1 is a button up 
  case CLARITEL_ONOFF_HOOK:
         
         //remember clarisysOffHook will be 0 when it's off hook.
         if (gClaritelGetOnOffState() == 0)
            bClarisysOffHook = true;
         else
            bClarisysOffHook = false;

         //these next few lines sets the variable bIxIsOffHook based on whether
         //the spkrmode is NOT ringing. So, if spkrMode has a value, and the
         //RINGER_ENABLED bit is not set, then we can say that ix is off hook.
         //In that case it would be in headset or speaker mode.
         spkrMode = pPhoneTask->getSpeakerMode();
         if (spkrMode & PsPhoneTask::SPEAKERPHONE_ENABLED ||
            spkrMode & PsPhoneTask::HEADSET_ENABLED ||
            spkrMode & PsPhoneTask::HANDSET_ENABLED)
            bIxIsOffHook = true;
         else
            bIxIsOffHook = false;

         //only send the FKEY_SPEAKER button to ix when the clarisys
         //is transistioning to the correct mode as ix.
         //it may be that clarisys is sending the current state of the button
         //when the button it pressed to go off hook.
         if ((bClarisysOffHook && !bIxIsOffHook) ||
             (!bClarisysOffHook && bIxIsOffHook))
         {

   			iLookupId = FKEY_SPEAKER;
            createAndExecuteButtonPress(iLookupId,TRUE);
            createAndExecuteButtonPress(iLookupId,FALSE);
         
            if (wParam == 1)
			      gClaritelSetMuteState(false);

            //bring window to foreground
            if (wParam == 0) //clarisys is off hook
            {
               HWND hWnd = FindWindow(NULL,"SIP Softphone (TM)");
               if (hWnd)
               {
                  ShowWindow(hWnd,SW_SHOW);
                  ShowWindow(hWnd,SW_RESTORE);
               }
            }
         }
		break;

    case CLARITEL_KEYPAD_PRESS:

      switch(lParam)
		{
			case 1:	iLookupId = DIAL_1;
					break;
			case 2:	iLookupId = DIAL_2;
					break;
			case 3:	iLookupId = DIAL_3;
					break;
			case 4:	iLookupId = DIAL_4;
					break;
			case 5:	iLookupId = DIAL_5;
					break;
			case 6:	iLookupId = DIAL_6;
					break;
			case 7:	iLookupId = DIAL_7;
					break;
			case 8:	iLookupId = DIAL_8;
					break;
			case 9:	iLookupId = DIAL_9;
					break;
			case 0:	iLookupId = DIAL_0;
					break;
			case 10:	iLookupId = DIAL_STAR;
						break;
			case 11:	iLookupId = DIAL_POUND;
						break;

		}
      
      createAndExecuteButtonPress(iLookupId,bIsButtonDown);

		break;

	case CLARITEL_MUTE_CHANGE:
		if (wParam == 0 && !DmaTask::isMuteEnabled())		
			;
		else
		if (wParam == 1 && DmaTask::isMuteEnabled())		
			;
		else
		{
			iLookupId = FKEY_MUTE;
         createAndExecuteButtonPress(iLookupId,TRUE);
         createAndExecuteButtonPress(iLookupId,FALSE);
		}

		break;
   case WM_USER_IX_CHANGE_STATE:
         if ((!gClaritelGetMuteState() && wParam == 1) ||
             (gClaritelGetMuteState() && !wParam))
            gClaritelSetMuteState(wParam);
      break;
	
   default:
      //since we didn't handle the message here, pass it on...
		return DefWindowProc(hwnd,uMsg,wParam,lParam);
  }
  return 0;
}



void claritel_RegisterWnd(HINSTANCE hInstance)
{
  WNDCLASS wc = 
  {
    CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW,
    claritel_WindowProc,
    0,0,
    hInstance,
    NULL,
    NULL,
    NULL,
    NULL,
    "claritelHiddenWnd"
  };

  RegisterClass(&wc);
}





void shutdownClaritel()
{
   DWORD exitCode;
   int exitCount = 0; //keeps locked message loop from locking us here

   //send message to thread to close
   SendMessage(g_ClarisysHiddenWnd,WM_QUIT,0,0);
   
   //now wait until it's closed
   do
   {
      GetExitCodeThread(ghMessageThread, &exitCode);
      Sleep(100);
      exitCount++;
   } while (exitCode == STILL_ACTIVE  && exitCount < 10);

   //Remove listener for "mute" state changes
   DmaTask::setMuteListener(NULL);
   
   if(gClaritelClose) 
   {
      gClaritelClose();
   }      
 
   //Unitialize Claritel.dll
   FreeLibrary(gClaritelDll);
   gClaritelDll=NULL;
}


//returns TRUE if dll is loaded ok, and all functions have been found
BOOL LoadClaritelDll()
{
   UtlBoolean retVal = FALSE;

	//Attempt to load Claritel.dll
	gClaritelDll=NULL;
	gClaritelDll=LoadLibrary("env\\Claritel.dll");

	if(gClaritelDll) //If successfull
	{
		gClaritelInitialize = NULL;
		gClaritelClose = NULL;
		gClaritelStartTone = NULL;
		gClaritelStopTone = NULL;
		gClaritelLock = NULL;
		gClaritelUnlock = NULL;
      gClaritelGetOnOffState = NULL;
      gClaritelGetMuteState = NULL;
		gClaritelGetSpState = NULL;
		gClaritelSetMuteState = NULL;
		gClaritelSetEchoSup = NULL;
		gClaritelSetVolPopup = NULL;

		//attempt to load functions
		gClaritelInitialize = (LPCLARITELINITIALIZE)GetProcAddress(gClaritelDll,"Initialize");
		gClaritelClose = (LPCLARITELCLOSE)GetProcAddress(gClaritelDll,"Close");
		gClaritelStartTone = (LPCLARITELSTARTTONE)GetProcAddress(gClaritelDll,"StartTone");
		gClaritelStopTone = (LPCLARITELSTOPTONE)GetProcAddress(gClaritelDll,"StopTone");
		gClaritelLock = (LPCLARITELLOCK)GetProcAddress(gClaritelDll,"Lock");
		gClaritelUnlock = (LPCLARITELUNLOCK)GetProcAddress(gClaritelDll,"Unlock");
		gClaritelGetOnOffState = (LPCLARITELGETONOFFSTATE)GetProcAddress(gClaritelDll,"GetOnOffState");
		gClaritelGetMuteState = (LPCLARITELGETMUTESTATE)GetProcAddress(gClaritelDll,"GetMuteState");
		gClaritelGetSpState = (LPCLARITELGETSPSTATE)GetProcAddress(gClaritelDll,"GetSpState");
		gClaritelSetMuteState = (LPCLARITELSETMUTESTATE)GetProcAddress(gClaritelDll,"SetMuteState");
		gClaritelSetEchoSup = (LPCLARITELSETECHOSUP)GetProcAddress(gClaritelDll,"SetEchoSup");
		gClaritelSetVolPopup = (LPCLARITELSETVOLPOPUP)GetProcAddress(gClaritelDll,"SetVolPopup");



		//If anyone of the functions failed to load
		//do not use any of the function pointers
		if(gClaritelInitialize == NULL || gClaritelClose == NULL ||
			gClaritelStartTone == NULL || gClaritelStopTone == NULL ||
			gClaritelLock == NULL || gClaritelUnlock == NULL ||
			gClaritelGetOnOffState == NULL || gClaritelGetMuteState == NULL ||
			gClaritelGetSpState == NULL || gClaritelSetMuteState == NULL ||
			gClaritelSetEchoSup == NULL || gClaritelSetVolPopup == NULL)
			retVal = FALSE;
      else
         retVal = TRUE;
	
	}
   

   //if we failed, reset all function pointers and 
   if (!retVal)
   {
      if (gClaritelDll)
      {
			FreeLibrary(gClaritelDll);
   		gClaritelDll=NULL;
      }
		gClaritelInitialize = NULL;
		gClaritelClose = NULL;
		gClaritelStartTone = NULL;
		gClaritelStopTone = NULL;
		gClaritelLock = NULL;
		gClaritelUnlock = NULL;
		gClaritelGetOnOffState = NULL;
		gClaritelGetMuteState = NULL;
		gClaritelGetSpState = NULL;
		gClaritelSetMuteState = NULL;
		gClaritelSetEchoSup = NULL;
		gClaritelSetVolPopup = NULL;
		gClaritelDll=NULL;
   }

	return retVal;

}

HWND createHiddenClaritelWnd()
{
    HWND hWnd = CreateWindow( "claritelHiddenWnd","Hidden Claritel Window",
			 WS_POPUP ,1,1,100,100, GetDesktopWindow(),NULL,g_hDllInstance,NULL);

    if (hWnd)
    {
        g_ClarisysHiddenWnd = hWnd;
        ShowWindow(hWnd,SW_HIDE);
        UpdateWindow(hWnd);
    }

    return hWnd;
}

void setHandsetMuteState(bool state)
{
	if(gClaritelDll) // If successful
	{
	   PostMessage(g_ClarisysHiddenWnd,WM_USER_IX_CHANGE_STATE,state,0);
   }
}

BOOL startupClaritelPhone()
{
    BOOL retval = FALSE;

//---------------------Initialize Claritel Phone------------------------//
//	The Claritel.dll exports functions that initialize the Claritel Phone
	if (LoadClaritelDll())		// Call Helper function to dynamically load Claritel.dll
   {
      gClaritelSetVolPopup(FALSE);

      UtlString enableEchoSupStr;
      UtlBoolean bEchoSuppress = TRUE;
      //check to see if we should enable echo suppress
      OsConfigDb *pConfigDb = Pinger::getPingerTask()->getConfigDb();
      if (pConfigDb->get("PHONESET_IX_ECHO_SUPPRESS",enableEchoSupStr) == OS_SUCCESS)
      {
         if (enableEchoSupStr == "DISABLE")
            bEchoSuppress = FALSE;
      }

      gClaritelSetEchoSup(bEchoSuppress);
      //dialtone might be enabled on clarisys. shut it off
      gClaritelStopTone();
	   startClaritelMessageProcessingThread();
      
      // Set up listener for changes to the "mute" state
       DmaTask::setMuteListener(setHandsetMuteState);

      retval = true;
   }

	return retval;
}

#ifdef __cplusplus
}  //endif extern "C"
#endif
