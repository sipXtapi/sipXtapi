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

   enum {
      NUM_PAYLOAD_TYPES = (2<<7),
      MAX_ACTIVE_PAYLOAD_TYPES = 10
   };

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

#ifdef INCLUDE_RTCP /* [ */
     /// Retrieve the RTCP Connection interface associated with this MpRtpInputConnection
   IRTCPConnection *getRTCPConnection(void);
#endif /* INCLUDE_RTCP ] */

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

     /// Starts receiving RTP and RTCP packets.
   void prepareStartReceiveRtp(OsSocket& rRtpSocket, OsSocket& rRtcpSocket);
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

#endif  // _MpRtpInputConnection_h_
