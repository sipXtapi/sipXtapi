// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_util_SipParser.cpp#2 $
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
#include "net/SipMessage.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"

#include "net/Url.h"

#define MAX_PARAMS 16 


jobjectArray buildStringArray(JNIEnv* pEnv, UtlString strings[], int iItems)
{
    jobjectArray array = NULL ;

    jclass classString = pEnv->FindClass("java/lang/String") ;
    if (classString != NULL)
    {        
        array = pEnv->NewObjectArray(iItems, classString, NULL) ;
        if (array != NULL)
        {
            for (int i=0; i<iItems; i++)
            {
                jstring jsString = CSTRTOJSTR(pEnv, strings[i].data()) ;
                pEnv->SetObjectArrayElement(array, i, jsString) ;
            }
        }
    }

    return array ;
}


/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_createParser
 * Signature: ()J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_util_SipParser_JNI_1createParser
  (JNIEnv *pEnv, jclass clazz)
{
    Url* pUrl = new Url("") ;
    return (jlong) (unsigned int) pUrl ;
}


/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_destroyParser
 * Signature: (JI)V
 */
extern "C" 
JNIEXPORT void JNICALL Java_org_sipfoundry_util_SipParser_JNI_1destroyParser
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jint unused)
{
    Url* pUrl = (Url*) (unsigned int) lHandle ;
    if (pUrl != NULL)
    {
        delete pUrl ;
    }
}


/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_resetContent
 * Signature: (JLjava/lang/String;)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_util_SipParser_JNI_1resetContent
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jstring jsNewContent)
{
    Url* pUrl = (Url*) (unsigned int) lHandle ;
    char* szNewContent = JSTRTOCSTR(pEnv, jsNewContent) ;

    if (pUrl != NULL)
    {
        Url urlNew(szNewContent) ;
        *pUrl = urlNew ;
    }
    
    RELEASECSTR(pEnv, jsNewContent, szNewContent) ;
}


/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_isIncludeAngleBracketsSet
 * Signature: (JI)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_util_SipParser_JNI_1isIncludeAngleBracketsSet
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jint unused)
{
    jboolean bIsSet = FALSE ;

    Url* pUrl = (Url*) (unsigned int) lHandle ;
    if (pUrl != NULL)
    {
        bIsSet = pUrl->isIncludeAngleBracketsSet() ;
    }

    return bIsSet ;    
}

/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_setIncludeAngleBrackets
 * Signature: (JZ)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_util_SipParser_JNI_1setIncludeAngleBrackets
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jboolean bInclude)
{
    Url* pUrl = (Url*) (unsigned int) lHandle ;
    if (pUrl != NULL)
    {
        if (bInclude)
            pUrl->includeAngleBrackets() ;
        else
            pUrl->removeAngleBrackets() ;         
    }
}



/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_render
 * Signature: (JI)Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_util_SipParser_JNI_1render
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jint unused)
{
    jstring jsRC = NULL ;

    Url* pUrl = (Url*) (unsigned int) lHandle ;
    if (pUrl != NULL)
    {
        jsRC = CSTRTOJSTR(pEnv, pUrl->toString().data()) ;        
    }

    return jsRC ;
}


/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_getScheme
 * Signature: (JI)Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_util_SipParser_JNI_1getScheme
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jint unused)
{
    jstring jsRC = NULL ;

    Url* pUrl = (Url*) (unsigned int) lHandle ;
    if (pUrl != NULL)
    {
        UtlString strRC ;

        pUrl->getUrlType(strRC) ;
        jsRC = CSTRTOJSTR(pEnv, strRC.data()) ;        
    }

    return jsRC ;
}


/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_setScheme
 * Signature: (JLjava/lang/String;)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_util_SipParser_JNI_1setScheme
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jstring jsScheme)
{
    char* szScheme = JSTRTOCSTR(pEnv, jsScheme) ;
    Url* pUrl = (Url*) (unsigned int) lHandle ;

    if ((szScheme != NULL) && (pUrl != NULL))
    {
        pUrl->setUrlType(szScheme) ;
    }   

    RELEASECSTR(pEnv, jsScheme, szScheme) ;
}

