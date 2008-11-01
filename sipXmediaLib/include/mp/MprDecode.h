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

#ifndef _MprDecode_h_ /* [ */
#define _MprDecode_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpMisc.h"
#include "mp/MpRtpBuf.h"
#include "mp/MpAudioResource.h"
#include "mp/MpFlowGraphMsg.h"
#include "mp/MpResourceMsg.h"
#include "mp/MpDecoderPayloadMap.h"
#include "mp/MpJitterBufferEstimation.h"
#include "sdp/SdpCodec.h"
#include "os/OsBSem.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class MpDecoderBase;
class MprRecorder;
class MpJitterBuffer;
class MprDejitter;
class MpPlcBase;
class OsNotification;

/// The "Decode" media processing resource
class MprDecode : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   static const UtlContainableType TYPE; ///< Class name, used for run-time checks.

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprDecode(const UtlString &rName,
             const UtlString &plcName = "");

     /// Destructor
   virtual
   ~MprDecode();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Reset decoder to the initial state to be able process new stream.
   static OsStatus reset(const UtlString& namedResource, OsMsgQ& fgQ);

     /// Reset decoder to the initial state to be able process new stream.
   OsStatus reset();

     /// Provide set of codecs this decode resource will be able to decode.
   static OsStatus selectCodecs(const UtlString& namedResource,
                                OsMsgQ& fgQ,
                                SdpCodec* codecs[],
                                int numCodecs);

     /// Clear set of codecs this resource is able to decode.
   static OsStatus deselectCodecs(const UtlString& namedResource,
                                  OsMsgQ& fgQ);

     /// Set DTMF notification.
   OsStatus setDtmfNotify(OsNotification *pNotify);
     /**<
     *  DEPRECATED! Listen for the DTMF notification messages instead.
     */
     
     /// Clear DTMF notification.
   OsStatus clearDtmfNotify();
     /**<
     *  DEPRECATED! Listen for the DTMF notification messages instead.
     */

     /// Change PLC algorithm to one with given name. THIS METHOD DOES NOT WORK!
   static OsStatus setPlc(const UtlString& namedResource,
                          OsMsgQ& fgQ,
                          const UtlString& plcName = "");
     /**<
     *  Sends an MPRM_SET_PLC message to the named MprDecode resource
     *  within the flowgraph who's queue is supplied. When the message 
     *  is received, the above resource will change PLC algorithm to
     *  chosen one.
     *
     *  @param[in] namedResource - the name of the resource to send a message to.
     *  @param[in] fgQ - the queue of the flowgraph containing the resource which
     *             the message is to be received by.
     *  @param[in] plcName - name of PLC algorithm to use.
     *             See MpJitterBuffer::setPlc() for more details.
     *  @returns the result of attempting to queue the message to this resource.
     */

     /// Pair this decode resource with the dejitter resource.
   void setMyDejitter(MprDejitter* newDJ, UtlBoolean ownDj);
     /**<
     *  If \p ownDj is TRUE, then dejitter instance will be freed in
     *  the destructor of this decoder.
     */

     /// Add incoming RTP packet to the decoding queue
   OsStatus pushPacket(const MpRtpBufPtr &pRtp);
     /**<
     *  @return OS_SUCCESS on success
     *  @return OS_LIMIT_REACHED if too many codecs used in incoming RTP packets
     */

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

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   typedef enum
   {
      SET_DTMF_NOTIFY = MpFlowGraphMsg::RESOURCE_SPECIFIC_START
   } AddlMsgTypes;

   typedef enum
   {
      MPRM_SET_PLC = MpResourceMsg::MPRM_EXTERNAL_MESSAGE_START,
      MPRM_DESELCT_CODECS,
      MPRM_RESET
   } AddlResMsgTypes;

   MpJitterBuffer* mpJB;            ///< Pointer to JitterBuffer instance
   UtlBoolean mIsJBInitialized;     ///< Is JB initialized or not?
   OsNotification* mpDtmfNotication;

   MprDejitter* mpMyDJ;             ///< Dejitter instance, used by this decoder.
   UtlBoolean   mOwnDJ;             ///< Is dejitter owned by this decoder?
   UtlBoolean mIsStreamInitialized; ///< Have we received at least one packet?
   struct StreamState
   {
      unsigned sampleRate;          ///< Sample rate of this stream.
      UtlBoolean isFirstRtpPulled;  ///< Have we got first packet from JB queue?
      RtpSeq rtpStreamSeq;          ///< Current sequence number of RTP stream.
      RtpTimestamp rtpStreamPosition; ///< Current pulling JB position (in RTP timestamp units).
      int32_t rtpStreamHint;        ///< Recommended playback position (in RTP timestamp units).
      uint32_t playbackStreamPosition; ///< Current playback position (in samples).
      unsigned playbackFrameSize;   ///< Stream frame size of this stream (in samples).
   } mStreamState;

   MpJitterBufferEstimation *mpJbEstimationState; ///< State of JB delay estimation.

   /// List of the codecs to be used to decode media.
   /**
   *  Pointer to array of length mNumCurrentCodecs of MpDecoderBase*'s
   *  which represent the codecs, or NULL if mNumCurrentCodecs == 0.
   */
   MpDecoderBase** mpCurrentCodecs;
   int             mNumCurrentCodecs; ///< Length of mpCurrentCodecs array.

   /// Similar list of all codecs that have ever been listed on mpCurrentCodecs.
   MpDecoderBase** mpPrevCodecs;
   int             mNumPrevCodecs; ///< Length of mpPrevCodecs array.

   MpDecoderPayloadMap mDecoderMap; ///< Mapping of payload types to decoder instances.

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond);

     /// Decode RTP packet if it belongs to signaling codec.
   UtlBoolean tryDecodeAsSignalling(const MpRtpBufPtr &rtp);
     /**<
     *  @retval TRUE - packet was decoded as signaling
     *  @retval FALSE - packet is not signaling
     */

     /// Handle old style messages for this resource.
   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);

     /// Handle new style messages for this resource.
   virtual UtlBoolean handleMessage(MpResourceMsg& rMsg);

     /// Replace mpCurrentCodecs with pCodecs.
   UtlBoolean handleSelectCodecs(SdpCodec* pCodecs[], int numCodecs);
     /**<
     *  Copy the codecs in mpCurrentCodecs onto mpPrevCodecs and deletes pCodecs.
     */

     /// @brief Remove all codecs from mpCurrentCodecs, transferring them to
     /// mpPrevCodecs.
   UtlBoolean handleDeselectCodecs(UtlBoolean shouldLock = TRUE);

     /// Remove one codec from mpConnection's payload type decoder table.
   UtlBoolean handleDeselectCodec(MpDecoderBase* pDecoder);
     /**<
     *  @note Caller must hold mLock.
     */

     /// Change PLC algorithm to one provided.
   UtlBoolean handleSetPlc(const UtlString &plcName);

     /// Handle the FLOWGRAPH_SET_DTMF_NOTIFY message.
   UtlBoolean handleSetDtmfNotify(OsNotification* n);
     /**<
     *  @returns <b>TRUE</b>
     */

     /// Handle MPRM_RESET message.
   UtlBoolean handleReset();

     /// Copy constructor (not implemented for this class)
   MprDecode(const MprDecode& rMprDecode);

     /// Assignment operator (not implemented for this class)
   MprDecode& operator=(const MprDecode& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  /* _MprDecode_h_ ] */
