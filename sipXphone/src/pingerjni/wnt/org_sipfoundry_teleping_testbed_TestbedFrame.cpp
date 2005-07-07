// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/wnt/com_pingtel_teleping_testbed_TestbedFrame.cpp#3 $
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
#include <jni.h>
#include <os/iostream>
#include <assert.h>
#include <io.h>
#include <sys/stat.h>

#include "os/osConfigDb.h"
#include "pinger/Pinger.h"
#include "pinger/PingerInfo.h"
#include "pingerjni/JNIHelpers.h"

//global DLL instance variable for any part of this code which might need the handle 
//to the dll
HINSTANCE g_hDllInstance = NULL;

//EXTERNAL FUNCTIONS USED FOR CLARISYS HANDSET
extern "C" void startupClaritelPhone();
extern "C" void shutdownClaritel();


// APPLICATION INCLUDES

/*
 * Class:     org_sipfoundry_sipxphone_testbed_TestbedFrame
 * Method:    JNI_pingerStart
 * Signature: ()V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_sipxphone_testbed_TestbedFrame_JNI_1pingerStart
  (JNIEnv *jenv, jclass clazz)
{
   if (!jniInitJVMReference()) 
   {
      LOOKUP_FAILURE() ;
   }

	Pinger::getPingerTask();
   
	//osPrintf("** JNI_PingerStart - entered method **\r\n") ;

}

extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_sipxphone_testbed_TestbedFrame_JNI_1restoreMinimizedWindow
  (JNIEnv *jenv, jclass clazz)
{
    HWND hWnd = FindWindow(NULL,"sip softphone (TM)");
    if (hWnd)
    {
        ShowWindow(hWnd,SW_SHOW);
        ShowWindow(hWnd,SW_RESTORE);
    }


}

/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_signalStartupFinished
 * Signature: ()V
 * Description: Signals to the launch app that instant xpressa has started, and it should hide.
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_sipxphone_testbed_TestbedFrame_JNI_1signalStartupFinished
  (JNIEnv *jenv, jclass clazz)
{
#ifdef _WIN32
	HANDLE hStartupEvent = CreateEvent( NULL,TRUE,FALSE,"XpressaStartupComplete");
   BOOL bKillClarisys = TRUE ;
   UtlString setting ;

	SetEvent(hStartupEvent);


   // The PHONESET_IX_RESTART_CLARISYS setting controls whether we restart
   // the clarisys infrastructure whenever we start instant xpressa.  This
   // works around a detach bug, where we can't reattach (and receive buttons
   // presses) after restarting instant xpressa.
   OsConfigDb *pConfigDb = Pinger::getPingerTask()->getConfigDb();
   if (pConfigDb->get("PHONESET_IX_RESTART_CLARISYS", setting) == OS_SUCCESS)
   {
      if ( setting.compareTo("DISABLE", UtlString::ignoreCase) == 0 )
          bKillClarisys = FALSE;
   }

   if (bKillClarisys)
   {
      HWND hWnd = FindWindow("IPhoneServiceWndClass", NULL) ;
      if (hWnd != NULL)
      {
         PostMessage(hWnd, WM_QUIT, 0, 0) ;
      }
   }

   startupClaritelPhone();
#endif

}


/*
 * Class:     org_sipfoundry_sipxphone_testbed_TestbedFrame
 * Method:    JNI_shutdownHook
 * Signature: ()V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_sipxphone_testbed_TestbedFrame_JNI_1shutdownHook
  (JNIEnv *jenv, jclass clazz)
{
#ifdef _WIN32
	shutdownClaritel() ;
#endif

   //attempt to initialize the Claritel Phone.
   //The reason this is done here, is that we need java
   //up when a handset is initialized.
   //The state of the handset may cause buttons presses to be sent 
   //which will cause gui changes (buttons lighting)
   startupClaritelPhone();

}




BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,  // handle to the DLL module
  DWORD fdwReason,     // reason for calling function
  LPVOID lpvReserved   // reserved
)
{
    if (g_hDllInstance == NULL)
        g_hDllInstance = hinstDLL;

	return TRUE;
}
