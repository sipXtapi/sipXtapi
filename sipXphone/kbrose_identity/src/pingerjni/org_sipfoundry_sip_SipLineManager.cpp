// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_sip_SipLineManager.cpp#2 $
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
#include "net/SipLineMgr.h"
#include "net/SipRefreshMgr.h"
#include "net/SipLine.h"
#include "net/SipUserAgent.h"
#include "pinger/Pinger.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"
#include "pingerjni/JavaSUAListenerDispatcher.h"
#if defined(_WIN32)
#include <windows.h>
#define sleep Sleep
#elif defined(__pingtel_on_posix__)
#include <unistd.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* ============================ FUNCTIONS ================================= */

SipLineMgr* getLineManager()
{
   SipLineMgr* pLineManager = NULL ;

   Pinger* pPingerTask = Pinger::getPingerTask() ;
   if (pPingerTask != NULL)
   {
      pLineManager = pPingerTask->getLineManager() ;
      pPingerTask->getRefreshManager();
   }

   return pLineManager ;
}


//copy of surbhi's method in WebUiLinemanagerUtil
void getUriForUrl(Url &userEnteredUrl , Url &uri)
{
   UtlString host;
   userEnteredUrl.getHostAddress(host);
   if ( host.isNull())
   {
      //dynamic IP address
      Pinger* pinger = Pinger::getPingerTask();
      SipUserAgent* UA = pinger->getSipUserAgent();
      if (pinger && UA)
      {
         uri = userEnteredUrl;
         UtlString contact;
         UtlString contactHost;
         UA->getContactUri(&contact);
         Url contactUri(contact);
         contactUri.getHostAddress(contactHost);
         int contactPort = contactUri.getHostPort();
         //set correct value of host and port
         uri.setHostAddress(contactHost);
         uri.setHostPort(contactPort);
      }
   }
   else
   {
      //just get uri from user entered url
      UtlString identityUri;
      userEnteredUrl.getUri(identityUri);
      uri = Url(identityUri);
   }
}

/*
 * Class:     org_sipfoundry_sip_SipLineManager
 * Method:    JNI_createLine
 * Signature: (Ljava/lang/String;Ljava/lang/String;ZZIZ)Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sip_SipLineManager_JNI_1createLine
  (JNIEnv *pEnv, jclass clazz, jstring jsRegisterUrl, jstring jsUser, jboolean bVisible, jboolean bAutoEnable, jint iState, jboolean bUseCallHandling)
{
    jstring jsIdentity = NULL ;

    char* szRegisterUrl = JSTRTOCSTR(pEnv, jsRegisterUrl) ;
    char* szUser = JSTRTOCSTR(pEnv, jsUser) ;

    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipLineManager_JNI_1createLine") ;

    Url urlRegister(szRegisterUrl) ;
    UtlString strUser(szUser) ;

	Url uriRegister;
    getUriForUrl(urlRegister, uriRegister);

	SipLine line(urlRegister,uriRegister, strUser, bVisible, iState, bAutoEnable, bUseCallHandling) ;
    SipLineMgr *pLineManager = getLineManager() ;
    if (pLineManager != NULL)
    {
        pLineManager->addLine(line) ;
		Url urlIdentity = line.getIdentity() ;
        UtlString strIdentity = urlIdentity.toString() ;
        jsIdentity = CSTRTOJSTR(pEnv, strIdentity.data()) ;
    }

    JNI_END_METHOD() ;

    RELEASECSTR(pEnv, jsRegisterUrl, szRegisterUrl) ;
    RELEASECSTR(pEnv, jsUser, szUser) ;

    return jsIdentity ;
}


/*
 * Class:     org_sipfoundry_sip_SipLineManager
 * Method:    JNI_deleteLine
 * Signature: (Ljava/lang/String;I)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_sip_SipLineManager_JNI_1deleteLine
  (JNIEnv *pEnv, jclass clazz, jstring jsIdentity, jint unused)
{
    jboolean bSuccess = false ;

    char* szIdentity = JSTRTOCSTR(pEnv, jsIdentity) ;

    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipLineManager_JNI_1deleteLine") ;

    SipLineMgr *pLineManager = getLineManager() ;
    if ((pLineManager != NULL) && (szIdentity != NULL))
    {
        Url urlIdentity(szIdentity) ;
        pLineManager->deleteLine(urlIdentity) ;
    }

    JNI_END_METHOD() ;

    RELEASECSTR(pEnv, jsIdentity, szIdentity) ;

    return bSuccess ;
}


/*
 * Class:     org_sipfoundry_sip_SipLineManager
 * Method:    JNI_1getLine
 * Signature: (Ljava/lang/String;Ljava/lang/String)Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sip_SipLineManager_JNI_1getLine
  (JNIEnv *pEnv, jclass clazz, jstring jsToUrl, jstring jsLocalContact)
{
    jstring jsResults = NULL ;
    char* szToUrl = JSTRTOCSTR(pEnv, jsToUrl) ;
    char* szLocalContact = JSTRTOCSTR(pEnv, jsLocalContact) ;

    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipLineManager_JNI_1deleteLine") ;

    SipLineMgr *pLineManager = getLineManager() ;
    if ((pLineManager != NULL) && (szToUrl != NULL) && (szLocalContact != NULL))
    {
        UtlString toUrl(szToUrl) ;
        UtlString localContact(szLocalContact) ;
        SipLine line ;

        if (pLineManager->getLine(toUrl, localContact, line))
        {
            Url urlIdentity = line.getIdentity() ;

            UtlString identity = urlIdentity.toString() ;
            jsResults = CSTRTOJSTR(pEnv, identity.data());
        }
    }

    JNI_END_METHOD() ;

    RELEASECSTR(pEnv, jsToUrl, szToUrl) ;
    RELEASECSTR(pEnv, jsLocalContact, szLocalContact) ;

    return jsResults ;
}


/*
 * Class:     org_sipfoundry_sip_SipLineManager
 * Method:    JNI_getLines
 * Signature: ()[[Ljava/lang/Object;
 */
