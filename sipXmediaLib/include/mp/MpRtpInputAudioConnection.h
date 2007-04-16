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
class OsNotification;
class MprRecorder;
class SdpCodec;

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

   typedef enum
   {
      DisablePremiumSound = FALSE,
      EnablePremiumSound = TRUE
   } PremiumSoundOptions;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpRtpInputAudioConnection(UtlString& resourceName,
                             MpConnectionID myID, 
                             MpFlowGraphBase* pParent,
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

     /// Starts receiving RTP and RTCP packets.
   void startReceiveRtp(SdpCodec* pCodecs[], int numCodecs,
                        OsSocket& rRtpSocket, OsSocket& rRtcpSocket);

     /// Stops receiving RTP and RTCP packets.
   void stopReceiveRtp(void);

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

     /// Get decoder for this payload type
   MpDecoderBase* mapPayloadType(int payloadType);

     /// Disables or enables the premium sound.
   void setPremiumSound(PremiumSoundOptions op);

   // TODO:  this should become a resource message handled by the resource:
     /// Handle the FLOWGRAPH_SET_DTMF_NOTIFY message.
   UtlBoolean handleSetDtmfNotify(OsNotification* n);
     /**<
     *  @Returns <b>TRUE</b>
     */

   // TODO: this is butt ugly.  The connection should not know anything
   // about a recorder.  This should be and event or something like that.
   UtlBoolean setDtmfTerm(MprRecorder *pRecorder);

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /// @brief perform the enable operation specific to the MpRtpInputAudioConnection
   virtual UtlBoolean handleEnable();

   /// @brief perform the disable operation specific to the MpRtpInputAudioConnection
   virtual UtlBoolean handleDisable();

   /// @brief This method does the real work for the media processing resource and 
   /// must be defined in each class derived from this one.
   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame=80,
                                     int samplesPerSecond=8000);

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
   JB_inst*           mpJB_inst;       ///< Pointer to JitterBuffer instance

   MpDecoderBase*     mpPayloadMap[NUM_PAYLOAD_TYPES];
                                       ///< Map RTP payload types to our decoders
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpRtpInputAudioConnection_h_
