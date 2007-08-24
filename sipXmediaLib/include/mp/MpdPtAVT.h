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

// FORWARD DECLARATIONS

class OsNotification;

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
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

     /// @copydoc MpDecoderBase::setDtmfNotify()
   virtual UtlBoolean setDtmfNotify(OsNotification* n);

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

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static const MpCodecInfo smCodecInfo; ///< static information about the codec

   int mCurrentToneKey;                ///< The key ID
   unsigned int mPrevToneSignature;    ///< The timestamp for last KEYUP event
   unsigned int mCurrentToneSignature; ///< The starting timestamp
   unsigned int mToneDuration;         ///< last reported duration
   OsNotification* mpNotify;     ///< Object to signal on key-down/key-up events

   void signalKeyDown(const MpRtpBufPtr &pPacket);
   void signalKeyUp(const MpRtpBufPtr &pPacket);
};

#endif  // _MpdPtAVT_h_
