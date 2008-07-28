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


#ifndef _MpRtpInputConnection_h_
#define _MpRtpInputConnection_h_

#include "rtcp/RtcpConfig.h"

// FORWARD DECLARATIONS
class MprDejitter;
class MprFromNet;
class OsSocket;
#ifdef INCLUDE_RTCP /* [ */
struct IRTCPSession;
struct IRTCPConnection;
#endif /* INCLUDE_RTCP ] */

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsMutex.h>
#include <mp/MpResource.h>
#include <mp/MpTypes.h>

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
class MpRtpInputConnection : public MpResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpRtpInputConnection(const UtlString& resourceName,
                        MpConnectionID myID, 
                        IRTCPSession *piRTCPSession);

     /// Destructor
   virtual
   ~MpRtpInputConnection();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{


//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// return the connection ID of this connection.
   inline MpConnectionID getConnectionId(void) const;

#ifdef INCLUDE_RTCP /* [ */
     /// Retrieve the RTCP Connection interface associated with this MpRtpInputConnection
   IRTCPConnection *getRTCPConnection(void);
#endif /* INCLUDE_RTCP ] */

   MprFromNet* getFromNet() { return mpFromNet; }
//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

     /// Starts receiving RTP and RTCP packets.
   void doPrepareStartReceiveRtp(OsSocket& rRtpSocket, OsSocket& rRtcpSocket);
     /**<
     *  @note: Someday may be made protected, if MpVideoCallFlowGraph will not
     *         need access to it.
     */

     /// Stops receiving RTP and RTCP packets.
   void prepareStopReceiveRtp();
     /**<
     *  @note: Someday may be made protected, if MpVideoCallFlowGraph will not
     *         need access to it.
     */

   MprFromNet*        mpFromNet;       ///< Inbound component: FromNet
   MprDejitter*       mpDejitter;      ///< Inbound component: Dejitter
   MpConnectionID     mMyID;           ///< ID within parent flowgraph
   UtlBoolean         mInRtpStarted;   ///< Are we currently receiving RTP stream?
   OsMutex            mLock;

#ifdef INCLUDE_RTCP /* [ */
   IRTCPSession    *mpiRTCPSession;    ///< RTCP Session Interface pointer
   IRTCPConnection *mpiRTCPConnection; ///< RTCP Connection Interface pointer

#endif /* INCLUDE_RTCP ] */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Default constructor
   MpRtpInputConnection();

     /// Copy constructor (not implemented for this type)
   MpRtpInputConnection(const MpRtpInputConnection& rMpRtpInputConnection);

     /// Assignment operator (not implemented for this type)
   MpRtpInputConnection& operator=(const MpRtpInputConnection& rhs);
};

/* ============================ INLINE METHODS ============================ */

MpConnectionID MpRtpInputConnection::getConnectionId(void) const
{
   return mMyID;
}

#endif  // _MpRtpInputConnection_h_
