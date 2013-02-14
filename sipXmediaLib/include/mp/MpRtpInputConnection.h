//  
// Copyright (C) 2006-2013 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
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

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpResource.h>
#include <mp/MpResourceMsg.h>
#include <mp/MprRtpDispatcher.h>
#include <mp/MpTypes.h>
#include <utl/UtlString.h>
#include <os/OsMutex.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MprFromNet;
class MprRtpDispatcher;
class OsSocket;
struct IRTCPSession;
struct IRTCPConnection;

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
                        IRTCPSession *piRTCPSession = NULL,
                        int maxRtpStreams = 1,
                        MprRtpDispatcher::RtpStreamAffinity rtpStreamAffinity = MprRtpDispatcher::ADDRESS_AND_PORT);
     /**<
     *  @note If rtpStreamAffinity is set to ADDRESS_AND_PORT, then only one
     *        RTP stream is allowed, i.e. maxRtpStreams must be 1.
     */

     /// Destructor
   virtual
   ~MpRtpInputConnection();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Process one frame of audio
   UtlBoolean processFrame();

     /// Starts receiving RTP and RTCP packets.
   void setSockets(OsSocket& rRtpSocket, OsSocket& rRtcpSocket);
     /**<
     *  @warning This method is not synchronous! I.e. it directly modifies
     *           resource structure without message passing.
     */

     /// Stops receiving RTP and RTCP packets.
   void releaseSockets();
     /**<
     *  @warning This method is not synchronous! I.e. it directly modifies
     *           resource structure without message passing.
     */

     /// @copydoc MpResource::setConnectionId()
   void setConnectionId(MpConnectionID connectionId);

     /// @copydoc MpRtpDispatcher::setRtpInactivityTimeout()
   static OsStatus setRtpInactivityTimeout(const UtlString& namedResource,
                                           OsMsgQ& fgQ,
                                           int timeoutMs);

     /// @copydoc MprFromNet::setSsrcDiscard()
   static OsStatus enableSsrcDiscard(const UtlString& namedResource,
                                     OsMsgQ& fgQ,
                                     UtlBoolean enable, RtpSRC ssrc);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

#ifdef INCLUDE_RTCP /* [ */

//// DO WE STILL NEED THIS?

     /// Retrieve the RTCP Connection interface associated with this MpRtpInputConnection
   IRTCPConnection *getRTCPConnection();
#endif /* INCLUDE_RTCP ] */

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   enum
   {
      MPRM_SET_INACTIVITY_TIMEOUT = MpResourceMsg::MPRM_EXTERNAL_MESSAGE_START,
      MPRM_ENABLE_SSRC_DISCARD,
      MPRM_DISABLE_SSRC_DISCARD
   };

   MprFromNet*        mpFromNet;       ///< UDP to RTP converter
   MprRtpDispatcher*  mpRtpDispatcher; ///< RTP stream dispatcher
   int                mMaxRtpStreams;  ///< Maximum number of RTP streams
   MprRtpDispatcher::RtpStreamAffinity  mRtpStreamAffinity; ///< Algorithm used to dispatch incoming RTP packets
   UtlBoolean         mIsRtpStarted;   ///< Are we currently receiving RTP stream?

#ifdef INCLUDE_RTCP /* [ */
   IRTCPConnection *mpiRTCPConnection; ///< RTCP Connection Interface pointer
#endif /* INCLUDE_RTCP ] */

     /// @copydoc MpResource::connectOutput()
   UtlBoolean connectOutput(MpResource& rTo, int toPortIdx, int fromPortIdx);

     /// @copydoc MpResource::disconnectOutput()
   UtlBoolean disconnectOutput(int outPortIdx);

     /// @copydoc MpResource::setFlowGraph()
   OsStatus setFlowGraph(MpFlowGraphBase* pFlowGraph);

     /// Handle resource messages for this resource.
   virtual UtlBoolean handleMessage(MpResourceMsg& rMsg);

     /// Handle message to set MpRtpDispatcher inactivity timeout.
   void handleSetInactivityTimeout(const OsTime &timeout);

     /// Handle message to enable/disable stream discard.
   void handleEnableSsrcDiscard(UtlBoolean enable, RtpSRC ssrc);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Copy constructor (not implemented for this type)
   MpRtpInputConnection(const MpRtpInputConnection& rMpRtpInputConnection);

     /// Assignment operator (not implemented for this type)
   MpRtpInputConnection& operator=(const MpRtpInputConnection& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpRtpInputConnection_h_