/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_getHost
 * Signature: (JI)Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_util_SipParser_JNI_1getHost
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jint unused)
{
    jstring jsRC = NULL ;

    Url* pUrl = (Url*) (unsigned int) lHandle ;
    if (pUrl != NULL)
    {
        UtlString strRC ;

        pUrl->getHostAddress(strRC) ;
        jsRC = CSTRTOJSTR(pEnv, strRC.data()) ;        
    }

    return jsRC ;
}


/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_getIdentity
 * Signature: (JI)Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_util_SipParser_JNI_1getIdentity
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jint unused)
{
    jstring jsRC = NULL ;

    Url* pUrl = (Url*) (unsigned int) lHandle ;
    if (pUrl != NULL)
    {
        UtlString strRC ;

        pUrl->getIdentity(strRC) ;
        jsRC = CSTRTOJSTR(pEnv, strRC.data()) ;        
    }

    return jsRC ;
}

/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_setHost
 * Signature: (JLjava/lang/String;)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_util_SipParser_JNI_1setHost
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jstring jsHost)
{
    char* szHost = JSTRTOCSTR(pEnv, jsHost) ;
    Url* pUrl = (Url*) (unsigned int) lHandle ;

    if ((szHost != NULL) && (pUrl != NULL))
    {
        pUrl->setHostAddress(szHost) ;
    }   

    RELEASECSTR(pEnv, jsHost, szHost) ;
}

/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_getPort
 * Signature: (JI)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_util_SipParser_JNI_1getPort
  (JNIEnv* pEnv, jclass clazz, jlong lHandle, jint unused)
{
    int iRC = 0 ;

    Url* pUrl = (Url*) (unsigned int) lHandle ;
    if (pUrl != NULL)
    {
        iRC = pUrl->getHostPort() ;
    }

    return iRC ;
}


/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_setPort
 * Signature: (JI)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_util_SipParser_JNI_1setPort
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jint iPort)
{
    Url* pUrl = (Url*) (unsigned int) lHandle ;
    if (pUrl != NULL)
    {
        pUrl->setHostPort(iPort) ;
    }
}

/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_getUser
 * Signature: (JI)Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_util_SipParser_JNI_1getUser
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jint unused)
{
    jstring jsRC = NULL ;

    Url* pUrl = (Url*) (unsigned int) lHandle ;
    if (pUrl != NULL)
    {
        UtlString strRC ;

        pUrl->getUserId(strRC) ;
        jsRC = CSTRTOJSTR(pEnv, strRC.data()) ;        
    }

    return jsRC ;
}

/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_setUser
 * Signature: (JLjava/lang/String;)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_util_SipParser_JNI_1setUser
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jstring jsUser)
{
    char* szUser = JSTRTOCSTR(pEnv, jsUser) ;
    Url* pUrl = (Url*) (unsigned int) lHandle ;

    if ((szUser != NULL) && (pUrl != NULL))
    {
        pUrl->setUserId(szUser) ;
    }   

    RELEASECSTR(pEnv, jsUser, szUser) ;
}

/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_getDisplayName
 * Signature: (JI)Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_util_SipParser_JNI_1getDisplayName
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jint unused)
{
    jstring jsRC = NULL ;

    Url* pUrl = (Url*) (unsigned int) lHandle ;
    if (pUrl != NULL)
    {
        UtlString strRC ;

        pUrl->getDisplayName(strRC) ;
        jsRC = CSTRTOJSTR(pEnv, strRC.data()) ;        
    }

    return jsRC ;
}

/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_setDisplayName
 * Signature: (JLjava/lang/String;)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_util_SipParser_JNI_1setDisplayName
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jstring jsDisplayName)
{
    char* szDisplayName = JSTRTOCSTR(pEnv, jsDisplayName) ;
    Url* pUrl = (Url*) (unsigned int) lHandle ;

    if ((szDisplayName != NULL) && (pUrl != NULL))
    {
        pUrl->setDisplayName(szDisplayName) ;
    }   

    RELEASECSTR(pEnv, jsDisplayName, szDisplayName) ;
}


/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_getFieldParameters
 * Signature: (JI)[Ljava/lang/String;
 */
extern "C"
JNIEXPORT jobjectArray JNICALL Java_org_sipfoundry_util_SipParser_JNI_1getFieldParameters
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jint unused)
{
    jobjectArray objRC = NULL ;

    Url* pUrl = (Url*) (unsigned int) lHandle ;
    if (pUrl != NULL)
    {
        UtlString names[MAX_PARAMS] ;
        UtlString values[MAX_PARAMS] ;
        int iActual = 0 ;

        pUrl->getFieldParameters(MAX_PARAMS, names, values, iActual) ;
        objRC = buildStringArray(pEnv, names, iActual) ;
    }    
    return objRC ;
}

