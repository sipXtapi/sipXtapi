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


#ifndef _MprToNet_h_
#define _MprToNet_h_

#include "rtcp/RtcpConfig.h"

// Defining this option includes debug code for introducing sudden changes
// to several fields of the outgoing RTP headers (SSRC, seq#, timestamp).
#define ENABLE_PACKET_HACKING
#undef ENABLE_PACKET_HACKING

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsSocket.h"
#include "mp/NetInTask.h"
#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/ISetSenderStatistics.h"
#endif /* INCLUDE_RTCP ] */

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MprFromNet;

/**
*  @brief The RTP writer
*/
class MprToNet
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum { RESERVED_RTP_PACKET_HEADER_BYTES = 76
             ///< 76 =    4 for encryption prefix +<br>
             ///<        12 for basic packet header +<br>
             ///<      15*4 for CSRC list
   };

#ifdef ENABLE_PACKET_HACKING /* [ */
   static int sDebug1;
   static int sDebug2;
   static int sDebug3;
   static int sDebug4;
   static int sDebug5;
#endif /* ENABLE_PACKET_HACKING ] */

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprToNet();

     /// Destructor
   virtual
   ~MprToNet();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

      /// Set the outbound RTP and RTCP sockets.
   OsStatus setSockets(OsSocket& rRtpSocket, OsSocket& rRtcpSocket);
     /**<
     *  @returns Always OS_SUCCESS for now.
     */

     /// Clear the outbound RTP and RTCP sockets.
   OsStatus resetSockets();
     /**<
     *  @returns Always OS_SUCCESS for now.
     */

     /// Connect us to our corresponding FromNet, for RTCP stats.
   OsStatus setRtpPal(MprFromNet* pal);

   int writeRtp(int payloadType, UtlBoolean markerState,
      unsigned char* payloadData, int payloadOctets, unsigned int timestamp,
      void* csrcList);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

// These accessors were added by DMG to allow a Connection to access and modify
// rtp and rtcp stream information
   void   setSSRC(int iSSRC);
#ifdef INCLUDE_RTCP /* [ */
   void   setRTPAccumulator(ISetSenderStatistics *piRTPAccumulator);
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

   MprFromNet*  mpFromNetPal;
   int          mRtcpPackets;
   int          mRtcpFrameCount;
   int          mRtcpFrameLimit;
#ifdef DEBUG /* [ */
   unsigned int mRtpSampleCounter;
#endif /* DEBUG ] */

   // RTP State
   unsigned int mTimestampDelta;
   unsigned int mSeqNum;
   unsigned int mSSRC;
   OsSocket*    mpRtpSocket;
   OsSocket*    mpRtcpSocket;
   int          mNumRtpWriteErrors;
   int          mNumRtcpWriteErrors;

#ifdef ENABLE_PACKET_HACKING /* [ */
   void adjustRtpPacket(struct RtpHeader* p);
#endif /* ENABLE_PACKET_HACKING ] */

#ifdef INCLUDE_RTCP /* [ */
   /// Allow outbound RTP stream to accumulate RTP packet statistics
   ISetSenderStatistics *mpiRTPAccumulator;
#endif /* INCLUDE_RTCP ] */

     /// Copy constructor (not implemented for this class)
   MprToNet(const MprToNet& rMprToNet);

     /// Assignment operator (not implemented for this class)
   MprToNet& operator=(const MprToNet& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprToNet_h_