#define MAX_SIP_LINES   32
extern "C"
JNIEXPORT jobjectArray JNICALL Java_org_sipfoundry_sip_SipLineManager_JNI_1getLines
  (JNIEnv *pEnv, jclass clazz)
{
    jobjectArray lines = NULL ;
    jobjectArray identities = NULL ;
    jobjectArray objArray = NULL ;    //for holding array or lines and indentities
    int iActualLines = 0 ;
    int i ;

    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipLineManager_JNI_1getLines") ;

    SipLineMgr *pLineManager = getLineManager() ;
    if (pLineManager != NULL)
    {
        int iMaxLines = pLineManager->getNumLines() ;

        SipLine** apLines = new SipLine*[iMaxLines] ;

        for (i=0; i<iMaxLines; i++)
        {
            apLines[i] = new SipLine() ;
        }

        pLineManager->getLines(iMaxLines, iActualLines, apLines) ;

        jclass classString = pEnv->FindClass("java/lang/String") ;
        if (classString != NULL)
        {
            lines = pEnv->NewObjectArray(iActualLines, classString, NULL) ;
            identities = pEnv->NewObjectArray(iActualLines, classString, NULL) ;

            if (lines != NULL && identities != NULL)
            {
                for (i=0; i<iActualLines; i++)
                {
                    Url lineUrl = apLines[i]->getUserEnteredUrl() ;
                    UtlString strUserEnteredUrl = lineUrl.toString() ;

                    Url urlIdentity = apLines[i]->getIdentity() ;
                    UtlString strIdentity = urlIdentity.toString() ;

//osPrintf("************* JNI getLines strUserEnteredUrl : %s\n", strUserEnteredUrl.data());
//osPrintf("************* JNI getLines identity : %s\n", strIdentity.data());
                    jstring jsUserEnteredUrl = CSTRTOJSTR(pEnv, strUserEnteredUrl.data()) ;
                    jstring jsIdentity = CSTRTOJSTR(pEnv, strIdentity.data()) ;

                    pEnv->SetObjectArrayElement(lines, i, jsUserEnteredUrl) ;
                    pEnv->SetObjectArrayElement(identities, i, jsIdentity) ;
                }
            }
        }

        jclass classObject = pEnv->FindClass("java/lang/Object");
        if (classObject != NULL)
        {
            objArray = pEnv->NewObjectArray(2, pEnv->GetObjectClass(lines), NULL) ;
            if (objArray != NULL)
            {
                pEnv->SetObjectArrayElement(objArray, 0, lines) ;
                pEnv->SetObjectArrayElement(objArray, 1, identities) ;
            }

        }

        // Clean up memory
        for (i=0; i<iMaxLines; i++)
        {
            delete apLines[i] ;
            apLines[i] = NULL ;
        }
        delete[] apLines ;
    }

    JNI_END_METHOD() ;

    return objArray ;
}

