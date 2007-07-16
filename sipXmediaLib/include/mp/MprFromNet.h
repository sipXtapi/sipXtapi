//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MprFromNet_h_
#define _MprFromNet_h_

#include "rtcp/RtcpConfig.h"

// SYSTEM INCLUDES
#ifdef _WIN32 /* [ */
#include <winsock2.h>
#endif /* _WIN32 ] */

// APPLICATION INCLUDES

class MprDejitter;

#include "os/OsDefs.h"
#include "os/OsSocket.h"
#include "mp/NetInTask.h"
#include "mp/MpUdpBuf.h"
#include "mp/MpRtpBuf.h"
#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/IRTPDispatch.h"
#include "rtcp/INetDispatch.h"
#endif /* INCLUDE_RTCP ] */

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// The "From Network" media processing resource
class MprFromNet
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprFromNet();

     /// Destructor
   virtual ~MprFromNet();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// @brief Set the inbound RTP and RTCP sockets.
   OsStatus setSockets(OsSocket& rRtpSocket, OsSocket& rRtcpSocket);
     /** @returns Always OS_SUCCESS for now. */

     /// @brief Deregister the inbound RTP and RTCP sockets.
   OsStatus resetSockets();
     /** @returns Always OS_SUCCESS for now. */

     /// Take in a buffer from the NetIn task
   OsStatus pushPacket(const MpUdpBufPtr &buf, bool isRtcp);

     /// Inform this object of its sibling dejitter object.
   void setMyDejitter(MprDejitter* newDJ);

     /// Inform this object of its sibling ToNet's destination
   void setDestIp(OsSocket& newDest);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

#ifdef INCLUDE_RTCP /* [ */
     /// @brief These accessors were added by DMG to allow a Connection to access and modify
     /// rtp and rtcp stream informations
   void setDispatchers(IRTPDispatch *piRTPDispatch, INetDispatch *piRTCPDispatch);

#else /* INCLUDE_RTCP ] [ */
     /// retrieve the RR info needed to complete an RTCP packet
   OsStatus getRtcpStats(MprRtcpStats& stats);
#endif /* INCLUDE_RTCP ] */

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsMutex          mMutex;
   UtlBoolean       mRegistered;
   MprDejitter*     mpDejitter;
#ifdef INCLUDE_RTCP /* [ */
   INetDispatch*    mpiRTCPDispatch;
   IRTPDispatch*    mpiRTPDispatch;
#else /* INCLUDE_RTCP ] [ */
   rtpHandle        mInRtpHandle;
   int              mRtcpCount;
#endif /* INCLUDE_RTCP ] */

   unsigned long mPrevIP;
   int mPrevPort;
   int mNumPushed;
   int mNumWarnings;

   int mPrefSsrc;               ///< current "preferred SSRC"
   UtlBoolean mPrefSsrcValid;
   unsigned long mRtpDestIp;    ///< where this connection is sending to
   int mRtpDestPort;

   int mNumNonPrefPackets;
   int mRtpRtcpMatchSsrc;
   UtlBoolean mRtpRtcpMatchSsrcValid;
   int mRtpDestMatchIpOnlySsrc;
   UtlBoolean mRtpDestMatchIpOnlySsrcValid;
   int mRtpOtherSsrc;
   UtlBoolean mRtpOtherSsrcValid;
   static const int SSRC_SWITCH_MISMATCH_COUNT;

     /// Copy constructor (not implemented for this class)
   MprFromNet(const MprFromNet& rMprFromNet);

     /// Assignment operator (not implemented for this class)
   MprFromNet& operator=(const MprFromNet& rhs);

#ifndef INCLUDE_RTCP /* [ */
     /// Update the RR info for the current incoming packet
   OsStatus rtcpStats(struct RtpHeader *h);
#endif /* INCLUDE_RTCP ] */

   MprDejitter* getMyDejitter(void);

     /// Parse UDP packet and return filled RTP packet buffer.
   static MpRtpBufPtr parseRtpPacket(const MpUdpBufPtr &buf);

     /// Set current "preferred SSRC".
   int setPrefSsrc(int newSsrc);

     /// Get current "preferred SSRC".
   int getPrefSsrc(void);


};

/* ============================ INLINE METHODS ============================ */
#ifdef INCLUDE_RTCP /* [ */
inline  void  MprFromNet::setDispatchers(IRTPDispatch *piRTPDispatch, INetDispatch *piRTCPDispatch)
{
// Set the dispatch pointers for both RTP and RTCP
   mpiRTPDispatch   = piRTPDispatch;
   mpiRTCPDispatch  = piRTCPDispatch;
}

#endif /* INCLUDE_RTCP ] */

#endif  // _MprFromNet_h_
