// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/JavaSUAListenerDispatcher.cpp#3 $
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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#   include <io.h>
#endif
#include <sys/stat.h>
#include <fcntl.h> 
#include <jni.h>

// APPLICATION INCLUDES
#include "pingerjni/JavaSUAListenerDispatcher.h"

#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"

#include "tao/TaoString.h"
#include "pinger/Pinger.h"
#include "net/SipConfigServerAgent.h"
#include "net/SipSession.h"

/////////////////////////////////////////////////////////////////////////////////
//The code maked as CODE_BREAKS_SDS_BUILD for some reason will cause unresolved
//link errors when the config server trys to load this .so from java.
//We will leave this out until sipxchange 1.1 is finished, then remove the 
//ifdef and find out why it's not working.  (DWW, DLH)
/////////////////////////////////////////////////////////////////////////////////
#ifdef CODE_BREAKS_SDS_BUILD
#include "net/SipRefreshMgr.h"
#endif


#include "net/SipUserAgent.h"
#include "net/SipMessageEvent.h"
#include "net/SipLine.h"
#include "net/SipLineEvent.h"
#include "net/NameValueTokenizer.h"
#include "net/NetAttributeTokenizer.h"
#include "net/Url.h"
#include "cp/CallManager.h"
#include "os/OsConnectionSocket.h"
#include "os/OsDateTime.h"
#include "os/OsTask.h"
#include "os/OsLock.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#undef VERBOSE_DEBUG

// STATIC VARIABLE INITIALIZATIONS
JavaSUAListenerDispatcher* JavaSUAListenerDispatcher::m_spInstance = 0 ;
OsBSem JavaSUAListenerDispatcher::m_sLock(OsBSem::Q_FIFO, OsBSem::FULL);
UtlString JavaSUAListenerDispatcher::m_sTaskName = "JavaSUADisp" ;
OsMutex	JavaSUAListenerDispatcher::m_sMutexListeners(OsMutex::Q_FIFO) ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */


/* ============================ CREATORS ================================== */

// Constructor
JavaSUAListenerDispatcher::JavaSUAListenerDispatcher()
    : OsServerTask(m_sTaskName)
    , m_htSipListeners()
    , m_htLineListeners()
{
}

// Copy constructor
JavaSUAListenerDispatcher::JavaSUAListenerDispatcher(const JavaSUAListenerDispatcher& rJavaSUAListenerDispatcher)
{
}


// Singleton accessor method
JavaSUAListenerDispatcher* JavaSUAListenerDispatcher::getInstance()
{
    // Guard creation to avoid creating multiple instance
    m_sLock.acquire() ;

    // If we don't have an instance, create and start it.
    if (m_spInstance == NULL) {
        m_spInstance = new JavaSUAListenerDispatcher() ;
        UtlBoolean bStarted = m_spInstance->start() ;

        // Complain if we don't start
        assert(bStarted) ;
    }

    // Release guard when safe.
    m_sLock.release() ;
    
    return m_spInstance ;
}

