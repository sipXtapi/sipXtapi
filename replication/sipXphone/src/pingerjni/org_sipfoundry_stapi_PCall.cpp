// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_stapi_PCall.cpp#2 $
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
#include <jni.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "cp/CallManager.h"
#include "pinger/Pinger.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"
#include "pingerjni/JavaSUAListenerDispatcher.h"
#include "net/SipUserAgent.h"
#include "net/SipMessage.h"
#include "os/OsFS.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* ============================ FUNCTIONS ================================= */

/*
 * Class:     org_sipfoundry_stapi_PCall
 * Method:    addDtmfListener
 * Signature: (Ljava/lang/String;Ljava/lang/Object;J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_stapi_PCall_addDtmfListener
  (JNIEnv* pEnv, jclass clazz, jstring jsCallId, jobject jobjListener, jlong lHashcode)
{
#ifndef BUILD_SDS
   CallManager* pCallManager = Pinger::getPingerTask()->getCallManager();

   // Convert Data: Java --> C
   char* szCallId = (jsCallId) ? (char *) pEnv->GetStringUTFChars(jsCallId, 0) : NULL ;

   if (pCallManager != NULL)
   {
      pCallManager->addToneListener(szCallId, (unsigned int) JavaSUAListenerDispatcher::getInstance()) ;
   }

   // Clean up
   if (szCallId != NULL)
      pEnv->ReleaseStringUTFChars(jsCallId, szCallId) ; 
#endif
}

/*
 * Class:     org_sipfoundry_stapi_PCall
 * Method:    startRecord
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_stapi_PCall_startRecording
  (JNIEnv* pEnv, jclass clazz, jstring jsFileName, jstring jsCallId, 
   jint msLength, jint msSilenceLength)
{
   CallManager* pCallManager = Pinger::getPingerTask()->getCallManager();

   // Convert Data: Java --> C
   char* szCallId = (jsCallId) ? (char *) pEnv->GetStringUTFChars(jsCallId, 0) : NULL ;
   char* szFileName = (jsFileName) ? (char *) pEnv->GetStringUTFChars(jsFileName, 0) : NULL ;
    
   //these two are hidden from java
   //no need to set these (for now) from java
   int duration;
   int dtmfterm;

   if (pCallManager != NULL)
   {
      OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
      OsProtectedEvent* recordEvent = eventMgr->alloc();
      OsFileSystem::remove(szFileName);
      pCallManager->ezRecord(szCallId, 
                        (int)msLength, 
                        (int)msSilenceLength,
                        duration,
                        szFileName,
                        dtmfterm,
                        recordEvent) ;
   }

   // Clean up
   if (szCallId != NULL)
      pEnv->ReleaseStringUTFChars(jsCallId, szCallId) ; 
}

/*
 * Class:     org_sipfoundry_stapi_PCall
 * Method:    JNI_stopRecord
 * Signature: (V;)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_stapi_PCall_stopRecording
  (JNIEnv* pEnv, jclass clazz, jstring jsCallId)
{
   CallManager* pCallManager = Pinger::getPingerTask()->getCallManager();


   if (pCallManager != NULL)
   {
       char* szCallId = (jsCallId) ? (char *) pEnv->GetStringUTFChars(jsCallId, 0) : NULL ;
       
       osPrintf("Calling CallManager->stopRecording()\n");
       pCallManager->stopRecording(szCallId);
       osPrintf("DONE Calling CallManager->stopRecording()\n");

       
        // Clean up
        if (szCallId != NULL)
            pEnv->ReleaseStringUTFChars(jsCallId, szCallId) ; 

   }

}
