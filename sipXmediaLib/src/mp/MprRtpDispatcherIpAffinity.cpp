//  
// Copyright (C) 2007-2017 SIPez LLC.  All rights reserved.
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
#include <mp/MpRtpBuf.h>
#include <mp/MprRtpDispatcherIpAffinity.h>
#include <mp/MprDejitter.h>
#include <os/OsSocket.h>
#include <os/OsSysLog.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const int MprRtpDispatcherIpAffinity::SSRC_SWITCH_MISMATCH_COUNT = 8;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprRtpDispatcherIpAffinity::MprRtpDispatcherIpAffinity(const UtlString& rName,
                                                       int connectionId)
: MprRtpDispatcher(rName, connectionId)
, mpDejitter(new MprDejitter())
, mPrefSsrcValid(FALSE)
, mRtpDestIp(0)
, mRtpDestPort(0)
, mNumPushed(0)
, mNumDropped(0)
, mTotalWarnings(0)
, mNumWarnings(0)
, mNumNonPrefPackets(0)
, mRtpDestMatchIpOnlySsrc(0)
, mRtpDestMatchIpOnlySsrcValid(FALSE)
, mRtpOtherSsrc(0)
, mRtpOtherSsrcValid(FALSE)
{
   mRtpStream.mStreamId = 0;
}

// Destructor
MprRtpDispatcherIpAffinity::~MprRtpDispatcherIpAffinity()
{
   // We own Dejitter and should delete it.
   delete mpDejitter;
}

/* ============================ MANIPULATORS ============================== */

OsStatus MprRtpDispatcherIpAffinity::pushPacket(MpRtpBufPtr &pRtp)
{
   OsLock lock(mMutex);
   OsStatus    ret = OS_SUCCESS;
   RtpSRC      thisSsrc;
   const in_addr* fromIP = &pRtp->getIP();
   int fromPort = pRtp->getUdpPort();

   mNumPushed++;
   if (0 == (mNumPushed & ((1<<11)-1)))
      mNumWarnings = 0; // every 2048

   // What SSRC is this from?
   thisSsrc = pRtp->getRtpSSRC();
   if (!mPrefSsrcValid || !mRtpStream.isActive())
   {
      setPrefSsrc(thisSsrc);
      mRtpStream.activate(fromIP->s_addr, fromPort);
      // Notify about stream activation
      if (areNotificationsEnabled())
      {
         mRtpStream.postStreamNotif(mpEventMsgQ, mResourceName, mConnectionId,
                                    MprnRtpStreamActivityMsg::STREAM_START);
      }
   }

   // If this is from our preferred SSRC,
   if (thisSsrc == getPrefSsrc())
   {
      mNumNonPrefPackets = 0;

      // Track when the source IP changes
      if ((mRtpStream.mAddress != (int)(fromIP->s_addr)) ||
          (mRtpStream.mPort != fromPort))
      {
         mRtpStream.mAddress = fromIP->s_addr;
         mRtpStream.mPort = fromPort;
         // Notify about stream change
         if (areNotificationsEnabled())
         {
            mRtpStream.postStreamNotif(mpEventMsgQ, mResourceName, mConnectionId,
                                       MprnRtpStreamActivityMsg::STREAM_CHANGE);
         }
      }

      mRtpStream.pushPacket(pRtp);
      // Warning! pRtp may contain some random packet or even NULL from now on.
   }
   // Not from our preferred SSRC
   else
   {
      mTotalWarnings++;

      if (mNumWarnings++ < 20)
      {
         UtlString Old(""), New("");
         struct in_addr t;
         t.s_addr = mRtpDestIp;
         OsSocket::inet_ntoa_pt(t, Old);
         OsSocket::inet_ntoa_pt(*fromIP, New);
         if (OsSysLog::willLog(FAC_MP, PRI_DEBUG))
            osPrintf("**MprFromNet recvd RTP pkt from SSRC %d (%s:%d), "
                     "expected SSRC %d (%s:%u)\n",
                     thisSsrc, New.data(), fromPort,
                     getPrefSsrc(), Old.data(), mRtpDestPort);
      }
      // If it's from the same IP:port, SSRC must have changed
      if ((fromIP->s_addr == mRtpDestIp) && 
          (fromPort == mRtpDestPort))
      {
         setPrefSsrc(thisSsrc);
         // Notify about stream change
         if (areNotificationsEnabled())
         {
            mRtpStream.postStreamNotif(mpEventMsgQ, mResourceName, mConnectionId,
                                       MprnRtpStreamActivityMsg::STREAM_CHANGE);
         }
      }
      //
      //else if (mRtpRtcpMatchSsrcValid && thisSsrc == mRtpRtcpMatchSsrc)
      //{
      //    setPrefSsrc(thisSsrc);
      //}
      // OK, definitely from someone else
      else
      {
         mNumNonPrefPackets++;

         // If this is from the same IP as pref'd but different port,
         if (fromIP->s_addr == mRtpDestIp)
         {
            mRtpDestMatchIpOnlySsrc = thisSsrc;
            mRtpDestMatchIpOnlySsrcValid = TRUE;
         }
         // Hey, this isn't even from the same IP as pref'd!
         else
         {
            mRtpOtherSsrc = thisSsrc;
            mRtpOtherSsrcValid = TRUE;
         }

         // If we get this many consecutive pkts from non-pref'd,
         //  switch preferred to the new guy
         if (mNumNonPrefPackets >= SSRC_SWITCH_MISMATCH_COUNT)
         {
            setPrefSsrc(mRtpDestMatchIpOnlySsrcValid ?
                        mRtpDestMatchIpOnlySsrc : mRtpOtherSsrc);
            // Notify about stream change
            if (areNotificationsEnabled())
            {
               mRtpStream.postStreamNotif(mpEventMsgQ, mResourceName, mConnectionId,
                                          MprnRtpStreamActivityMsg::STREAM_CHANGE);
            }
         }
      }
      mNumDropped++;
   }

   return ret;
}

