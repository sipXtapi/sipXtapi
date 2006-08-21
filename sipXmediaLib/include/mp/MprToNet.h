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
#include "mp/MpFlowGraphMsg.h"
#include "mp/MpAudioResource.h"
#include "mp/NetInTask.h"
#include "mp/MprFromNet.h"
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

/**
*  @brief The "To Network" media processing resource
*/
class MprToNet : public MpAudioResource
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
   MprToNet(const UtlString& rName, int samplesPerFrame, int samplesPerSec);

     /// Destructor
   virtual
   ~MprToNet();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// @brief Sends a @link MprToNet::SET_SOCKETS SET_SOCKETS @endlink message
     /// to this resource to set the outbound RTP and RTCP sockets.
   OsStatus setSockets(OsSocket& rRtpSocket, OsSocket& rRtcpSocket);
     /**<
     *  @returns the result of attempting to queue the message to this resource.
     */

     /// @brief Sends a @link MprToNet::RESET_SOCKETS RESET_SOCKETS @endlink
     /// message to this resource to stop sending RTP and RTCP sockets.
   OsStatus resetSockets();
     /**<
     *  @returns OS_SUCCESS, unless unable to queue message.
     */

     /// Connect us to our corresponding FromNet, for RTCP stats.
   OsStatus setRtpPal(MprFromNet* pal);

     /// Time to send an RTCP message
   OsStatus sendRtcpPacket(void);

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
   typedef enum
   {
      SET_SOCKETS = MpFlowGraphMsg::RESOURCE_SPECIFIC_START,
      RESET_SOCKETS
   } AddlMsgTypes;

#ifdef DEBUG /* [ */
   unsigned int mRtpSampleCounter;
#endif /* DEBUG ] */
   MprFromNet*  mpFromNetPal;
   int          mRtcpPackets;
   int          mRtcpFrameCount;
   int          mRtcpFrameLimit;

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

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame=80,
                                    int samplesPerSecond=8000);

     /// Handle messages for this resource.
   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);

     /// Copy constructor (not implemented for this class)
   MprToNet(const MprToNet& rMprToNet);

     /// Assignment operator (not implemented for this class)
   MprToNet& operator=(const MprToNet& rhs);

   void sentRtcpPacket(void);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprToNet_h_