/*
 * Class:     org_sipfoundry_sip_SipLineManager
 * Method:    JNI_getDefaultLine
 * Signature: ()[Ljava/lang/String;
 */
extern "C"
JNIEXPORT jobjectArray JNICALL Java_org_sipfoundry_sip_SipLineManager_JNI_1getDefaultLine
  (JNIEnv *pEnv, jclass clazz)
{
    jobjectArray lines = NULL ;
    jstring jsFullDefaultLine = NULL ;
    jstring jsUriDefaultLine = NULL ;

    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipLineManager_JNI_1getDefaultLine") ;

    SipLineMgr *pLineManager = getLineManager() ;
    if (pLineManager != NULL)
    {
        //create two array entries to hold full and just uri part of line
        jclass classString = pEnv->FindClass("java/lang/String") ;
        if (classString != NULL)
        {
            lines = pEnv->NewObjectArray(2, classString, NULL) ;
            if (lines != NULL)
            {
                UtlString strOutboundLine ;

                pLineManager->getDefaultOutboundLine(strOutboundLine) ;
                Url url(strOutboundLine);
                UtlString strUri;

                url.getUri(strUri);

                jsFullDefaultLine = CSTRTOJSTR(pEnv, url.toString().data()) ;
                jsUriDefaultLine = CSTRTOJSTR(pEnv, strUri.data()) ;

                //now fill in the the array elements
                pEnv->SetObjectArrayElement(lines, 0, jsFullDefaultLine) ;
                pEnv->SetObjectArrayElement(lines, 1, jsUriDefaultLine) ;
            }
        }
    }

    JNI_END_METHOD() ;

    return lines ;
}



