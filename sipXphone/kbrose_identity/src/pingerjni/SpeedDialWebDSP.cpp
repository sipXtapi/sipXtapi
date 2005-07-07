// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/SpeedDialWebDSP.cpp#2 $
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
#include "pingerjni/JNIHelpers.h"
#include "pingerjni/debug_jni.h"

#include "os/OsDefs.h"

#include "os/OsLock.h"
#include "os/OsMutex.h"




/*
 * Class:     SpeedDialWebDSP
 * Method:    addSpeeddialEntry
 */
extern "C" 
void JNI_addSpeeddialEntry (const char*  strId, 
						const char*  strLabel,
						const char*  strNumber, 
						const char*  strUrl)
{
#if DEBUG_WEB_UI
	if (IsJNIDebuggingEnabled())
		osPrintf("BEGIN addSpeeddialEntry method\n") ;
#endif

	// Create or attach to the VM
	JavaVM* pVM ;
	JNIEnv* pEnv ;

   int iPriority = jniGetCurrentPriority() ; 

	// Create or attach to a VM
	if (jniGetVMReference(&pVM, &pEnv)) {			

		// Create the new Objects
		jstring jStrId		= pEnv->NewStringUTF(strId) ;
		jstring jStrLabel	= pEnv->NewStringUTF(strLabel) ;
		jstring jStrNumber	= pEnv->NewStringUTF(strNumber) ;
		jstring jStrUrl		= pEnv->NewStringUTF(strUrl) ;

		// Attach to JVM method

		const char * className = "org/sipfoundry/sipxphone/sys/directoryservice/provider/SpeedDialWebDSP" ;
		jclass eventClass = pEnv->FindClass(className) ;
		if (eventClass != NULL) {
			jmethodID addMethod = pEnv->GetStaticMethodID(eventClass, "addEntry", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V") ;
			
			if (addMethod != NULL) {
				pEnv->CallStaticVoidMethod(eventClass, addMethod, jStrId, jStrLabel, jStrNumber, jStrUrl) ;
			} else {
				osPrintf("JNI: addSpeeddialEntry couldn't get method\n") ;
			}
		
		} else {
			osPrintf("JNI: addSpeeddialEntry couldn't get class\n") ;
		}

		jniReleaseVMReference(pVM) ;	
	} else {
		osPrintf("JNI: addSpeeddialEntry couldn't get VM ref\n") ;
	}

   jniResetPriority(iPriority) ;

#if DEBUG_WEB_UI
	if (IsJNIDebuggingEnabled())
		osPrintf("END addSpeeddialEntry method\n") ;
#endif
}


/*
 * Method:    deleteSpeeddialEntry
 */
extern "C" 
void JNI_deleteSpeeddialEntry (const char* strId)
{
#if DEBUG_WEB_UI
	if (IsJNIDebuggingEnabled())
		osPrintf("BEGIN deleteSpeeddialEntry method\n") ;
#endif

	// Create or attach to the VM
	JavaVM* pVM ;
	JNIEnv* pEnv ;

   int iPriority = jniGetCurrentPriority() ; 

	// Create or attach to a VM
	if (jniGetVMReference(&pVM, &pEnv)) {			

		// Create the new Objects
		jstring jStrId		= pEnv->NewStringUTF(strId) ;

		// Attach to JVM method

		const char * className = "org/sipfoundry/sipxphone/sys/directoryservice/provider/SpeedDialWebDSP" ;
		jclass eventClass = pEnv->FindClass(className) ;
		if (eventClass != NULL) {
			jmethodID deleteMethod = pEnv->GetStaticMethodID(eventClass, "deleteEntry", "(Ljava/lang/String;)V") ;
			
			if (deleteMethod != NULL) {
				pEnv->CallStaticVoidMethod(eventClass, deleteMethod, jStrId) ;
			} else {
				osPrintf("JNI: deleteSpeeddialEntry couldn't get method\n") ;
			}
		
		} else {
			osPrintf("JNI: deleteSpeeddialEntry couldn't get class\n") ;
		}

		jniReleaseVMReference(pVM) ;	
	} else {
		osPrintf("JNI: deleteSpeeddialEntry couldn't get VM ref\n") ;
	}

   jniResetPriority(iPriority) ;

#if DEBUG_WEB_UI
	if (IsJNIDebuggingEnabled())
		osPrintf("END deleteSpeeddialEntry method\n") ;
#endif


}

/////
/*
 * Method:    deleteAllSpeeddialEntries
 */
extern "C" 
void JNI_deleteAllSpeeddialEntries ()
{

#if DEBUG_WEB_UI
	if (IsJNIDebuggingEnabled())
		osPrintf("BEGIN JNI_deleteAllSpeeddialEntries method\n") ;
#endif

	// Create or attach to the VM
	JavaVM* pVM ;
	JNIEnv* pEnv ;
   
   int iPriority = jniGetCurrentPriority() ; 

	// Create or attach to a VM
	if (jniGetVMReference(&pVM, &pEnv)) {			

		// Create the new Objects
		
		// Attach to JVM method

		const char * className = "org/sipfoundry/sipxphone/sys/directoryservice/provider/SpeedDialWebDSP" ;
		jclass eventClass = pEnv->FindClass(className) ;
		if (eventClass != NULL) {
			jmethodID deleteMethod = pEnv->GetStaticMethodID(eventClass, "deleteAllEntries", "()V") ;
			
			if (deleteMethod != NULL) {
				pEnv->CallStaticVoidMethod(eventClass, deleteMethod) ;
			} else {
				osPrintf("JNI: JNI_deleteAllSpeeddialEntries couldn't get method\n") ;
			}
		
		} else {
			osPrintf("JNI: JNI_deleteAllSpeeddialEntries couldn't get class\n") ;
		}

		jniReleaseVMReference(pVM) ;	
	} else {
		osPrintf("JNI: JNI_deleteAllSpeeddialEntries couldn't get VM ref\n") ;
	}

   jniResetPriority(iPriority) ;

#if DEBUG_WEB_UI
	if (IsJNIDebuggingEnabled())
		osPrintf("END JNI_deleteAllSpeeddialEntries method\n") ;
#endif

}
////


/*
 * Class:     SpeedDialWebDSP
 * Method:    getSpeeddialIdEntries
 */
extern "C" 
void JNI_getSpeeddialEntries(const char* strType, int iMaxItems, char *entries[], int& iActualItems)
{
#if DEBUG_WEB_UI
	if (IsJNIDebuggingEnabled())
		osPrintf("BEGIN getSpeeddialIdEntries method\n") ;
#endif

	// Create or attach to the VM
	JavaVM* pVM ;
	JNIEnv* pEnv ;

   int iPriority = jniGetCurrentPriority() ; 

	// Create or attach to a VM
	if (jniGetVMReference(&pVM, &pEnv)) {			

		// Attach to JVM method

		const char * className = "org/sipfoundry/sipxphone/sys/directoryservice/provider/SpeedDialWebDSP" ;
		jclass eventClass = pEnv->FindClass(className) ;
		if (eventClass != NULL) {
		
			// Create the new Objects
			jstring jStrType		= pEnv->NewStringUTF(strType) ;
			jmethodID getMethod = pEnv->GetStaticMethodID(eventClass, "getSpeeddialEntries", "(Ljava/lang/String;)[Ljava/lang/String;") ;
			
			if (getMethod != NULL) {
				jobjectArray array = (jobjectArray) pEnv->CallStaticObjectMethod(eventClass, getMethod, jStrType) ;	
				if (pEnv->ExceptionOccurred()) {
					pEnv->ExceptionDescribe() ;
					pEnv->ExceptionClear() ;              
				}
            else
            {
				   int iLength = pEnv->GetArrayLength(array) ;				
				   
				   for (int i=0; (i<iLength) && (i<iMaxItems); i++) {
					   iActualItems++ ;
					   jstring string = (jstring) pEnv->GetObjectArrayElement(array, i) ;
					   if (string != NULL) {
						   char* szString = (char *) pEnv->GetStringUTFChars(string, 0) ;
						   strcpy(entries[i], szString) ;
						   pEnv->ReleaseStringUTFChars(string, szString) ;
					   } else
						   strcpy(entries[i], "") ;
               }
				}

			} else {
				osPrintf("JNI: getSpeeddialIdEntries couldn't get method\n") ;
			}
		
		} else {
			osPrintf("JNI: getSpeeddialIdEntries couldn't get class\n") ;
		}

		jniReleaseVMReference(pVM) ;
	} else {
		osPrintf("JNI: getSpeeddialIdEntries couldn't get VM ref\n") ;
	}

   jniResetPriority(iPriority) ;

#if DEBUG_WEB_UI
	if (IsJNIDebuggingEnabled())
		osPrintf("END deleteSpeeddialEntry method\n") ;
#endif


}


//getNumberOfEntries
/*
 * Class:     SpeedDialWebDSP
 * Method:    getNumberOfEntries
 */
extern "C" 
int JNI_getNumberOfEntries()
{
#if DEBUG_WEB_UI
	if (IsJNIDebuggingEnabled())
		osPrintf("BEGIN getNumberOfEntries method\n") ;
#endif

    int iPriority = jniGetCurrentPriority() ;

	// Create or attach to the VM
	JavaVM* pVM ;
	JNIEnv* pEnv ;
	int     iRC = -1 ;

	// Create or attach to a VM
	if (jniGetVMReference(&pVM, &pEnv)) {			

		// Attach to JVM method

		const char * className = "org/sipfoundry/sipxphone/sys/directoryservice/provider/SpeedDialWebDSP" ;
		jclass eventClass = pEnv->FindClass(className) ;
		if (eventClass != NULL) {
		
			// Create the new Objects
			jmethodID getMethod = pEnv->GetStaticMethodID(eventClass, "getNumberOfEntries", "()I") ;
			
			if (getMethod != NULL) {
				iRC = (jint) pEnv->CallStaticIntMethod(eventClass, getMethod) ;	
				if (pEnv->ExceptionOccurred()) {
					pEnv->ExceptionDescribe() ;
					pEnv->ExceptionClear() ;
				}
				
			} else {
				osPrintf("JNI: getNumberOfEntries couldn't get method\n") ;
			}
         
		} else {
			osPrintf("JNI: getNumberOfEntries couldn't get class\n") ;
		}

		jniReleaseVMReference(pVM) ;	
	} else {
		osPrintf("JNI: getNumberOfEntries couldn't get VM ref\n") ;
	}

   jniResetPriority(iPriority) ;

#if DEBUG_WEB_UI
	if (IsJNIDebuggingEnabled())
		osPrintf("END getNumberOfEntries method\n") ;
#endif
    

	return iRC ;
}


/*
 * Class:     SpeedDialWebDSP
 * Method:    doesIDAlreadyExist
 */
extern "C" 
int JNI_doesIDAlreadyExist(const char* strID)
{
#if DEBUG_WEB_UI
	if (IsJNIDebuggingEnabled())
		osPrintf("BEGIN doesIDAlreadyExist method\n") ;
#endif

    int iPriority = jniGetCurrentPriority() ;

	// Create or attach to the VM
	JavaVM* pVM ;
	JNIEnv* pEnv ;
	int     iRC = -1 ;

	// Create or attach to a VM
	if (jniGetVMReference(&pVM, &pEnv)) {			

		// Attach to JVM method

		const char * className = "org/sipfoundry/sipxphone/sys/directoryservice/provider/SpeedDialWebDSP" ;
		jclass eventClass = pEnv->FindClass(className) ;
		if (eventClass != NULL) {
		
			// Create the new Objects
			jmethodID getMethod = pEnv->GetStaticMethodID(eventClass, "doesIDAlreadyExist", "(Ljava/lang/String;)I") ;
			
			if (getMethod != NULL) {
				jstring jStrID		    = pEnv->NewStringUTF(strID); 
				iRC = (jint) pEnv->CallStaticIntMethod(eventClass, getMethod, jStrID) ;	
				if (pEnv->ExceptionOccurred()) {
					pEnv->ExceptionDescribe() ;
					pEnv->ExceptionClear() ;
				}
				
			} else {
				osPrintf("JNI: doesIDAlreadyExist couldn't get method\n") ;
			}
		
		} else {
			osPrintf("JNI: doesIDAlreadyExist couldn't get class\n") ;
		}

		jniReleaseVMReference(pVM) ;	
	} else {
		osPrintf("JNI: doesIDAlreadyExist couldn't get VM ref\n") ;
	}

   jniResetPriority(iPriority) ;

#if DEBUG_WEB_UI
	if (IsJNIDebuggingEnabled())
		osPrintf("END doesIDAlreadyExist method\n") ;
#endif

	return iRC ;
}



/*
 * Class:     SpeedDialWebDSP
 * Method:    isDataSourceReadOnly
 */
extern "C" 
int JNI_isSpeedDialReadOnly()
{
#if DEBUG_WEB_UI
	if (IsJNIDebuggingEnabled())
		osPrintf("BEGIN isSpeedDialReadOnly method\n") ;
#endif    

	// Create or attach to the VM
	JavaVM* pVM ;
	JNIEnv* pEnv ;
	int     iRC = -1 ;

   int iPriority = jniGetCurrentPriority() ;

	// Create or attach to a VM
	if (jniGetVMReference(&pVM, &pEnv)) {			

		// Attach to JVM method

		const char * className = "org/sipfoundry/sipxphone/sys/directoryservice/provider/SpeedDialWebDSP" ;
		jclass eventClass = pEnv->FindClass(className) ;
		if (eventClass != NULL) {
		
			// Create the new Objects
			jmethodID getMethod = pEnv->GetStaticMethodID(eventClass, "isDataSourceReadOnly", "()I") ;
			
			if (getMethod != NULL) {
				iRC = (jint) pEnv->CallStaticIntMethod(eventClass, getMethod) ;	
				if (pEnv->ExceptionOccurred()) {
					pEnv->ExceptionDescribe() ;
					pEnv->ExceptionClear() ;
				}
				
			} else {
				osPrintf("JNI: isSpeedDialReadOnly couldn't get method\n") ;
			}
		
		} else {
			osPrintf("JNI: isSpeedDialReadOnly couldn't get class\n") ;
		}

		jniReleaseVMReference(pVM) ;	
	} else {
		osPrintf("JNI: isSpeedDialReadOnly couldn't get VM ref\n") ;
	}

   jniResetPriority(iPriority) ;

#if DEBUG_WEB_UI
	if (IsJNIDebuggingEnabled())
		osPrintf("END isSpeedDialReadOnly method\n") ;
#endif
    
	return iRC ;
}



/*
 * Class:     SpeedDialWebDSP
 * Method:    getNextAvailableID
 */
extern "C" 
int JNI_getNextAvailableID()
{
#if DEBUG_WEB_UI
	if (IsJNIDebuggingEnabled())
		osPrintf("BEGIN getNextAvailableID method\n") ;
#endif    

	// Create or attach to the VM
	JavaVM* pVM ;
	JNIEnv* pEnv ;
	int     iRC = -1 ;

   int iPriority = jniGetCurrentPriority() ;

	// Create or attach to a VM
	if (jniGetVMReference(&pVM, &pEnv)) {			

		// Attach to JVM method

		const char * className = "org/sipfoundry/sipxphone/sys/directoryservice/provider/SpeedDialWebDSP" ;
		jclass eventClass = pEnv->FindClass(className) ;
		if (eventClass != NULL) {
		
			// Create the new Objects
			jmethodID getMethod = pEnv->GetStaticMethodID(eventClass, "getNextAvailableID", "()I") ;
			
			if (getMethod != NULL) {
				iRC = (jint) pEnv->CallStaticIntMethod(eventClass, getMethod) ;	
				if (pEnv->ExceptionOccurred()) {
					pEnv->ExceptionDescribe() ;
					pEnv->ExceptionClear() ;
				}
				
			} else {
				osPrintf("JNI: getNextAvailableID couldn't get method\n") ;
			}
		
		} else {
			osPrintf("JNI: getNextAvailableID couldn't get class\n") ;
		}

		jniReleaseVMReference(pVM) ;	
	} else {
		osPrintf("JNI: getNextAvailableID couldn't get VM ref\n") ;
	}

   jniResetPriority(iPriority) ;

#if DEBUG_WEB_UI
	if (IsJNIDebuggingEnabled())
		osPrintf("END getNextAvailableID method\n") ;
#endif
    
	return iRC ;
}


/*
 * Class:     SpeedDialWebDSP
 * Method:    validateEntry
 */
extern "C" 
char* JNI_validateEntry(const char* strID, const char* strLabel, 
					  const char* strPhoneNumber, const char* strURL, 
					  int isPhoneNumber , const char* strNewLine)
{
#if DEBUG_WEB_UI
	if (IsJNIDebuggingEnabled())
		osPrintf("BEGIN validateEntry method\n") ;
#endif
	
	// Create or attach to the VM
	JavaVM* pVM ;
	JNIEnv* pEnv ;
	jstring  strError;
	char* szString = NULL ;

   int iPriority = jniGetCurrentPriority() ;	

	// Create or attach to a VM
	if (jniGetVMReference(&pVM, &pEnv)) {			

		// Attach to JVM method

		const char * className = "org/sipfoundry/sipxphone/sys/directoryservice/provider/SpeedDialWebDSP" ;
		jclass eventClass = pEnv->FindClass(className) ;
		if (eventClass != NULL) {
		
			// Create the new Objects
			jmethodID getMethod = 
				pEnv->GetStaticMethodID
					  (eventClass, "validateEntry", 
					 "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZLjava/lang/String;)Ljava/lang/String;") ;
			
			if (getMethod != NULL) {
				jstring jStrID		    = pEnv->NewStringUTF(strID); 
				jstring jStrLabel	    = pEnv->NewStringUTF(strLabel); 
				jstring jStrPhoneNumber = pEnv->NewStringUTF(strPhoneNumber); 
				jstring jStrURL		    = pEnv->NewStringUTF(strURL); 
				jstring jStrNewLine		= pEnv->NewStringUTF(strNewLine);
				jboolean jIsPhoneNumber ;
				if( isPhoneNumber == 0 )
					jIsPhoneNumber = JNI_FALSE;
				else	
					jIsPhoneNumber = JNI_TRUE;	

				strError = (jstring) pEnv->CallStaticObjectMethod(eventClass, getMethod, 
									jStrID, jStrLabel, jStrPhoneNumber, 
									jStrURL, jIsPhoneNumber, jStrNewLine) ;
					
				
				
				if (pEnv->ExceptionOccurred()) {
					pEnv->ExceptionDescribe() ;
					pEnv->ExceptionClear() ;
					szString  = NULL ;
				}
            else
            {
				   if (strError != NULL) {	
					   szString = (char *) pEnv->GetStringUTFChars(strError, 0) ;
				       //strcpy(returnString, szString) ;
					   //pEnv->ReleaseStringUTFChars(strError, szString) ;
				   }
            }
			} else {
				osPrintf("JNI: validateEntry couldn't get method\n") ;
			}
		
		} else {
			osPrintf("JNI: validateEntry couldn't get class\n") ;
		}

		jniReleaseVMReference(pVM) ;	
	} else {
		osPrintf("JNI:  validateEntry couldn't get VM ref\n") ;
	}

   jniResetPriority(iPriority) ;

#if DEBUG_WEB_UI
	if (IsJNIDebuggingEnabled())
		osPrintf("END validateEntry method\n") ;
#endif
   
   return szString;	
}



