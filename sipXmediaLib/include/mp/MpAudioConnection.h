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


#ifndef _MpAudioConnection_h_
#define _MpAudioConnection_h_

// FORWARD DECLARATIONS
class MpCallFlowGraph;
class MpFlowGraphBase;
class MpDecoderBase;
class MpResource;
class MprDecode;
class MprEncode;
class OsNotification;
class MprRecorder;

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpConnection.h"
#include "mp/JB/jb_typedefs.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

/**
*  @brief Connection container for audio part of call.
*/
class MpAudioConnection : public MpConnection
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   friend class MpCallFlowGraph;

   typedef enum
   {
      DisablePremiumSound = FALSE,
      EnablePremiumSound = TRUE
   } PremiumSoundOptions;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpAudioConnection(MpConnectionID myID, MpCallFlowGraph* pParent,
                     int samplesPerFrame, int samplesPerSec);

     /// Destructor
   virtual
   ~MpAudioConnection();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Disables the input path of the connection.
   OsStatus disableIn();
     /**<
     *  @see See MpConnection::disable() for more information.
     */

     /// Disables the output path of the connection.
   OsStatus disableOut();
     /**<
     *  @see See MpConnection::disable() for more information.
     */

     /// Enables the input path of the connection.
   OsStatus enableIn();
     /**<
     *  @see See MpConnection::enable() for more information.
     */

     /// Enables the output path of the connection.
   OsStatus enableOut();
     /**<
     *  @see See MpConnection::enable() for more information.
     */

     /// Starts sending RTP and RTCP packets.
   void startSendRtp(OsSocket& rRtpSocket, OsSocket& rRtcpSocket,
                     SdpCodec* pPrimary, SdpCodec* pDtmf);

     /// Starts sending RTP and RTCP packets.
   void startSendRtp(SdpCodec& rCodec,
                     OsSocket& rRtpSocket, OsSocket& rRtcpSocket);

     /// Stops sending RTP and RTCP packets.
   void stopSendRtp();

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
   MpAudioConnection();

     /// Copy constructor (not implemented for this type)
   MpAudioConnection(const MpAudioConnection& rMpAudioConnection);

     /// Assignment operator (not implemented for this type)
   MpAudioConnection& operator=(const MpAudioConnection& rhs);

   MpFlowGraphBase*   mpFlowGraph;     ///< Parent flowgraph
   MprEncode*         mpEncode;        ///< Outbound component: Encoder
   MprDecode*         mpDecode;        ///< Inbound component: Decoder
   int                mBridgePort;     ///< Where we are connected on the bridge
   JB_inst*           mpJB_inst;       ///< Pointer to JitterBuffer instance

   MpDecoderBase*     mpPayloadMap[NUM_PAYLOAD_TYPES];
                                       ///< Map RTP payload types to our decoders
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpAudioConnection_h_
