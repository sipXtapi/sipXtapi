// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_telephony_PtCall.cpp#2 $
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
#include "os/OsLock.h"
#include "os/OsMutex.h"

#include "ptapi/PtCall.h"
#include "ptapi/PtTerminal.h"
#include "ptapi/PtAddress.h"
#include "ptapi/PtSessionDesc.h"
#include "ptapi/PtConnection.h"
#include "pingerjni/CallListenerHelpers.h"
#include "pingerjni/PtCallListenerWrap.h"
#include "pingerjni/PtConnectionListenerWrap.h"
#include "pingerjni/PtTerminalConnectionListenerWrap.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"

#include "pinger/Pinger.h"
#include "cp/CallManager.h"
#include "net/SipLineMgr.h"



/*
 * We need to keep a reference to all of our listener wrappers/containers.
 * This simple array is a hack, but does the job for now.
 */
#define MAX_CALL_LISTENERS	16			// Max # of listeners

typedef enum tagCWTYPE {
	CWT_CALL_LISTENER,
	CWT_CONNECTION_LISTENER,
	CWT_TERMINAL_CONNECTION_LISTENER
} CWTYPE ;


static OsMutex	g_mutexListeners(OsMutex::Q_FIFO) ;		// Guard for any listener operations

static int		g_numCallWrappers = 0 ;					// Number of call listeners currently in service 
static void*	g_pCallWrappers[MAX_CALL_LISTENERS] ;	// Actual list of call listeners
static CWTYPE	g_eCallTypes[MAX_CALL_LISTENERS] ;		// Type of listener (needed for casting)


void dumpCallListeners(PtCall* pCall) ;


/*
 * Class:     org_sipfoundry_telephony_PtCall
 * Method:    JNI_connect
 * Signature: (JJJLjava/lang/String;)[J
 */
extern "C"
JNIEXPORT jlongArray JNICALL Java_org_sipfoundry_telephony_PtCall_JNI_1connect
	(JNIEnv* pEnv, jclass clazz, jlong lHandle, jlong lTerminal, jlong lAddress, jstring jsSIPURL)
{

    PtCall*       pCall = (PtCall*) (unsigned int) lHandle ;
	PtTerminal*   pTerminal = (PtTerminal*) (unsigned int) lTerminal ;
	PtAddress*	  pAddress = (PtAddress*) (unsigned int) lAddress ;
	PtSessionDesc desc ;
	char*         szSIPURL = (jsSIPURL) ? (char *)pEnv->GetStringUTFChars(jsSIPURL, 0) : NULL ;
	jlongArray    connections = NULL ;
	PtConnection* pNative_connections = NULL ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtCall_JNI_1connect") ;

	if (pCall != NULL) {
       
        //set the outbound line for this call to be pAddress
        //if it is not null.
       if( pAddress != NULL )
       {
            CallManager* pCallManager = Pinger::getPingerTask()->getCallManager() ;
            SipLineMgr* pLineMgr = Pinger::getPingerTask()->getLineManager() ;

            if (pCallManager != NULL)
            {
                char pCallid[128];
	            pCall->getCallId(pCallid, 127);
                char pName[128];
	            pAddress->getName(pName, 127);
                CONTACT_TYPE contactType = AUTO ;

                if (pLineMgr)
                {
                    Url identity(pName) ;
                    LINE_CONTACT_TYPE eLineType = LINE_CONTACT_LOCAL ;
                    if (pLineMgr->getContactTypeForLine(pName, eLineType))
                    {
                        contactType = (CONTACT_TYPE) eLineType ;
                    }
                }

                pCallManager->setOutboundLineForCall(pCallid, pName, contactType);
            }
        }

   	  PtStatus status = pCall->connect(*pTerminal, *pAddress, szSIPURL, &desc) ;
        if (status != 0) {
            API_FAILURE(status) ;
            jniThrowException(pEnv, status) ;		    
        } else {
		    // Get native PTAPI connections
		    int nConnections = 0 ;
		    if ((pCall->numConnections(nConnections) == 0) && (nConnections > 0)) {

			    // Get native PTAPI connections 
			    pNative_connections = new PtConnection[nConnections] ;
			    int nItems = 0 ;
			    
			    status = pCall->getConnections(pNative_connections, nConnections, nItems) ;
			    if (status != 0)
				    API_FAILURE(status) ;
			    
			    // convert to an array of longs
			    jlong longs[8] ;
			    if (nItems > 8)
				    nItems = 8 ;
			    for (int i=0; i<nItems; i++) {
				    PtConnection *pConnection = new PtConnection() ;
				    *pConnection = pNative_connections[i] ;
				    longs[i] = (jlong) (unsigned int) pConnection ;
			    }

			    // bulk copy into our array
			    connections = pEnv->NewLongArray(nItems) ;
			    pEnv->SetLongArrayRegion(connections, 0, nItems, longs) ;
		    } else
			    connections = pEnv->NewLongArray(0) ;

		    if (pNative_connections != NULL)
			    delete[] pNative_connections ;
	    }
    }


	if (szSIPURL) 
		pEnv->ReleaseStringUTFChars(jsSIPURL, szSIPURL) ;

	JNI_END_METHOD() ;

	return connections ;	
}

/*
 * Class:     org_sipfoundry_telephony_PtCall
 * Method:    JNI_hold
 * Signature: (JZ;)[V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtCall_JNI_1hold
	(JNIEnv* pEnv, jclass clazz, jlong lHandle, jboolean bBridgeParticipants)
{
	PtCall*       pCall = (PtCall*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtCall_JNI_1hold") ;

   if (pCall != NULL) 
   {
      PtStatus status = pCall->hold(bBridgeParticipants);
      if (status != 0) 
      {
         osPrintf("JNI: PtCall::hold returned %d\n", status) ;
         jniThrowException(pEnv, status) ;		    
      }
   }
	JNI_END_METHOD();	
}


/*
 * Class:     org_sipfoundry_telephony_PtCall
 * Method:    JNI_unhold
 * Signature: (JZ;)[V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtCall_JNI_1unhold
	(JNIEnv* pEnv, jclass clazz, jlong lHandle, jboolean bRemoteParticipants)
{
	PtCall*       pCall = (PtCall*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtCall_JNI_1unhold") ;

   if (pCall != NULL) 
   {
      PtStatus status = pCall->unhold(bRemoteParticipants);
      if (status != 0) 
      {
         osPrintf("JNI: PtCall::unhold returned %d\n", status) ;
         jniThrowException(pEnv, status) ;		    
      }
   }
	JNI_END_METHOD();	
}




/*
 * Class:     org_sipfoundry_telephony_PtCall
 * Method:    JNI_addCallListener_call
 * Signature: (JLjavax/telephony/CallListener;I)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_PtCall_JNI_1addCallListener_1call
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jobject jobjListener, jint id, jlong ulExludeFilter)
{	
	PtCall* pCall = (PtCall*) (unsigned int) lHandle ;
	jlong lRC = 0 ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtCall_JNI_1addCallListener_1call") ;

   initializeJNICaches(pEnv) ;

	OsLock lock(g_mutexListeners) ;

	if (pCall != NULL) {
		if (g_numCallWrappers < MAX_CALL_LISTENERS) 
      {
			g_pCallWrappers[g_numCallWrappers] = new PtCallListenerWrap(NULL, id, pEnv->NewGlobalRef(jobjListener), ulExludeFilter) ;
			g_eCallTypes[g_numCallWrappers] = CWT_CALL_LISTENER ;

			lRC = (jlong) (unsigned int) g_pCallWrappers[g_numCallWrappers] ;
			PtStatus status = pCall->addCallListener(*(PtCallListenerWrap*)g_pCallWrappers[g_numCallWrappers]) ;
			if (status != 0)
				API_FAILURE(status) ;

			g_numCallWrappers++ ;
		} else {
         LOOKUP_FAILURE() ;
		}
	}

	JNI_END_METHOD() ;
	
	return lRC ;
}


/*
 * Class:     org_sipfoundry_telephony_PtCall
 * Method:    JNI_addCallListener_conn
 * Signature: (JLjavax/telephony/ConnectionListener;I)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_PtCall_JNI_1addCallListener_1conn
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jobject jobjListener, jint id, jlong ulExludeFilter)
{	
	PtCall* pCall = (PtCall*) (unsigned int) lHandle ;
	jlong lRC = 0 ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtCall_JNI_1addCallListener_1conn") ;

   initializeJNICaches(pEnv) ;

	OsLock lock(g_mutexListeners) ;

	if (pCall != NULL) {
		if (g_numCallWrappers < MAX_CALL_LISTENERS) {
			g_pCallWrappers[g_numCallWrappers] = new PtConnectionListenerWrap(NULL, id, pEnv->NewGlobalRef(jobjListener), ulExludeFilter) ;
			g_eCallTypes[g_numCallWrappers] = CWT_CONNECTION_LISTENER ;

			lRC = (jlong) (unsigned int) g_pCallWrappers[g_numCallWrappers] ;
			PtStatus status = pCall->addCallListener(*((PtConnectionListenerWrap*)g_pCallWrappers[g_numCallWrappers])) ;
			if (status != 0)
				API_FAILURE(status) ;

			g_numCallWrappers++ ;
		} else {
         API_FAILURE(-1) ;
		}
	}

	JNI_END_METHOD() ;

	return lRC ;
}


/*
 * Class:     org_sipfoundry_telephony_PtCall
 * Method:    JNI_addCallListener_term
 * Signature: (JLjavax/telephony/TerminalConnectionListener;I)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_PtCall_JNI_1addCallListener_1term
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jobject jobjListener, jint id, jlong ulExludeFilter)
{	
	PtCall* pCall = (PtCall*) (unsigned int) lHandle ;
	jlong lRC = 0 ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtCall_JNI_1addCallListener_1term") ;

   initializeJNICaches(pEnv) ;

	OsLock lock(g_mutexListeners) ;

	if (pCall != NULL) {
		if (g_numCallWrappers < MAX_CALL_LISTENERS) {
			
			g_pCallWrappers[g_numCallWrappers] = new PtTerminalConnectionListenerWrap(NULL, id, pEnv->NewGlobalRef(jobjListener), ulExludeFilter) ;
			g_eCallTypes[g_numCallWrappers] = CWT_TERMINAL_CONNECTION_LISTENER ;

			lRC = (jlong) (unsigned int) g_pCallWrappers[g_numCallWrappers] ;
			PtStatus status = pCall->addCallListener(*(PtTerminalConnectionListenerWrap*)g_pCallWrappers[g_numCallWrappers]) ;
			if (status != 0)
				API_FAILURE(status) ;

			g_numCallWrappers++ ;
		} else {
			API_FAILURE(-1) ;
		}
	}

	JNI_END_METHOD() ;

	return lRC ;
}


/*
 * Class:     org_sipfoundry_telephony_PtCall
 * Method:    JNI_removeCallListener
 * Signature: (JLjavax/telephony/CallListener;I)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtCall_JNI_1removeCallListener
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jobject jobjListener, jint id)
{

/** WARNING: This code is cloned in PtCall and PtTerminal exception this version
	         does not actually delete the object */

	bool bFound		= false ;
	int  iDelObject = -1 ;		// ID of object we found/deleted

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtCall_JNI_1removeCallListener") ;

	OsLock lock(g_mutexListeners) ;

	PtCall* pCall = (PtCall*) (unsigned int) lHandle ;

	if (pCall != NULL) {

		/**
		 * Step 1: Find the matching listener in our listener list, and kill it
		 */
		for (int i=0;i<g_numCallWrappers; i++)  {
			CWTYPE type = g_eCallTypes[i] ;

			/*
			 * This part is definitely ugly.  We need to figure out what type of 
			 * reference we have and then cast and kill.  Alternatively, we could
			 * have three different remove methods (much like add)...  Not much
			 * fun, either.
			 */
			switch (type) {
				case CWT_CALL_LISTENER:
					{
						PtCallListenerWrap* pReference = (PtCallListenerWrap*) g_pCallWrappers[i] ;

						if (pReference->getHashID() == id) {
//							osPrintf("** Removing Listener hashid=0x%08X, reference=0x%08X\n", id, pReference) ;

							// Remove the Call Listener
							PtStatus status = pCall->removeCallListener(*pReference) ;
							if (status != 0)
								API_FAILURE(status) ;

							// Free up java objects and our own wrapper obj
							pEnv->DeleteGlobalRef(pReference->getListener()) ;
                            pReference->clearListener() ;
							g_pCallWrappers[i] = NULL ;
							iDelObject = i ;
							bFound = true ;
							break ;
						}
					}					
					break ;
				case CWT_CONNECTION_LISTENER:
					{						
						PtConnectionListenerWrap* pReference = (PtConnectionListenerWrap*) g_pCallWrappers[i] ;
						
						if (pReference->getHashID() == id) {
//							osPrintf("** Removing Listener hashid=0x%08X, reference=0x%08X\n", id, pReference) ;

							// Remove the Call Listener
							PtStatus status = pCall->removeCallListener(*pReference) ;
							if (status != 0)
								API_FAILURE(status) ;
							
							// Free up java objects and our own wrapper obj
							pEnv->DeleteGlobalRef(pReference->getListener()) ;
                            pReference->clearListener() ;                            
							g_pCallWrappers[i] = NULL ;
							iDelObject = i ;
							bFound = true ;
							break ;
						}
					}
					break ;
				case CWT_TERMINAL_CONNECTION_LISTENER:
					{
						PtTerminalConnectionListenerWrap* pReference = (PtTerminalConnectionListenerWrap*) g_pCallWrappers[i] ;

						if (pReference->getHashID() == id) {
//							osPrintf("** Removing Listener hashid=0x%08X, reference=0x%08X\n", id, pReference) ;

							// Remove the Call Listener
							PtStatus status = pCall->removeCallListener(*pReference) ;
							if (status != 0)
								API_FAILURE(status) ;
							
							// Free up java objects and our own wrapper obj
							pEnv->DeleteGlobalRef(pReference->getListener()) ;
                            pReference->clearListener() ;
							g_pCallWrappers[i] = NULL ;
							iDelObject = i ;
							bFound = true ;
							break ;							
						}
					}
					break ;
			}
		}

		/**
		 * Step 2: Update our bookwork and clean up our listener list
		 */
		if (bFound == TRUE) {			
			g_numCallWrappers-- ;
			for (int j=iDelObject; j<g_numCallWrappers; j++) {
				g_pCallWrappers[j] = g_pCallWrappers[j+1] ;
				g_eCallTypes[j]    = g_eCallTypes[j+1] ;
			}
		} else {
			API_FAILURE(-1) ;
		}
	}

	JNI_END_METHOD() ;
}


