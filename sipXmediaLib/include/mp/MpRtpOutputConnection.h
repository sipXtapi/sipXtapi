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
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MpRtpOutputConnection_h_
#define _MpRtpOutputConnection_h_

#include "rtcp/RtcpConfig.h"

// FORWARD DECLARATIONS
class MprDejitter;
class MprToNet;
class MprFromNet;
class OsSocket;
class SdpCodec;
#ifdef INCLUDE_RTCP /* [ */
struct IRTCPSession;
struct IRTCPConnection;
#endif /* INCLUDE_RTCP ] */

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsMutex.h>
#include <mp/MpResource.h>
#include <mp/MpTypes.h>
#include "mediaInterface/IMediaTransportAdapter.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

/**
*  @brief Connection container for the inbound and outbound network paths to a
*  single remote party.
*/
class MpRtpOutputConnection : public MpResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum {
      NUM_PAYLOAD_TYPES = (2<<7),
      MAX_ACTIVE_PAYLOAD_TYPES = 10
   };

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpRtpOutputConnection(const UtlString& resourceName,
                         MpConnectionID myID, 
                         IRTCPSession *piRTCPSession);

     /// Destructor
   virtual
   ~MpRtpOutputConnection();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

#ifdef INCLUDE_RTCP /* [ */
     /// A new SSRC has been generated for the Session
   void reassignSSRC(int iSSRC);
#endif /* INCLUDE_RTCP ] */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// return the connection ID of this connection.
   inline MpConnectionID getConnectionId(void) const;

#ifdef INCLUDE_RTCP /* [ */
     /// Retrieve the RTCP Connection interface associated with this MpRtpOutputConnection
   IRTCPConnection *getRTCPConnection(void);
#endif /* INCLUDE_RTCP ] */

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

     /// Starts sending RTP and RTCP packets.
   void prepareStartSendRtp(IMediaTransportAdapter* pAdapter);
     /**<
     *  @note: Someday may be made protected, if MpVideoCallFlowGraph will not
     *         need access to it.
     */

     /// Stops sending RTP and RTCP packets.
   void prepareStopSendRtp();
     /**<
     *  @note: Someday may be made protected, if MpVideoCallFlowGraph will not
     *         need access to it.
     */

   MprToNet*          mpToNet;         ///< Outbound component: ToNet
   MpConnectionID     mMyID;           ///< ID within parent flowgraph
   UtlBoolean         mOutRtpStarted;  ///< Are we currently sending RTP stream?

#ifdef INCLUDE_RTCP /* [ */
   IRTCPSession    *mpiRTCPSession;    ///< RTCP Session Interface pointer
   IRTCPConnection *mpiRTCPConnection; ///< RTCP Connection Interface pointer

#endif /* INCLUDE_RTCP ] */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Default constructor
   MpRtpOutputConnection();

     /// Copy constructor (not implemented for this type)
   MpRtpOutputConnection(const MpRtpOutputConnection& rMpRtpOutputConnection);

     /// Assignment operator (not implemented for this type)
   MpRtpOutputConnection& operator=(const MpRtpOutputConnection& rhs);
};

/* ============================ INLINE METHODS ============================ */

MpConnectionID MpRtpOutputConnection::getConnectionId(void) const
{
   return mMyID;
}

#endif  // _MpRtpOutputConnection_h_
