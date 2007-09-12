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


#ifndef _MpdPtAVT_h_
#define _MpdPtAVT_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpDecoderBase.h"
#include "mp/RtpHeader.h"

// FORWARD DECLARATIONS
class OsNotification;
class MpFlowGraphBase;

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
/// Payload data structure.
struct AvtPacket
{
   uint8_t  key;
   uint8_t  dB;  ///< Bit allocation: 0-5 - Volume; 6 - Reserved; 7 - End bit.
   uint16_t duration;
};

// TYPEDEFS

/**
*  @brief RFC2833 DTMF Decoder class (Formerly Pingtel's AVT/Tone decoder)
*
*  MpdPtAVT is a decoder class that is used for decoding RFC2833 DTMF tones
*  embedded in RTP signaling.
*  
*  @NOTE The Pingtel AVT decoder was created initially prior to the acceptance 
*        of rfc2833, hence the name of this class.
*  
*  TODO: This should be updated to support RFC4733 (which may not require any work)
*  
*  
*  http://tools.ietf.org/html/rfc2833
*  http://tools.ietf.org/html/rfc4733
*/
class MpdPtAVT: public MpDecoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpdPtAVT(int payloadType);
     /**<
     *  @param[in] payloadType - RTP payload type associated with this decoder
     *  @param[in] connId - The ID of the connection this 'decoder' is associated with.
     *  @param[in] pFlowGraph  - Pointer to the flowgraph that the decoder is part of. (used for notifications)
     */

     /// Destructor
   virtual ~MpdPtAVT(void);

     /// @copydoc MpDecoderBase::initDecode()
   virtual OsStatus initDecode();

     /// @copydoc MpDecoderBase::freeDecode()
   virtual OsStatus freeDecode(void);

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Inspect incoming RTP packet and signal DTMF events.
   virtual int decode(const MpRtpBufPtr &pPacket,
                      unsigned decodedBufferLength,
                      MpAudioSample *samplesBuffer);
     /**<
     *  @param[in] pPacket - Pointer to a media buffer
     *  @param[in] decodedBufferLength - Length of the samplesBuffer (in samples)
     *  @param[out] samplesBuffer - Buffer for decoded samples
     *  @retval 0 - always returns zero, as DTMF does not decode samples, it just
     *              signals DTMF events.
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// @copydoc MpDecoderBase::getInfo()
   virtual const MpCodecInfo* getInfo() const;

     /// @copydoc MpDecoderBase::getSignalingData()
   virtual OsStatus getSignalingData(uint8_t &event,
                                     UtlBoolean &isStarted,
                                     UtlBoolean &isStopped,
                                     uint16_t &duration);

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static const MpCodecInfo smCodecInfo; ///< static information about the codec

   UtlBoolean mHaveValidData;        ///< Does mLastRtpHeader and mLastPacketData
                                     ///< contain valid data?
   RtpHeader mLastRtpHeader;         ///< Copy of last received RTP packet header.
   AvtPacket mLastPacketData;        ///< Copy of last received RTP packet payload data.

   UtlBoolean mIsEventActive;        ///< TRUE, if some event is currently active.
   uint8_t mActiveEvent;             ///< The key ID of active event.
   RtpTimestamp mLastKeyUpTimetsamp; ///< The timestamp for last KEYUP event.
   RtpTimestamp mStartingTimestamp;  ///< The timestamp of active event.
   int32_t mLastEventDuration;       ///< Last reported event duration.

     /// Reset decoder to no-active-event state.
   void resetEventState();
};

#endif  // _MpdPtAVT_h_
