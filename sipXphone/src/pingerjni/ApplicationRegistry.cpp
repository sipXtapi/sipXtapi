// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/ApplicationRegistry.cpp#2 $
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
#ifdef __pingtel_on_posix__
#include <malloc.h>
#endif

#include "os/OsDefs.h"
#include "os/OsSysLog.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/ApplicationRegistry.h"
#include "pingerjni/JNIHelpers.h"
#include "pingerjni/JNIClassPaths.h"

/*
    public static int cgiInstallApplication(java.lang.String);
        // (Ljava/lang/String;)I

	public static int cgiUninstallApplication(java.lang.String);
        // (Ljava/lang/String;)I

    public static java.lang.String cgiQueryInstalledApplications()[];
        // ()[Ljava/lang/String;
*/


/*****************************************************************************
 */
int JNI_installJavaApplication(const char *szClassnameOrURL)
{
	JavaVM* pVM ;
	JNIEnv* pEnv ;
	int     iRC = -2 ;

	int iPriority = jniGetCurrentPriority() ;

	JNI_BEGIN_METHOD("JNI_installJavaApplication") ;

	// Create or attach to a VM
	if (jniGetVMReference(&pVM, &pEnv)) {

		jclass    cls ;
		jmethodID methodid ;

		cls = pEnv->FindClass(APPLICATION_REGISTRY) ;
		if (cls != NULL) 
      {
			jobject jobjString = pEnv->NewStringUTF(szClassnameOrURL) ;
			methodid = pEnv->GetStaticMethodID(cls, "cgiInstallApplication", "(Ljava/lang/String;)I") ;
			if (methodid != NULL) 
         {
				iRC = pEnv->CallStaticIntMethod(cls, methodid, jobjString) ;
				if (pEnv->ExceptionOccurred()) 
            {
					pEnv->ExceptionDescribe() ;
					pEnv->ExceptionClear() ;
					iRC = -6 ;
               LOOKUP_FAILURE() ;
				}
			} 
         else 
         {
				iRC = -5 ;
            LOOKUP_FAILURE() ;
			}
		}
      else 
      {
			iRC = -4 ;
         LOOKUP_FAILURE() ;
		}

		jniReleaseVMReference(pVM) ;			
		jniResetPriority(iPriority) ;
	} 
   else
   {
		iRC = -3 ;
      LOOKUP_FAILURE() ;
   }
  
	JNI_END_METHOD() ;

	return iRC ;
}


/*****************************************************************************
 */
int JNI_uninstallJavaApplication(const char *szClassnameOrURL)
{
	JavaVM* pVM ;
	JNIEnv* pEnv ;
	int     iRC = -2 ;

	int iPriority = jniGetCurrentPriority() ;

	JNI_BEGIN_METHOD("JNI_uninstallJavaApplication") ;

	// Create or attach to a VM
	if (jniGetVMReference(&pVM, &pEnv)) 
   {
		jclass    cls ;
		jmethodID methodid ;

		cls = pEnv->FindClass(APPLICATION_REGISTRY) ;
		if (cls != NULL) 
      {
			jobject jobjString = pEnv->NewStringUTF(szClassnameOrURL) ;
			methodid = pEnv->GetStaticMethodID(cls, "cgiUninstallApplication", "(Ljava/lang/String;)I") ;
			if (methodid != NULL) 
         {
				iRC = pEnv->CallStaticIntMethod(cls, methodid, jobjString) ;
				if (pEnv->ExceptionOccurred()) {
					pEnv->ExceptionDescribe() ;
					pEnv->ExceptionClear() ;
					iRC = -6 ;
               LOOKUP_FAILURE() ;
				}
			} 
         else 
         {
				iRC = -5 ;
            LOOKUP_FAILURE() ;
			}
		}
      else 
      {
			iRC = -4 ;
         LOOKUP_FAILURE() ;
		}

		jniReleaseVMReference(pVM) ;			
		jniResetPriority(iPriority) ;
	} 
   else
   {
		iRC = -3 ;
      LOOKUP_FAILURE() ;
   }

	JNI_END_METHOD() ;

	return iRC ;
}