/*
 * Class:     org_sipfoundry_sip_SipLineManager
 * Method:    JNI_setDefaultLine
 * Signature: (Ljava/lang/String;I)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_sip_SipLineManager_JNI_1setDefaultLine
  (JNIEnv *pEnv, jclass clazz, jstring jsIdentity, jint unused)
{
    jboolean bSuccess = FALSE ;

    char* szIdentity = JSTRTOCSTR(pEnv, jsIdentity) ;

    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipLineManager_JNI_1setDefaultLine") ;

    SipLineMgr *pLineManager = getLineManager() ;
    if ((pLineManager != NULL) && (szIdentity != NULL))
    {
        UtlString strIdentity(szIdentity) ;
        Url      urlIdentity(strIdentity) ;
        pLineManager->setDefaultOutboundLine(urlIdentity) ;
    }
    //CpCallManager pCallMgr = getCallManager();
    //if ((pCallMgr != NULL) && (szIdentity != NULL))
    //{
    //    pCallMgr->setOutboundLine(szIdentity);
    //}

    JNI_END_METHOD() ;

    RELEASECSTR(pEnv, jsIdentity, szIdentity) ;

    return bSuccess ;
}


/*
 * Class:     org_sipfoundry_sip_SipLineManager
 * Method:    JNI_enableLine
 * Signature: (Ljava/lang/String;Z)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_sip_SipLineManager_JNI_1enableLine
  (JNIEnv *pEnv, jclass clazz, jstring jsIdentity, jboolean bEnable)
{
    jboolean bSuccess = FALSE ;

    char* szIdentity = JSTRTOCSTR(pEnv, jsIdentity) ;

    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipLineManager_JNI_1enableLine") ;

    SipLineMgr *pLineManager = getLineManager() ;
    if ((pLineManager != NULL) && (szIdentity != NULL))
    {
        UtlString strIdentity(szIdentity) ;
        Url      urlIdentity(strIdentity) ;

        if (bEnable)
            pLineManager->enableLine(urlIdentity) ;
        else
            pLineManager->disableLine(urlIdentity) ;
    }

    JNI_END_METHOD() ;

    RELEASECSTR(pEnv, jsIdentity, szIdentity) ;

    return bSuccess ;
}


/*
 * Class:     org_sipfoundry_sip_SipLineManager
 * Method:    JNI_getLineState
 * Signature: (Ljava/lang/String;I)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_sip_SipLineManager_JNI_1getLineState
  (JNIEnv *pEnv, jclass clazz, jstring jsIdentity, jint unused)
{
    jint iState = 0 ;

    char* szIdentity = JSTRTOCSTR(pEnv, jsIdentity) ;

    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipLineManager_JNI_1getLineState") ;

    SipLineMgr *pLineManager = getLineManager() ;
    if ((pLineManager != NULL) && (szIdentity != NULL))
    {
        UtlString strIdentity(szIdentity) ;
        Url      urlIdentity(strIdentity) ;

        iState = pLineManager->getStateForLine(urlIdentity) ;
    }

    JNI_END_METHOD() ;

    RELEASECSTR(pEnv, jsIdentity, szIdentity) ;

    return iState ;
}


/*
 * Class:     org_sipfoundry_sip_SipLineManager
 * Method:    JNI_enableAutoEnable
 * Signature: (Ljava/lang/String;Z)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_sip_SipLineManager_JNI_1enableAutoEnable
  (JNIEnv *pEnv, jclass clazz, jstring jsIdentity, jboolean bEnable)
{
    jboolean bSuccess = FALSE ;

    char* szIdentity = JSTRTOCSTR(pEnv, jsIdentity) ;

    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipLineManager_JNI_1enableAutoEnable") ;

    SipLineMgr *pLineManager = getLineManager() ;
    if ((pLineManager != NULL) && (szIdentity != NULL))
    {
        UtlString strIdentity(szIdentity) ;
        Url      urlIdentity(strIdentity) ;

        pLineManager->setAutoEnableForLine(urlIdentity, bEnable) ;
    }


    JNI_END_METHOD() ;

    RELEASECSTR(pEnv, jsIdentity, szIdentity) ;

    return bSuccess ;
}


/*
 * Class:     org_sipfoundry_sip_SipLineManager
 * Method:    JNI_isAutoEnableEnabled
 * Signature: (Ljava/lang/String;I)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_sip_SipLineManager_JNI_1isAutoEnableEnabled
  (JNIEnv *pEnv, jclass clazz, jstring jsIdentity, int unused)
{
    jboolean bEnabled = FALSE ;

    char* szIdentity = JSTRTOCSTR(pEnv, jsIdentity) ;

    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipLineManager_JNI_1isAutoEnableEnabled") ;

    SipLineMgr *pLineManager = getLineManager() ;
    if ((pLineManager != NULL) && (szIdentity != NULL))
    {
        UtlString strIdentity(szIdentity) ;
        Url      urlIdentity(strIdentity) ;

        bEnabled = pLineManager->getEnableForLine(urlIdentity) ;
    }

    JNI_END_METHOD() ;

    RELEASECSTR(pEnv, jsIdentity, szIdentity) ;

    return bEnabled ;
}


/*
 * Class:     org_sipfoundry_sip_SipLineManager
 * Method:    JNI_enableShowLine
 * Signature: (Ljava/lang/String;Z)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_sip_SipLineManager_JNI_1enableShowLine
  (JNIEnv *pEnv, jclass clazz, jstring jsIdentity, jboolean bEnable)
{
    jboolean bSuccess = FALSE ;

    char* szIdentity = JSTRTOCSTR(pEnv, jsIdentity) ;

    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipLineManager_JNI_1enableShowLine") ;

    SipLineMgr *pLineManager = getLineManager() ;
    if ((pLineManager != NULL) && (szIdentity != NULL))
    {
        UtlString strIdentity(szIdentity) ;
        Url      urlIdentity(strIdentity) ;

        pLineManager->setVisibilityForLine(urlIdentity, bEnable) ;
    }

    JNI_END_METHOD() ;

    RELEASECSTR(pEnv, jsIdentity, szIdentity) ;

    return bSuccess ;
}


/*
 * Class:     org_sipfoundry_sip_SipLineManager
 * Method:    JNI_isShowLineEnabled
 * Signature: (Ljava/lang/String;I)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_sip_SipLineManager_JNI_1isShowLineEnabled
(JNIEnv *pEnv, jclass clazz, jstring jsIdentity, int unused)
{
    jboolean bEnabled = FALSE ;

    char* szIdentity = JSTRTOCSTR(pEnv, jsIdentity) ;

    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipLineManager_JNI_1isShowLineEnabled") ;

    SipLineMgr *pLineManager = getLineManager() ;
    if ((pLineManager != NULL) && (szIdentity != NULL))
    {
        UtlString strIdentity(szIdentity) ;
        Url      urlIdentity(strIdentity) ;

        bEnabled = pLineManager->getVisibilityForLine(urlIdentity) ;
    }

    JNI_END_METHOD() ;

    RELEASECSTR(pEnv, jsIdentity, szIdentity) ;

    return bEnabled ;
}


/*
 * Class:     org_sipfoundry_sip_SipLineManager
 * Method:    JNI_enableUseCallHandlingSettings
 * Signature: (Ljava/lang/String;Z)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_sip_SipLineManager_JNI_1enableUseCallHandlingSettings
  (JNIEnv *pEnv, jclass clazz, jstring jsIdentity, jboolean bEnable)
{
    jboolean bSuccess = FALSE ;

    char* szIdentity = JSTRTOCSTR(pEnv, jsIdentity) ;

    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipLineManager_JNI_1enableUseCallHandlingSettings") ;

    SipLineMgr *pLineManager = getLineManager() ;
    if ((pLineManager != NULL) && (szIdentity != NULL))
    {
        UtlString strIdentity(szIdentity) ;
        Url      urlIdentity(strIdentity) ;

        pLineManager->setCallHandlingForLine(urlIdentity, bEnable) ;
    }

    JNI_END_METHOD() ;

    RELEASECSTR(pEnv, jsIdentity, szIdentity) ;

    return bSuccess ;
}


/*
 * Class:     org_sipfoundry_sip_SipLineManager
 * Method:    JNI_isUseCallHandlingSettingsEnabled
 * Signature: (Ljava/lang/String;I)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_sip_SipLineManager_JNI_1isUseCallHandlingSettingsEnabled
  (JNIEnv *pEnv, jclass clazz, jstring jsIdentity, int unused)
{
    jboolean bEnabled = FALSE ;

    char* szIdentity = JSTRTOCSTR(pEnv, jsIdentity) ;

    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipLineManager_JNI_1isUseCallHandlingSettingsEnabled") ;

    SipLineMgr *pLineManager = getLineManager() ;
    if ((pLineManager != NULL) && (szIdentity != NULL))
    {
        UtlString strIdentity(szIdentity) ;
        Url      urlIdentity(strIdentity) ;

        bEnabled = pLineManager->getCallHandlingForLine(urlIdentity) ;
    }

    JNI_END_METHOD() ;

    RELEASECSTR(pEnv, jsIdentity, szIdentity) ;

    return bEnabled ;
}


/*
 * Class:     org_sipfoundry_sip_SipLineManager
 * Method:    JNI_setUser
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_sip_SipLineManager_JNI_1setUser
  (JNIEnv *pEnv, jclass clazz, jstring jsIdentity, jstring jsUser)
{
    jboolean bSuccess = FALSE ;

    char* szIdentity = JSTRTOCSTR(pEnv, jsIdentity) ;
    char* szUser = JSTRTOCSTR(pEnv, jsUser) ;

    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipLineManager_JNI_1setUser") ;

    SipLineMgr *pLineManager = getLineManager() ;
    if ((pLineManager != NULL) && (szIdentity != NULL) && (szUser != NULL))
    {
        UtlString strIdentity(szIdentity) ;
        Url      urlIdentity(strIdentity) ;
        UtlString strUser(szUser) ;

        pLineManager->setUserForLine(urlIdentity, strUser) ;
    }


    JNI_END_METHOD() ;

    RELEASECSTR(pEnv, jsIdentity, szIdentity) ;
    RELEASECSTR(pEnv, jsUser, szUser) ;

    return bSuccess ;
}


/*
 * Class:     org_sipfoundry_sip_SipLineManager
 * Method:    JNI_getUser
 * Signature: (Ljava/lang/String;I)Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sip_SipLineManager_JNI_1getUser
  (JNIEnv *pEnv, jclass clazz, jstring jsIdentity, jint unused)
{
    jstring jsUser = NULL ;
    char* szIdentity = JSTRTOCSTR(pEnv, jsIdentity) ;

    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipLineManager_JNI_1getUser") ;

    SipLineMgr *pLineManager = getLineManager() ;
    if ((pLineManager != NULL) && (szIdentity != NULL))
    {
        UtlString strIdentity(szIdentity) ;
        Url      urlIdentity(strIdentity) ;
        UtlString strUser ;

        pLineManager->getUserForLine(urlIdentity, strUser) ;
        jsUser = CSTRTOJSTR(pEnv, strUser.data()) ;
    }

    JNI_END_METHOD() ;

    RELEASECSTR(pEnv, jsIdentity, szIdentity) ;

    return jsUser ;
}



/*
 * Class:     org_sipfoundry_sip_SipLineManager
 * Method:    JNI_addLineListener
 * Signature: (Lorg/sipfoundry/sip/event/SipLineListener;J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_sip_SipLineManager_JNI_1addLineListener
  (JNIEnv *pEnv, jclass clazz, jobject jobjListener, jlong lHashCode)
{    
    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipLineManager_JNI_1addLineListener") ;

    JavaSUAListenerDispatcher::getInstance()->takeListenerLock() ;

    // Obtain references to the SUA dispatcher and line manager.  We use
    // the SUA dispatcher to receive and redispatch line events.
    JavaSUAListenerDispatcher* pDispatcher = JavaSUAListenerDispatcher::getInstance() ;
    SipLineMgr* pLineManager = getLineManager() ;
    if ((pDispatcher != NULL) && (pLineManager != NULL))
    {
        // NOTE: We need to keep track of the global object that we create along
        // with the hash code of the actual listener.  This information is used
        // later we the listener is removed.
        jobject jobjGlobalRefListener = pEnv->NewGlobalRef(jobjListener) ;
        pDispatcher->addLineJNIListenerMapping((unsigned int) lHashCode, jobjGlobalRefListener) ;

        // Finally, add the observer
        pLineManager->addMessageObserver(*pDispatcher->getMessageQueue(), jobjGlobalRefListener) ;
    }

    JavaSUAListenerDispatcher::getInstance()->releaseListenerLock() ;
    JNI_END_METHOD() ;    
}


/*
 * Class:     org_sipfoundry_sip_SipLineManager
 * Method:    JNI_removeLineListener
 * Signature: (Lorg/sipfoundry/sip/event/SipLineListener;J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_sip_SipLineManager_JNI_1removeLineListener
  (JNIEnv *pEnv, jclass clazz, jobject jobjListener, jlong lHashCode)
{    
    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipLineManager_JNI_1removeLineListener") ;

    JavaSUAListenerDispatcher::getInstance()->takeListenerLock() ;

    // Obtain references to the SUA dispatcher and line manager.  We use
    // the SUA dispatcher to receive and redispatch line events.
    JavaSUAListenerDispatcher* pDispatcher = JavaSUAListenerDispatcher::getInstance() ;
    SipLineMgr* pLineManager = getLineManager() ;
    if ((pDispatcher != NULL) && (pLineManager != NULL))
    {
        jobject jobjGlobalRefListener ;
        jobjGlobalRefListener = pDispatcher->getLineJNIListenerMapping((unsigned int) lHashCode) ;
        if (jobjGlobalRefListener != NULL)
        {
            if (pLineManager->removeMessageObserver(*(JavaSUAListenerDispatcher::getInstance()->getMessageQueue()), jobjGlobalRefListener))
            {
                pDispatcher->removeLineJNIListenerMapping((unsigned int) lHashCode) ;
                pEnv->DeleteGlobalRef(jobjGlobalRefListener) ;
            }
            else
            {
               API_FAILURE(-1) ;
            }
        }
        else
        {
           API_FAILURE(-1) ;
        }
    }

    JavaSUAListenerDispatcher::getInstance()->releaseListenerLock() ;

    JNI_END_METHOD() ;

}

/*
 * Class:     org_sipfoundry_sip_SipLineManager
 * Method:    JNI_1getCredentials
 * Signature: ()[Ljava/lang/String;
 */
