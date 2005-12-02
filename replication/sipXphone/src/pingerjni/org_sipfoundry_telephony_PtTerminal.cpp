// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_telephony_PtTerminal.cpp#2 $
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
#include "os/OsSocket.h"
#include "os/OsLock.h"
#include "os/OsMutex.h"

#include "ptapi/PtAddress.h"
#include "ptapi/PtTerminal.h"
#include "ptapi/PtEventMask.h"
#include "ptapi/PtComponent.h"
#include "ptapi/PtComponentGroup.h"
#include "ptapi/PtProviderListener.h"
#include "ptapi/PtTerminalComponentListener.h"
#include "ptapi/PtPhoneButton.h"
#include "ptapi/PtPhoneHookswitch.h"
#include "ptapi/PtPhoneMicrophone.h"
#include "ptapi/PtPhoneSpeaker.h"
#include "ptapi/PtPhoneRinger.h"
#include "ptapi/PtPhoneLamp.h"
#include "ptapi/PtPhoneDisplay.h"
#include "pingerjni/PtTerminalComponentListenerContainer.h"
#include "pingerjni/PtCallListenerWrap.h"
#include "pingerjni/CallListenerHelpers.h"
#include "pingerjni/PtConnectionListenerWrap.h"
#include "pingerjni/PtTerminalConnectionListenerWrap.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"

// extern UtlString g_strExtension ;

typedef enum tagCWTYPE {
	CWT_CALL_LISTENER,
	CWT_CONNECTION_LISTENER,
	CWT_TERMINAL_CONNECTION_LISTENER
} CWTYPE ;


/*
 * We need to keep a reference to all of our terminal listener containers.
 * This simple array is a hack, but does the job for now.
 */
#define MAX_TERMINAL_LISTENERS	16		// Max # of listeners
static int g_iTermContainers = 0 ;				// Current count of listeners
static PtTerminalComponentListenerContainer* g_pTerminalContainers[MAX_TERMINAL_LISTENERS] ;
static OsMutex				g_mutexTermListeners(OsMutex::Q_PRIORITY) ;

#define MAX_CALL_LISTENERS	16			// Max # of listeners

static int					g_numCallWrappers = 0 ;				// Current count of listeners
static void*				g_pCallWrappers[MAX_CALL_LISTENERS] ;
static CWTYPE				g_eCallTypes[MAX_CALL_LISTENERS] ;
static OsMutex				g_mutexListeners(OsMutex::Q_PRIORITY) ;

/*
 * Class:     org_sipfoundry_telephony_PtTerminal
 * Method:    JNI_getComponents
 * Signature: (J)[J
 */
extern "C"
JNIEXPORT jlongArray JNICALL Java_org_sipfoundry_telephony_PtTerminal_JNI_1getComponents
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	jlongArray components = NULL ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtTerminal_JNI_1getComponents") ;

	PtTerminal* pTerminal = (PtTerminal*) (unsigned int) lHandle ;
	if (pTerminal != NULL) {
		// We support a max of 16 components?!?
		PtComponent* comps[100] ;
		int size = 100 ;
		int items = 0 ;

		PtStatus status = pTerminal->getComponents(comps, size, items) ;
		if (status != 0)
			API_FAILURE(status) ;


		// native -> java

		jlong longs[100] ;
		int j = 0 ;
		for (int i=0; i<items; i++) {
			PtComponent* pComponent = NULL ;
			int			 iType ;

			if (comps[i] != NULL) {
				comps[i]->getType(iType) ;

				switch (iType) {
					case PtComponent::BUTTON:
						pComponent = new PtPhoneButton(*((PtPhoneButton*)comps[i])) ;
						break ;
					case PtComponent::HOOKSWITCH:
						pComponent = new PtPhoneHookswitch(*((PtPhoneHookswitch*)comps[i])) ;
						break ;
					case PtComponent::LAMP:
						pComponent = new PtPhoneLamp(*((PtPhoneLamp*)comps[i])) ;
						break ;
					case PtComponent::MICROPHONE:
						pComponent = new PtPhoneMicrophone(*((PtPhoneMicrophone*)comps[i])) ;
						break ;
					case PtComponent::RINGER:
						pComponent = new PtPhoneRinger(*((PtPhoneRinger*)comps[i])) ;
						break ;
					case PtComponent::SPEAKER:
						pComponent = new PtPhoneSpeaker(*((PtPhoneSpeaker*)comps[i])) ;
						break ;
					case PtComponent::DISPLAY:
						pComponent = new PtPhoneDisplay(*((PtPhoneDisplay*)comps[i])) ;
						break ;
					default:
						pComponent = NULL ;						
				}

				if (pComponent != NULL) {
					longs[j++] = (jlong) (unsigned int) pComponent ;
				}
			} else {
				API_FAILURE(-1) ;
			}
		}

		// bulk copy into our array
		components = pEnv->NewLongArray(j) ;
		pEnv->SetLongArrayRegion(components, 0, j, longs) ;
	}

	JNI_END_METHOD() ;

	return components ;
}


