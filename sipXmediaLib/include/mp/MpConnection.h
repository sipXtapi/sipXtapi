//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MpConnection_h_
#define _MpConnection_h_

#include "rtcp/RtcpConfig.h"

// FORWARD DECLARATIONS
class MpCallFlowGraph;
class MpDecoderBase;
class MprDecode;
class MprDejitter;
class MprEncode;
class MprToNet;
class MprFromNet;
class OsSocket;
class OsNotification;
class SdpCodec;
class MprRecorder;

typedef struct GIPSNETEQ_inst NETEQ_inst;

// SYSTEM INCLUDES
// #include <...>

// APPLICATION INCLUDES
#include "mp/JB/jb_typedefs.h"
#include "mp/MprFromNet.h"
#include "mp/MprToNet.h"
#include "mp/MpResource.h"
#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/IRTCPConnection.h"
#endif /* INCLUDE_RTCP ] */

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
class MpConnection
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   friend class MpCallFlowGraph;

   enum {
      NUM_PAYLOAD_TYPES = (2<<7),
      MAX_ACTIVE_PAYLOAD_TYPES = 10,
   };

   typedef enum
   {
      DisablePremiumSound,
      EnablePremiumSound
   } PremiumSoundOptions;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpConnection(MpCallFlowGraph* pParent, MpConnectionID myID,
                                 int samplesPerFrame, int samplesPerSec);

     /// Destructor
   virtual
   ~MpConnection();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Disables the input path of the connection.
   OsStatus disableIn();
     /**<
     *  @see See disable() for more information.
     */

     /// Disables the output path of the connection.
   OsStatus disableOut();
     /**<
     *  @see See disable() for more information.
     */

     /// Disables both paths, of the connection.
   OsStatus disable(); // Both in and out
     /**<
     *  Resources on the path(s) will also be disabled by these calls.
     *  If the flow graph is not "started", this call takes effect
     *  immediately.  Otherwise, the call takes effect at the start of the
     *  next frame processing interval.
     *
     *  @returns <b>OS_SUCCESS</b> - for now, these methods always return success
     */

     /// Enables the input path of the connection.
   OsStatus enableIn();
     /**<
     *  @see See enable() for more information.
     */

     /// Enables the output path of the connection.
   OsStatus enableOut();
     /**<
     *  @see See enable() for more information.
     */

     /// Enables both paths of the connection.
   OsStatus enable(); // Both in and out
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

     /// Starts sending RTP and RTCP packets.
   void startSendRtp(OsSocket& rRtpSocket, OsSocket& rRtcpSocket,
                     SdpCodec* pPrimary, SdpCodec* pDtmf, SdpCodec* pSecondary);

     /// Starts sending RTP and RTCP packets.
   void startSendRtp(SdpCodec& rCodec,
                     OsSocket& rRtpSocket, OsSocket& rRtcpSocket);

     /// Stops sending RTP and RTCP packets.
   void stopSendRtp(void);

     /// Starts receiving RTP and RTCP packets.
   void startReceiveRtp(SdpCodec* pCodecs[], int numCodecs,
                        OsSocket& rRtpSocket, OsSocket& rRtcpSocket);

     /// Stops receiving RTP and RTCP packets.
   void stopReceiveRtp(void);

     /// Save the port number that was assigned by the bridge.
   OsStatus setBridgePort(int port);

     /// Start sending DTMF tone.
   void startTone(int toneId);

     /// Stop sending DTMF tone.
   void stopTone(void);

#ifdef INCLUDE_RTCP /* [ */
     /// A new SSRC has been generated for the Session
   void reassignSSRC(int iSSRC);
#endif /* INCLUDE_RTCP ] */

     /// Add an RTP payload type to decoder instance mapping table
   void addPayloadType(int payloadId, MpDecoderBase* pDecoder);

     /// Remove an RTP payload type from decoder instance map
   void deletePayloadType(int payloadId);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Returns a pointer to the JB instance, creating it if necessary
   JB_inst* getJBinst(UtlBoolean optional = FALSE);
     /**<
     *  If the instance has not been created, but the argument "optional" is
     *  TRUE, then do not create it, just return NULL.
     */

     /// Returns the resource to link to upstream resource's outPort.
   MpResource* getSinkResource(void);

     /// Returns the resource to link to downstream resource's inPort.
   MpResource* getSourceResource(void);

     /// Retrieve the port number that was assigned by the bridge.
   int getBridgePort(void);

#ifdef INCLUDE_RTCP /* [ */
     /// Retrieve the RTCP Connection interface associated with this MpConnection
   IRTCPConnection *getRTCPConnection(void);
#endif /* INCLUDE_RTCP ] */

     /// Get decoder for this payload type
   MpDecoderBase* mapPayloadType(int payloadType);

     /// Disables or enables the GIPS premium sound.
   void setPremiumSound(PremiumSoundOptions op);

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

     /// Handle the FLOWGRAPH_SET_DTMF_NOTIFY message.
   UtlBoolean handleSetDtmfNotify(OsNotification* n);
     /**<
     *  @Returns <b>TRUE</b>
     */

   UtlBoolean setDtmfTerm(MprRecorder *pRecorder);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Default constructor
   MpConnection();

     /// Copy constructor (not implemented for this type)
   MpConnection(const MpConnection& rMpConnection);

     /// Assignment operator (not implemented for this type)
   MpConnection& operator=(const MpConnection& rhs);

   MpCallFlowGraph*   mpFlowGraph;     ///< Parent flowgraph
   MprEncode*         mpEncode;        ///< Outbound component: Encoder
   MprToNet*          mpToNet;         ///< Outbound component: ToNet
   MprFromNet*        mpFromNet;       ///< Inbound component: FromNet
   MprDejitter*       mpDejitter;      ///< Inbound component: Dejitter
   MprDecode*         mpDecode;        ///< Inbound component: Decoder
   MpConnectionID     mMyID;           ///< ID within parent flowgraph
   int                mBridgePort;     ///< Where we are connected on the bridge
   UtlBoolean         mInEnabled;      ///< Current state of inbound components
   UtlBoolean         mOutEnabled;     ///< Current state of outbound components
   UtlBoolean         mInRtpStarted;   ///< Are we currently receiving RTP stream?
   UtlBoolean         mOutRtpStarted;  ///< Are we currently sending RTP stream?
   JB_inst*           mpJB_inst;       ///< Pointer to JitterBuffer instance

   MpDecoderBase*     mpPayloadMap[NUM_PAYLOAD_TYPES];
                                       ///< Map RTP payload types to our decoders
   OsMutex            mLock;

#ifdef INCLUDE_RTCP /* [ */
     /// RTCP Connection Interface pointer
   IRTCPConnection *mpiRTCPConnection;

#endif /* INCLUDE_RTCP ] */
};

/* ============================ INLINE METHODS ============================ */
#ifdef INCLUDE_RTCP /* [ */
inline IRTCPConnection *MpConnection::getRTCPConnection(void)
{
    return(mpiRTCPConnection);

}

inline void MpConnection::reassignSSRC(int iSSRC)
{

//  Set the new SSRC
    mpToNet->setSSRC(iSSRC);

    return;

}
#endif /* INCLUDE_RTCP ] */

#endif  // _MpConnection_h_
