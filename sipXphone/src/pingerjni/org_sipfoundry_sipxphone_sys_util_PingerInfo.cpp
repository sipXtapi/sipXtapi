// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_xpressa_sys_util_PingerInfo.cpp#3 $
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
#include "os/OsTime.h"
#include "os/OsDateTime.h"
#include "os/OsUtil.h"
#include "os/OsTask.h"
#include "licensemanager/BaseLicenseManager.h"
#include <net/SipLine.h>
#include <net/SipLineMgr.h>
#include <net/SipRefreshMgr.h>
#include <pinger/Pinger.h>

#include "pinger/PingerInfo.h"
#include "pinger/Pinger.h"
#include "pingerjni/JNIHelpers.h"

extern "C" int GetCallTaskCount();

/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_getFlashFileSystemLocation
 * Signature: ()Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1getFlashFileSystemLocation
  (JNIEnv * pEnv, jclass)
{
	PingerInfo	pingerInfo ;
	jstring		strRet ; 
	UtlString	strFlash ;

	pingerInfo.getFlashFileSystemLocation(strFlash) ;

	strRet = pEnv->NewStringUTF(strFlash.data()) ;

	return strRet ;
}


/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_getTimeInMS
 * Signature: ()J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1getTimeInMS
  (JNIEnv* pEnv, jclass clazz)
{
    OsTime time;

    OsDateTime::getCurTimeSinceBoot(time)  ;
		
	return time.cvtToMsecs() ;                
}



/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_getVersion
 * Signature: ()Ljava/lang/String;
 */
extern "C" 
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1getVersion
  (JNIEnv * pEnv, jclass)
{
	PingerInfo	pingerInfo ;
	jstring		strRet ; 
	UtlString	strValue ;

	pingerInfo.getVersion(strValue) ;

	strRet = pEnv->NewStringUTF(strValue.data()) ;

	return strRet ;
}


/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_getKernelBuildDate
 * Signature: ()Ljava/lang/String;
 */
extern "C" 
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1getKernelBuildDate
  (JNIEnv * pEnv, jclass)
{
	PingerInfo	pingerInfo ;
	jstring		strRet ; 
	UtlString	strValue ;

	pingerInfo.getBuildDate(strValue) ;

	strRet = pEnv->NewStringUTF(strValue.data()) ;

	return strRet ;
}


/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_getKernelBuildComment
 * Signature: ()Ljava/lang/String;
 */
extern "C" 
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1getKernelBuildComment
  (JNIEnv * pEnv, jclass)
{
	PingerInfo	pingerInfo ;
	jstring		strRet ; 
	UtlString	strValue ;

	pingerInfo.getVersionComment(strValue) ;

	strRet = pEnv->NewStringUTF(strValue.data()) ;

	return strRet ;
}


/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_getPlatformType
 * Signature: ()Ljava/lang/String;
 */
extern "C" 
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1getPlatformType
  (JNIEnv * pEnv, jclass)
{
	PingerInfo	pingerInfo ;
	jstring		strRet ; 
	UtlString	strValue ;

	pingerInfo.getPlatformType(strValue) ;

	strRet = pEnv->NewStringUTF(strValue.data()) ;

	return strRet ;
}

/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_getCallTaskCount
 * Signature: ()Z;
 */
extern "C" 
JNIEXPORT jint JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1getCallTaskCount
  (JNIEnv* pEnv, jclass clazz)
{
    jint iRC = 0 ; 

    iRC = GetCallTaskCount() ;

	return iRC ;
}


/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_getBoardType
 * Signature: ()Z;
 */
extern "C" 
JNIEXPORT jint JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1getBoardType
  (JNIEnv* pEnv, jclass clazz)
{
    jint iRC = -1 ; // PBT_UNKNOWN

	return iRC ;
}




/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_getUptime
 * Signature: ()Ljava/lang/String;
 */
extern "C" 
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1getUptime
  (JNIEnv * pEnv, jclass clazz)
{
	jstring		strRet ; 
	UtlString	strUptime;

	PingerInfo::getUptime(strUptime) ;

	strRet = pEnv->NewStringUTF(strUptime.data()) ;

	return strRet ;
}


/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_digest
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
extern "C" 
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1digest
  (JNIEnv * pEnv, jclass, jstring jsDigest)
{
	PingerInfo pingerInfo ;
	UtlString   strDigest ;
	
	const char* digest = (const char*) pEnv->GetStringUTFChars(jsDigest, 0) ;

	pingerInfo.digest(digest, strDigest) ;

	return pEnv->NewStringUTF(strDigest.data()) ;

}

/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_getMacAddress
 * Signature: ()Ljava/lang/String;
 */
extern "C" 
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1getMacAddress
  (JNIEnv * pEnv, jclass)
{
	PingerInfo pingerInfo ;
	UtlString   strMac ; 

	pingerInfo.getMacAddress(strMac) ;
	
	return pEnv->NewStringUTF(strMac.data() ) ;
}


