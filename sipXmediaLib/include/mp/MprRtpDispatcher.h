//  
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifndef _MprRtpDispatcher_h_
#define _MprRtpDispatcher_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
//#include <mp/MpResource.h>
//#include "mp/MpEvents.h"
#include <mp/MpRtpBuf.h>
#include <mp/MprDecode.h>
#include <mp/MprnRtpStreamActivityMsg.h>
#include <utl/UtlString.h>
#include <utl/UtlInt.h>
#include <os/OsMsgQ.h>
#include <os/OsDateTime.h>
#include <os/OsLock.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MprDecode;
class OsMsgDispatcher;

/**
*  @brief Class for RTP stream dispatcher abstraction.
*
*  @nosubgrouping
*/
class MprRtpDispatcher
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprRtpDispatcher(const UtlString& rName, int connectionId);
     /**<
     *  @param name - name of this RTP dispatcher.
     *  @param connectionId - connection ID to be used in notifications.
     *  @param pMsgQ - notification dispatcher to send notifications to.
     */

     /// Destructor
   virtual ~MprRtpDispatcher();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Dispatch RTP packet.
   virtual OsStatus pushPacket(const MpRtpBufPtr &pRtp) = 0;

     /// Check activity of the RTP streams and send notifications if needed.
   virtual void checkRtpStreamsActivity() = 0;

     /// Connect given output to the decode resource.
   virtual UtlBoolean connectOutput(int outputIdx, MprDecode *pDecode) = 0;

     /// Disconnect given output from the decode resource.
   virtual UtlBoolean disconnectOutput(int outputIdx) = 0;

     /// Set notification dispatcher.
   inline void setNotificationDispatcher(OsMsgDispatcher *pMsgQ);
     /**<
     *  Set notification dispatcher to NULL to disable notifications.
     */

     /// Enable/disable notifications.
   inline void setNotificationsEnabled(UtlBoolean enabled);

     /// Set ID of the connection to which this resource belongs.
   inline void setConnectionId(MpConnectionID connectionId);

     /// Set stream activity timeout.
   inline void setRtpInactivityTimeout(const OsTime &inactiveTime);
     /**<
     *  Sets time which should pass after last received RTP packet before
     *  stream is marked as inactive.
     */

     /// Set default stream activity timeout.
   static inline
   void setDefaultRtpInactivityTimeout(const OsTime &inactiveTime);
     /**<
     *  Sets default time which should pass after last received RTP packet
     *  before stream is marked as inactive.
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

     // Returns true, if notifications are enabled to be sent.
   inline UtlBoolean areNotificationsEnabled() const;

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /**
   *  @brief Information, specific for an RTP stream.
   *
   *  MpRtpStream is inherited from UtlInt to hold SSRC associated with
   *  this stream and to store streams in UtlHashBag with fast access by SSRC
   *  value.
   */
   struct MpRtpStream : public UtlInt
   {
      MpRtpStream()
      : mStreamId(-1)
      , mStreamActive(FALSE)
      , mAddress(0)
      , mPort(-1)
      , mpDecode(NULL)
      {}

      int                 mStreamId;       ///< Abstract stream ID, used by higher
                                           ///< levels to identify a stream.
      OsTime              mLastPacketTime; ///< Time of last packet receive.
      UtlBoolean          mStreamActive;   ///< Is this stream active or not?
      int                 mAddress;        ///< IP address of the stream source.
      int                 mPort;           ///< Port of the stream source.
      MprDecode          *mpDecode;        ///< Pointer to Decode resource.
                                           ///< Also used as a "connected" mark -
                                           ///< NULL means this stream is not connected.

        /// Push packet to the stream for processing.
      inline void pushPacket(const MpRtpBufPtr &pRtp);

        /// Mark stream as active.
      inline void activate(int fromIp, int fromPort);

        /// Mark stream as inactive.
      inline void deactivate();

        /// Mark stream as inactive, if timeout occurred.
      UtlBoolean timeoutDeactivate(const OsTime &timeout);
        /**<
        *  @returns TRUE if stream just have been deactivated, TRUE otherwise.
        */

        /// Is this stream marked as active?
      inline UtlBoolean isActive() const;

        /// Get stream SSRC.
      inline RtpSRC getSSRC() const;

        /// Set stream SSRC.
      inline void setSSRC(RtpSRC ssrc);

        /// Send notification on stream activation/deactivation.
      OsStatus postStreamNotif(OsMsgDispatcher *pMsgQ,
                               const UtlString &resourceName,
                               MpConnectionID connId,
                               MprnRtpStreamActivityMsg::StreamState state) const;
   };

   OsMutex          mMutex;          ///< Mutex to synchronize access to this resource.
   UtlString        mResourceName;   ///< Name of the owner resource to be used
                                     ///< in event notifications.
   MpConnectionID   mConnectionId;   ///< ConnID to send in event notifications
   OsMsgDispatcher* mpEventMsgQ;     ///< MsgQ for event notifications
   UtlBoolean       mNotificationsEnabled; ///< Are notifications enabled?

   OsTime           mRtpInactiveTime; ///< Time after which stream is considered
                     ///< inactive if does not receive RTP packets.

   static OsTime    smDefaultRtpInactiveTime; ///< Default value for mRtpInactiveTime.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Copy constructor (not implemented for this class)
   MprRtpDispatcher(const MprRtpDispatcher& rMprRtpDispatcher);

     /// Assignment operator (not implemented for this class)
   MprRtpDispatcher& operator=(const MprRtpDispatcher& rhs);

};

