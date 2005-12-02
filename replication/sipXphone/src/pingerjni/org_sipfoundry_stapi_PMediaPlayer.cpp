// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_stapi_PMediaPlayer.cpp#2 $
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
#include "mp/MpStreamPlayer.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"
#include "ptapi/PtTerminalConnection.h"
#include "ptapi/PtDefs.h"

/*
 * Class:     org_sipfoundry_stapi_PMediaPlayer
 * Method:    JNI_createPlayer
 * Signature: (JLjava/lang/String;I)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_stapi_PMediaPlayer_JNI_1createPlayer
  (JNIEnv* pEnv, jclass clazz, jlong lTCHandle, jstring jsUrl, jint flags)
{
   MpStreamPlayer* pPlayer = NULL ;

   JNI_BEGIN_METHOD("Java_org_sipfoundry_stapi_PMediaPlayer_JNI_1createPlayer") ;

   PtTerminalConnection* pTC = (PtTerminalConnection*) (unsigned int) lTCHandle ;
   if (pTC != NULL)
   {
      const char *szUrl = JSTRTOCSTR(pEnv, jsUrl);
      PtStatus status = pTC->createPlayer(&pPlayer, szUrl, (int) flags) ;
      if (status != 0) 
      {
         API_FAILURE(status) ;
      }

      // HACK, HACK: we need to block in the lower layers, not poll here...
      int tries=5;
      while ((pPlayer == NULL) && (tries-- > 0))
         OsTask::delay(250) ;                  
            
      RELEASECSTR(pEnv, jsUrl, szUrl) ;      
   }

   JNI_END_METHOD() ;

   return (long) pPlayer ;
}


/*
 * Class:     org_sipfoundry_stapi_PMediaPlayer
 * Method:    JNI_destroyPlayer
 * Signature: (JJ)V
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_stapi_PMediaPlayer_JNI_1destroyPlayer
  (JNIEnv* pEnv, jclass clazz, jlong lHandle, jlong lTCHandle)
{
   JNI_BEGIN_METHOD("Java_org_sipfoundry_stapi_PMediaPlayer_JNI_1destroyPlayer") ;
   jint iRC = -1 ;
   
   MpStreamPlayer* pPlayer = (MpStreamPlayer*) (unsigned int) lHandle ;
   PtTerminalConnection* pTC = (PtTerminalConnection*) (unsigned int) lTCHandle ;

   if (pPlayer && pTC)
   {
      PtStatus status = pTC->destroyPlayer(pPlayer) ;
      iRC = status;
      if (status != PT_SUCCESS) 
      {
         API_FAILURE(status) ;
      }
   }

   JNI_END_METHOD() ;
   return iRC;
}


/*
 * Class:     org_sipfoundry_stapi_PMediaPlayer
 * Method:    JNI_realize
 * Signature: (JZ)V
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_stapi_PMediaPlayer_JNI_1realize
  (JNIEnv* pEnv , jclass clazz, jlong lHandle, jboolean jbBlock)
{
   JNI_BEGIN_METHOD("Java_org_sipfoundry_stapi_PMediaPlayer_JNI_1realize") ;
   jint iRC = -1 ;
   
   MpStreamPlayer* pPlayer = (MpStreamPlayer*) (unsigned int) lHandle ;
   if (pPlayer != NULL)
   {
      OsStatus status = pPlayer->realize(jbBlock) ;
      iRC = status;
      if (status != OS_SUCCESS) 
      {
         API_FAILURE(status) ;
      }
   }

   JNI_END_METHOD() ;
   return iRC;
}


/*
 * Class:     org_sipfoundry_stapi_PMediaPlayer
 * Method:    JNI_prefetch
 * Signature: (JZ)V
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_stapi_PMediaPlayer_JNI_1prefetch
  (JNIEnv* pEnv, jclass clazz, jlong lHandle, jboolean jbBlock)
{
   JNI_BEGIN_METHOD("Java_org_sipfoundry_stapi_PMediaPlayer_JNI_1prefetch") ;
   jint iRC = -1 ;
   
   MpStreamPlayer* pPlayer = (MpStreamPlayer*) (unsigned int) lHandle ;
   if (pPlayer != NULL)
   {
      OsStatus status = pPlayer->prefetch(jbBlock) ;
      iRC = status;
      if (status != OS_SUCCESS) 
      {
         API_FAILURE(status) ;
      }
   }

   JNI_END_METHOD() ;
   return iRC;
}



/*
 * Class:     org_sipfoundry_stapi_PMediaPlayer
 * Method:    JNI_play
 * Signature: (JZ)V
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_stapi_PMediaPlayer_JNI_1play
  (JNIEnv* pEnv, jclass clazz, jlong lHandle, jboolean jbBlock)
{
   JNI_BEGIN_METHOD("Java_org_sipfoundry_stapi_PMediaPlayer_JNI_1play") ;
   jint iRC = -1 ;
   
   MpStreamPlayer* pPlayer = (MpStreamPlayer*) (unsigned int) lHandle ;
   if (pPlayer != NULL)
   {
      OsStatus status = pPlayer->play(jbBlock) ;
      iRC = status;
      if (status != OS_SUCCESS) 
      {
         API_FAILURE(status) ;
      }
   }

   JNI_END_METHOD() ;
   return iRC;
}



/*
 * Class:     org_sipfoundry_stapi_PMediaPlayer
 * Method:    JNI_rewind
 * Signature: (JZ)V
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_stapi_PMediaPlayer_JNI_1rewind
  (JNIEnv* pEnv, jclass clazz, jlong lHandle, jboolean jbBlock)
{
   JNI_BEGIN_METHOD("Java_org_sipfoundry_stapi_PMediaPlayer_JNI_1rewind") ;
   jint iRC = -1 ;
   
   MpStreamPlayer* pPlayer = (MpStreamPlayer*) (unsigned int) lHandle ;
   if (pPlayer != NULL)
   {
      OsStatus status = pPlayer->rewind(jbBlock) ;
      iRC = status;
      if (status != OS_SUCCESS) 
      {
         API_FAILURE(status) ;
      }
   }

   JNI_END_METHOD() ;
   return iRC;
}


/*
 * Class:     org_sipfoundry_stapi_PMediaPlayer
 * Method:    JNI_pause
 * Signature: (JI)V
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_stapi_PMediaPlayer_JNI_1pause
  (JNIEnv* pEnv, jclass clazz, jlong lHandle, jint unused)
{
   JNI_BEGIN_METHOD("Java_org_sipfoundry_stapi_PMediaPlayer_JNI_1pause") ;
   jint iRC = -1 ;
   
   MpStreamPlayer* pPlayer = (MpStreamPlayer*) (unsigned int) lHandle ;
   if (pPlayer != NULL)
   {
      OsStatus status = pPlayer->pause();
      iRC = status;
      if (status != OS_SUCCESS) 
      {
         API_FAILURE(status) ;
      }
   }

   JNI_END_METHOD() ;
   return iRC;
}


/*
 * Class:     org_sipfoundry_stapi_PMediaPlayer
 * Method:    JNI_setLoopCount
 * Signature: (JI)V
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_stapi_PMediaPlayer_JNI_1setLoopCount
  (JNIEnv* pEnv, jclass clazz, jlong lHandle, jint iLoopCount, jint unused)
{
   JNI_BEGIN_METHOD("Java_org_sipfoundry_stapi_PMediaPlayer_JNI_1setLoopCount") ;
   jint iRC = -1 ;
   
   MpStreamPlayer* pPlayer = (MpStreamPlayer*) (unsigned int) lHandle ;
   if (pPlayer != NULL)
   {
      OsStatus status = pPlayer->setLoopCount((int)iLoopCount);
      iRC = status;
      if (status != OS_SUCCESS) 
      {
         API_FAILURE(status) ;
      }
   }

   JNI_END_METHOD() ;
   return iRC;
}


/*
 * Class:     org_sipfoundry_stapi_PMediaPlayer
 * Method:    JNI_stop
 * Signature: (JI)V
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_stapi_PMediaPlayer_JNI_1stop
  (JNIEnv* pEnv, jclass clazz, jlong lHandle, jint unused)
{
   JNI_BEGIN_METHOD("Java_org_sipfoundry_stapi_PMediaPlayer_JNI_1stop") ;
   jint iRC = -1 ;
   
   MpStreamPlayer* pPlayer = (MpStreamPlayer*) (unsigned int) lHandle ;
   if (pPlayer != NULL)
   {
      OsStatus status = pPlayer->stop();
      iRC = status;
      if (status != OS_SUCCESS) 
      {
         API_FAILURE(status) ;
      }
   }

   JNI_END_METHOD() ;
   return iRC;
}

/*
 * Class:     org_sipfoundry_stapi_PMediaPlayer
 * Method:    JNI_getState
 * Signature: (JI)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_stapi_PMediaPlayer_JNI_1getState
  (JNIEnv* pEnv, jclass clazz, jlong lHandle, jint unused)
{
   jint iRC = 0 ;

   JNI_BEGIN_METHOD("Java_org_sipfoundry_stapi_PMediaPlayer_JNI_1getState") ;

   MpStreamPlayer* pPlayer = (MpStreamPlayer*) (unsigned int) lHandle ;
   if (pPlayer != NULL)
   {
      PlayerState state ;
      OsStatus status = pPlayer->getState(state) ;
      iRC = state;
      if (status != OS_SUCCESS) 
      {
         API_FAILURE(status) ;
      }
   }

   JNI_END_METHOD() ;

   return iRC ;
}
