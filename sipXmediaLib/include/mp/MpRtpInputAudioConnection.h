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
class MpCallFlowGraph;
class MpFlowGraphBase;
class MpDecoderBase;
class MpResource;
class MprDecode;
class OsNotification;
class MprRecorder;

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpRtpInputConnection.h"
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
class MpRtpInputAudioConnection : public MpRtpInputConnection
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
   MpRtpInputAudioConnection(MpConnectionID myID, 
                             MpCallFlowGraph* pParent,
                             int samplesPerFrame, 
                             int samplesPerSec);

     /// Destructor
   virtual
   ~MpRtpInputAudioConnection();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Disables the input path of the connection.
   //virtual OsStatus disable();
     /**<
     *  @see See MpRtpInputConnection::disable() for more information.
     */

     /// Enables the input path of the connection.
   virtual OsStatus enable();
     /**<
     *  @see See MpRtpInputConnection::enable() for more information.
     */

     /// Starts receiving RTP and RTCP packets.
   void startReceiveRtp(SdpCodec* pCodecs[], int numCodecs,
                        OsSocket& rRtpSocket, OsSocket& rRtcpSocket);

     /// Stops receiving RTP and RTCP packets.
   void stopReceiveRtp(void);

     /// Save the port number that was assigned by the bridge.
   OsStatus setBridgePort(int port);

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

     /// Returns the resource to link to downstream resource's inPort.
   MpResource* getSourceResource(void);

     /// Retrieve the port number that was assigned by the bridge.
   int getBridgePort(void);

     /// Get decoder for this payload type
   MpDecoderBase* mapPayloadType(int payloadType);

     /// Disables or enables the premium sound.
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
   MpRtpInputAudioConnection();

     /// Copy constructor (not implemented for this type)
   MpRtpInputAudioConnection(const MpRtpInputAudioConnection& rMpRtpInputAudioConnection);

     /// Assignment operator (not implemented for this type)
   MpRtpInputAudioConnection& operator=(const MpRtpInputAudioConnection& rhs);

   MpFlowGraphBase*   mpFlowGraph;     ///< Parent flowgraph
   MprDecode*         mpDecode;        ///< Inbound component: Decoder
   int                mBridgePort;     ///< Where we are connected on the bridge
   JB_inst*           mpJB_inst;       ///< Pointer to JitterBuffer instance

   MpDecoderBase*     mpPayloadMap[NUM_PAYLOAD_TYPES];
                                       ///< Map RTP payload types to our decoders
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpRtpInputAudioConnection_h_
