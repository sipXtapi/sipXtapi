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


#ifndef _MpJitterBuffer_h_
#define _MpJitterBuffer_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpRtpBuf.h"
#include "mp/MpTypes.h"
#include "mp/MpResampler.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpDecoderBase;

/// Class for managing dejitter/decode of incoming RTP.
class MpJitterBuffer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpJitterBuffer(unsigned int sampleRate);

     /// Destructor
   virtual
   ~MpJitterBuffer();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Push packet into jitter buffer.
   OsStatus pushPacket(MpRtpBufPtr &rtpPacket);
     /**<
     *  Packet will be decoded and decoded data will be copied to internal buffer.
     *  If no decoder is available for this packet's payload type packet will be
     *  ignored.
     *
     *  @note This implementation does not check packets sequence number in any
     *  manner. So it behave very bad if packets come reordered or if some
     *  packets are missed.
     *
     *  @retval OS_SUCCESS if RTP packet was successfully decoded.
     *  @retval OS_FAILED in case of any problems.
     */

     /// Get samples from jitter buffer
   int getSamples(MpAudioSample *samplesBuffer, int samplesNumber);
     /**<
     *  @param voiceSamples - (out) buffer for audio samples
     *  @param samplesNumber - (in) number of samples to write
     *  
     *  @return Number of samples written to samplesBuffer
     */
   
     /// Set available decoders.
   int setCodecList(MpDecoderBase** codecList, int codecCount);
     /**<
     *  This function iterates through the provided list of decoders and fills
     *  internal payload type to decoder mapping. See payloadMap.
     *  
     *  @returns Always return 0 for now.
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

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   static const int JbPayloadMapSize = 128;
   static const int JbQueueSize = (12 * (2 * 80)); // 24 packets 10ms each
                                                   // or 12 packets, 20 mS each
                                                   // or 4 packets 60 mS each.
   static const int JbResampleBufSize = (6 * 160); ///< Size of temporary resample buffer.

   unsigned int mSampleRate;        ///< Output sample rate for decoded data.
                                    ///< Samples from codecs with different
                                    ///< sample rate will be resampled to this
                                    ///< sample rate.
   MpAudioSample JbResampleBuf[JbResampleBufSize]; ///< Buffer, used to temporarily
                                    ///< store decoded data if it need to be
                                    ///< resampled.
   MpResamplerBase *mpResampler;    ///< Resampler instance to convert codec
                                    ///< sample rate to flowgraph sample rate.

   int JbQCount;                    ///< Number of decoded samples in JbQ.
   int JbQIn;                       ///< Write pointer position in JbQ.
   int JbQOut;                      ///< Read pointer position in JbQ.
   MpAudioSample JbQ[JbQueueSize];  ///< Buffer for decoded audio.

   MpDecoderBase* payloadMap[JbPayloadMapSize]; ///< Map of RTP payload types
                                    ///< to decoders.

   /// Copy constructor
   MpJitterBuffer(const MpJitterBuffer& rMpJitterBuffer);

   /// Assignment operator
   MpJitterBuffer& operator=(const MpJitterBuffer& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpJitterBuffer_h_
