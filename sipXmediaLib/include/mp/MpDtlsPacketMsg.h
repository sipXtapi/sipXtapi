//
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MpDtlsPacketMsg_h_
#define _MpDtlsPacketMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpResourceMsg.h"
#include <utl/UtlString.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Message ferrying an inbound DTLS record from the NetInTask thread
/// (where MprFromNet::pushPacket runs) to the media thread (where
/// MprFromNet::handleMessage runs and where MpDtls processes
/// handshake bytes).
///
/// Posted by MprFromNet::pushPacket after RFC 7983 first-byte demux
/// identifies the packet as a DTLS record. Consumed by
/// MprFromNet::handleMessage, which forwards the bytes to the
/// connection's MpDtls engine via processIncomingPacket().
///
/// The packet bytes are copied into a UtlString at message construction
/// because the incoming MpUdpBufPtr buffer may be recycled before the
/// media thread gets to it.
class MpDtlsPacketMsg : public MpResourceMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor. Copies `dataLen` bytes from `data` into the message.
    MpDtlsPacketMsg(const UtlString& targetResourceName,
                    const char* data,
                    int dataLen,
                    UtlBoolean fromRtcpTransport = FALSE)
      : MpResourceMsg(MPRM_DTLS_PACKET, targetResourceName)
      , mPacket(data, dataLen)
      , mFromRtcpTransport(fromRtcpTransport)
   {};

     /// Copy constructor
    MpDtlsPacketMsg(const MpDtlsPacketMsg& resourceMsg)
      : MpResourceMsg(resourceMsg)
      , mPacket(resourceMsg.mPacket)
      , mFromRtcpTransport(resourceMsg.mFromRtcpTransport)
   {};

     /// Create a copy of this msg object (which may be of a derived type)
   OsMsg* createCopy(void) const
   {
      return(new MpDtlsPacketMsg(*this));
   }

     /// Destructor
   ~MpDtlsPacketMsg() {};

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Assignment operator
   MpDtlsPacketMsg& operator=(const MpDtlsPacketMsg& rhs)
   {
      if(&rhs == this)
      {
         return(*this);
      }

      MpResourceMsg::operator=(rhs);
      mPacket = rhs.mPacket;
      mFromRtcpTransport = rhs.mFromRtcpTransport;

      return *this;
   }

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   const UtlString& getPacket() const {return mPacket;}

     /// TRUE if this record arrived on the RTCP socket, and so belongs to the
     /// RTCP transport's DTLS association rather than the RTP one.  Without
     /// rtcp-mux a connection runs two independent associations (RFC 5764
     /// section 3) and records must not be crossed between them.
   UtlBoolean isFromRtcpTransport() const {return mFromRtcpTransport;}

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    UtlString mPacket;
    UtlBoolean mFromRtcpTransport;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpDtlsPacketMsg_h_
