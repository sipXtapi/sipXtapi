//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MprEncode_h_
#define _MprEncode_h_

#include "mp/MpMisc.h"

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpAudioResource.h"
#include "sdp/SdpCodec.h"
#include "mp/MpFlowGraphMsg.h"
#include "mp/MprToNet.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpEncoderBase;

/**
*  @brief The "Encode" media processing resource.
*/
class MprEncode : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   friend class MpRtpOutputAudioConnection;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     // Constructor
   MprEncode(const UtlString& rName, int samplesPerFrame, int samplesPerSec);

     // Destructor
   virtual
   ~MprEncode();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

   OsStatus selectCodecs(SdpCodec* pPrimaryCodec,
                         SdpCodec* pDtmfCodec);

   OsStatus deselectCodecs(void);

     /// Set ToNet resource which will send generated RTP packets.
   void setMyToNet(MprToNet* myToNet);

     /// Send "begin tone" DTMF RTP packet.
   OsStatus startTone(int toneId);

     /// Send "stop tone" DTMF RTP packet.
   OsStatus stopTone(void);

     /// Enable or disable internal DTX.
   OsStatus enableDTX(UtlBoolean dtx);
     /**<
     *  @note Codec still may use its DTX features.
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame=80,
                                     int samplesPerSecond=8000);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   typedef enum
   {
      SELECT_CODECS = MpFlowGraphMsg::RESOURCE_SPECIFIC_START,
      DESELECT_CODECS,
      START_TONE,
      STOP_TONE,
      ENABLE_DTX
   } AddlMsgTypes;

   enum {
      TONE_STOP_PACKETS = 3, ///< MUST BE > 0
      HANGOVER_PACKETS = 25  ///< At 20 ms each, 500 ms.
   };

   static const int RTP_KEEP_ALIVE_FRAME_INTERVAL;

   MpEncoderBase* mpPrimaryCodec;
   unsigned char* mpPacket1Payload; ///< Packet buffer for primary RTP stream
   int   mPacket1PayloadBytes;      ///< Size of mpPacket1Payload buffer
   int   mPayloadBytesUsed;         ///< Number of bytes in mpPacket1Payload,
                                    ///<  already filled with encoded data
   unsigned int mStartTimestamp1;
   UtlBoolean mActiveAudio1;        ///< Does current RTP packet contain active voice?
   UtlBoolean mMarkNext1;           ///< Set Mark bit on next RTP packet
   int   mConsecutiveInactive1;     ///< Number of RTP packets with active voice data
   int   mConsecutiveActive1;
   int   mConsecutiveUnsentFrames1;
   UtlBoolean mDoesVad1;    ///< Does codec its own VAD?
   UtlBoolean mDisableDTX;  ///< Disable internal DTX.

   MpEncoderBase* mpDtmfCodec;
   unsigned char* mpPacket2Payload; ///< packet buffer for DTMF event RTP stream
   int   mPacket2PayloadBytes;      ///< 4
   unsigned int   mStartTimestamp2; ///< sample time when tone starts
   unsigned int   mLastDtmfSendTimestamp;
   int   mDtmfSampleInterval;       ///< # samples between AVT packets
   int   mCurrentTone;  ///< AVT event code for current tone
   int   mNumToneStops; ///< set to # of end packets to send when tone stops
   int   mTotalTime;    ///< # samples tone was active, set when tone stops
   int   mNewTone;      ///< set when tone starts

   unsigned int   mCurrentTimestamp;

   MprToNet* mpToNet;  ///< Pointer to ToNet resource, which will send generated
                       ///< RTP packets.

     /// Handle messages for this resource.
   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);

     /// @brief Get maximum payload size, estimated from
     /// MpEncoderBase::getMaxPacketBits().
   int payloadByteLength(MpEncoderBase& rEncoder);

     /// Allocate memory for RTP packet.
   OsStatus allocPacketBuffer(MpEncoderBase& rEncoder,
                              unsigned char*& rpPacketPayload,
                              int& rPacketPayloadBytes);

   void handleSelectCodecs(MpFlowGraphMsg& rMsg);

   void handleDeselectCodecs(void);

     /// Translate our tone ID into RFC2833 values.
   int lookupTone(int toneId);

     /// Handle message to send "begin tone" DTMF RTP packet.
   void handleStartTone(int toneId);

     /// Handle message to enable or disable internal DTX.
   void handleEnableDTX(UtlBoolean dtx);

     /// Handle message to send "stop tone" DTMF RTP packet.
   void handleStopTone(void);

     /// Encode audio buffer and send it.
   void doPrimaryCodec(MpAudioBufPtr in, unsigned int startTs);

     /// Encode and send DTMF tone.
   void doDtmfCodec(unsigned int startTs, int sPFrame, int sPSec);

     /// Copy constructor (not implemented for this class)
   MprEncode(const MprEncode& rMprEncode);

     /// Assignment operator (not implemented for this class)
   MprEncode& operator=(const MprEncode& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprEncode_h_
