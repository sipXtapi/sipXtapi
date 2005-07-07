// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_xpressa_sys_startup_PingerApp.cpp#2 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#include <jni.h>
#include <assert.h>

#include "net/SipUserAgent.h"

#include "os/OsDefs.h"
#include "os/OsFS.h"

#include "pinger/Pinger.h"
#include "pinger/PingerMsg.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"


/*
 * Class:     org_sipfoundry_sipxphone_sys_startup_PingerApp
 * Method:    JNI_consolePrint
 * Signature: (Ljava/lang/String;I)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_sipxphone_sys_startup_PingerApp_JNI_1consolePrint
  (JNIEnv *pEnv, jclass clazz, jstring jsOutput, jint type)
{
	char *szOutput = (jsOutput) ? (char *) pEnv->GetStringUTFChars(jsOutput, 0) : NULL ;

	if (szOutput) 
   {
		osPrintf("%s", szOutput) ;
		pEnv->ReleaseStringUTFChars(jsOutput, szOutput) ;
	}
}



/*
 * Class:     org_sipfoundry_sipxphone_sys_startup_PingerApp
 * Method:    JNI_notifyJavaInitialized
 * Signature: ()V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_sipxphone_sys_startup_PingerApp_JNI_1notifyJavaInitialized
  (JNIEnv *pEnv, jclass clazz)
{
	JNI_BEGIN_METHOD("Java_org_sipfoundry_sipxphone_sys_startup_PingerApp_JNI_1notifyJavaInitialized") ;

    PingerMsg javaInitialized(PingerMsg::JAVA_INITIALIZED);
    Pinger::getPingerTask()->getMessageQueue()->send(javaInitialized) ;
    
	JNI_END_METHOD() ;
}


/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_isSipLogEnabled
 * Signature: ()Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1isSipLogEnabled
  (JNIEnv *pEnv, jclass clazz)
{
	JNI_BEGIN_METHOD("Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1isSipLogEnabled") ;

#ifdef BUILD_SDS
    SipUserAgent* pUserAgent = SipConfigServerAgent::
        getSipConfigServerAgent()->getSipUserAgent() ;
#else
    SipUserAgent* pUserAgent = Pinger::getPingerTask()->getSipUserAgent() ;
#endif
	jboolean bLogging = FALSE ;

	if (pUserAgent != NULL)
	{
		bLogging = pUserAgent->isMessageLoggingEnabled() ;
	}
   
	JNI_END_METHOD() ;

	return bLogging ;
}

/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_saveSipLog
 * Signature: (Ljava/lang/String;)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1saveSipLog
  (JNIEnv *pEnv, jclass clazz, jstring jsFileSpec)
{
	JNI_BEGIN_METHOD("Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1storeSipLog") ;

	UtlString sipLog ;
	char* szFileSpec = JSTRTOCSTR(pEnv, jsFileSpec) ;
	jboolean bSuccess = FALSE ;

#ifdef BUILD_SDS
    SipUserAgent* pUserAgent = SipConfigServerAgent::
        getSipConfigServerAgent()->getSipUserAgent() ;
#else
    SipUserAgent* pUserAgent = Pinger::getPingerTask()->getSipUserAgent() ;
#endif

	
	if ((pUserAgent != NULL) && (szFileSpec != NULL))
	{
		pUserAgent->getMessageLog(sipLog);

		OsFile file(szFileSpec) ;

		if (file.open(OsFile::READ_WRITE | OsFile::TRUNCATE) == OS_SUCCESS)
		{
			unsigned long written = 0;
			const char* szLogData = sipLog.data() ;
			const unsigned long nLogData = sipLog.length() ;

			if (file.write(szLogData, nLogData, written) == OS_SUCCESS)
			{
				assert(nLogData == written) ;
				bSuccess = TRUE ;
			}
			file.close() ;
		}
	}

	RELEASECSTR(pEnv, jsFileSpec, szFileSpec) ;
	JNI_END_METHOD() ;

	return bSuccess ;
}


/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_enableSipLog
 * Signature: (Z)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1enableSipLog
  (JNIEnv *pEnv, jclass clazz, jboolean jbEnable)
{
	JNI_BEGIN_METHOD("Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1storeSipLog") ;

#ifdef BUILD_SDS
    SipUserAgent* pUserAgent = SipConfigServerAgent::
        getSipConfigServerAgent()->getSipUserAgent() ;
#else
    SipUserAgent* pUserAgent = Pinger::getPingerTask()->getSipUserAgent() ;
#endif

	if (pUserAgent != NULL)
	{
		if (jbEnable)
		{
			// Enable the SIP Log
			pUserAgent->clearMessageLog() ;
			pUserAgent->startMessageLog() ;
		}
		else
		{
			// Disable the SIP Log
			pUserAgent->stopMessageLog() ;
			pUserAgent->clearMessageLog() ;  
		}
	}

	JNI_END_METHOD() ;
}
