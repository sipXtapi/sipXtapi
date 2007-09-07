//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef SIPX_VIDEO // [

#ifndef _MpVideoCallFlowGraph_h_
#define _MpVideoCallFlowGraph_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "sdp/SdpCodec.h"
#include "os/OsSocket.h"

// Include RTCP if supported
#include "rtcp/RtcpConfig.h"
#ifdef INCLUDE_RTCP // [
#include "rtcp/RTCManager.h"
#endif // INCLUDE_RTCP ]

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpCaptureDeviceBase;
class MpConnection;
class MpCaptureTask;
class MpRemoteVideoTask;
class MpVideoStreamParams;

/// Video call flow graph encapsulate all video processing needed for video call.
class MpVideoCallFlowGraph
#ifdef INCLUDE_RTCP // [
   : public CBaseClass
   , public IRTCPNotify
#endif // INCLUDE_RTCP ]
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor
   MpVideoCallFlowGraph(MpCaptureDeviceBase *pCaptureDevice = NULL, const MpVideoStreamParams* pCaptureParams = NULL);
     /**<
     *  @param pCaptureDeviceManager - See mpDeviceManager. This object will be
     *                                 freed in destructor.
     */

     /// Destructor
   virtual
   ~MpVideoCallFlowGraph();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Starts sending RTP and RTCP packets.
   OsStatus startSendRtp(SdpCodec& rPrimaryCodec,
                         OsSocket& rRtpSocket, OsSocket& rRtcpSocket);

     /// Stops sending RTP and RTCP packets.
   void stopSendRtp();

     /// Starts receiving RTP and RTCP packets.
   OsStatus startReceiveRtp(SdpCodec* pCodecs[], int numCodecs,
                            OsSocket& rRtpSocket, OsSocket& rRtcpSocket);

     /// Stops receiving RTP and RTCP packets.
   void stopReceiveRtp();

     /// Set window for remote party display.
   void setVideoWindow(const void *hWnd);

//@}

     /// Set handle for window to which we output local preview video (global for all calls).
   static
   OsStatus setVideoPreviewWindow(void *hWnd);

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Get handle for window to which we output remote video.
   void *getVideoWindow() const;

//@}

     /// Get handle for window to which we output local preview video (global for all calls).
   static
   void* getVideoPreviewWindow() {return smpPreviewWindow;}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{


//@}

/* =========================== CALLBACKS ================================== */
///@name Callbacks
//@{

#ifdef INCLUDE_RTCP // [

/**
 * Method Name:  GetEventInterest()
 * Returns:     unsigned long - Mask of Event Interests
 * Description: The GetEventInterest() event method shall allow the dispatcher
 *              of notifications to access the event interests of a subscriber
 *              and use these wishes to dispatch RTCP event notifications
 */
    virtual unsigned long GetEventInterest(void);

/**
 * Method Name: LocalSSRCCollision()
 * Inputs:      IRTCPConnection *piRTCPConnection - Interface to
 *                                                   associated RTCP Connection
 *              IRTCPSession    *piRTCPSession    - Interface to associated
 *                                                   RTCP Session
 * Description: The LocalSSRCCollision() event method shall inform the
 *              recipient of a collision between the local SSRC and one
 *              used by one of the remote participants.
 */
    virtual void LocalSSRCCollision(IRTCPConnection    *piRTCPConnection, 
                                    IRTCPSession       *piRTCPSession);


/**
 * Method Name: RemoteSSRCCollision()
 * Inputs:      IRTCPConnection *piRTCPConnection - Interface to associated
 *                                                    RTCP Connection
 *              IRTCPSession    *piRTCPSession    - Interface to associated
 *                                                    RTCP Session
 * Description: The RemoteSSRCCollision() event method shall inform the
 *              recipient of a collision between two remote participants.
 */
    virtual void RemoteSSRCCollision(IRTCPConnection    *piRTCPConnection,  
                                     IRTCPSession       *piRTCPSession);


/**
 * Macro Name:  DECLARE_IBASE_M
 * Description: This implements the IBaseClass functions used and exposed by
 *              derived classes.
 */
DECLARE_IBASE_M

#endif // INCLUDE_RTCP ]

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   static void         *smpPreviewWindow;

   MpCaptureDeviceBase *mpCaptureDevice; ///< Video capture device we use in this call.
   MpConnection        *mpConnection;    ///< RTP input/output part.
   MpCaptureTask       *mpCaptureTask;   ///< Sink for frames generated by capture device.
   MpRemoteVideoTask   *mpRemoteVideoTask; ///< Task for rendering remote video.
   bool                 mReceiving;      ///< Does we listen for RTP?
   bool                 mSending;        ///< Does we send RTP?

#ifdef INCLUDE_RTCP // [
     /// RTCP session interface pointer for this video call
   IRTCPSession* mpiRTCPSession;
     /// Event Interest Attribute for RTCP Notifications
   unsigned long mulEventInterest;
#endif // INCLUDE_RTCP ]

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpVideoCallFlowGraph_h_

#endif // SIPX_VIDEO ]
