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

#include "mp/MpCodecInfo.h"

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

MpCodecInfo::~MpCodecInfo()
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
