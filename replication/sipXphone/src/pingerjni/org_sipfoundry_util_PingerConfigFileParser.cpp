// $Id$
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
#include "pinger/Pinger.h"
#include "net/SipUserAgent.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"


/*
 * Class:     org_sipfoundry_util_PingerConfigFileParser
 * Method:    loadConfigDbFromFile
 * Signature: (Ljava/lang/String;)[B
 */
extern "C" 
JNIEXPORT jbyteArray JNICALL Java_org_sipfoundry_util_PingerConfigFileParser_loadConfigDbFromFile
  (JNIEnv *pEnv, jclass clazz, jstring jfilename)
{
	JNI_BEGIN_METHOD("Java_org_sipfoundry_util_PingerConfigFileParser_loadConfigDbFromFile");
    
    OsStatus status = OS_FAILED;
    jbyteArray jbuff = NULL;
    if (jfilename != NULL && clazz != NULL)
    {
        OsConfigDb cfg;
        char *filename = JSTRTOCSTR(pEnv, jfilename);
        if (filename != NULL)
        {
            status = cfg.loadFromFile(filename);
            if (status == OS_SUCCESS)
            {
                int buffSize = cfg.calculateBufferSize();
                if (buffSize > 0)
                {
                    char *buff = new char[buffSize];
                    cfg.storeToBuffer(buff);
                    int buffLen = strlen(buff);
                    jbuff = pEnv->NewByteArray((jsize)buffLen);
                    pEnv->SetByteArrayRegion(jbuff, 0, buffLen, (jbyte *)buff);
                }
            }
        }
        RELEASECSTR(pEnv, jfilename, filename);
        
    }
    JNI_END_METHOD() ;

    return jbuff;
}

/*
 * Class:     org_sipfoundry_util_PingerConfigFileParser
 * Method:    storeConfigDbToFile
 * Signature: (Ljava/lang/String;[C)V
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_util_PingerConfigFileParser_storeConfigDbToFile
  (JNIEnv *pEnv, jclass clazz, jstring jfilename, jbyteArray jbuff)
{
	JNI_BEGIN_METHOD("Java_org_sipfoundry_util_PingerConfigFileParser_storeConfigDbToFile");

    OsStatus status = OS_FAILED;
    if (clazz != NULL && jfilename != NULL && jbuff != NULL)
    {
        int sz = pEnv->GetArrayLength(jbuff);
        if (sz > 0)
        {
            char* buff = new char[sz + 1];
            pEnv->GetByteArrayRegion(jbuff, 0, sz, (jbyte *)buff);
			buff[sz] = 0;
    
            OsConfigDb cfg;
            status = cfg.loadFromBuffer(buff);
            if (status == OS_SUCCESS)
            {
                char *filename = JSTRTOCSTR(pEnv, jfilename);
                status = cfg.storeToFile(filename);
                RELEASECSTR(pEnv, jfilename, filename);
            }
        }
    }
    
	JNI_END_METHOD();
    return status == OS_SUCCESS;
}





