// $Id: //depot/OPENDEV/sipXphone/include/pingerjni/com_pingtel_DialerHack.h#2 $
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
/* Header for class org_sipfoundry_DialerHack */

#ifndef _Included_org_sipfoundry_DialerHack
#define _Included_org_sipfoundry_DialerHack
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_sipfoundry_DialerHack
 * Method:    Dial
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_sipfoundry_DialerHack_Dial
  (JNIEnv *, jobject, jstring);

/*
 * Class:     org_sipfoundry_DialerHack
 * Method:    OffHook
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_sipfoundry_DialerHack_OffHook
  (JNIEnv *, jobject);

/*
 * Class:     org_sipfoundry_DialerHack
 * Method:    OnHook
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_sipfoundry_DialerHack_OnHook
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