void MprRtpDispatcherIpAffinity::checkRtpStreamsActivity()
{
   OsLock lock(mMutex);

   // Check for stream timeout
   if (mRtpStream.timeoutDeactivate(mRtpInactiveTime))
   {
      // Notify about stream deactivation
      if (areNotificationsEnabled())
      {
         mRtpStream.postStreamNotif(mpEventMsgQ, mResourceName, mConnectionId,
                                    MprnRtpStreamActivityMsg::STREAM_STOP);
      }
   }
}

UtlBoolean MprRtpDispatcherIpAffinity::connectOutput(int outputIdx,
                                                     MpResource* pushRtpToResource)
{
   OsLock lock(mMutex);
   
   // Check is output index correct?
   if (outputIdx != 0)
   {
      return FALSE;
   }

   // Check is this output already connected?
   if (mRtpStream.mpOutputResource != NULL)
   {
      return FALSE;
   }

   // Initialize stream and add it to the inactive list.
   mRtpStream.mpOutputResource = pushRtpToResource;

   return TRUE;
}

UtlBoolean MprRtpDispatcherIpAffinity::disconnectOutput(int outputIdx)
{
   OsLock lock(mMutex);

   // Check is output index correct?
   if (outputIdx != 0)
   {
      return FALSE;
   }

   // Check is this output connected?
   if (mRtpStream.mpOutputResource == NULL)
   {
      return FALSE;
   }

   // Mark stream as disconnected.
   mRtpStream.mpOutputResource = NULL;

   return TRUE;
}

void MprRtpDispatcherIpAffinity::setPreferredIp(unsigned long address, int port)
{
   OsLock lock(mMutex);
   mRtpDestIp = address;
   mRtpDestPort = port;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

RtpSRC MprRtpDispatcherIpAffinity::getPrefSsrc()
{
   return mRtpStream.getSSRC();
}

int MprRtpDispatcherIpAffinity::setPrefSsrc(unsigned int newSsrc)
{
   mRtpStream.setSSRC(newSsrc);
   mPrefSsrcValid = TRUE;
   mNumNonPrefPackets = 0;
   //mRtpRtcpMatchSsrcValid = FALSE;
   mRtpDestMatchIpOnlySsrcValid = FALSE;
   mRtpOtherSsrcValid = FALSE;
   return 0;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

