//  
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MprnIceNominatedMsg_h_
#define _MprnIceNominatedMsg_h_

// APPLICATION INCLUDES
#include "mp/MpResNotificationMsg.h"
#include "utl/UtlString.h"

/// Notification fired when an ICE candidate pair is nominated by the remote
/// peer (i.e. a STUN Binding Request arrives with the USE-CANDIDATE attribute
/// set). Carries the source IP address and port of the nominating packet,
/// which are the address the remote peer will send media from.
///
/// Receivers should call setConnectionDestination() with these values to
/// configure the RTP/DTLS stack for the nominated path.
class MprnIceNominatedMsg : public MpResNotificationMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /// Constructor
   MprnIceNominatedMsg(const UtlString& resourceName,
                       MpConnectionID   connectionId,
                       const UtlString& remoteIp,
                       int              remotePort)
   : MpResNotificationMsg(MPRNM_ICE_CANDIDATE_NOMINATED, resourceName, connectionId)
   , mRemoteIp(remoteIp)
   , mRemotePort(remotePort)
   {}

   /// Copy constructor
   MprnIceNominatedMsg(const MprnIceNominatedMsg& rhs)
   : MpResNotificationMsg(rhs)
   , mRemoteIp(rhs.mRemoteIp)
   , mRemotePort(rhs.mRemotePort)
   {}

   /// Create a copy of this message (required by OsMsg dispatch infrastructure)
   virtual OsMsg* createCopy(void) const
   {
      return new MprnIceNominatedMsg(*this);
   }

   /// Destructor
   virtual ~MprnIceNominatedMsg() {}

   /// Assignment operator
   MprnIceNominatedMsg& operator=(const MprnIceNominatedMsg& rhs)
   {
      if (this != &rhs)
      {
         MpResNotificationMsg::operator=(rhs);
         mRemoteIp   = rhs.mRemoteIp;
         mRemotePort = rhs.mRemotePort;
      }
      return *this;
   }

   /// IP address of the nominated remote candidate (e.g. "192.168.1.42")
   const UtlString& getRemoteIp()   const { return mRemoteIp; }

   /// UDP port of the nominated remote candidate
   int              getRemotePort() const { return mRemotePort; }

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlString mRemoteIp;
   int       mRemotePort;
};

#endif  // _MprnIceNominatedMsg_h_