/*****************************************************************************
 */
void JNI_queryInstalledJavaApplications(int iMaxItems, char *szClassnameOrURL[], int& iActualItems)
{
	JavaVM* pVM ;
	JNIEnv* pEnv ;

	iActualItems = 0 ;
	int iPriority = jniGetCurrentPriority() ;

	JNI_BEGIN_METHOD("JNI_queryInstalledJavaApplications") ;

	// Create or attach to a VM
	if (jniGetVMReference(&pVM, &pEnv)) {

		jclass    cls ;
		jmethodID methodid ;

		cls = pEnv->FindClass(APPLICATION_REGISTRY) ;
		if (cls != NULL) {
			methodid = pEnv->GetStaticMethodID(cls, "cgiQueryInstalledApplications", "()[Ljava/lang/String;") ;
			if (methodid != NULL) {
				jobjectArray array = (jobjectArray) pEnv->CallStaticObjectMethod(cls, methodid) ;	
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
						   strcpy(szClassnameOrURL[i], szString) ;
						   pEnv->ReleaseStringUTFChars(string, szString) ;
					   } else
						   strcpy(szClassnameOrURL[i], "") ;
				   }
            }
			} else {				
            LOOKUP_FAILURE() ;
			}
		} else {
			LOOKUP_FAILURE() ;
		}

		jniReleaseVMReference(pVM) ;			
		jniResetPriority(iPriority) ;

		JNI_END_METHOD() ;
	}
}



/////////////////////
/*****************************************************************************
 */
void JNI_queryInstalledJavaApplications
	(int iMaxItems, APPINFO* pAppInfo, int& iActualItems)
{
	JavaVM* pVM ;
	JNIEnv* pEnv ;

	iActualItems = 0 ;
	int iPriority = jniGetCurrentPriority() ;

	JNI_BEGIN_METHOD("JNI_queryInstalledJavaApplications") ;

	// Create or attach to a VM
	if (jniGetVMReference(&pVM, &pEnv)) {

		jclass    cls ;
		jmethodID methodid ;

		cls = pEnv->FindClass(APPLICATION_REGISTRY) ;
		if (cls != NULL) {
			methodid = pEnv->GetStaticMethodID
					(cls, "cgiQueryInstalledApplicationsForTitleAndURL", "()[[Ljava/lang/String;") ;
			
			if (methodid != NULL) {
				jobjectArray array = 
						(jobjectArray) pEnv->CallStaticObjectMethod(cls, methodid) ;	
				if (pEnv->ExceptionOccurred()) {
					pEnv->ExceptionDescribe() ;
					pEnv->ExceptionClear() ;
				}
            else
            {
				   int iLength = pEnv->GetArrayLength(array) ;
				   for (int i=0; (i<iLength) && (i<iMaxItems); i++) {
					   iActualItems++ ;
					   jobjectArray titleAndURLArray = 
							   (jobjectArray) pEnv->GetObjectArrayElement(array, i) ;
					   
					   jstring stringTitle = 
							   (jstring) pEnv->GetObjectArrayElement(titleAndURLArray, 0) ;
					   
					   jstring stringURL   = 
							   (jstring) pEnv->GetObjectArrayElement(titleAndURLArray, 1) ;
				   
					   if (stringTitle != NULL) {
						   char* szString = (char *) pEnv->GetStringUTFChars(stringTitle, 0) ;
						   strncpy(pAppInfo[i].szTitle, szString, MAX_TITLE_LENGTH) ;
						   pEnv->ReleaseStringUTFChars(stringTitle, szString) ;
					   } else
						   strncpy(pAppInfo[i].szTitle, "", MAX_TITLE_LENGTH) ;
					    
					   if (stringURL != NULL) {
						   char* szString = (char *) pEnv->GetStringUTFChars(stringURL, 0) ;
						   strncpy(pAppInfo[i].szUrl, szString, MAX_URL_LENGTH) ;
						   pEnv->ReleaseStringUTFChars(stringURL, szString) ;
					   } else
						   strncpy(pAppInfo[i].szUrl, "", MAX_URL_LENGTH) ;
					   
					   //osPrintf("title of %dth app is %s" , i, pAppInfo[i].szTitle);
					   //osPrintf("URL is %dth app is %s" , 	i, pAppInfo[i].szUrl);
               }

				}

			} else {
            LOOKUP_FAILURE() ;
			}
		} else {
         LOOKUP_FAILURE() ;
		}

		jniReleaseVMReference(pVM) ;		
		jniResetPriority(iPriority) ;

		JNI_END_METHOD() ;
	}
}
/////////////////////