// Destructor
JavaSUAListenerDispatcher::~JavaSUAListenerDispatcher()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
JavaSUAListenerDispatcher&
JavaSUAListenerDispatcher::operator=(const JavaSUAListenerDispatcher& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


// Take the lock protected listener list manipulation
void JavaSUAListenerDispatcher::takeListenerLock()
{
    m_sMutexListeners.acquire() ;
}


// Release the lock protected listener list manipulation
void JavaSUAListenerDispatcher::releaseListenerLock()
{
    m_sMutexListeners.release() ;
}
 

/*
 * This method gets called by handleMessage method if the 
 * message type is TAO MSG.
 * It calls the method in ToneListener in the Java layer.
*/                                         
UtlBoolean JavaSUAListenerDispatcher::processDtmfDigit(const char* szCallId, 
                                                      const char* szConnection,
                                                      const int   iDigit,
                                                      const int   iDuration)
{
   //osPrintf("---------------------------------------------------------------------------\n");
   //osPrintf("Received remote Dtmf Press: %d (duration=%d) %s %s\n", iDigit, iDuration, szCallId, szConnection) ;
   //osPrintf("----------------------------------------------------------------------------\n");
   
    JavaVM*   pVM ;
    JNIEnv*   pEnv ;
        
    JNI_BEGIN_METHOD("JavaSUADisp: processDtmfDigit") ;

    if (jniGetVMReference(&pVM, &pEnv)) {
		if (pEnv != NULL) {
			jclass classToneListener = pEnv->FindClass(TONE_LISTENER_CLASS) ;
			if (classToneListener != NULL) {
				jmethodID methodid = 
						pEnv->GetMethodID(classToneListener, TONE_LISTENER_METHOD , 
												   TONE_LISTENER_METHOD_SIGNATURE) ;
				jobject jobjToneListener = getToneListener(pEnv);       
				
				
				if (methodid != NULL) {
					jobject jobjStringCallID     = pEnv->NewStringUTF(szCallId) ;
					jobject jobjStringConnection = pEnv->NewStringUTF(szConnection) ;
                
					if( jobjStringCallID != NULL && jobjStringConnection != NULL ){
						pEnv->CallVoidMethod(jobjToneListener, methodid, jobjStringCallID, 
											 jobjStringConnection, iDigit, iDuration) ;
						if (pEnv->ExceptionOccurred()) 
						{
							pEnv->ExceptionDescribe() ;
							pEnv->ExceptionClear() ;
						}
					}
				} else
					LOOKUP_FAILURE() ;
			} else
				LOOKUP_FAILURE() ;
		}
		jniReleaseVMReference(pVM) ;
    }

    JNI_END_METHOD() ;

    return TRUE ;
}


// Handle Incoming messages.
UtlBoolean JavaSUAListenerDispatcher::handleMessage(OsMsg& eventMessage)
{   
   int msgType = eventMessage.getMsgType();
   int msgSubType = eventMessage.getMsgSubType();
   UtlBoolean bRC = TRUE ;

   takeListenerLock() ;

   if (msgType == OsMsg::TAO_MSG &&
      msgSubType == TaoMessage::EVENT)
   {
      TaoMessage* pMessage = (TaoMessage*) (&eventMessage) ;

      TaoString arg = TaoString(pMessage->getArgList(), TAOMESSAGE_DELIMITER);
      UtlString os1 = arg[0] ; // Call-Id
      UtlString os2 = arg[1];  // Coded tone info
      UtlString os3 = arg[2];  // Connection 

      unsigned int toneInfo = atoi(os2.data()) ;

      int iButton    = (toneInfo >> 16) ;
      int iDuration  = (toneInfo & 0xFFFF) ;
      int isButtonUp = (toneInfo & 0x80000000);

      if( isButtonUp )
      {
         //bitwise "and" it with 0x0003FFF
         //basicaly the opposite of hat you do
         //inside MpdPtAVT::signalKeyUp(MpBufPtr pPacket)
         iButton = iButton &  0x0003FFF;
      }      
      bRC = processDtmfDigit(os1.data(), os3.data(), iButton, iDuration) ;          
   }    
   else if(msgType == OsMsg::PHONE_APP &&
         msgSubType == CallManager::CP_SIP_MESSAGE)
   {
		SipMessage* sipMessage = (SipMessage*) ((SipMessageEvent&)eventMessage).getMessage() ;
		int   messageStatus = ((SipMessageEvent&)eventMessage).getMessageStatus() ;
        void *pKey = (void *)sipMessage->getResponseListenerData() ;
        UtlInt Key((unsigned int)pKey);

        //check that the listener is contained in the list before trying to handle it
        UtlInt* pVal = (UtlInt*)m_htSipListeners.findValue(&Key);
#ifdef VERBOSE_DEBUG
        osPrintf("JavaSUA: Process message for key %08X, ref=%08X\n",
                pKey, (pVal == NULL) ? NULL :  pVal->getValue()) ;

        UtlString msgBytes ;
        int iLength ;
        sipMessage->getBytes(&msgBytes, &iLength) ;
        osPrintf("JavaSUA Msg:\n%s\n", msgBytes.data()) ;
#endif        
        if (pVal)
        {
            UtlString method;
            void *vpListener = (void *)pVal->getValue();
            
		    switch (messageStatus) 
            {
                case SipMessageEvent::APPLICATION:
                    if (vpListener != NULL) 
                    {
                        dispatchSipMessage(sipMessage, vpListener) ;
                    }
                    break ;
                case SipMessageEvent::TRANSPORT_ERROR:
                case SipMessageEvent::AUTHENTICATION_RETRY:
                case SipMessageEvent::SESSION_REINVITE_TIMER:
                    // osPrintf("%s: received unhandled message of status %d\n", m_sTaskName.data(), messageStatus) ;
                    break ;
                default:
                    // osPrintf("%s: received message of status UNKNOWN\n", m_sTaskName.data()) ;
                    break ;
            }
        }
    } 
    else if (msgType == OsMsg::LINE_MGR_MSG)
    {
        SipLineEvent* pLineEvent = (SipLineEvent*) &eventMessage ;
        dispatchLineEvent(pLineEvent) ;
    }

    releaseListenerLock() ;

    return bRC ;    
}


void JavaSUAListenerDispatcher::addSipJNIListenerMapping(int iKey, jobject jobjValue)
{
    UtlInt* pDictKey = new UtlInt(iKey) ;
    UtlInt* pDictValue = new UtlInt((unsigned int) jobjValue) ;

    // Make sure this listener wasn't already added
    assert(getSipJNIListenerMapping(iKey) == NULL) ;

    m_htSipListeners.insertKeyAndValue(pDictKey, pDictValue) ;

    // Make sure the add worked
    assert(getSipJNIListenerMapping(iKey) != NULL) ;


#ifdef VERBOSE_DEBUG
    osPrintf("JavaSUA: Added binding for hashcode=%08X, globalref=%08X\n", iKey, jobjValue) ;
#endif
}


void JavaSUAListenerDispatcher::removeSipJNIListenerMapping(int iKey)
{    
    UtlInt key(iKey) ;

    // Make sure the mapping exists
    assert(getSipJNIListenerMapping(iKey) != NULL) ;

    m_htSipListeners.destroy(&key) ;

    // Make sure the remove worked
    assert(getSipJNIListenerMapping(iKey) == NULL) ;

#ifdef VERBOSE_DEBUG
    osPrintf("JavaSUA: Removing binding for hascode=%08X\n", iKey) ;
#endif
}

void JavaSUAListenerDispatcher::addLineJNIListenerMapping(int iKey, jobject jobjValue)
{    
    UtlInt* pDictKey = new UtlInt(iKey) ;
    UtlInt* pDictValue = new UtlInt((unsigned int) jobjValue) ;   

    // Make sure this listener wasn't already added
    assert(getLineJNIListenerMapping(iKey) == NULL) ;

    m_htLineListeners.insertKeyAndValue(pDictKey, pDictValue) ;       

    // Make sure the add worked
    assert(getLineJNIListenerMapping(iKey) != NULL) ;
}


void JavaSUAListenerDispatcher::removeLineJNIListenerMapping(int iKey)
{
    UtlInt key(iKey) ;

    // Make sure the mapping exists
    assert(getLineJNIListenerMapping(iKey) != NULL) ;

    m_htLineListeners.destroy(&key) ;

    // Make sure the remove worked
    assert(getLineJNIListenerMapping(iKey) == NULL) ;
}


void* 
JavaSUAListenerDispatcher::createSipRequestObject(const char* szText, void* pEnv)
{
    void* pRC = NULL ;
    JNIEnv* pJEnv = (JNIEnv*) pEnv ;

    if (pJEnv != NULL) 
    {
	    jclass classMessage = pJEnv->FindClass(SIP_REQUEST_CLASS) ;
	    if (classMessage != NULL) 
        {
#ifdef VERBOSE_DEBUG
            osPrintf("createSipRequestObject: pEnv=%08X, classMessage=%08X\n",
                    pJEnv, classMessage) ;
#endif
		    jmethodID cid = pJEnv->GetMethodID(classMessage, SIP_REQUEST_CONST_METHOD, SIP_REQUEST_CONST_SIGNATURE) ;
            if (cid != NULL) 
            {
                jobject jobjString = pJEnv->NewStringUTF(szText) ;
#ifdef VERBOSE_DEBUG
                osPrintf("createSipRequestObject: cid=%08X, jobjString=%08X\n",
                    cid, jobjString) ;
#endif
                if (jobjString != NULL) 
                {
                    jobject objEvent = pJEnv->NewObject(classMessage, cid, jobjString) ;
                    if (objEvent != NULL) 
                    {
                        pRC = objEvent ;
                    } 
                    else 
                        LOOKUP_FAILURE() ;
                } 
                else 
                    LOOKUP_FAILURE() ;
            } 
            else
                LOOKUP_FAILURE() ;
        } 
        else
            LOOKUP_FAILURE() ;
    }

    return pRC ;
}


void* JavaSUAListenerDispatcher::createSipResponseObject(const char* szText, void* pEnv)
{
    void* pRC = NULL ;
    JNIEnv* pJEnv = (JNIEnv*) pEnv ;

    if (pJEnv != NULL) 
    {
	    jclass classMessage = pJEnv->FindClass(SIP_RESPONSE_CLASS) ;
	    if (classMessage != NULL) 
        {
#ifdef VERBOSE_DEBUG
            osPrintf("createSipResponseObject: pEnv=%08X, classMessage=%08X\n",
                    pJEnv, classMessage) ;
#endif
		    jmethodID cid = pJEnv->GetMethodID(classMessage, SIP_RESPONSE_CONST_METHOD, SIP_RESPONSE_CONST_SIGNATURE) ;
            if (cid != NULL) 
            {
                jobject jobjString = pJEnv->NewStringUTF(szText) ;

#ifdef VERBOSE_DEBUG
                osPrintf("createSipResponseObject: cid=%08X, jobjString=%08X\n",
                        cid, jobjString) ;
#endif

                if (jobjString != NULL) 
                {
                    jobject objMessage = pJEnv->NewObject(classMessage, cid, jobjString) ;
                    if (objMessage != NULL) 
                    {
                        pRC = objMessage ;
                    } 
                    else 
                        LOOKUP_FAILURE() ;
                } 
                else 
                    LOOKUP_FAILURE() ;
            } 
            else
                LOOKUP_FAILURE() ;
        } 
        else
            LOOKUP_FAILURE() ;
    }

    return pRC ;
}



/* ============================ ACCESSORS ================================= */

// Get the mapping of one jobject from an integer key
jobject JavaSUAListenerDispatcher::getSipJNIListenerMapping(int iKey)
{
    jobject jobjValue = NULL ;

    UtlInt  key(iKey) ;
    UtlInt* pDictValue = (UtlInt*) m_htSipListeners.findValue(&key) ;
    if (pDictValue != NULL)
    {
         jobjValue = (jobject) pDictValue->getValue() ;
    }
    else
    {
        osPrintf("JavaSUA: Unable to find SIP listener for key %d\n", iKey) ;
    }

#ifdef VERBOSE_DEBUG
    osPrintf("JavaSUA: Search for hashcode=%08X resulted in global ref %08X\n", iKey, jobjValue) ;
#endif

    return jobjValue ;
}


// Get the mapping of one jobject from an integer key
jobject JavaSUAListenerDispatcher::getLineJNIListenerMapping(int iKey)
{
    jobject jobjValue = NULL ;

    UtlInt key(iKey) ;
    UtlInt* pDictValue = (UtlInt*) m_htLineListeners.findValue(&key) ;
    if (pDictValue != NULL)
    {
         jobjValue = (jobject) pDictValue->getValue() ;
    }
    else
    {
        osPrintf("JavaSUA: Unable to find line listener for key %d\n", iKey) ;
    }    

    return jobjValue ;
}

/*
 * Gets a singleton static instance of ToneListener java object. 
 */
jobject JavaSUAListenerDispatcher::getToneListener(void* pEnv)
{
	JNIEnv* pJEnv = (JNIEnv*) pEnv ;
	
	//making it static because we only want one instance of
	//tone listener.
	static jobject listenerObject = NULL;

    if( listenerObject == NULL )
    {
		if (pJEnv != NULL) 
        {
			jclass classListener = pJEnv->FindClass(TONE_LISTENER_CLASS) ;
			if (classListener != NULL) 
            {
				jmethodID methodid = pJEnv->GetMethodID(classListener, TONE_LISTENER_CONST , 
															TONE_LISTENER_CONST_SIGNATURE) ;
				listenerObject = pJEnv->NewObject(classListener, methodid) ;
				// make it a global ref as we do not want this instance to
				// be garbage collected by Java.
				listenerObject = pJEnv->NewGlobalRef(listenerObject);  
			}
		}
	}

	return listenerObject;
}
  

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


UtlBoolean
JavaSUAListenerDispatcher::dispatchSipMessage(const SipMessage* pMessage,
                                              const void*       pListener)
{
    JavaVM*   pVM = NULL ;
    JNIEnv*   pEnv = NULL ;
    UtlBoolean bSuccess = false ;

    int iPriority = jniGetCurrentPriority() ; 

    JNI_BEGIN_METHOD("JavaSUADisp: dispatchMessage") ;

    if (jniGetVMReference(&pVM, &pEnv)) 
    {
        jobject jobjListener = (jobject) pListener ;        
        jclass clazz = pEnv->GetObjectClass(jobjListener) ;
        if (clazz != NULL) 
        {
            jmethodID mid = pEnv->GetMethodID(clazz, NEW_MESSAGE_LISTENER_METHOD, NEW_MESSAGE_LISTENER_SIGNATURE) ;
            if (mid != NULL) 
            {
#ifdef VERBOSE_DEBUG
                osPrintf("dispatchSipMessage: pEnv=%08X, pVM=%08X, clazz=%08X, mid=%08X\n",
                        pEnv, pVM, clazz, mid) ;
                osPrintf("dispatchSipMessage: listener=%08X msg=%08X\n",
                        jobjListener, pMessage) ;
#endif
			    jobject objEvent = (jobject) createSipEventObject(pMessage, pEnv) ;
				if (objEvent != NULL) 
                {
				    pEnv->CallVoidMethod(jobjListener, mid, objEvent) ;
					if (pEnv->ExceptionOccurred())
                    {
					    pEnv->ExceptionDescribe() ;
						pEnv->ExceptionClear() ;
                    } 
                    else
                    {
                        bSuccess = true ;
                    }
                    pEnv->DeleteLocalRef(objEvent) ;
                }
            }
            else
            {
                LOOKUP_FAILURE() ;
            }

            pEnv->DeleteLocalRef(clazz) ;
        }
        jniReleaseVMReference(pVM) ;			
    }
    jniResetPriority(iPriority) ;

    JNI_END_METHOD() ;    

    return bSuccess ;
}


void* 
JavaSUAListenerDispatcher::createSipEventObject(const SipMessage* pMessage, void *pEnv)
{
    void* pRC = NULL ;
    JNIEnv* pJEnv = (JNIEnv*) pEnv ;

    if (pJEnv != NULL)
    {
	    jclass classEvent = pJEnv->FindClass(NEW_SIP_EVENT_CLASS) ;
	    if (classEvent != NULL) 
        {
		    jmethodID cid = pJEnv->GetMethodID(classEvent, 
                    NEW_SIP_EVENT_CONST_METHOD, 
                    NEW_SIP_EVENT_CONST_SIGNATURE) ;
            if (cid != NULL) 
            {
                UtlBoolean bFinalReponse = false ;
                jobject jobjNewMessage = NULL ;

                // Build the SipRequest or SipReponse Mesage
                UtlString contents ;
                int iLen ;
                pMessage->getBytes(&contents, &iLen) ;

                if (pMessage->isResponse()) 
                {
                    int iStatusCode = pMessage->getResponseStatusCode() ;
                    if ((((iStatusCode >= SIP_2XX_CLASS_CODE) && 
                            (iStatusCode < SIP_2XX_CLASS_CODE)) || 
                            (iStatusCode >= SIP_4XX_CLASS_CODE))) 
                    {
                        bFinalReponse = true ;
                    }

#ifdef VERBOSE_DEBUG
                    osPrintf("createSipEventObject (r): pEnv=%08X, classEvent=%08X, cid=%08X\n",
                            pEnv, classEvent, cid) ;
                    osPrintf("createSipEventObject (r): msgLen=%d, statusCode=%d, finalResponse=%d\n",
                        iLen, iStatusCode, bFinalReponse) ;
#endif

                    jobjNewMessage = (jobject) createSipResponseObject(contents.data(), pEnv) ;
                } 
                else 
                {
#ifdef VERBOSE_DEBUG
                    osPrintf("createSipEventObject (!r): pEnv=%08X, classEvent=%08X, cid=%08X\n",
                            pEnv, classEvent, cid) ;
                    osPrintf("createSipEventObject (!r): msgLen=%d\n", iLen) ;
#endif
                    jobjNewMessage = (jobject) createSipRequestObject(contents.data(), pEnv) ;
                }
                
                if (jobjNewMessage != NULL) 
                {
#ifdef VERBOSE_DEBUG
                    osPrintf("createSipEventObject: jobjNewMessage=%08X\n", jobjNewMessage) ;
#endif
                    jobject objEvent = pJEnv->NewObject(classEvent, cid, jobjNewMessage, bFinalReponse) ;
                    if (objEvent != NULL) 
                    {
                        pRC = objEvent ;
                    }
                }
            } 
            else
               LOOKUP_FAILURE() ;
        } 
        else
            LOOKUP_FAILURE() ;
    }

    return pRC ;
}



UtlBoolean JavaSUAListenerDispatcher::dispatchLineEvent(SipLineEvent* pEvent)
{
    JavaVM*   pVM ;
    JNIEnv*   pEnv ;
    UtlBoolean bSuccess = false ;
        
    int iPriority = jniGetCurrentPriority() ; 

    if (jniGetVMReference(&pVM, &pEnv)) 
    {
        jobject jobjListener = (jobject) pEvent->getObserverData() ;
        if (jobjListener != NULL)
        {
            jclass clazz = pEnv->GetObjectClass(jobjListener) ;
            if (clazz != NULL) 
            {
                jmethodID mid = NULL ;

                switch (pEvent->getMessageType())
                {
                    case SipLineEvent::SIP_LINE_EVENT_SUCCESS:
                        mid = pEnv->GetMethodID(clazz, SIP_LINE_LISTENER_METHOD_ENABLED, SIP_LINE_LISTENER_SIGNATURE) ;
                        break ;
                    case SipLineEvent::SIP_LINE_EVENT_FAILED:
                        mid = pEnv->GetMethodID(clazz, SIP_LINE_LISTENER_METHOD_FAILED, SIP_LINE_LISTENER_SIGNATURE) ;
                        break ;
                    case SipLineEvent::SIP_LINE_EVENT_NO_RESPONSE:
                        mid = pEnv->GetMethodID(clazz, SIP_LINE_LISTENER_METHOD_TIMEOUT, SIP_LINE_LISTENER_SIGNATURE) ;
                        break ;
                    case SipLineEvent::SIP_LINE_EVENT_LINE_ADDED:
                        mid = pEnv->GetMethodID(clazz, SIP_LINE_LISTENER_METHOD_ADDED, SIP_LINE_LISTENER_SIGNATURE) ;
                        break ;
                    case SipLineEvent::SIP_LINE_EVENT_LINE_DELETED:
                        mid = pEnv->GetMethodID(clazz, SIP_LINE_LISTENER_METHOD_DELETED, SIP_LINE_LISTENER_SIGNATURE) ;
                        break ;
                    case SipLineEvent::SIP_LINE_EVENT_LINE_CHANGED:
                        mid = pEnv->GetMethodID(clazz, SIP_LINE_LISTENER_METHOD_CHANGED, SIP_LINE_LISTENER_SIGNATURE) ;
                        break ;
                    case SipLineEvent::SIP_LINE_EVENT_OUTBOUND_CHANGED:
                        mid = pEnv->GetMethodID(clazz, SIP_LINE_LISTENER_METHOD_OUTBOUND, SIP_LINE_LISTENER_SIGNATURE) ;
                        break ;
                }
                
                if (mid != NULL) 
                {
			         jobject objEvent = (jobject) createSipLineEventObject(*pEvent, pEnv) ;
				      if (objEvent != NULL) 
                  {
					      pEnv->CallVoidMethod(jobjListener, mid, objEvent) ;
					      if (pEnv->ExceptionOccurred()) 
                     {
						      pEnv->ExceptionDescribe() ;
						      pEnv->ExceptionClear() ;
                     } 
                     else
                     {
                        bSuccess = true ;
                     }
                     pEnv->DeleteLocalRef(objEvent) ;

                  }
               }   
            }
        }
        jniReleaseVMReference(pVM) ;			
    }
    jniResetPriority(iPriority) ;   

    return bSuccess ;
}


void* JavaSUAListenerDispatcher::createSipLineObject(const char* szIdentity, const char* szDisplayName, void* pEnv)
{
    void* jobjRC = NULL ;
    JNIEnv* pJEnv = (JNIEnv*) pEnv ;

    if (pJEnv != NULL) {
	    jclass classSipLine = pJEnv->FindClass(SIP_LINE_CLASS) ;
	    if (classSipLine != NULL) {
		    jmethodID cid = pJEnv->GetMethodID(classSipLine, SIP_LINE_CONST_METHOD, SIP_LINE_CONST_SIGNATURE) ;
            if (cid != NULL) {
                jobject jobjStrIdentity = pJEnv->NewStringUTF(szIdentity) ;
                jobject jobjStrDisplayName = pJEnv->NewStringUTF(szDisplayName) ;
                if ((jobjStrIdentity != NULL) && (jobjStrDisplayName != NULL)) {
                    jobject objLine = pJEnv->NewObject(classSipLine, cid, jobjStrIdentity, jobjStrDisplayName) ;
                    if (objLine != NULL) {
                        jobjRC = objLine ;
                    } else 
                        LOOKUP_FAILURE() ;
                } else 
                    LOOKUP_FAILURE() ;
            } else
                LOOKUP_FAILURE() ;
        } else
            LOOKUP_FAILURE() ;
    }

    return jobjRC ;    
}

void* JavaSUAListenerDispatcher::createSipLineEventObject(SipLineEvent& event, void* pEnv)
{
    void* jobjRC = NULL ;
    JNIEnv* pJEnv = (JNIEnv*) pEnv ;

    if (pJEnv != NULL) 
    {
	    jclass classSipLine = pJEnv->FindClass(SIP_LINE_EVENT_CLASS) ;
	    if (classSipLine != NULL) 
        {
		    jmethodID cid = pJEnv->GetMethodID(classSipLine, SIP_LINE_EVENT_CONST_METHOD, SIP_LINE_EVENT_CONST_SIGNATURE) ;
            if (cid != NULL) 
            {
                // Pull out params from event object
                UtlString realm = event.getRealm() ;
                UtlString scheme = event.getScheme() ;                
                int iResponseCode = event.getSipReturnCode() ;
                UtlString responseText = event.getSipReturnText() ;
                SipLine* pLine = event.getLine() ;                

                // Convert them into java objects

                jobject jobjResponseText = pJEnv->NewStringUTF(responseText.data()) ;
                if (jobjResponseText == NULL)
                {
                    LOOKUP_FAILURE() ;
                    return NULL ;
                }

                jobject jobjRealm = pJEnv->NewStringUTF(realm.data()) ;
                if (jobjRealm == NULL)
                {
                    LOOKUP_FAILURE() ;
                    return NULL ;
                }
                jobject jobjScheme = pJEnv->NewStringUTF(scheme.data()) ;
                if (jobjScheme == NULL)
                {
                    LOOKUP_FAILURE() ;
                    return NULL ;
                }
                
                jobject jobjLine = NULL ;
                if (pLine != NULL)
                {                 
                    Url urlIdentity = pLine->getIdentity() ;
                    Url urlDisplay =  pLine->getUserEnteredUrl() ;
                    jobjLine = (jobject) createSipLineObject(urlIdentity.toString().data(), 
                            urlDisplay.toString().data(), 
                            pJEnv) ;
                    if (jobjLine == NULL)
                    {
                        LOOKUP_FAILURE() ;
                        return NULL ;
                    }
                }             

                // Invoke the constructor
                jobject objLineEvent = pJEnv->NewObject(classSipLine, cid, jobjLine, iResponseCode, jobjResponseText, jobjRealm, jobjScheme) ;
                if (objLineEvent != NULL) 
                {
                    jobjRC = objLineEvent ;
                } 
                else 
                    LOOKUP_FAILURE() ;
            } 
            else
                LOOKUP_FAILURE() ;
        } 
        else
            LOOKUP_FAILURE() ;
    }

    return jobjRC ;   
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */


/**
 * set the CSeq number to a "request" message if none has been provided.
 */
void setCSeqNumberIfNone(SipMessage message)
{
    int seqNum;
	UtlString seqMethod;
    if(! message.getCSeqField(&seqNum, &seqMethod))
    {
        UtlString callID;
		message.getCallIdField(&callID);
		UtlString remoteAddress;
		message.getToField(&remoteAddress);
#ifdef BUILD_SDS
//TODO:
#else
	    CallManager* callManager = Pinger::getPingerTask()->getCallManager();
	    callManager->getNextSipCseq(callID, remoteAddress, seqNum) ;
#endif
		
		//TO DO: need to do error handling here.
		message.getRequestMethod(&seqMethod);
        message.setCSeqField(seqNum, seqMethod.data());
    }
}

/////////////////////////////////////////////////////////////////////////////////
//The code maked as CODE_BREAKS_SDS_BUILD for some reason will cause unresolved
//link errors when the config server trys to load this .so from java.
//We will leave this out until sipxchange 1.1 is finished, then remove the 
//ifdef and find out why it's not working.  (DWW, DLH)
/////////////////////////////////////////////////////////////////////////////////
#ifdef CODE_BREAKS_SDS_BUILD
/*
 * Class:     org_sipfoundry_sip_SipUserAgent
 * Method:    JNI_newSubscribeMessage
 * Signature: (Ljava/lang/String;)V
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sip_SipUserAgent_JNI_1newSubscribeMessage
  (JNIEnv *pEnv, jclass clazz, jstring jsMessage)
{
    jstring newMsg = NULL;
    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipUserAgent_JNI_1newSubscribeMessage");

    char* szMessage = (jsMessage) ? (char *) pEnv->GetStringUTFChars(jsMessage, 0) : NULL;
    if (szMessage != NULL) {
        SipMessage message((const char*)szMessage);
        SipRefreshMgr* refreshMgr = Pinger::getPingerTask()->getRefreshManager();
        if (refreshMgr->newSubscribeMsg(message))
        {
            int length = 0;
            UtlString modifiedMsg;
            message.getBytes(&modifiedMsg, &length);
            newMsg = pEnv->NewStringUTF(modifiedMsg.data());
        }
    }

    JNI_END_METHOD() ;

    return newMsg;
}
#endif //CODE_BREAKS_SDS_BUILD

/*
 * Class:     org_sipfoundry_sip_SipUserAgent
 * Method:    JNI_addIncomingListener
 * Signature: (ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Lorg/sipfoundry/sip/event/NewSipMessageListener;J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_sip_SipUserAgent_JNI_1addIncomingListener
  (JNIEnv *pEnv, jclass clazz, jint iType, jstring jsMethod,
  jstring jsEventType, jstring jsCallId, jstring jsToURL, jstring jsFromURL,
  jobject jobjListener, jlong jlHashCode)
{    
    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipUserAgent_JNI_1addIncomingListener") ;   

    JavaSUAListenerDispatcher::getInstance()->takeListenerLock() ;

//osPrintf("Add incoming listener: 0x%08X\n", jobjListener) ;

#ifdef BUILD_SDS
    SipUserAgent* pUserAgent = SipConfigServerAgent::
                getSipConfigServerAgent()->getSipUserAgent() ;
#else
    SipUserAgent* pUserAgent = Pinger::getPingerTask()->getSipUserAgent() ;
#endif

    // Massage Parameters from Java types to Cish
    UtlBoolean iWantsRequests = ((iType == 0) || (iType == -1)) ;
    UtlBoolean iWantsResponses = ((iType == 1) || (iType == -1)) ;
    char*     szMethod = (jsMethod) ? (char *) pEnv->GetStringUTFChars(jsMethod, 0) : NULL ;
    char*     szEventType = (jsEventType) ? (char *) pEnv->GetStringUTFChars(jsEventType, 0) : NULL ;
    char*     szCallId = (jsCallId) ? (char *) pEnv->GetStringUTFChars(jsCallId, 0) : NULL ;
    char*     szToURL =  (jsToURL) ? (char *) pEnv->GetStringUTFChars(jsToURL, 0) : NULL ;
    char*     szFromURL =  (jsFromURL) ? (char *) pEnv->GetStringUTFChars(jsFromURL, 0) : NULL ;
    jobject   jobjGlobalRefListener = pEnv->NewGlobalRef(jobjListener) ;

/*
osPrintf("\n") ;
osPrintf("Adding Incoming Listener:\n") ;
osPrintf("       Method: %s\n", (szMethod != NULL) ? szMethod : "") ;
osPrintf("     Requests: %d\n", iWantsRequests) ;
osPrintf("    Responses: %d\n", iWantsResponses) ;
osPrintf("   Event Type: %s\n", (szEventType != NULL) ? szEventType : "") ;
osPrintf("       CallId: %s\n", (szCallId != NULL) ? szCallId : "") ;
osPrintf("       To URL: %s\n", (szToURL != NULL) ? szToURL : "") ;
osPrintf("     From URL: %s\n", (szFromURL != NULL) ? szFromURL : "") ;
osPrintf("     Listener: 0x%08X\n", jobjListener) ;
osPrintf("   ListenerGR: 0x%08X\n", jobjGlobalRefListener) ;
osPrintf("\n") ;
*/

    SipSession* pSession = NULL ;
    if (szCallId && szToURL && szFromURL)
    {
        pSession = new SipSession(szCallId, szToURL, szFromURL) ;
    }

    // Remember the listener/global reference
    JavaSUAListenerDispatcher::getInstance()->addSipJNIListenerMapping(
                (unsigned int) jlHashCode, jobjGlobalRefListener) ;
    
    // Add our Observer
    pUserAgent->addMessageObserver(
        *(JavaSUAListenerDispatcher::getInstance()->getMessageQueue()), 
        szMethod, iWantsRequests, iWantsResponses, TRUE, FALSE, szEventType, 
        pSession, (void *) (unsigned int) jlHashCode) ;


    // Clean up
    if (szMethod != NULL)
        pEnv->ReleaseStringUTFChars(jsMethod, szMethod) ; 
    if (szEventType != NULL)
        pEnv->ReleaseStringUTFChars(jsEventType, szEventType) ; 
    if (szCallId != NULL)
        pEnv->ReleaseStringUTFChars(jsCallId, szCallId) ; 
    if (szToURL != NULL)
        pEnv->ReleaseStringUTFChars(jsToURL, szToURL) ; 
    if (szFromURL != NULL)
        pEnv->ReleaseStringUTFChars(jsFromURL, szFromURL) ; 
    if (pSession != NULL)
        delete pSession ;

    JavaSUAListenerDispatcher::getInstance()->releaseListenerLock() ;

    JNI_END_METHOD() ;    
}

/*
 * Class:     org_sipfoundry_sip_SipUserAgent
 * Method:    JNI_removeIncomingListener
 * Signature: (Lorg/sipfoundry/sip/event/NewSipMessageListener;J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_sip_SipUserAgent_JNI_1removeIncomingListener
  (JNIEnv *pEnv, jclass clazz, jobject jobjListener, jlong jlHashCode)
{
    JavaSUAListenerDispatcher::getInstance()->takeListenerLock() ;

    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipUserAgent_JNI_1removeIncomingListener") ;

// osPrintf("Remove incoming listener: 0x%08X\n", jobjListener) ;

#ifdef BUILD_SDS
    SipUserAgent* pUserAgent = SipConfigServerAgent::
                getSipConfigServerAgent()->getSipUserAgent() ;
#else
    SipUserAgent* pUserAgent = Pinger::getPingerTask()->getSipUserAgent() ;
#endif

    JavaSUAListenerDispatcher *pDispatcher = JavaSUAListenerDispatcher::getInstance() ;
    jobject jobjGlobalRefListener ;
    jobjGlobalRefListener = pDispatcher->getSipJNIListenerMapping((unsigned int) jlHashCode) ;
    if (jobjGlobalRefListener != NULL)    
    {
        pDispatcher->removeSipJNIListenerMapping((unsigned int) jlHashCode) ;
        if (pUserAgent->removeMessageObserver(*(JavaSUAListenerDispatcher::getInstance()->getMessageQueue()), (void *) (unsigned int) jlHashCode))
        {                        
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

    JavaSUAListenerDispatcher::getInstance()->releaseListenerLock() ;
    
    JNI_END_METHOD() ;
}



/*
 * Class:     org_sipfoundry_sip_SipUserAgent
 * Method:    JNI_postRequest
 * Signature: (Ljava/lang/String;Lorg/sipfoundry/sip/event/NewSipMessageListener;)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_sip_SipUserAgent_JNI_1postRequest
  (JNIEnv *pEnv, jclass clazz, jstring jsMessage, jobject jobjListener)
{
    JavaSUAListenerDispatcher::getInstance()->takeListenerLock() ;

    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipUserAgent_JNI_1postRequest") ;

#ifdef BUILD_SDS
    SipUserAgent* pUserAgent = SipConfigServerAgent::
                getSipConfigServerAgent()->getSipUserAgent() ;
#else
    SipUserAgent* pUserAgent = Pinger::getPingerTask()->getSipUserAgent() ;
#endif

    char* szMessage = (jsMessage) ? (char *) pEnv->GetStringUTFChars(jsMessage, 0) : NULL ;
    if (szMessage != NULL) {
        SipMessage message((const char*) szMessage) ;
        //just to make sure it is a REQUEST
		if( !message.isResponse() ) {
			//and make sure it has CSeq number.
			setCSeqNumberIfNone(message);
		}
        if (jobjListener == NULL) {
            pUserAgent->send(message) ;
        } else {
            pUserAgent->send(message, 
                (JavaSUAListenerDispatcher::getInstance()->getMessageQueue()),
                pEnv->NewGlobalRef(jobjListener)) ;
        }
        pEnv->ReleaseStringUTFChars(jsMessage, szMessage) ; 
    }    

    JavaSUAListenerDispatcher::getInstance()->releaseListenerLock() ;

    JNI_END_METHOD() ;
}


/*
 * Class:     org_sipfoundry_sip_SipUserAgent
 * Method:    JNI_addExtension
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_sip_SipUserAgent_JNI_1addExtension
  (JNIEnv *pEnv, jclass clazz, jstring jsExtension, jstring jsTest)
{
    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipUserAgent_JNI_1addExtension") ;
    char* szExtension = (jsExtension) ? (char *) pEnv->GetStringUTFChars(jsExtension, 0) : NULL ;
    if (szExtension != NULL) {
#ifdef BUILD_SDS
    SipUserAgent* pUserAgent = SipConfigServerAgent::
                getSipConfigServerAgent()->getSipUserAgent() ;
#else
    SipUserAgent* pUserAgent = Pinger::getPingerTask()->getSipUserAgent() ;
#endif
        pUserAgent->allowExtension(szExtension) ;
        pEnv->ReleaseStringUTFChars(jsExtension, szExtension) ; 
    }

    JNI_END_METHOD() ;    
}


/*
 * Class:     org_sipfoundry_sip_SipUserAgent
 * Method:    JNI_addMethod
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_sip_SipUserAgent_JNI_1addMethod
  (JNIEnv *pEnv, jclass clazz, jstring jsMethod, jstring jsTest)
{
    JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipUserAgent_JNI_1addMethod") ;
    char* szMethod = (jsMethod) ? (char *) pEnv->GetStringUTFChars(jsMethod, 0) : NULL ;
    if (szMethod != NULL) {
#ifdef BUILD_SDS
    SipUserAgent* pUserAgent = SipConfigServerAgent::
                getSipConfigServerAgent()->getSipUserAgent() ;
#else
    SipUserAgent* pUserAgent = Pinger::getPingerTask()->getSipUserAgent() ;
#endif
        pUserAgent->allowMethod(szMethod) ;
        pEnv->ReleaseStringUTFChars(jsMethod, szMethod) ; 
    }

    JNI_END_METHOD() ;    
}

/*
 * Class:     org_sipfoundry_sip_SipUserAgent
 * Method:    JNI_isMethodAllowed
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_sip_SipUserAgent_JNI_1isMethodAllowed
  (JNIEnv *pEnv, jclass clazz, jstring jsMethod, jstring jsTest)
{
    jboolean bRet = false;
	JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipUserAgent_JNI_1isMethodAllowed") ;
    char* szMethod = (jsMethod) ? (char *) pEnv->GetStringUTFChars(jsMethod, 0) : NULL ;
    if (szMethod != NULL) {
#ifdef BUILD_SDS
    SipUserAgent* pUserAgent = SipConfigServerAgent::
                getSipConfigServerAgent()->getSipUserAgent() ;
#else
    SipUserAgent* pUserAgent = Pinger::getPingerTask()->getSipUserAgent() ;
#endif
        bRet = (jboolean) (pUserAgent->isMethodAllowed(szMethod));
        pEnv->ReleaseStringUTFChars(jsMethod, szMethod) ; 
    }

    JNI_END_METHOD() ;
    return bRet;
}


/*
 * Class:     org_sipfoundry_sip_SipUserAgent
 * Method:    JNI_isExtensionAllowed
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_sip_SipUserAgent_JNI_1isExtensionAllowed
  (JNIEnv *pEnv, jclass clazz, jstring jsExtension, jstring jsTest)
{
   jboolean bRet = false;
	JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipUserAgent_JNI_1isExtensionAllowed") ;
    char* szExtension = (jsExtension) ? (char *) pEnv->GetStringUTFChars(jsExtension, 0) : NULL ;
    if (szExtension != NULL) {
#ifdef BUILD_SDS
    SipUserAgent* pUserAgent = SipConfigServerAgent::
                getSipConfigServerAgent()->getSipUserAgent() ;
#else
    SipUserAgent* pUserAgent = Pinger::getPingerTask()->getSipUserAgent() ;
#endif
        bRet = (jboolean) (pUserAgent->isExtensionAllowed(szExtension));
        pEnv->ReleaseStringUTFChars(jsExtension, szExtension) ; 
    }

    JNI_END_METHOD() ;
    return bRet;
}



/*
 * Class:     org_sipfoundry_sip_SipSession
 * Method:    JNI_1getNextCSeqNumber
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 * Gets the next CSequence number for the session with the callid of jsCallID
 * and remoteAddress of jsRemoteAddress
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_sip_SipSession_JNI_1getNextCSeqNumber
  (JNIEnv *pEnv, jclass clazz, jstring jsCallID, jstring jsRemoteAddress)
{
   JNI_BEGIN_METHOD("Java_org_sipfoundry_sip_SipSession_JNI_1getNextCSeqNumber") ;
   char* szCallID = (jsCallID) ? (char *) pEnv->GetStringUTFChars(jsCallID, 0) : NULL ;
   char* szRemoteAddress = (jsRemoteAddress) ? (char *) pEnv->GetStringUTFChars(jsRemoteAddress, 0) : NULL ;
   jint jiNextCSeq = 1;
	int iNextCSeq = 1; 
   if( (szCallID != NULL) && (szRemoteAddress != NULL)){

#ifdef BUILD_SDS
		//TODO
#else
	  CallManager* callManager = Pinger::getPingerTask()->getCallManager();
	  callManager->getNextSipCseq(szCallID, szRemoteAddress, iNextCSeq) ;
#endif

	  jiNextCSeq = (jint) iNextCSeq;	
	}
	pEnv->ReleaseStringUTFChars(jsCallID, szCallID) ; 
   pEnv->ReleaseStringUTFChars(jsRemoteAddress, szRemoteAddress) ; 
    
   JNI_END_METHOD() ;    
   return jiNextCSeq; 
}


