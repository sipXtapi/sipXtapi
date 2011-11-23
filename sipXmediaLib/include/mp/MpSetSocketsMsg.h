//  
// Copyright (C) 2007-2011 SIPez LLC.  Al rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpSetSocketsMsg_h_
#define _MpSetSocketsMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpResourceMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Message used to set gains in bridge mix matrix.
class MpSetSocketsMsg : public MpResourceMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpSetSocketsMsg(const UtlString& targetResourceName,
                   OsSocket* rtpSocket,
                   OsSocket* rtcpSocket)
      : MpResourceMsg(MPRM_SET_SOCKETS, targetResourceName)
      , mRtpSocket(rtpSocket)
      , mRtcpSocket(rtcpSocket)
   {};

     /// Copy constructor
   MpSetSocketsMsg(const MpSetSocketsMsg& resourceMsg)
      : MpResourceMsg(resourceMsg)
      , mRtpSocket(resourceMsg.mRtpSocket)
      , mRtcpSocket(resourceMsg.mRtcpSocket)
   {};

     /// Create a copy of this msg object (which may be of a derived type)
   OsMsg* createCopy(void) const 
   {
      return(new MpSetSocketsMsg(*this)); 
   }

     /// Destructor
   ~MpSetSocketsMsg() {};

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Assignment operator
   MpSetSocketsMsg& operator=(const MpSetSocketsMsg& rhs)
   {
      if(&rhs == this)
      {
         return(*this);
      }

      MpResourceMsg::operator=(rhs);
      mRtpSocket = rhs.mRtpSocket;
      mRtcpSocket = rhs.mRtcpSocket;

      return *this;
   }

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   OsSocket* getRtpSocket() const {return mRtpSocket;}
   OsSocket* getRtcpSocket() const {return mRtcpSocket;}

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsSocket* mRtpSocket;   ///< RTP socket
   OsSocket* mRtcpSocket;  ///< RTCP socket
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpSetSocketsMsg_h_