/*
 * Class:     org_sipfoundry_telephony_PtCall
 * Method:    JNI_getConnections
 * Signature: (J)[J
 */
extern "C"
JNIEXPORT jlongArray JNICALL Java_org_sipfoundry_telephony_PtCall_JNI_1getConnections
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtCall*       pCall = (PtCall*) (unsigned int) lHandle ;
	jlongArray    connections = NULL ;
	PtConnection* pNative_connections = NULL ; 

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtCall_JNI_1getConnections") ;

	if (pCall != NULL) {

		int nConnections = 0 ;
		if ((pCall->numConnections(nConnections) == 0) && (nConnections > 0)) {

			// Get native PTAPI connections
			pNative_connections = new PtConnection[nConnections] ;
			int nItems = 0 ;

			PtStatus status = pCall->getConnections(pNative_connections, nConnections, nItems) ;
			if (status != 0)
				API_FAILURE(status) ;
			
			// convert to an array of longs
			jlong longs[8] ;
			if (nItems > 8)
				nItems = 8 ;
			for (int i=0; i<nItems; i++) {
				PtConnection *pConnection = new PtConnection() ;
				*pConnection = pNative_connections[i] ;
				longs[i] = (jlong) (unsigned int) pConnection ;
			}

			// bulk copy into our array
			connections = pEnv->NewLongArray(nItems) ;
			pEnv->SetLongArrayRegion(connections, 0, nItems, longs) ;
		} else
			connections = pEnv->NewLongArray(0) ;

		if (pNative_connections != NULL)
			delete[] pNative_connections ;
	}

	JNI_END_METHOD() ;

	return connections ;	
}


/*
 * Class:     org_sipfoundry_telephony_PtCall
 * Method:    JNI_getCallID
 * Signature: (J)Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_telephony_PtCall_JNI_1getCallID
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	jstring jsRC = NULL ;
	PtCall* pCall = (PtCall*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtCall_JNI_1getCallID") ;

	if (pCall != NULL) {
		char szCallId[128] ;
		strcpy(szCallId, "") ;
		
		pCall->getCallId(szCallId, sizeof(szCallId)) ;

		if (szCallId) {
			jsRC = pEnv->NewStringUTF(szCallId) ;
		} else {
			jsRC = pEnv->NewStringUTF("") ;
		}
	}

	JNI_END_METHOD() ;

	return jsRC ;
}



/*
 * Class:     org_sipfoundry_telephony_PtCall
 * Method:    JNI_finalize
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtCall_JNI_1finalize
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	if (lHandle != 0) {
#if DEBUG_FINALIZE
		if (IsJNIDebuggingEnabled())
			osPrintf("JNI: freeing call 0x%08X\n", lHandle) ;
#endif
		delete (PtCall*) (unsigned int) lHandle ;
	}
}


/*
 * Class:     org_sipfoundry_telephony_PtCall
 * Method:    JNI_finalizeCallListener
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtCall_JNI_1finalizeCallListener
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtCallListenerWrap* pReference = (PtCallListenerWrap*) (unsigned int) lHandle ;
	if (pReference != NULL) {
#if DEBUG_FINALIZE
		if (IsJNIDebuggingEnabled())
			osPrintf("JNI: freeing call listener 0x%08X\n", lHandle) ;
#endif
		delete pReference ;
	}
}

/*
 * Class:     org_sipfoundry_telephony_PtCall
 * Method:    JNI_finalizeConnectionListener
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtCall_JNI_1finalizeConnectionListener
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtConnectionListenerWrap* pReference = (PtConnectionListenerWrap*) (unsigned int) lHandle ;
	if (pReference != NULL) {
#if DEBUG_FINALIZE
		if (IsJNIDebuggingEnabled())
			osPrintf("JNI: freeing connection listener 0x%08X\n", lHandle) ;
#endif
		delete pReference ;
	}
}


/*
 * Class:     org_sipfoundry_telephony_PtCall
 * Method:    JNI_finalizeTerminalConnectionListener
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtCall_JNI_1finalizeTerminalConnectionListener
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtTerminalConnectionListenerWrap* pReference = (PtTerminalConnectionListenerWrap*) (unsigned int) lHandle ;
	if (pReference != NULL) {
#if DEBUG_FINALIZE
		if (IsJNIDebuggingEnabled())
			osPrintf("JNI: freeing terminal connection listener 0x%08X\n", lHandle) ;
#endif
		delete pReference ;
	}
}



/*
 * Class:     org_sipfoundry_telephony_PtCall
 * Method:    JNI_disablePremiumSound
 * Signature: (Ljava/lang/String;I)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtCall_JNI_1disablePremiumSound
  (JNIEnv *pEnv, jclass clazz, jstring jsCallId, jint unused)
{
    CallManager* pCallManager = Pinger::getPingerTask()->getCallManager() ;
    if (pCallManager != NULL)
    {
        char* szCallId = JSTRTOCSTR(pEnv, jsCallId) ;
        pCallManager->stopPremiumSound(szCallId) ;
        RELEASECSTR(pEnv, jsCallId, szCallId) ;
    }
}


/**
 * Dump 'the' list of call listeners
 */
