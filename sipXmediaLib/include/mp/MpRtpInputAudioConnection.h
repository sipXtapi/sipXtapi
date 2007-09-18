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


#ifndef _MpRtpInputAudioConnection_h_
#define _MpRtpInputAudioConnection_h_

// FORWARD DECLARATIONS
class MpFlowGraphBase;
class MpDecoderBase;
class MpResource;
class MprDecode;
class SdpCodec;
class OsNotification;

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpRtpInputConnection.h"

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
class MpRtpInputAudioConnection : public MpRtpInputConnection
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpRtpInputAudioConnection(const UtlString& resourceName,
                             MpConnectionID myID, 
                             int samplesPerFrame, 
                             int samplesPerSec);

     /// Destructor
   virtual
   ~MpRtpInputAudioConnection();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Process one frame of audio
   UtlBoolean processFrame(void);

     /// Add an RTP payload type to decoder instance mapping table
   void addPayloadType(int payloadId, MpDecoderBase* pDecoder);

     /// Remove an RTP payload type from decoder instance map
   void deletePayloadType(int payloadId);

     /// Handle the FLOWGRAPH_SET_DTMF_NOTIFY message.
   UtlBoolean handleSetDtmfNotify(OsNotification* n);
     /**<
     *  @todo This should become a resource message handled by the resource!
     *
     *  @Returns <b>TRUE</b>
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Get decoder for this payload type
   MpDecoderBase* mapPayloadType(int payloadType);

     /// Queue a message to start receiving RTP and RTCP packets.
   static OsStatus startReceiveRtp(OsMsgQ& messageQueue,
                                   const UtlString& resourceName,
                                   SdpCodec* pCodecs[], 
                                   int numCodecs,
                                   OsSocket& rRtpSocket, 
                                   OsSocket& rRtcpSocket);


     /// Queue a message to stop receiving RTP and RTCP packets.
   static OsStatus stopReceiveRtp(OsMsgQ& messageQueue,
                                  const UtlString& resourceName);

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

     /// Handles an incoming resource message for this media processing object.
   virtual UtlBoolean handleMessage(MpResourceMsg& rMsg);
     /**<
     *  @returns TRUE if the message was handled, otherwise FALSE.
     */

     /// Perform the enable operation specific to the MpRtpInputAudioConnection
   virtual UtlBoolean handleEnable();

     /// Perform the disable operation specific to the MpRtpInputAudioConnection
   virtual UtlBoolean handleDisable();

     /// Starts receiving RTP and RTCP packets.
   void handleStartReceiveRtp(SdpCodec* pCodecs[], int numCodecs,
                              OsSocket& rRtpSocket, OsSocket& rRtcpSocket);

     /// Stops receiving RTP and RTCP packets.
   void handleStopReceiveRtp(void);

     /// Sets the flowgraph in any child resources of this resource.
   virtual OsStatus setFlowGraph(MpFlowGraphBase* pFlowGraph);
     /**<
     *  @param[in] pFlowGraph - pointer to a flowgraph to store for future use.
     *  
     *  @see MpResource::setFlowGraph()
     */

     /// Sets the notification enabled status in any child resources of this resource.
   virtual OsStatus setNotificationsEnabled(UtlBoolean enable);
     /**<
     *  @see MpResource::setNotificationsEnabled()
     */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Copy constructor (not implemented for this type)
   MpRtpInputAudioConnection(const MpRtpInputAudioConnection& rMpRtpInputAudioConnection);

     /// Assignment operator (not implemented for this type)
   MpRtpInputAudioConnection& operator=(const MpRtpInputAudioConnection& rhs);

   MprDecode*         mpDecode;        ///< Inbound component: Decoder

   MpDecoderBase*     mpPayloadMap[NUM_PAYLOAD_TYPES];
                                       ///< Map RTP payload types to our decoders
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpRtpInputAudioConnection_h_