/*
 * Class:     org_sipfoundry_telephony_PtTerminal
 * Method:    JNI_getName
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_sipfoundry_telephony_PtTerminal_JNI_1getName
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	jstring jsRC = NULL ;
	char    szName[64] ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtTerminal_JNI_1getName") ;

	strcpy(szName, "") ;
	PtTerminal* pTerminal = (PtTerminal*) (unsigned int) lHandle ;
	if (pTerminal != NULL) {

		PtStatus status = pTerminal->getName(szName, sizeof(szName)) ;
		if (status != 0)
			API_FAILURE(status) ;
	}


	jsRC = pEnv->NewStringUTF(szName) ;

	JNI_END_METHOD() ;

	return jsRC ;
}



/*
 * Class:     org_sipfoundry_telephony_PtTerminal
 * Method:    JNI_getComponentGroups
 * Signature: (J)[J
 */
extern "C"
JNIEXPORT jlongArray JNICALL Java_org_sipfoundry_telephony_PtTerminal_JNI_1getComponentGroups
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	jlongArray componentGroups = NULL ;


	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtTerminal_JNI_1getComponentGroups") ;

	PtTerminal* pTerminal = (PtTerminal*) (unsigned int) lHandle ;
	if (pTerminal != NULL) {

		// We support a max of 8 component groups?!?
		PtComponentGroup groups[8] ;
		int size = 8 ;
		int items = 0 ;

		PtStatus status = pTerminal->getComponentGroups(groups, size, items) ;
		if (status != 0)
			API_FAILURE(status) ;

		// native -> java

		// convert to an array of longs
		jlong longs[8] ;
		for (int i=0; i<items; i++) {
			PtComponentGroup* pComponentGroup = new PtComponentGroup() ;
			*pComponentGroup  = groups[i] ; 
			longs[i] = (jlong) (unsigned int) pComponentGroup ;
		}

		// bulk copy into our array
		componentGroups = pEnv->NewLongArray(items) ;
		pEnv->SetLongArrayRegion(componentGroups, 0, items, longs) ;
	}

	JNI_END_METHOD() ;

	return componentGroups ;
}

/*
 * Class:     org_sipfoundry_telephony_PtTerminal
 * Method:    JNI_addTerminalListener
 * Signature: (JLjavax/telephony/TerminalListener;J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtTerminal_JNI_1addTerminalListener
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jobject terminalListener, jlong lFilter)
{	
	UtlString host ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtTerminal_JNI_1addTerminalListener") ;
    
	OsLock lock(g_mutexTermListeners) ;
	PtTerminal* pTerminal = (PtTerminal*) (unsigned int) lHandle ;
	if (pTerminal != NULL) {
		if (g_iTermContainers < MAX_TERMINAL_LISTENERS) {
			OsSocket::getHostIp(&host) ;

			g_pTerminalContainers[g_iTermContainers] = new PtTerminalComponentListenerContainer(host.data(), NULL, terminalListener, (void*) pEnv) ;

            g_pTerminalContainers[g_iTermContainers]->setEventFilter(lFilter) ;

			PtStatus status = pTerminal->addTerminalListener(*g_pTerminalContainers[g_iTermContainers]) ;
			if (status != 0)
				API_FAILURE(status) ;
			g_iTermContainers++ ;
		} else {
			API_FAILURE(-1) ;
		}		
	}

	JNI_END_METHOD() ;
}


/*
 * Class:     org_sipfoundry_telephony_PtTerminal
 * Method:    JNI_removeTerminalListener
 * Signature: (JLjavax/telephony/TerminalListener;)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtTerminal_JNI_1removeTerminalListener
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jobject terminalListener)
{	
	bool bFound = false ;
	PtTerminal* pTerminal = (PtTerminal*) (unsigned int) lHandle ;


	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtTerminal_JNI_1removeTerminalListener") ;

	OsLock lock(g_mutexTermListeners) ;

	if (pTerminal != NULL) {
		for (int i=0;i<g_iTermContainers; i++) {
			if (g_pTerminalContainers[i]->getTerminalListener() == terminalListener) {
				bFound = true ;

				PtTerminalComponentListenerContainer* pReference = g_pTerminalContainers[i] ;
				
				// collapse the list by one element
				g_iTermContainers-- ;
				for (int j=i; j<g_iTermContainers; j++) {
					g_pTerminalContainers[j] = g_pTerminalContainers[j+1] ;					
				}

				PtStatus status = pTerminal->removeTerminalListener(*pReference) ;
				if (status != 0)
					API_FAILURE(status) ;

            pReference->cleanup(pEnv) ;

				delete pReference ;
				 
				break ;
			}
		}

		if (!bFound) {
			API_FAILURE(-1) ;
		}
	}

	JNI_END_METHOD() ;
}



/*
 * Class:     org_sipfoundry_telephony_PtTerminal
 * Method:    JNI_getAddresses
 * Signature: (J)[J
 */
