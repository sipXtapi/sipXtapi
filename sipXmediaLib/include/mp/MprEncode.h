//  
// Copyright (C) 2006-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MprEncode_h_
#define _MprEncode_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpAudioResource.h"
#include "sdp/SdpCodec.h"
#include "mp/MpFlowGraphMsg.h"
#include "mp/MpResourceMsg.h"
#include "mp/MprToNet.h"
#include "mp/MpMisc.h"
#include "mp/MpResampler.h"

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

   static const UtlContainableType TYPE; ///< Class name, used for run-time checks.

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     // Constructor
   MprEncode(const UtlString& rName);

     // Destructor
   virtual
   ~MprEncode();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Tell encoder which codecs to use.
   static OsStatus selectCodecs(const UtlString& namedResource,
                                OsMsgQ& fgQ,
                                SdpCodec* pPrimaryCodec,
                                SdpCodec* pDtmfCodec);

     /// Free selected codecs.
   static OsStatus deselectCodecs(const UtlString& namedResource,
                                  OsMsgQ& fgQ);

     /// Set ToNet resource which will send generated RTP packets.
   void setMyToNet(MprToNet* myToNet);
     /**<
     *  @warning This method is not synchronous! I.e. it directly modifies
     *           resource structure without message passing.
     */

     /// Send "begin tone" DTMF RTP packet.
   static OsStatus startTone(const UtlString& namedResource,
                             OsMsgQ& fgQ,
                             int toneId);

     /// Send "stop tone" DTMF RTP packet.
   static OsStatus stopTone(const UtlString& namedResource,
                            OsMsgQ& fgQ);

     /// Enable or disable internal DTX.
   static OsStatus enableDtx(const UtlString& namedResource,
                             OsMsgQ& fgQ,
                             UtlBoolean dtx);
     /**<
     *  @note Codec still may use its DTX features.
     */

     /// Set maximum duration of one packet in milliseconds.
   static OsStatus setMaxPacketTime(const UtlString& namedResource,
                                    OsMsgQ& fgQ,
                                    unsigned int maxPacketTime);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// @copydoc UtlContainable::getContainableType()
   UtlContainableType getContainableType() const;

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
                                     int samplesPerFrame,
                                     int samplesPerSecond);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   typedef enum
   {
      MPRM_DESELECT_CODECS = MpResourceMsg::MPRM_EXTERNAL_MESSAGE_START,
      MPRM_START_TONE,
      MPRM_STOP_TONE,
      MPRM_SET_MAX_PACKET_TIME,
      MPRM_ENABLE_DTX,
      MPRM_DISABLE_DTX
   } AddlResMsgTypes;


   enum {
      TONE_STOP_PACKETS = 3, ///< MUST BE > 0
      HANGOVER_PACKETS = 25  ///< At 20 ms each, 500 ms.
   };

   static const int RTP_KEEP_ALIVE_FRAME_INTERVAL;

///@name Audio codec state variables
//@{
   MpEncoderBase* mpPrimaryCodec;
   unsigned char* mpPacket1Payload; ///< Packet buffer for primary RTP stream
   int   mMaxPacketSamples;         ///< Maximum number of samples in RTP packet.
   int   mPacket1PayloadBytes;      ///< Size of mpPacket1Payload buffer
   int   mPayloadBytesUsed;         ///< Number of bytes in mpPacket1Payload,
                                    ///<  already filled with encoded data
   unsigned int mSamplesPacked;     ///< Number of samples already encoded
                                    ///<  to current packet.
   unsigned int mStartTimestamp1;   ///< Timestamp of packets being encoded.
   UtlBoolean mActiveAudio1;        ///< Does current RTP packet contain active voice?
   UtlBoolean mMarkNext1;           ///< Set Mark bit on next RTP packet
   int   mConsecutiveInactive1;     ///< Number of RTP packets with active voice data
   int   mConsecutiveActive1;
   int   mConsecutiveUnsentFrames1;
   UtlBoolean mDoesVad1;    ///< Does codec its own VAD?
   UtlBoolean mDisableDTX;  ///< Disable internal DTX.
   UtlBoolean mEnableG722Hack;   ///< Should we cheat with RTP clock rate in case
                                 ///< G.722 is selected. According to section 4.5.2
                                 ///< of RFC 3551 we should use 8K RTP clock rate
                                 ///< instead of 16K for G.722 because of historical
                                 ///< error in RFC 1890.
   UtlBoolean mDoG722Hack;       ///< Should we apply RTP clock rate halving to
                                 ///< workaround G.722 spec bug? See mEnableG722Hack
                                 ///< for better description.
//@}

///@name Resampler-related variables.
//@{
   UtlBoolean  mNeedResample;    ///< Is resampling needed?
   MpResamplerBase *mpResampler; ///< Resampler to convert flowgraph sample rate
                                 ///<  to codec's sample rate.
   unsigned int mResampleBufLen; ///< Length of mpResampleBuf.
   MpAudioSample *mpResampleBuf; ///< Temporary buffer used to store resampled
                                 ///< audio samples before passing them to encoder.
//@}

///@name DTMF codec state variables
//@{
   MpEncoderBase* mpDtmfCodec;
   unsigned char* mpPacket2Payload; ///< packet buffer for DTMF event RTP stream
   int   mPacket2PayloadBytes;      ///< 4
   unsigned int   mStartTimestamp2; ///< sample time when tone starts
   unsigned int   mLastDtmfSendTimestamp;
   int   mDtmfSampleInterval;       ///< # samples between AVT packets
   int   mCurrentTone;  ///< AVT event code for current tone
   int   mNumToneStops; ///< set to # of end packets to send when tone stops
   int   mTotalTime;    ///< # samples tone was active, set when tone stops
   UtlBoolean mNewTone;      ///< set when tone starts
//@}


///@name General encoding state
//@{
   unsigned int   mCurrentTimestamp;
   unsigned int   mMaxPacketTime;  ///< Maximum duration of one packet in milliseconds.

   MprToNet* mpToNet;  ///< Pointer to ToNet resource, which will send generated
                       ///< RTP packets.
//@}

     /// Handle resource messages for this resource.
   virtual UtlBoolean handleMessage(MpResourceMsg& rMsg);

     /// Allocate memory for RTP packet.
   OsStatus allocPacketBuffer(const MpEncoderBase& rEncoder,
                              unsigned char*& rpPacketPayload,
                              int& rPacketPayloadBytes);

   void handleSelectCodecs(int newCodecsCount, SdpCodec** newCodecs);

   void handleDeselectCodecs(void);

     /// Translate our tone ID into RFC2833 values.
   int lookupTone(int toneId);

     /// Handle message to send "begin tone" DTMF RTP packet.
   void handleStartTone(int toneId);

     /// Handle message to enable or disable internal DTX.
   void handleEnableDTX(UtlBoolean dtx);

     /// Handle message to set maximum duration of one packet.
   void handleSetMaxPacketTime(unsigned maxPacketTime);

     /// Handle message to send "stop tone" DTMF RTP packet.
   void handleStopTone(void);

     /// Encode audio buffer and send it.
   void doPrimaryCodec(MpAudioBufPtr in);

     /// Encode and send DTMF tone.
   void doDtmfCodec(int samplesPerFrame, int samplesPerSecond);

     /// Send notification about TX start.
   void notifyStartTx();

     /// Send notification about TX stop.
   void notifyStopTx();

     /// Copy constructor (not implemented for this class)
   MprEncode(const MprEncode& rMprEncode);

     /// Assignment operator (not implemented for this class)
   MprEncode& operator=(const MprEncode& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprEncode_h_