/*****************************************************************************
 */
void JNI_dumpInstalledJavaApplications()
{
	char* szApps[10] ;
	int   i ;

	for (i=0; i<10; i++) {
		szApps[i] = (char*) malloc(256) ;
		memset(szApps[i], 0, 256) ;
	}

	int iActualApps = 0 ;
	JNI_queryInstalledJavaApplications(10, szApps, iActualApps) ;

	for (i=0; i<iActualApps; i++) {
		osPrintf("%d: %s\n", i, szApps[i]) ;
	}


	for (i=0; i<10; i++) {
		free(szApps[i]) ;
	}
}

/*****************************************************************************
 */
void JNI_queryLoadedApplications(int iMaxItems, char *szClassnameOrURL[], int& iActualItems)
{
	JavaVM* pVM ;
	JNIEnv* pEnv ;

	iActualItems = 0 ;
	int iPriority = jniGetCurrentPriority() ;

	JNI_BEGIN_METHOD("JNI_getLoadedApplications") ;

	// Create or attach to a VM
	if (jniGetVMReference(&pVM, &pEnv)) {

		jclass    cls ;
		jmethodID methodid ;

		cls = pEnv->FindClass(APPLICATION_MANAGER) ;
		if (cls != NULL) {
			methodid = pEnv->GetStaticMethodID(cls, "JNI_getLoadedApplications", "()[Ljava/lang/String;") ;
			if (methodid != NULL) {
				jobjectArray array = (jobjectArray) pEnv->CallStaticObjectMethod(cls, methodid) ;	
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
						   strcpy(szClassnameOrURL[i], szString) ;
						   pEnv->ReleaseStringUTFChars(string, szString) ;
					   } else
						   strcpy(szClassnameOrURL[i], "") ;
				   }
            }

			} else {
            LOOKUP_FAILURE() ;
			}
		} else {
         LOOKUP_FAILURE() ;
		}

		jniReleaseVMReference(pVM) ;			
		jniResetPriority(iPriority) ;

		JNI_END_METHOD() ;
	}
}
    
/**
 *  This gives you a list of all USER apps and the apps that have title
 *  and  icons.
 */
void JNI_queryAllInstalledApplications(int iMaxItems, char *szTitles[], int& iActualItems)
{
	JavaVM* pVM ;
	JNIEnv* pEnv ;

	iActualItems = 0 ;
	int iPriority = jniGetCurrentPriority() ;

	JNI_BEGIN_METHOD("JNI_queryAllInstalledApplications") ;

	// Create or attach to a VM
	if (jniGetVMReference(&pVM, &pEnv)) {

		jclass    cls ;
		jmethodID methodid ;

		cls = pEnv->FindClass(APPLICATION_REGISTRY) ;
		if (cls != NULL) {
			methodid = pEnv->GetStaticMethodID(cls, "cgiQueryAllInstalledApplications", "()[Ljava/lang/String;") ;
			if (methodid != NULL) {
				jobjectArray array = (jobjectArray) pEnv->CallStaticObjectMethod(cls, methodid) ;	
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
						   strcpy(szTitles[i], szString) ;
						   pEnv->ReleaseStringUTFChars(string, szString) ;
					   } else
						   strcpy(szTitles[i], "") ;
               }
				}

			} else {
            LOOKUP_FAILURE() ;
			}
		} else {
         LOOKUP_FAILURE() ;
		}

		jniReleaseVMReference(pVM) ;			
		jniResetPriority(iPriority) ;

		JNI_END_METHOD() ;
	}


}