#if 0
extern "C"
JNIEXPORT jobjectArray JNICALL Java_org_sipfoundry_sip_SipLineManager_JNI_1getCredentials
  (JNIEnv *pEnv, jclass clazz, jstring jsIdentity)
{
	JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipLineManager_JNI_1getCredentials") ;
    jobjectArray credentials = NULL ;
	char* szIdentity = JSTRTOCSTR(pEnv, jsIdentity) ;

	SipLineMgr *pLineManager = getLineManager() ;
	if ((pLineManager != NULL) && (szIdentity != NULL))
	{

		Url urlIdentity(szIdentity) ;
		int noOfCredentials = pLineManager->getNumOfCredentialsForLine(urlIdentity);

		UtlString *realm = new UtlString[noOfCredentials];
		UtlString *type = new UtlString[noOfCredentials];
		UtlString *userId = new UtlString[noOfCredentials];
		UtlString *passToken = new UtlString[noOfCredentials];
		int retVal = 0;
		int i = 0;

		jclass classString = pEnv->FindClass("java/lang/String") ;
		if (classString != NULL)
		{
			credentials = pEnv->NewObjectArray(4, classString, NULL) ;

			pLineManager->getCredentialListForLine
				(urlIdentity, noOfCredentials , retVal, realm, userId, type, passToken);
			if (i < retVal){

				pEnv->SetObjectArrayElement(credentials, 0, CSTRTOJSTR(pEnv, realm[0].data())  ) ;
				pEnv->SetObjectArrayElement(credentials, 1, CSTRTOJSTR(pEnv, userId[0].data())  ) ;
				pEnv->SetObjectArrayElement(credentials, 2, CSTRTOJSTR(pEnv, type[0].data() )  ) ;
				pEnv->SetObjectArrayElement(credentials, 3, CSTRTOJSTR(pEnv, passToken[0].data())  ) ;
			}

		}
	}
    RELEASECSTR(pEnv, jsIdentity, szIdentity) ;
	JNI_END_METHOD() ;
	return credentials ;
}
#endif

