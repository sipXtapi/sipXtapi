//  
// Copyright (C) 2008 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

// SYSTEM INCLUDES
#include <os/OsIntTypes.h>
#include <assert.h>
//#include <os/OsSysLog.h>

// APPLICATION INCLUDES
#include <mp/MprVoiceActivityNotifier.h>
#include <mp/MpVadBase.h>
#include <mp/MpIntResourceMsg.h>
#include <mp/MprnIntMsg.h>
#include <mp/MpFlowGraphBase.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// DEFINES
// MACROS
// STATIC VARIABLE INITIALIZATIONS
const int MprVoiceActivityNotifier::smDefaultLevelPeriodMs = 1000;
const int MprVoiceActivityNotifier::smDefaultInactivityTimeoutMs = 500;

/* //////////////////////////////// PUBLIC //////////////////////////////// */

MprVoiceActivityNotifier::MprVoiceActivityNotifier(const UtlString& rName,
                                                   int levelPeriodMs,
                                                   int inactivityTimeoutMs)
: MpAudioResource(rName, 1, 1, 0, 1)
, mLevelPeriodMs(levelPeriodMs)
, mLevelPeriodSmp(0)
, mLevelSamplesPassed(0)
, mInactivityTimeoutMs(inactivityTimeoutMs)
, mInactivityTimeoutSmp(0)
, mInactivitySamples(0)
, mEnergyLevelMax(0)
, mStreamState(STREAM_SILENT)
{
}

MprVoiceActivityNotifier::~MprVoiceActivityNotifier()
{
}

/* =============================== CREATORS =============================== */

/* ============================= MANIPULATORS ============================= */

OsStatus MprVoiceActivityNotifier::chageNotificationPeriod(const UtlString& namedResource, 
                                                           OsMsgQ& fgQ,
                                                           int notificationPeriodMs)
{
   MpIntResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_CHANGE_NOTIFICATION_PERIOD,
                        namedResource,
                        notificationPeriodMs);
   return fgQ.send(msg);
}

OsStatus MprVoiceActivityNotifier::changeInactivityTimeout(const UtlString& namedResource, 
                                                           OsMsgQ& fgQ,
                                                           int inactivityTimeoutMs)
{
   MpIntResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_CHANGE_INACTIVITY_TIMEOUT,
                        namedResource,
                        inactivityTimeoutMs);
   return fgQ.send(msg);
}

/* ============================== ACCESSORS =============================== */

/* =============================== INQUIRY ================================ */

/* ////////////////////////////// PROTECTED /////////////////////////////// */

UtlBoolean MprVoiceActivityNotifier::doProcessFrame(MpBufPtr inBufs[],
                                                    MpBufPtr outBufs[],
                                                    int inBufsSize,
                                                    int outBufsSize,
                                                    UtlBoolean isEnabled,
                                                    int samplesPerFrame,
                                                    int samplesPerSecond)
{
   // We're disabled or have nothing to process.
   if ( outBufsSize == 0 || inBufsSize == 0 )
   {
      return TRUE;
   }
   if (!isEnabled)
   {
      outBufs[0] = inBufs[0];
      return TRUE;
   }

   MpAudioBufPtr pBuf = inBufs[0];

   // Send START/STOP notifications.
   if (  mStreamState == STREAM_ACTIVE
      && !(pBuf.isValid() && isActiveAudio(pBuf->getSpeechType())))
   {
      // End of active voice
      if (mInactivitySamples < mInactivityTimeoutSmp)
      {
         mInactivitySamples += mpFlowGraph->getSamplesPerFrame();
      }
      else
      {
         mStreamState = STREAM_SILENT;
         sendNotification(MpResNotificationMsg::MPRNM_VOICE_STOPPED);
//         printf("%s: STOPPED\n", getName().data());
      }
   }
   else if (  mStreamState == STREAM_SILENT
           && pBuf.isValid() && isActiveAudio(pBuf->getSpeechType()))
   {
      // First active packet in a stream
      mStreamState = STREAM_ACTIVE;
      mInactivitySamples = 0;
      sendNotification(MpResNotificationMsg::MPRNM_VOICE_STARTED);
//      printf("%s: STARTED\n", getName().data());
   }

   // Send energy level notifications.
   if (pBuf.isValid())
   {
      int energy = pBuf->getEnergy();
      if (energy >= 0)
      {
         if (mLevelSamplesPassed < mLevelPeriodSmp)
         {
            mLevelSamplesPassed += mpFlowGraph->getSamplesPerFrame();
            mEnergyLevelMax = sipx_max(mEnergyLevelMax, energy);
         }
         else
         {
            mLevelSamplesPassed = 0;
            MprnIntMsg msg(MpResNotificationMsg::MPRNM_ENERGY_LEVEL,
                           getName(),
                           mEnergyLevelMax);
            sendNotification(msg);
//            printf("%s level: %d\n", getName().data(), energy);
            mEnergyLevelMax = 0;
         }
      }
      else
      {
         mLevelSamplesPassed = 0;
      }
   }

   outBufs[0] = pBuf;

   return TRUE;
}

UtlBoolean MprVoiceActivityNotifier::handleMessage(MpResourceMsg& rMsg)
{
   switch (rMsg.getMsg())
   {
   case MPRM_CHANGE_NOTIFICATION_PERIOD:
      {
         MpIntResourceMsg *pMsg = (MpIntResourceMsg*)&rMsg;
         handleChageNotificationPeriod(pMsg->getData());
      }
      break;
   case MPRM_CHANGE_INACTIVITY_TIMEOUT:
      {
         MpIntResourceMsg *pMsg = (MpIntResourceMsg*)&rMsg;
         handleChangeInactivityTimeout(pMsg->getData());
      }
      break;
   default:
      return MpResource::handleMessage(rMsg);
   }
   return TRUE;
}

UtlBoolean MprVoiceActivityNotifier::handleChageNotificationPeriod(int periodMs)
{
   mLevelPeriodMs = periodMs;
   if (mpFlowGraph != NULL)
   {
      mLevelPeriodSmp = mLevelPeriodMs * mpFlowGraph->getSamplesPerSec()
                      / 1000;
   }
   return TRUE;
}

UtlBoolean MprVoiceActivityNotifier::handleChangeInactivityTimeout(int timeout)
{
   mInactivityTimeoutMs = timeout;
   if (mpFlowGraph != NULL)
   {
      mInactivityTimeoutSmp = mInactivityTimeoutMs * mpFlowGraph->getSamplesPerSec()
                            / 1000;
   }
   return TRUE;
}

OsStatus MprVoiceActivityNotifier::setFlowGraph(MpFlowGraphBase* pFlowGraph)
{
   OsStatus res =  MpAudioResource::setFlowGraph(pFlowGraph);

   if (res == OS_SUCCESS)
   {
      // Check whether we've been added to flowgraph or removed.
      if (pFlowGraph != NULL)
      {
         mLevelPeriodSmp = mLevelPeriodMs * mpFlowGraph->getSamplesPerSec() / 1000;
         mInactivityTimeoutSmp = mInactivityTimeoutMs * mpFlowGraph->getSamplesPerSec()
                               / 1000;
      }
      else
      {
      }
   }

   return res;
}

/* /////////////////////////////// PRIVATE //////////////////////////////// */

/* ============================== FUNCTIONS =============================== */


