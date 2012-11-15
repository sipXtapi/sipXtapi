//  
// Copyright (C) 2006-2012 SIPez LLC.  All rights reserved.
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
#include <mp/MprToNet.h>

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

     /// Set sockets to which data will be sent.
   void setSockets(OsSocket& rRtpSocket, OsSocket& rRtcpSocket);
     /**<
     *  @warning This method is not synchronous! I.e. it directly modifies
     *           resource structure without message passing.
     */

     /// Release sockets to which data will be sent.
   void releaseSockets();
     /**<
     *  @warning This method is not synchronous! I.e. it directly modifies
     *           resource structure without message passing.
     */

#ifdef INCLUDE_RTCP /* [ */
     /// A new SSRC has been generated for the Session
   void reassignSSRC(int iSSRC);
#endif /* INCLUDE_RTCP ] */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// @copydoc MpResource::setFlowGraph()
   OsStatus setFlowGraph(MpFlowGraphBase* pFlowGraph);

#ifdef INCLUDE_RTCP /* [ */
     /// Retrieve the RTCP Connection interface associated with this MpRtpOutputConnection
   IRTCPConnection *getRTCPConnection(void);
#endif /* INCLUDE_RTCP ] */

   inline RtpSRC getSSRC() const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   MprToNet*          mpToNet;         ///< Outbound component: ToNet
   UtlBoolean         mOutRtpStarted;  ///< Are we currently sending RTP stream?

#ifdef INCLUDE_RTCP /* [ */
   IRTCPSession    *mpiRTCPSession;    ///< RTCP Session Interface pointer
   IRTCPConnection *mpiRTCPConnection; ///< RTCP Connection Interface pointer

#endif /* INCLUDE_RTCP ] */

     /// @copydoc MpResource::processFrame()
   UtlBoolean processFrame();

     /// @copydoc MpResource::connectInput()
   UtlBoolean connectInput(MpResource& rFrom, int fromPortIdx, int toPortIdx);

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

RtpSRC MpRtpOutputConnection::getSSRC() const
{
   return mpToNet->getSSRC();
}

#endif  // _MpRtpOutputConnection_h_