void dumpCallListeners(PtCall* pCall)
{
	char szCallId[128] ;
	strcpy(szCallId, "") ;

	if (pCall != NULL)
		pCall->getCallId(szCallId, sizeof(szCallId)) ;

	osPrintf("** DUMPING CALL LISTENERS: %s **\n", szCallId) ;

	for (int i=0;i<g_numCallWrappers; i++)  {
		CWTYPE type = g_eCallTypes[i] ;

		switch (type) 
		{
			case CWT_CALL_LISTENER:
				{
					PtCallListenerWrap* pReference = (PtCallListenerWrap*) g_pCallWrappers[i] ;
					osPrintf("CWT_CALL_LISTENER reference=%08X id=%08X\n", 
                            (unsigned int) pReference, (unsigned int) pReference->getHashID()) ;
				}
				break ;
			case CWT_CONNECTION_LISTENER:
				{						
					PtConnectionListenerWrap* pReference = (PtConnectionListenerWrap*) g_pCallWrappers[i] ;
					osPrintf("CWT_CONNECTION_LISTENER reference=%08X id=%08X\n",
                            (unsigned int) pReference, (unsigned int) pReference->getHashID()) ;
				}
				break ;
			case CWT_TERMINAL_CONNECTION_LISTENER:
				{						
					PtTerminalConnectionListenerWrap* pReference = (PtTerminalConnectionListenerWrap*) g_pCallWrappers[i] ;
					osPrintf("CWT_CONNECTION_LISTENER reference=%08X id=%08X\n",
                            (unsigned int) pReference, (unsigned int) pReference->getHashID()) ;
				}
				break ;
		}
	}

	osPrintf("\n") ;
}