extern "C"
JNIEXPORT jlongArray JNICALL Java_org_sipfoundry_telephony_PtTerminal_JNI_1getAddresses
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtTerminal* pTerminal = (PtTerminal*) (unsigned int) lHandle ;
	jlongArray addresses = NULL ;


	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtTerminal_JNI_1getAddresses") ;

	if (pTerminal != NULL) {
		PtAddress native_address[8] ;
		int       nItems = 0 ;

		PtStatus status = pTerminal->getAddresses(native_address, 8, nItems) ;
		if (status != 0)
			API_FAILURE(status) ;

		// convert to an array of longs
		jlong longs[8] ;
		for (int i=0; i<nItems; i++) {
			longs[i] = (jlong) (unsigned int) new PtAddress(native_address[i]) ;
		}

		// bulk copy into our array
		addresses = pEnv->NewLongArray(nItems) ;
		pEnv->SetLongArrayRegion(addresses, 0, nItems, longs) ;
	} else
		API_FAILURE(-1) ;

	JNI_END_METHOD() ;

	return addresses ;
}



/*
 * Class:     org_sipfoundry_telephony_PtTerminal
 * Method:    JNI_addCallListener_call
 * Signature: (JLjavax/telephony/CallListener;I)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtTerminal_JNI_1addCallListener_1call
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jobject jobjListener, jint id, jlong ulExcludeFilter)
{	
	PtTerminal* pTerminal = (PtTerminal*) (unsigned int) lHandle ;	

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtTerminal_JNI_1addCallListener_1call") ;

   initializeJNICaches(pEnv) ;

	OsLock lock(g_mutexListeners) ;

	if (pTerminal != NULL) {
		if (g_numCallWrappers < MAX_CALL_LISTENERS) {

			g_pCallWrappers[g_numCallWrappers] = new PtCallListenerWrap(NULL, id, pEnv->NewGlobalRef(jobjListener), ulExcludeFilter) ;
			g_eCallTypes[g_numCallWrappers] = CWT_CALL_LISTENER ;

			PtStatus status = pTerminal->addCallListener(*(PtCallListenerWrap*)g_pCallWrappers[g_numCallWrappers]) ;
			if (status != 0)
				API_FAILURE(status) ;

			g_numCallWrappers++ ;
		} else {
			API_FAILURE(-1) ;
		}
	}

	JNI_END_METHOD() ;

}

/*
 * Class:     org_sipfoundry_telephony_PtTerminal
 * Method:    JNI_addCallListener_conn
 * Signature: (JLjavax/telephony/CallListener;I)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtTerminal_JNI_1addCallListener_1conn
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jobject jobjListener, jint id, jlong ulExcludeFilter)
{	
	PtTerminal* pTerminal = (PtTerminal*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtTerminal_JNI_1addCallListener_1conn") ;

   initializeJNICaches(pEnv) ;

	OsLock lock(g_mutexListeners) ;

	if (pTerminal  != NULL) {
		if (g_numCallWrappers < MAX_CALL_LISTENERS) {
			g_pCallWrappers[g_numCallWrappers] = new PtConnectionListenerWrap(NULL, id, pEnv->NewGlobalRef(jobjListener), ulExcludeFilter) ;
			g_eCallTypes[g_numCallWrappers] = CWT_CONNECTION_LISTENER ;

			PtStatus status = pTerminal->addCallListener(*(PtConnectionListenerWrap*)g_pCallWrappers[g_numCallWrappers]) ;
			if (status != 0)
				API_FAILURE(status) ;

			g_numCallWrappers++ ;
		} else {
			API_FAILURE(-1) ;
		}
	}

	JNI_END_METHOD() ;
}

/*
 * Class:     org_sipfoundry_telephony_PtTerminal
 * Method:    JNI_addCallListener_term
 * Signature: (JLjavax/telephony/CallListener;I)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtTerminal_JNI_1addCallListener_1term
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jobject jobjListener, jint id, jlong ulExcludeFitler)
{	
	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtTerminal_JNI_1addCallListener_1term") ;

	OsLock lock(g_mutexListeners) ;

   initializeJNICaches(pEnv) ;

	PtTerminal* pTerminal = (PtTerminal*) (unsigned int) lHandle ;
	if (pTerminal  != NULL) {
		if (g_numCallWrappers < MAX_CALL_LISTENERS) {
			
			g_pCallWrappers[g_numCallWrappers] = new PtTerminalConnectionListenerWrap(NULL, id, pEnv->NewGlobalRef(jobjListener), ulExcludeFitler) ;
			g_eCallTypes[g_numCallWrappers] = CWT_TERMINAL_CONNECTION_LISTENER ;

			PtStatus status = pTerminal->addCallListener(*(PtTerminalConnectionListenerWrap*)g_pCallWrappers[g_numCallWrappers]) ;
			if (status != 0)
				API_FAILURE(status) ;

			g_numCallWrappers++ ;
		} else {
			API_FAILURE(-1) ;
		}
	}

	JNI_END_METHOD() ;
}


/*
 * Class:     org_sipfoundry_telephony_PtTerminal
 * Method:    JNI_removeCallListener
 * Signature: (JLjavax/telephony/CallListener;I)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtTerminal_JNI_1removeCallListener
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jobject jobjListener, jint id)
{
/** WARNING: This code is cloned in PtCall and PtTerminal */

	bool bFound		= false ;
	int  iDelObject = -1 ;		// ID of object we found/deleted

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtTerminal_JNI_1removeCallListener") ;

	OsLock lock(g_mutexListeners) ;

	PtTerminal* pTerminal = (PtTerminal*) (unsigned int) lHandle ;
	if (pTerminal != NULL) {

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

							// Remove the Call Listener
							PtStatus status = pTerminal->removeCallListener(*pReference) ;
							if (status != 0)
								API_FAILURE(status) ;
							
							// Free up java objects and our own wrapper obj
							pEnv->DeleteGlobalRef(pReference->getListener()) ;
							delete pReference ;
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

							// Remove the Call Listener
							PtStatus status = pTerminal->removeCallListener(*pReference) ;
							if (status != 0)
								API_FAILURE(status) ;
							
							// Free up java objects and our own wrapper obj
							pEnv->DeleteGlobalRef(pReference->getListener()) ;
							delete pReference ;
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

							// Remove the Call Listener
							PtStatus status = pTerminal->removeCallListener(*pReference) ;
							if (status != 0)
								API_FAILURE(status) ;
							
							// Free up java objects and our own wrapper obj
							pEnv->DeleteGlobalRef(pReference->getListener()) ;
							delete pReference ;
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
 * Class:     org_sipfoundry_telephony_PtTerminal
 * Method:    setCodecCPULimit
 * Signature: (JI)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtTerminal_JNI_1setCodecCPULimit
  (JNIEnv* pEnv, jclass clazz, jlong lHandle, jint iLimit)
{
   JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtTerminal_JNI_1setCodecCPULimit") ;

   PtTerminal* pTerminal = (PtTerminal*) (unsigned int) lHandle ;
   if (pTerminal != NULL)
   {
      pTerminal->setCodecCPULimit(iLimit) ;
   }

   JNI_END_METHOD() ;
}



/*
 * Class:     org_sipfoundry_telephony_PtTerminal
 * Method:    JNI_finalize
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtTerminal_JNI_1finalize
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	if (lHandle != 0) {
#if DEBUG_FINALIZE
		if (IsJNIDebuggingEnabled())
			osPrintf("JNI: freeing terminal 0x%08X\n", lHandle) ;
#endif
		delete (PtTerminal*) (unsigned int) lHandle ;
	}
}
