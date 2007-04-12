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
class SdpCodec;
#ifdef INCLUDE_RTCP /* [ */
struct IRTCPSession;
struct IRTCPConnection;
#endif /* INCLUDE_RTCP ] */

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsMutex.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef int MpConnectionID;

/**
*  @brief Connection container for the inbound and outbound network paths to a
*  single remote party.
*/
class MpRtpInputConnection
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
   MpRtpInputConnection(MpConnectionID myID, IRTCPSession *piRTCPSession);

     /// Destructor
   virtual
   ~MpRtpInputConnection();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Disables input path, of the connection.
   //virtual OsStatus disable(); // Both in 
     /**<
     *  Resources on the path(s) will also be disabled by these calls.
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *
     *  @returns <b>OS_SUCCESS</b> - for now, these methods always return success
     */

     /// Enables input path of the connection.
   virtual OsStatus enable();
     /**<
     *  Resources on the path(s) will also be enabled by these calls.
     *  Resources may allocate needed data (e.g. output path reframe buffer)
     *   during this operation.
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *
     *  @returns <b>OS_SUCCESS</b> - for now, these methods always return success
     */

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

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Return pointer to dejitter component of connection
   MprDejitter *getDejitter() const {return mpDejitter;}
     /**<
     *  This component should be used to receive RTP packets from remote party.
     */

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

   MprFromNet*        mpFromNet;       ///< Inbound component: FromNet
   MprDejitter*       mpDejitter;      ///< Inbound component: Dejitter
   MpConnectionID     mMyID;           ///< ID within parent flowgraph
   UtlBoolean         mInEnabled;      ///< Current state of inbound components
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
