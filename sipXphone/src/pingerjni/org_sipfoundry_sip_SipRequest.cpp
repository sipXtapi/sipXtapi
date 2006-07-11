// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_sip_SipRequest.cpp#2 $
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
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"
#include "pingerjni/JavaSUAListenerDispatcher.h"
#include "net/SipUserAgent.h"
#include "net/SipMessage.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* ============================ FUNCTIONS ================================= */

/*
 * Class:     org_sipfoundry_sip_SipRequest
 * Method:    JNI_buildResponse
 * Signature: (Ljava/lang/String;ILjava/lang/String;)Lorg/sipfoundry/sip/SipResponse;
 */
extern "C"
JNIEXPORT jobject JNICALL Java_org_sipfoundry_sip_SipRequest_JNI_1buildResponse
  (JNIEnv *pEnv, jclass clazz, jstring jsRequestText, jint iResponseCode, jstring jsResponseText)
{
    jobject jobjResponse = NULL ;

    // Convert Data: Java --> C
    char* szRequestText = (jsRequestText) ? (char *) pEnv->GetStringUTFChars(jsRequestText, 0) : NULL ;
    char* szResponseText = (jsResponseText) ? (char *) pEnv->GetStringUTFChars(jsResponseText, 0) : NULL ;

    // Build request
    if (szRequestText != NULL) {
        SipMessage response ; 

        // 1. Convert into a SipMessage
        SipMessage message((const char*) szRequestText) ;
        
        // 2. Massage into a response and pull out message text
        response.setResponseData(&message, iResponseCode, szResponseText) ;
        UtlString contents ;
        int iLen ;
        response.getBytes(&contents, &iLen) ;

        // 3. Leverage JavaSUAListenerDispatcher, and build the response object
        JavaSUAListenerDispatcher* pJavaSUA = JavaSUAListenerDispatcher::getInstance() ;
        jobjResponse = (jobject) pJavaSUA->createSipResponseObject(contents.data(), pEnv) ;
    }

    // Clean up
    if (szRequestText != NULL)
        pEnv->ReleaseStringUTFChars(jsRequestText, szRequestText) ; 

    if (szResponseText != NULL)
        pEnv->ReleaseStringUTFChars(jsResponseText, szResponseText) ; 

    return jobjResponse ;
}
