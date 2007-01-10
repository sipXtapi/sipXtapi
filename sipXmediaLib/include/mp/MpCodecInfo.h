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


#ifndef _MpCodecInfo_h_
#define _MpCodecInfo_h_

// SYSTEM INCLUDES
#include "sdp/SdpCodec.h"

// APPLICATION INCLUDES

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief Static information describing a codec.
*/
class MpCodecInfo
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpCodecInfo(
      SdpCodec::SdpCodecTypes codecType, ///< codec type
      const char* codecVersion,   ///< string identifying the codec version
      UtlBoolean  usesNetEq,      ///< indicats whether the codec uses GIPS NetEq
      unsigned    samplingRate,   ///< sampling rate for the PCM data expected
                                  ///< by the codec
      unsigned    numBitsPerSample, 
      unsigned    numChannels,    ///< number of channels supported by the codec
      unsigned    interleaveBlockSize, ///< size of the interleave block (in samples)
      unsigned    bitRate,        ///< bit rate for this codec (in bits per second)
      unsigned    minPacketBits,  ///< minimum number of bits in an encoded frame
      unsigned    avgPacketBits,  ///< average number of bits in an encoded frame
      unsigned    maxPacketBits,  ///< maximum number of bits in an encoded frame
      unsigned    numSamplesPerFrame, ///< number of PCM samples per input frame
                                      ///< for this codec
      unsigned    preCodecJitterBufferSize = 0, ///< requested length of jitter
                                  ///< buffer that the flowgraph should apply to
                                  ///< the stream prior to getting packets for
                                  ///< the codec. If set to 0, then there is NO
                                  ///< jitter buffer, which implies that the
                                  ///< codec itself is doing the JB function.
      UtlBoolean  signalingCodec = FALSE,
      UtlBoolean  doesVadCng = FALSE);

     /// Copy constructor
   MpCodecInfo(const MpCodecInfo& rMpCodecInfo);

     /// Destructor
   virtual
   ~MpCodecInfo();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Returns the codec type
   SdpCodec::SdpCodecTypes getCodecType(void) const;

     /// Returns a string identifying the codec version
   UtlString getCodecVersion(void) const;

     /// Returns the sampling rate for the PCM data expected by the codec
   unsigned getSamplingRate(void) const;

     /// Returns the PCM data sample size (in bits)
   unsigned getNumBitsPerSample(void) const;

     /// Returns the number of PCM samples per input frame for this codec
   unsigned getNumSamplesPerFrame(void) const;

     /// Returns the number of channels supported by the codec
   unsigned getNumChannels(void) const;

     /// Returns the size of the interleave block (in samples)
   unsigned getInterleaveBlockSize(void) const;
     /**<
     *  This value is not meaningful if the number of channels for the
     *  codec is equal to 1.
     */

     /// Returns the bit rate for this codec (in bits per second)
   unsigned getBitRate(void) const;
     /**<
     *  If the codec is variable rate, then the average expected bit rate
     *  should be returned.
     */

     /// Returns the minimum number of bits in an encoded frame
   unsigned getMinPacketBits(void) const;

     /// Returns the average number of bits in an encoded frame
   unsigned getAvgPacketBits(void) const;

     /// Returns the maximum number of bits in an encoded frame
   unsigned getMaxPacketBits(void) const;

     /// @brief Returns the length of the jitter buffer that the flowgraph should
     /// apply to the stream prior to getting packets for the codec.
   unsigned getPreCodecJitterBufferSize(void) const;
     /**<
     *  If set to 0, then there is NO jitter buffer, which implies that the codec
     *  itself is doing the JB function.
     */

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

     /// Returns TRUE if codec uses GIPS NetEq; otherwise returns FALSE
   UtlBoolean usesNetEq(void) const;

     /// Returns TRUE if codec is used for signaling; otherwise returns FALSE
   UtlBoolean isSignalingCodec(void) const;

     /// Returns TRUE if codec does its own VAD and CNG; otherwise returns FALSE
   UtlBoolean doesVadCng(void) const;

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   SdpCodec::SdpCodecTypes mCodecType;
   UtlString    mCodecVersion;
   UtlBoolean   mUsesNetEq;
   unsigned    mSamplingRate;
   unsigned    mNumBitsPerSample;
   unsigned    mNumSamplesPerFrame;
   unsigned    mNumChannels;
   unsigned    mInterleaveBlockSize;
   unsigned    mBitRate;
   unsigned    mMinPacketBits;
   unsigned    mAvgPacketBits;
   unsigned    mMaxPacketBits;
   unsigned    mPreCodecJitterBufferSize;
   UtlBoolean   mIsSignalingCodec;
   UtlBoolean   mDoesVadCng;

     /// Assignment operator
   MpCodecInfo& operator=(const MpCodecInfo& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpCodecInfo_h_