/*
 * Class:     org_sipfoundry_sip_SipLineManager
 * Method:    JNI_1getRealms
 * Signature: ()[Ljava/lang/String;
 */
extern "C"
JNIEXPORT jobjectArray JNICALL Java_org_sipfoundry_sip_SipLineManager_JNI_1getRealms
  (JNIEnv *pEnv, jclass clazz, jstring jsIdentity)
{
	JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipLineManager_JNI_1getRealms") ;
    jobjectArray realmsArray = NULL ;
	char* szIdentity = JSTRTOCSTR(pEnv, jsIdentity) ;

	SipLineMgr *pLineManager = getLineManager() ;
	if ((pLineManager != NULL) && (szIdentity != NULL))
	{

		Url urlIdentity(szIdentity) ;
		int noOfCredentials = pLineManager->getNumOfCredentialsForLine(urlIdentity);

		UtlString *realm = new UtlString[noOfCredentials];
		UtlString *type = new UtlString[noOfCredentials];
		UtlString *userId = new UtlString[noOfCredentials];
		UtlString *passToken = new UtlString[noOfCredentials];
		int retVal = 0;
		int i = 0;


		jclass classString = pEnv->FindClass("java/lang/String") ;
		if (classString != NULL)
		{
			realmsArray = pEnv->NewObjectArray(noOfCredentials, classString, NULL) ;

			pLineManager->getCredentialListForLine
				(urlIdentity, noOfCredentials , retVal, realm, userId, type, passToken);
			if( retVal > 0 ){
				while(i < noOfCredentials){
					pEnv->SetObjectArrayElement(realmsArray, i, CSTRTOJSTR(pEnv, realm[i].data())  ) ;
					i++;
				}
			}
		}
	}
    RELEASECSTR(pEnv, jsIdentity, szIdentity) ;
	JNI_END_METHOD() ;
	return realmsArray ;
}