/* ============================ INLINE METHODS ============================ */

void MprRtpDispatcher::setNotificationDispatcher(OsMsgDispatcher *pMsgQ)
{
   OsLock lock(mMutex);
   mpEventMsgQ = pMsgQ;
}

void MprRtpDispatcher::setNotificationsEnabled(UtlBoolean enabled)
{
   mNotificationsEnabled = enabled;
}

void MprRtpDispatcher::setConnectionId(MpConnectionID connectionId)
{
   OsLock lock(mMutex);
   mConnectionId = connectionId;
}

void MprRtpDispatcher::setRtpInactivityTimeout(const OsTime &inactiveTime)
{
   OsLock lock(mMutex);
   mRtpInactiveTime = inactiveTime;
}

void MprRtpDispatcher::setDefaultRtpInactivityTimeout(const OsTime &inactiveTime)
{
   smDefaultRtpInactiveTime = inactiveTime;
}

UtlBoolean MprRtpDispatcher::areNotificationsEnabled() const
{
   return mNotificationsEnabled;
}

/// Push packet to the stream for processing.
void MprRtpDispatcher::MpRtpStream::pushPacket(const MpRtpBufPtr &pRtp)
{
   OsDateTime::getCurTime(mLastPacketTime);
   mpDecode->pushPacket(pRtp);
}

/// Mark stream as active.
void MprRtpDispatcher::MpRtpStream::activate(int fromIp, int fromPort)
{
   // Mark stream as active and save sender IP.
   mStreamActive = TRUE;
   mAddress = fromIp;
   mPort = fromPort;

//   printf("Stream %X activated\n", getSSRC());
}

/// Mark stream as inactive.
void MprRtpDispatcher::MpRtpStream::deactivate()
{
   // Mark stream as inactive
   mStreamActive = FALSE;

//   printf("Stream %X deactivated\n", getSSRC());
}

UtlBoolean MprRtpDispatcher::MpRtpStream::isActive() const
{
   return mStreamActive;
}

RtpSRC MprRtpDispatcher::MpRtpStream::getSSRC() const
{
   return getValue();
}

void MprRtpDispatcher::MpRtpStream::setSSRC(RtpSRC ssrc)
{
   setValue(ssrc);
}

#endif  // _MprRtpDispatcher_h_
