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


#ifndef _MpJitterBuffer_h_
#define _MpJitterBuffer_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpRtpBuf.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
static const int JbPayloadMapSize = 128;
static const int JbQueueSize = (9 * (2 * 80)); // 9 packets, 20 mS each
                                               // or 3 packets 60 mS each.

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
   MpJitterBuffer();

     /// Destructor
   virtual
   ~MpJitterBuffer();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Push packet into jitter buffer.
   int pushPacket(MpRtpBufPtr &rtpPacket);
     /**<
     *  Packet will be decoded and decoded data will be copied to internal buffer.
     *  If no decoder is available for this packet's payload type packet will be
     *  ignored.
     *
     *  @note This implementation does not check packets sequence number in any
     *  manner. So it behave very bad if packets come reordered or if some
     *  packets are missed.
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

     /// Copy constructor
   MpJitterBuffer(const MpJitterBuffer& rMpJitterBuffer);

     /// Assignment operator
   MpJitterBuffer& operator=(const MpJitterBuffer& rhs);

   int JbQCount;
   int JbQIn;
   int JbQOut;
   MpAudioSample JbQ[JbQueueSize];

   MpDecoderBase* payloadMap[JbPayloadMapSize];
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpJitterBuffer_h_
