// 
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

#include "os/OsDefs.h"
#include "sipendpoint/JNIHelpers.h"
#include "net/SipUserAgent.h"
#include "net/SipConfigServerAgent.h"
#include "net/SipMessage.h"


/*
 * This method is invoked whenever a new client subscribes/registers with the 
 * SipConfigServerAgent.  It is this routines responsibility to package up 
 * the information and send it to Java land.
 */

#define SIP_CONFIG_USER_AGENT_CLASS "com/pingtel/pds/sds/SipConfigServerAgent"
#define ADD_SUBSCRIPTION_METHOD     "addSubscription"
#define ADD_SUBSCRIPTION_SIGNATURE  "(Ljava/lang/String;)V"
int EnrollmentCallback(const SipMessage& subscribeRequest)
{
    bool     bSuccess = false ;
    UtlString messageContents ;
    int      iLen ;
    JavaVM*  pVM ;
    JNIEnv*  pEnv ;
    
osPrintf("JNI: BEGIN Received Subscribe Request\n\n") ;

    // First: Tuck away our priority.  The JVM will change it beneath our feet
    int iPriority = jniGetCurrentPriority() ;
    // Pull the message contents out of the message
    subscribeRequest.getBytes(&messageContents, &iLen);
osPrintf("\n\n%s\n\n", messageContents.data()) ;

	// Create or attach to a JVM
	if (jniGetVMReference(&pVM, &pEnv)) {

		jclass    cls ;
		jmethodID methodid ;

        cls = pEnv->FindClass(SIP_CONFIG_USER_AGENT_CLASS) ;
		if (cls != NULL) {
            jobject jobjString = pEnv->NewStringUTF(messageContents.data()) ;
			methodid = pEnv->GetStaticMethodID(cls, ADD_SUBSCRIPTION_METHOD, ADD_SUBSCRIPTION_SIGNATURE) ;
			if (methodid != NULL) {
				pEnv->CallStaticVoidMethod(cls, methodid, jobjString) ;
				if (pEnv->ExceptionOccurred()) {
					pEnv->ExceptionDescribe() ;
					pEnv->ExceptionClear() ;
                } else {
                    bSuccess = true ;
                }
            } else {
                osPrintf("JNI: Unable to lookup method %s/%s on class %s\n", ADD_SUBSCRIPTION_METHOD, ADD_SUBSCRIPTION_SIGNATURE, SIP_CONFIG_USER_AGENT_CLASS) ;
            }
        } else {
            osPrintf("JNI: Unable to find class %s\n", SIP_CONFIG_USER_AGENT_CLASS) ;
        }

        bSuccess = true ;

        jniReleaseVMReference(pVM) ;
    }

    // Reset our priority.
    jniResetPriority(iPriority) ;
    return bSuccess ? 202 : 403 ;
}



/*
 * Class:     com_pingtel_pds_sds_SipConfigServerAgent
 * Method:    JNI_initializeCallback
 * Signature: ()V
 */
extern "C"
JNIEXPORT void JNICALL Java_com_pingtel_pds_sds_SipConfigServerAgent_JNI_1initializeCallback
  (JNIEnv *pEnv, jclass clazz)

{
    JNI_BEGIN_METHOD("Java_com_pingtel_pds_sds_SipConfigServerAgent_JNI_1initializeCallback") ;

    if (jniInitJVMReference() == false)
    {
        osPrintf("JNI: FAILED to initialize JVM!") ;
    }

    SipConfigServerAgent::getSipConfigServerAgent()->setEnrollmentCallback(EnrollmentCallback) ;

    JNI_END_METHOD() ;
}


/*
 * Class:     com_pingtel_pds_sds_SipConfigServerAgent
 * Method:    JNI_initializeUserAgent
 * Signature: ()V
 */
extern "C"
JNIEXPORT void JNICALL Java_com_pingtel_pds_sds_SipConfigServerAgent_JNI_1initializeUserAgent
  (JNIEnv *pEnv, jclass clazz)

{
    JNI_BEGIN_METHOD("Java_com_pingtel_pds_sds_SipConfigServerAgent_JNI_1initializeUserAgent") ;

    SipConfigServerAgent::getSipConfigServerAgent() ;

    JNI_END_METHOD() ;
}



/*
 * Class:     com_pingtel_pds_sds_SipConfigServerAgent
 * Method:    JNI_sendSipMessage
 * Signature: (Ljava/lang/String;)V
 */
extern "C"
JNIEXPORT void JNICALL Java_com_pingtel_pds_sds_SipConfigServerAgent_JNI_1sendSipMessage
  (JNIEnv *pEnv, jclass clazz, jstring jsMessage)
{
    osPrintf("JNI: Sending message...") ;

    char* szMessage = (jsMessage) ? (char *) pEnv->GetStringUTFChars(jsMessage, 0) : NULL ;

    if (szMessage != NULL) {
        SipMessage message((const char*) szMessage) ;

        SipUserAgent* pAgent = SipConfigServerAgent::getSipConfigServerAgent()->getSipUserAgent() ;
        pAgent->send(message) ;

        pEnv->ReleaseStringUTFChars(jsMessage, szMessage) ;
    }
}
