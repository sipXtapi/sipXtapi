// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
//  
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MprRtpPrepareStartReceiveMsg_h_
#define _MprRtpPrepareStartReceiveMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpResourceMsg.h"
#include "os/OsMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Message object used to communicate with the media processing task
class MprRtpPrepareStartReceiveMsg : public MpResourceMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor
   MprRtpPrepareStartReceiveMsg(const UtlString& targetResourceName,
                         OsSocket& rRtpSocket,
                         OsSocket& rRtcpSocket)
      : MpResourceMsg(MPRM_PREPARE_START_RECEIVE_RTP, targetResourceName)
      , mpRtpSocket(&rRtpSocket)
      , mpRtcpSocket(&rRtcpSocket)
   {
   };

   /// Copy constructor
   MprRtpPrepareStartReceiveMsg(const MprRtpPrepareStartReceiveMsg& resourceMsg)
      : MpResourceMsg(resourceMsg)
      , mpRtpSocket(resourceMsg.mpRtpSocket)
      , mpRtcpSocket(resourceMsg.mpRtcpSocket)
   {
   };

   /// Create a copy of this msg object (which may be of a derived type)
   OsMsg* createCopy(void) const 
   {
      return new MprRtpPrepareStartReceiveMsg(*this); 
   }

   /// Destructor
   ~MprRtpPrepareStartReceiveMsg() 
   {
   };

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   /// Assignment operator
   MprRtpPrepareStartReceiveMsg& operator=(const MprRtpPrepareStartReceiveMsg& rhs)
   {
      if (this == &rhs) 
         return *this;  // handle the assignment to self case

      MpResourceMsg::operator=(rhs);  // assign fields for parent class

      mpRtpSocket = rhs.mpRtpSocket;
      mpRtcpSocket = rhs.mpRtcpSocket;

      return *this;
   }

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{

   OsSocket* getRtpSocket(){return(mpRtpSocket);};

   OsSocket* getRtcpSocket(){return(mpRtcpSocket);};

   //@}

   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{

   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    OsSocket* mpRtpSocket;
    OsSocket* mpRtcpSocket;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprRtpStartReceiveMsg_h_
