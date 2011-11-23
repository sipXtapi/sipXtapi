//  
// Copyright (C) 2007-2011 SIPez LLC.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsIntTypes.h>
#include <mp/MprRtpDispatcherActiveSsrcs.h>
#include <mp/MpRtpBuf.h>
#include <mp/MprDejitter.h>
#include <os/OsDateTime.h>
#include <os/OsDefs.h>
#include <os/OsSysLog.h>
#include <utl/UtlHashBagIterator.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprRtpDispatcherActiveSsrcs::MprRtpDispatcherActiveSsrcs(const UtlString& rName,
                                                         int connectionId,
                                                         int streamsNum)
: MprRtpDispatcher(rName, connectionId)
, mRtpStreamsNum(streamsNum)
, mpStreamsArray(new MpRtpStream[mRtpStreamsNum])
, mNumPushed(0)
, mNumDropped(0)
{
   for (int i=0; i<mRtpStreamsNum; i++)
   {
      mpStreamsArray[i].mStreamId = i;
   }
}

// Destructor
MprRtpDispatcherActiveSsrcs::~MprRtpDispatcherActiveSsrcs()
{
   delete[] mpStreamsArray;
}

/* ============================ MANIPULATORS ============================== */

OsStatus MprRtpDispatcherActiveSsrcs::pushPacket(MpRtpBufPtr &pRtp)
{
   OsLock lock(mMutex);
   OsStatus ret = OS_SUCCESS;
   MpRtpStream *pRtpStream=NULL;

//   assert(mpRtpStreams != NULL);
//   assert(mRtpStreamsNum > 0);

   mNumPushed++;

   // Look for RTP stream to catch this packet.
   pRtpStream = lookupRtpStream(pRtp->getRtpSSRC(), pRtp->getIP(), pRtp->getUdpPort());

   // Do we have RTP stream for this packet?
   if (pRtpStream != NULL)
   {
      pRtpStream->pushPacket(pRtp);
   }
   else
   {
      mNumDropped++;
   }

   return ret;
}  

void MprRtpDispatcherActiveSsrcs::checkRtpStreamsActivity()
{
   OsLock lock(mMutex);
   MpRtpStream *pIterStream;
   UtlHashBagIterator iter(mActiveStreams);

   //  Loop over all RTP streams to see if an active stream has become
   //  inactive.  We do this by seeing how long it has been since the
   //  last packet was processed.
   while ((pIterStream = (MpRtpStream*)iter()))
   {
      // Check for stream timeout
      if (pIterStream->timeoutDeactivate(mRtpInactiveTime))
      {
         // Notify about stream deactivation
         if (areNotificationsEnabled())
         {
            pIterStream->postStreamNotif(mpEventMsgQ, mResourceName, mConnectionId,
                                         MprnRtpStreamActivityMsg::STREAM_STOP);
         }
      }
   }
}

UtlBoolean MprRtpDispatcherActiveSsrcs::connectOutput(int outputIdx,
                                                      MpResource* pushRtpToResource)
{
   OsLock lock(mMutex);
   UtlContainable *pTmp;

   // Check is output index correct?
   if (outputIdx < 0 || outputIdx >= mRtpStreamsNum)
   {
      return FALSE;
   }

   // Check is this output already connected?
   if (mpStreamsArray[outputIdx].mpOutputResource != NULL)
   {
      return FALSE;
   }

   // Initialize stream and add it to the inactive list.
   mpStreamsArray[outputIdx].mpOutputResource = pushRtpToResource;
   pTmp = mInactiveStreams.append(&mpStreamsArray[outputIdx]);
   assert(pTmp != NULL);

   return TRUE;
}

UtlBoolean MprRtpDispatcherActiveSsrcs::disconnectOutput(int outputIdx)
{
   OsLock lock(mMutex);
   // Check is output index correct?
   if (outputIdx < 0 || outputIdx >= mRtpStreamsNum)
   {
      return FALSE;
   }

   // Check is this output connected?
   if (mpStreamsArray[outputIdx].mpOutputResource == NULL)
   {
      return FALSE;
   }

   // Remove stream from the both active and inactive stream lists.
   // One of these calls will apparently fail, but we don't care.
   mActiveStreams.remove(&mpStreamsArray[outputIdx]);
   mInactiveStreams.remove(&mpStreamsArray[outputIdx]);

   // Mark stream as disconnected.
   mpStreamsArray[outputIdx].mpOutputResource = NULL;

   return TRUE;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

MprRtpDispatcher::MpRtpStream *MprRtpDispatcherActiveSsrcs::lookupRtpStream(
                                                               unsigned int ssrc, 
                                                               const in_addr &fromIp,
                                                               int fromPort)
{
   MpRtpStream *pStream = NULL;
   UtlInt streamKey(ssrc);

   // Look for active stream with given SSRC and return it if found.
   pStream = (MpRtpStream *)mActiveStreams.find(&streamKey);
   if (pStream != NULL)
   {
      if (!pStream->isActive())
      {
         // Mark stream as active
         pStream->activate(fromIp.s_addr, fromPort);

         // Notify about stream activation
         if (areNotificationsEnabled())
         {
            pStream->postStreamNotif(mpEventMsgQ, mResourceName, mConnectionId,
                                    MprnRtpStreamActivityMsg::STREAM_START);
         }
      }

      return pStream;
   }

   // No active stream with current SSRC.
   if (!mInactiveStreams.isEmpty())
   {
      // Allocate stream from pool of inactive streams.
      pStream = (MpRtpStream *)mInactiveStreams.removeAt(0);
      assert(pStream != NULL);

      // Set SSRC and add stream to list of active streams.
      pStream->setSSRC(ssrc);
      mActiveStreams.insert(pStream);

      // Enable decoder to start audio processing.
      pStream->mpOutputResource->enable();

      // Mark stream as active
      pStream->activate(fromIp.s_addr, fromPort);

      // Notify about stream activation
      if (areNotificationsEnabled())
      {
         pStream->postStreamNotif(mpEventMsgQ, mResourceName, mConnectionId,
                                  MprnRtpStreamActivityMsg::STREAM_START);
      }
   }
   else
   {
      UtlHashBagIterator iter(mActiveStreams);
      MpRtpStream *pIterStream;
      OsTime curTime;
      OsTime streamInactiveTime;

      OsDateTime::getCurTime(curTime);

      while ((pIterStream = (MpRtpStream*)iter()))
      {
         OsTime iterStreamInactiveTime = curTime-pIterStream->mLastPacketTime;
         if (iterStreamInactiveTime >= mRtpInactiveTime)
         {
            if ((pStream == NULL) ||
               (streamInactiveTime < iterStreamInactiveTime))
            {
               pStream = pIterStream;
               streamInactiveTime = curTime-pStream->mLastPacketTime;
            }
         }
      }

      if (pStream != NULL)
      {
         // Remove stream from the active streams list and re-add it with new SSRC
         pStream = (MpRtpStream*)mActiveStreams.removeReference(pStream);
         assert(pStream != NULL);

         pStream->setSSRC(ssrc);
         mActiveStreams.insert(pStream);

         // Reset decoder in preparation to handle new stream
         pStream->mpOutputResource->reset();

         // Mark stream as active
         pStream->activate(fromIp.s_addr, fromPort);

         // Notify about stream activation
         if (areNotificationsEnabled())
         {
            pStream->postStreamNotif(mpEventMsgQ, mResourceName, mConnectionId,
                                     MprnRtpStreamActivityMsg::STREAM_START);
         }
      }
   }

   return pStream;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