/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_setupGetLogicalIdUrl
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 * 
 * Have two versions of this method, as don't want to pass optional null parameter through JNI layer
 */
 extern "C" 
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1setupGetLogicalIdUrl
  (JNIEnv * pEnv, jclass, jstring jsAdminDomain, jstring jsAdminId, jstring jsAdminPwd, jstring jsSecret, jstring jsLogicalId)
{
	PingerInfo	pingerInfo ;
	UtlString	strURL ;

	const char * adminDomain= (const char *) pEnv->GetStringUTFChars(jsAdminDomain, 0) ;
	const char * adminId	= (const char *) pEnv->GetStringUTFChars(jsAdminId, 0) ;
	const char * adminPwd	= (const char *) pEnv->GetStringUTFChars(jsAdminPwd, 0) ;
	const char * secret		= (const char *) pEnv->GetStringUTFChars(jsSecret, 0) ;
	const char * logicalId  = (const char *) pEnv->GetStringUTFChars(jsLogicalId, 0) ;


	pingerInfo.setupGetLogicalIdUrl(strURL, adminDomain, adminId, adminPwd, secret, logicalId) ;

	return pEnv->NewStringUTF(strURL.data() );

}

 /*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_setupGetLogicalIdUrl2
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 */
 extern "C" 
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1setupGetLogicalIdUrl2
  (JNIEnv * pEnv, jclass, jstring jsAdminDomain, jstring jsAdminId, jstring jsAdminPwd, jstring jsSecret)
{
	PingerInfo	pingerInfo ;
	UtlString	strURL ;

	const char * adminDomain= (const char *) pEnv->GetStringUTFChars(jsAdminDomain, 0) ;
	const char * adminId	= (const char *) pEnv->GetStringUTFChars(jsAdminId, 0) ;
	const char * adminPwd	= (const char *) pEnv->GetStringUTFChars(jsAdminPwd, 0) ;
	const char * secret		= (const char *) pEnv->GetStringUTFChars(jsSecret, 0) ;

	pingerInfo.setupGetLogicalIdUrl(strURL, adminDomain, adminId, adminPwd, secret) ;

	return pEnv->NewStringUTF(strURL.data() );

}

/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_setupGetConfigUrl
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 */
 extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1setupGetConfigUrl
  (JNIEnv * pEnv, jclass , jstring jsAdminDomain, jstring jsLogicalId, jstring jsSecret)
{
	PingerInfo	pingerInfo ;
	UtlString	strURL ;

	const char * adminDomain= (const char *) pEnv->GetStringUTFChars(jsAdminDomain, 0) ;
	const char * logicalId	= (const char *) pEnv->GetStringUTFChars(jsLogicalId, 0) ;
	const char * secret		= (const char *) pEnv->GetStringUTFChars(jsSecret, 0) ;

	pingerInfo.setupGetConfigUrl(strURL, adminDomain, logicalId, secret) ;

	return pEnv->NewStringUTF(strURL.data() );

}


/* Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_getAdminDomain
 * Signature: ()Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1getAdminDomain
  (JNIEnv * pEnv, jclass)
{
	UtlString strDomain = "";
    
    #if defined(_WIN32) || defined(__pingtel_on_posix__)
	if (OsUtil::getProductType() == OsUtil::PRODUCT_INSTANT_XPRESSA)
    {
        BaseLicenseManager *lm = BaseLicenseManager::getInstance();
        lm->getAdminDomain(strDomain) ;
    }
	#endif

	return pEnv->NewStringUTF(strDomain.data() );
}


/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_getExpireDate
 * Signature: ()Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1getExpireDate
  (JNIEnv * pEnv, jclass)
{
	UtlString expTime = "";

    #if defined(_WIN32) || defined(__pingtel_on_posix__) 
    if (OsUtil::getProductType() == OsUtil::PRODUCT_INSTANT_XPRESSA)
    {
        BaseLicenseManager *lm = BaseLicenseManager::getInstance();
        lm->getExpireDateAsStr(expTime);

    }
	#endif
	
	return pEnv->NewStringUTF(expTime.data() );
}


/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_getHostName
 * Signature: ()Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1getHostName
  (JNIEnv * pEnv, jclass)
{
	UtlString strHost = "";

	#if defined(_WIN32) || defined(__pingtel_on_posix__) 
    if (OsUtil::getProductType() == OsUtil::PRODUCT_INSTANT_XPRESSA)
    {
        BaseLicenseManager *lm = BaseLicenseManager::getInstance();
        lm->getHostName(strHost) ;
    }
    #endif
	
	return pEnv->NewStringUTF(strHost.data() );
}

/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_getKey
 * Signature: ()Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1getKey
  (JNIEnv * pEnv, jclass)
{
	UtlString strKey = "";

    #if defined(_WIN32) || defined(__pingtel_on_posix__) 
    if (OsUtil::getProductType() == OsUtil::PRODUCT_INSTANT_XPRESSA)
    {
        BaseLicenseManager *lm = BaseLicenseManager::getInstance();
        lm->getKey(strKey) ;
    }
    #endif

	return pEnv->NewStringUTF(strKey.data() );
}


/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_getSerialNum
 * Signature: ()Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1getSerialNum
  (JNIEnv * pEnv, jclass)
{
    PingerInfo pingerInfo;
	UtlString strSerial ;

    pingerInfo.getSerialNum(strSerial);

	return pEnv->NewStringUTF(strSerial.data() );
}

/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_getKeyConfig
 * Signature: (Ljava/lang/String;)Z
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1isLicenseValid
  (JNIEnv * pEnv, jclass)
{
   
    jint retval = 1;
      
    BaseLicenseManager *lm = BaseLicenseManager::getInstance();
    retval = lm->isValid();    

	return (jint) retval ;
}


/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_getLicenseFailureCode
 * Signature: (Ljava/lang/String;)Z
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1getLicenseFailureCode
  (JNIEnv * pEnv, jclass)
{
    
    jint retval = 0;
    
    retval = BaseLicenseManager::getFailureCode();

	return (jint) retval ;
}

/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_getLicenseType
 * Signature: ()Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1getLicenseType
  (JNIEnv * pEnv, jclass)
{
	UtlString strLicenseType = "";

    #if defined(_WIN32) || defined(__pingtel_on_posix__)
	if (OsUtil::getProductType() == OsUtil::PRODUCT_INSTANT_XPRESSA)
    {
        BaseLicenseManager *lm = BaseLicenseManager::getInstance();
        lm->getLicenseType(strLicenseType) ;
    }
    #endif

	return pEnv->NewStringUTF(strLicenseType.data() );
}

/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_getFe
 * Signature: ()Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1getLicenseFeatures
  (JNIEnv * pEnv, jclass)
{
	UtlString strLicenseFeatures = "";

    BaseLicenseManager *lm = BaseLicenseManager::getInstance();
    lm->getFeatures(strLicenseFeatures) ;
 
	return pEnv->NewStringUTF(strLicenseFeatures.data() );
}

/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_getAppLimit
 * Signature: ()jint
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1getAppLimit
  (JNIEnv * pEnv, jclass)
{
    jint retval = -1;

    #if defined(_WIN32) || defined(__pingtel_on_posix__)
	if (OsUtil::getProductType() == OsUtil::PRODUCT_INSTANT_XPRESSA)
    {
        BaseLicenseManager *lm = BaseLicenseManager::getInstance();
        retval = lm->getAppLimit() ;
    }
    #endif
	
	return (jint) retval;
}


/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_startPinger
 * Signature: ()V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1startPinger
  (JNIEnv *, jclass)
{
	Pinger::getPingerTask();
}

/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_stopPinger
 * Signature: ()V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1stopPinger
  (JNIEnv *pEnv, jclass)
{
   Pinger::getPingerTask()->restart(FALSE, -1, "user initiated") ;
}

/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_unSubscribeAll
 * Signature: ()V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1unSubscribeAll
  (JNIEnv *pEnv, jclass)
{
    Pinger *pPinger = Pinger::getPingerTask();
    if (pPinger)
    {
        SipRefreshMgr *pRefreshMgr = pPinger->getRefreshManager();
        if (pRefreshMgr)
        {
            pRefreshMgr->unSubscribeAll();
        }
        else
        {
            osPrintf("ERROR: RefreshMgr wasn't available!\n");
        }
    }

}

/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_stopPinger
 * Signature: ()V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1unregisterAllLines
  (JNIEnv *pEnv, jclass)
{
    Pinger *pPinger = Pinger::getPingerTask();
    if (pPinger)
    {
        SipLineMgr *pLineMgr = pPinger->getLineManager();
        SipRefreshMgr *pRefreshMgr = pPinger->getRefreshManager();
        if (pLineMgr && pRefreshMgr)
        {
            int numLines = pLineMgr->getNumLines();
               // Allocate Lines
            if (numLines > 0)
            {
               SipLine *lines = new SipLine[numLines] ;
           
               // Get Actual lines and disable
               int actualLines = 0;
              if (lines && pLineMgr->getLines(numLines, actualLines, lines))
              {
                 for( int i =0 ; i< actualLines; i++)
                 {
                    if ( lines[i].getState() == SipLine::LINE_STATE_REGISTERED)
                    {
                       pLineMgr->disableLine(lines[i].getIdentity(), TRUE, lines[i].getLineId());//unregister for startup 
                    }
                 }
              }
           
              if (lines)
               delete [] lines;
            }

        }
        else
        {
            osPrintf("ERROR: RefreshMgr or LineMgr wasn't available!\n");
        }
    }   
}

/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_restoreFactoryDefaults
 * Signature: ()Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1restoreFactoryDefaults
  (JNIEnv *pEnv, jclass clazz)
{
	PingerInfo pingerInfo ;

	return pingerInfo.restoreDefaults() ;
}


/* ============================ Pinger Configuration ============================== */

/*  Manipulators  */

/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_getSipVersion
 * Signature: ()Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_sipxphone_sys_util_PingerInfo_JNI_1getSipVersion
  (JNIEnv * pEnv, jclass)
{
	jstring jsRet ;
	UtlString strSipVersion ;
	PingerInfo pingerInfo ;

	pingerInfo.getSipVersion(strSipVersion) ;

	jsRet = pEnv->NewStringUTF(strSipVersion.data()) ;
	return jsRet ;
}

