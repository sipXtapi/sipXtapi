// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_stapi_PMediaPlaylistPlayer.cpp#2 $
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
#include "mp/MpStreamPlaylistPlayer.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"
#include "ptapi/PtTerminalConnection.h"
#include "ptapi/PtDefs.h"


/*
 * Class:     org_sipfoundry_stapi_PMediaPlaylistPlayer
 * Method:    JNI_createPlaylistPlayer
 * Signature: (JI)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_stapi_PMediaPlaylistPlayer_JNI_1createPlaylistPlayer
  (JNIEnv* pEnv, jclass clazz, jlong lTCHandle, jint unused)
{
   MpStreamPlaylistPlayer* pPlayer = NULL ;

   JNI_BEGIN_METHOD("Java_org_sipfoundry_stapi_PMediaPlaylistPlayer_JNI_1createPlaylistPlayer") ;

   PtTerminalConnection* pTC = (PtTerminalConnection*) (unsigned int) lTCHandle ;
   if (pTC != NULL)
   {
      
      PtStatus status = pTC->createPlaylistPlayer(&pPlayer) ;
      if (status != 0) 
      {
         API_FAILURE(status) ;
      }

      // HACK, HACK: we need to block in the lower layers, not poll here...
      int tries=5;
      while ((pPlayer == NULL) && (tries-- > 0))
         OsTask::delay(250) ;                  
   }

   JNI_END_METHOD() ;

   return (long) pPlayer ;
}


/*
 * Class:     org_sipfoundry_stapi_PMediaPlaylistPlayer
 * Method:    JNI_destroyPlaylistPlayer
 * Signature: (JJ)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_stapi_PMediaPlaylistPlayer_JNI_1destroyPlaylistPlayer
  (JNIEnv* pEnv, jclass clazz, jlong lTCHandle, jlong lHandle)
{
   JNI_BEGIN_METHOD("Java_org_sipfoundry_stapi_PMediaPlaylistPlayer_JNI_1destroyPlaylistPlayer") ;

   MpStreamPlaylistPlayer* pPlayer = (MpStreamPlaylistPlayer*) (unsigned int) lHandle ;
   PtTerminalConnection* pTC = (PtTerminalConnection*) (unsigned int)  lTCHandle ;

   if (pPlayer && pTC)
   {
      PtStatus status = pTC->destroyPlaylistPlayer(pPlayer) ;
      if (status != 0) 
      {
         API_FAILURE(status) ;
      }
   }

   JNI_END_METHOD() ;
}


/*
 * Class:     org_sipfoundry_stapi_PMediaPlaylistPlayer
 * Method:    JNI_addUrl
 * Signature: (JLjava/lang/String;I)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_stapi_PMediaPlaylistPlayer_JNI_1addUrl
  (JNIEnv* pEnv, jclass clazz, jlong lHandle, jstring jsUrl, jint flags)
{
   JNI_BEGIN_METHOD("Java_org_sipfoundry_stapi_PMediaPlaylistPlayer_JNI_1addUrl") ;

   MpStreamPlaylistPlayer* pPlayer = (MpStreamPlaylistPlayer*) (unsigned int) lHandle ;
   const char *szUrl = JSTRTOCSTR(pEnv, jsUrl);
   if (pPlayer != NULL)
   {
      Url audioUrl(szUrl);
      pPlayer->add(audioUrl, flags) ;
   }
   RELEASECSTR(pEnv, jsUrl, szUrl) ;

   JNI_END_METHOD() ;
}

/*
 * Class:     org_sipfoundry_stapi_PMediaPlaylistPlayer
 * Method:    JNI_addBuffer
 * Signature: ([BI)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_stapi_PMediaPlaylistPlayer_JNI_1addBuffer
  (JNIEnv* pEnv, jclass clazz, jbyteArray bytes, jint unused)
{

}


/*
 * Class:     org_sipfoundry_stapi_PMediaPlaylistPlayer
 * Method:    JNI_realize
 * Signature: (JZ)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_stapi_PMediaPlaylistPlayer_JNI_1realize
  (JNIEnv* pEnv, jclass clazz, jlong lHandle, jboolean jbBlock)
{
   JNI_BEGIN_METHOD("Java_org_sipfoundry_stapi_PMediaPlaylistPlayer_JNI_1realize") ;
   
   MpStreamPlaylistPlayer* pPlayer = (MpStreamPlaylistPlayer*) (unsigned int) lHandle ;
   if (pPlayer != NULL)
   {
      pPlayer->realize(jbBlock) ;
   }

   JNI_END_METHOD() ;
}


/*
 * Class:     org_sipfoundry_stapi_PMediaPlaylistPlayer
 * Method:    JNI_prefetch
 * Signature: (JZ)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_stapi_PMediaPlaylistPlayer_JNI_1prefetch
  (JNIEnv* pEnv, jclass clazz, jlong lHandle, jboolean jbBlock)
{
   JNI_BEGIN_METHOD("Java_org_sipfoundry_stapi_PMediaPlaylistPlayer_JNI_1prefetch") ;
   
   MpStreamPlaylistPlayer* pPlayer = (MpStreamPlaylistPlayer*) (unsigned int) lHandle ;
   if (pPlayer != NULL)
   {
      pPlayer->prefetch(jbBlock) ;
   }

   JNI_END_METHOD() ;
}


/*
 * Class:     org_sipfoundry_stapi_PMediaPlaylistPlayer
 * Method:    JNI_play
 * Signature: (JZ)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_stapi_PMediaPlaylistPlayer_JNI_1play
  (JNIEnv* pEnv, jclass clazz, jlong lHandle, jboolean jbBlock)
{
   JNI_BEGIN_METHOD("Java_org_sipfoundry_stapi_PMediaPlaylistPlayer_JNI_1play") ;

   MpStreamPlaylistPlayer* pPlayer = (MpStreamPlaylistPlayer*) (unsigned int) lHandle ;
   if (pPlayer != NULL)
   {
      pPlayer->play(jbBlock) ;
   }

   JNI_END_METHOD() ;
}


/*
 * Class:     org_sipfoundry_stapi_PMediaPlaylistPlayer
 * Method:    JNI_pause
 * Signature: (JI)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_stapi_PMediaPlaylistPlayer_JNI_1pause
  (JNIEnv* pEnv, jclass clazz, jlong lHandle, jint unused)
{
   JNI_BEGIN_METHOD("Java_org_sipfoundry_stapi_PMediaPlaylistPlayer_JNI_1pause") ;

   MpStreamPlaylistPlayer* pPlayer = (MpStreamPlaylistPlayer*) (unsigned int) lHandle ;
   if (pPlayer != NULL)
   {
      pPlayer->pause();
   }

   JNI_END_METHOD() ;
}


/*
 * Class:     org_sipfoundry_stapi_PMediaPlaylistPlayer
 * Method:    JNI_stop
 * Signature: (JI)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_stapi_PMediaPlaylistPlayer_JNI_1stop
  (JNIEnv* pEnv, jclass  clazz, jlong lHandle, jint unused)
{
   JNI_BEGIN_METHOD("Java_org_sipfoundry_stapi_PMediaPlaylistPlayer_JNI_1stop") ;

   MpStreamPlaylistPlayer* pPlayer = (MpStreamPlaylistPlayer*) (unsigned int) lHandle ;
   if (pPlayer != NULL)
   {
      pPlayer->stop();
   }

   JNI_END_METHOD() ;
}


/*
 * Class:     org_sipfoundry_stapi_PMediaPlaylistPlayer
 * Method:    JNI_getState
 * Signature: (JI)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_stapi_PMediaPlaylistPlayer_JNI_1getState
  (JNIEnv* pEnv, jclass  clazz, jlong lHandle, jint unused)
{
   jint iRC = 0 ;

   JNI_BEGIN_METHOD("Java_org_sipfoundry_stapi_PMediaPlaylistPlayer_JNI_1getState") ;

   MpStreamPlaylistPlayer* pPlayer = (MpStreamPlaylistPlayer*) (unsigned int) lHandle ;
   if (pPlayer != NULL)
   {
      PlayerState state ;
      pPlayer->getState(state) ;
      iRC = state ;
   }

   JNI_END_METHOD() ;

   return iRC ;
}
