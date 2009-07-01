//  
// Copyright (C) 2006-2009 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2009 SIPfoundry Inc.
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
// APPLICATION INCLUDES
#include "mp/codecs/PlgDefsV1.h"
#include <utl/UtlString.h>

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
class MpCodecInfo : protected MppCodecInfoV1_1, protected MppCodecFmtpInfoV1_1
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Full constructor
   inline MpCodecInfo(const MppCodecInfoV1_1 &codecInfo,
                      const MppCodecFmtpInfoV1_1 &fmtpInfo);

     /// Partial constructor
   inline MpCodecInfo(const MppCodecInfoV1_1 &codecInfo);

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// @copydoc MppCodecInfoV1_1::codecManufacturer
   inline const char*  getCodecManufacturer() const;
     /// @copydoc MppCodecInfoV1_1::codecName
   inline const char*  getCodecName() const;
     /// @copydoc MppCodecInfoV1_1::codecVersion
   inline const char*  getCodecVersion() const;
     /// @copydoc MppCodecInfoV1_1::codecType
   inline unsigned     getCodecType() const;

     /// @copydoc MppCodecInfoV1_1::mimeSubtype
   inline const char*  getMimeSubtype() const;
     /// @copydoc MppCodecInfoV1_1::fmtpsNum
   inline unsigned     getFmtpsNum() const;
     /// @copydoc MppCodecInfoV1_1::fmtps
   inline const char** getFmtps() const;
     /// @copydoc MppCodecInfoV1_1::sampleRate
   inline unsigned     getSampleRate() const;
     /// @copydoc MppCodecInfoV1_1::numChannels
   inline unsigned     getNumChannels() const;
     /// @copydoc MppCodecInfoV1_1::framePacking
   inline unsigned     getFramePacking() const;

     /// @copydoc MppCodecFmtpInfoV1_1::minBitrate
   inline unsigned     getMinBitrate() const;
     /// @copydoc MppCodecFmtpInfoV1_1::maxBitrate
   inline unsigned     getMaxBitrate() const;
     /// @copydoc MppCodecFmtpInfoV1_1::numSamplesPerFrame
   inline unsigned     getNumSamplesPerFrame() const;
     /// @copydoc MppCodecFmtpInfoV1_1::minFrameBytes
   inline unsigned     getMinFrameBytes() const;
     /// @copydoc MppCodecFmtpInfoV1_1::maxFrameBytes
   inline unsigned     getMaxFrameBytes() const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

     /// @copydoc MppCodecFmtpInfoV1_1::signalingCodec
   inline UtlBoolean isSignalingCodec(void) const;
     /// @copydoc MppCodecFmtpInfoV1_1::doesVadCng
   inline UtlBoolean doesVadCng(void) const;
     /// @copydoc MppCodecFmtpInfoV1_1::haveInternalPLC
   inline UtlBoolean haveInternalPLC() const;

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

/* ============================ INLINE METHODS ============================ */

MpCodecInfo::MpCodecInfo(const MppCodecInfoV1_1 &codecInfo,
                         const MppCodecFmtpInfoV1_1 &fmtpInfo)
: MppCodecInfoV1_1(codecInfo)
, MppCodecFmtpInfoV1_1(fmtpInfo)
{
}

MpCodecInfo::MpCodecInfo(const MppCodecInfoV1_1 &codecInfo)
: MppCodecInfoV1_1(codecInfo)
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

const char* MpCodecInfo::getCodecManufacturer() const
{
   return codecManufacturer;
}

const char* MpCodecInfo::getCodecName() const
{
   return codecName;
}

const char* MpCodecInfo::getCodecVersion() const
{
   return codecVersion;
}

unsigned MpCodecInfo::getCodecType() const
{
   return codecType;
}


const char* MpCodecInfo::getMimeSubtype() const
{
   return mimeSubtype;
}

unsigned MpCodecInfo::getFmtpsNum() const
{
   return fmtpsNum;
}

const char** MpCodecInfo::getFmtps() const
{
   return fmtps;
}

unsigned MpCodecInfo::getSampleRate() const
{
   return sampleRate;
}

unsigned MpCodecInfo::getNumChannels() const
{
   return numChannels;
}

unsigned MpCodecInfo::getFramePacking() const
{
   return framePacking;
}


unsigned MpCodecInfo::getMinBitrate() const
{
   return minBitrate;
}

unsigned MpCodecInfo::getMaxBitrate() const
{
   return maxBitrate;
}

unsigned MpCodecInfo::getNumSamplesPerFrame() const
{
   return numSamplesPerFrame;
}

unsigned MpCodecInfo::getMinFrameBytes() const
{
   return minFrameBytes;
}

unsigned MpCodecInfo::getMaxFrameBytes() const
{
   return maxFrameBytes;
}

/* ============================ INQUIRY =================================== */

UtlBoolean MpCodecInfo::isSignalingCodec() const
{
   return signalingCodec;
}
UtlBoolean MpCodecInfo::doesVadCng() const
{
   return vadCng == CODEC_CNG_INTERNAL;
}
UtlBoolean MpCodecInfo::haveInternalPLC() const
{
   return packetLossConcealment == CODEC_PLC_INTERNAL;
}

#endif  // _MpCodecInfo_h_