/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_getURLParameters
 * Signature: (JI)[Ljava/lang/String;
 */
extern "C"
JNIEXPORT jobjectArray JNICALL Java_org_sipfoundry_util_SipParser_JNI_1getURLParameters
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jint unused)
{
    jobjectArray objRC = NULL ;

    Url* pUrl = (Url*) (unsigned int) lHandle ;
    if (pUrl != NULL)
    {
        UtlString names[MAX_PARAMS] ;
        UtlString values[MAX_PARAMS] ;
        int iActual = 0 ;

        pUrl->getUrlParameters(MAX_PARAMS, names, values, iActual) ;
        objRC = buildStringArray(pEnv, names, iActual) ;
    }    
    return objRC ;
    
}


/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_getHeaderParameters
 * Signature: (JI)[Ljava/lang/String;
 */
extern "C"
JNIEXPORT jobjectArray JNICALL Java_org_sipfoundry_util_SipParser_JNI_1getHeaderParameters
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jint unused)
{
    jobjectArray objRC = NULL ;

    Url* pUrl = (Url*) (unsigned int) lHandle ;
    if (pUrl != NULL)
    {
        UtlString names[MAX_PARAMS] ;
        UtlString values[MAX_PARAMS] ;
        int iActual = 0 ;

        pUrl->getHeaderParameters(MAX_PARAMS, names, values, iActual) ;
        objRC = buildStringArray(pEnv, names, iActual) ;
    }    
    return objRC ;    
}


/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_getFieldParameterValues
 * Signature: (JLjava/lang/String;)[Ljava/lang/String;
 */
extern "C"
JNIEXPORT jobjectArray JNICALL Java_org_sipfoundry_util_SipParser_JNI_1getFieldParameterValues
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jstring jsName)
{
    jobjectArray objRC = NULL ;
    char* szName = JSTRTOCSTR(pEnv, jsName) ;

    Url* pUrl = (Url*) (unsigned int) lHandle ;
    if ((pUrl != NULL) && (szName != NULL))
    {
        UtlString values[MAX_PARAMS] ;
        int index = 0 ;
        while ((index < MAX_PARAMS) && 
                pUrl->getFieldParameter(szName, values[index], index))
        {
            index++ ;   
        }

        objRC = buildStringArray(pEnv, values, index) ;
    }    

    RELEASECSTR(pEnv, jsName, szName) ;

    return objRC ;    
}


/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_getURLParameterValues
 * Signature: (JLjava/lang/String;)[Ljava/lang/String;
 */
extern "C"
JNIEXPORT jobjectArray JNICALL Java_org_sipfoundry_util_SipParser_JNI_1getURLParameterValues
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jstring jsName)
{
    jobjectArray objRC = NULL ;
    char* szName = JSTRTOCSTR(pEnv, jsName) ;

    Url* pUrl = (Url*) (unsigned int) lHandle ;
    if ((pUrl != NULL) && (szName != NULL))
    {
        UtlString values[MAX_PARAMS] ;
        int index = 0 ;
        while ((index < MAX_PARAMS) && 
                pUrl->getUrlParameter(szName, values[index], index))
        {
            index++ ;   
        }

        objRC = buildStringArray(pEnv, values, index) ;
    }    

    RELEASECSTR(pEnv, jsName, szName) ;

    return objRC ;
}


/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_getHeaderParameterValues
 * Signature: (JLjava/lang/String;)[Ljava/lang/String;
 */
extern "C"
JNIEXPORT jobjectArray JNICALL Java_org_sipfoundry_util_SipParser_JNI_1getHeaderParameterValues
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jstring jsName)
{
    jobjectArray objRC = NULL ;
    char* szName = JSTRTOCSTR(pEnv, jsName) ;

    Url* pUrl = (Url*) (unsigned int) lHandle ;
    if ((pUrl != NULL) && (szName != NULL))
    {
        UtlString values[MAX_PARAMS] ;
        int index = 0 ;
        while ((index < MAX_PARAMS) && 
                pUrl->getHeaderParameter(szName, values[index], index))
        {
            index++ ;   
        }

        objRC = buildStringArray(pEnv, values, index) ;
    }    

    RELEASECSTR(pEnv, jsName, szName) ;

    return objRC ;
}