/*
 * Class:     org_sipfoundry_sip_SipLineManager
 * Method:    JNI_removeRealm
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_sip_SipLineManager_JNI_1removeRealm
  (JNIEnv *pEnv, jclass clazz, jstring jsIdentity, jstring jsRealm)
{
    UtlBoolean bSuccess = FALSE ;

    char* szIdentity = JSTRTOCSTR(pEnv, jsIdentity) ;
	char* szRealm    = JSTRTOCSTR(pEnv, jsRealm) ;

    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipLineManager_JNI_1removeRealm") ;

    SipLineMgr *pLineManager = getLineManager() ;
    if ((pLineManager != NULL) && (szIdentity != NULL) && (szRealm != NULL))
    {
        UtlString strIdentity(szIdentity) ;
        Url      urlIdentity(strIdentity) ;
        UtlString strRealm(szRealm) ;


        bSuccess = pLineManager->deleteCredentialForLine(urlIdentity, strRealm) ;
	}

    JNI_END_METHOD() ;

    RELEASECSTR(pEnv, jsIdentity, szIdentity) ;
	RELEASECSTR(pEnv, jsRealm, szRealm) ;
	return (jboolean) bSuccess;

}



/*
 * Class:     org_sipfoundry_sip_SipLineManager
 * Method:    JNI_addRealm
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_sip_SipLineManager_JNI_1addRealm
  (JNIEnv *pEnv, jclass clazz, jstring jsIdentity, jstring jsRealm, jstring jsUserid, jstring jsPassword)
{
    UtlBoolean bSuccess = FALSE ;

    char* szIdentity    = JSTRTOCSTR(pEnv, jsIdentity) ;
	char* szRealm       = JSTRTOCSTR(pEnv, jsRealm) ;
	char* szUserid      = JSTRTOCSTR(pEnv, jsUserid) ;
	char* szPassword    = JSTRTOCSTR(pEnv, jsPassword) ;


    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipLineManager_JNI_1addRealm") ;

    SipLineMgr *pLineManager = getLineManager() ;
    if (   (pLineManager != NULL) && (szIdentity != NULL)
		&& (szRealm      != NULL) && (szUserid != NULL)
		&& (szPassword   != NULL) )
    {
        UtlString strIdentity(szIdentity) ;
        Url      urlIdentity(strIdentity) ;
        UtlString strRealm   (szRealm) ;
        UtlString strUserid  (szUserid) ;
        UtlString strPassword(szPassword) ;

		UtlString passwordToken;
		HttpMessage::buildMd5UserPasswordDigest(strUserid, strRealm, strPassword, passwordToken);
        bSuccess = pLineManager->addCredentialForLine(urlIdentity, strRealm, strUserid, passwordToken,
                                                      HTTP_DIGEST_AUTHENTICATION) ;
	}

    JNI_END_METHOD() ;

    RELEASECSTR(pEnv, jsIdentity, szIdentity) ;
	RELEASECSTR(pEnv, jsRealm,    szRealm) ;
	RELEASECSTR(pEnv, jsUserid,   szUserid) ;
	RELEASECSTR(pEnv, jsPassword, szPassword) ;

	return (jboolean) bSuccess;

}