/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_addFieldParameter
 * Signature: (JLjava/lang/String;Ljava/lang/String;)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_util_SipParser_JNI_1addFieldParameter
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jstring jsName, jstring jsValue)
{
    char* szName = JSTRTOCSTR(pEnv, jsName) ;
    char* szValue = JSTRTOCSTR(pEnv, jsValue) ;
    Url*  pUrl = (Url*) (unsigned int) lHandle ;
  
    if ((pUrl != NULL) && (szName != NULL) && (szValue != NULL))
    {
        pUrl->setFieldParameter(szName, szValue) ;
    }    
}


/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_addURLParameter
 * Signature: (JLjava/lang/String;Ljava/lang/String;)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_util_SipParser_JNI_1addURLParameter
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jstring jsName, jstring jsValue)
{
    char* szName = JSTRTOCSTR(pEnv, jsName) ;
    char* szValue = JSTRTOCSTR(pEnv, jsValue) ;
    Url*  pUrl = (Url*) (unsigned int) lHandle ;
  
    if ((pUrl != NULL) && (szName != NULL) && (szValue != NULL))
    {
        pUrl->setUrlParameter(szName, szValue) ;
    }        
}


/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_addHeaderParameter
 * Signature: (JLjava/lang/String;Ljava/lang/String;)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_util_SipParser_JNI_1addHeaderParameter
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jstring jsName, jstring jsValue)
{
    char* szName = JSTRTOCSTR(pEnv, jsName) ;
    char* szValue = JSTRTOCSTR(pEnv, jsValue) ;
    Url*  pUrl = (Url*) (unsigned int) lHandle ;
  
    if ((pUrl != NULL) && (szName != NULL) && (szValue != NULL))
    {
        pUrl->setHeaderParameter(szName, szValue) ;
    }            
}


/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_removeFieldParameters
 * Signature: (JLjava/lang/String;)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_util_SipParser_JNI_1removeFieldParameters
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jstring jsName)
{
    char* szName = JSTRTOCSTR(pEnv, jsName) ;
    Url*  pUrl = (Url*) (unsigned int) lHandle ;
  
    if ((pUrl != NULL) && (szName != NULL))
    {
        pUrl->removeFieldParameter(szName) ;
    }
}


/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_removeURLParameters
 * Signature: (JLjava/lang/String;)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_util_SipParser_JNI_1removeURLParameters
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jstring jsName)
{
    char* szName = JSTRTOCSTR(pEnv, jsName) ;
    Url*  pUrl = (Url*) (unsigned int) lHandle ;
  
    if ((pUrl != NULL) && (szName != NULL))
    {
        pUrl->removeUrlParameter(szName) ;
    }
}


/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_removeHeaderParameters
 * Signature: (JLjava/lang/String;)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_util_SipParser_JNI_1removeHeaderParameters
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jstring jsName)
{
    char* szName = JSTRTOCSTR(pEnv, jsName) ;
    Url*  pUrl = (Url*) (unsigned int) lHandle ;
  
    if ((pUrl != NULL) && (szName != NULL))
    {
        pUrl->removeHeaderParameter(szName) ;
    }
}


/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_removeAllFieldParameters
 * Signature: (JI)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_util_SipParser_JNI_1removeAllFieldParameters
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jint unused)
{
    Url*  pUrl = (Url*) (unsigned int) lHandle ;
    
    if (pUrl != NULL)
    {
        pUrl->removeFieldParameters() ;
    }
}


/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_removeAllURLParameters
 * Signature: (JI)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_util_SipParser_JNI_1removeAllURLParameters
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jint unused)
{
    Url*  pUrl = (Url*) (unsigned int) lHandle ;
    
    if (pUrl != NULL)
    {
        pUrl->removeUrlParameters() ;
    }
}


/*
 * Class:     org_sipfoundry_util_SipParser
 * Method:    JNI_removeAllHeaderParameters
 * Signature: (JI)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_util_SipParser_JNI_1removeAllHeaderParameters
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jint unused)
{
    Url*  pUrl = (Url*) (unsigned int) lHandle ;
    
    if (pUrl != NULL)
    {
        pUrl->removeHeaderParameters() ;
    }
}
