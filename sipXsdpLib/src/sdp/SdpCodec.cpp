//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2007 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <sdp/SdpCodec.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SdpCodec::SdpCodec(enum SdpCodecTypes sdpCodecType,
                   int payloadFormat,
                   const char* mimeType,
                   const char* mimeSubtype,
                   int sampleRate,
                   int preferredPacketLength,
                   int numChannels,
                   const char* formatSpecificData,
                   const SdpCodecCPUCost CPUCost,
                   const int BWCost,
                   const int videoFormat,
                   const int videoFmtp) :
   mCodecPayloadFormat(payloadFormat),
   mMimeType(mimeType),
   mMimeSubtype(mimeSubtype),
   mSampleRate(sampleRate),
   mPacketLength(preferredPacketLength),
   mNumChannels(numChannels),
   mFormatSpecificData(formatSpecificData),
   mCPUCost(CPUCost),
   mBWCost(BWCost),
   mVideoFormat(videoFormat),
   mVideoFmtp(videoFmtp)
{
   mMimeSubtype.toLower();
   mMimeType.toLower();
   setValue(sdpCodecType);
}

SdpCodec::SdpCodec(int payloadFormat,
                   const char* mimeType,
                   const char* mimeSubtype,
                   int sampleRate, // samples per second
                   int preferredPacketLength, // micro seconds
                   int numChannels,
                   const char* formatSpecificData) :
   mCodecPayloadFormat(payloadFormat),
   mMimeType(mimeType),
   mMimeSubtype(mimeSubtype),
   mSampleRate(sampleRate),
   mPacketLength(preferredPacketLength),
   mNumChannels(numChannels),
   mFormatSpecificData(formatSpecificData),
   mCPUCost(SDP_CODEC_CPU_LOW),
   mBWCost(SDP_CODEC_BANDWIDTH_NORMAL),
   mVideoFormat(SDP_VIDEO_FORMAT_QCIF),
   mVideoFmtp(0)
{
   mMimeSubtype.toLower();
   mMimeType.toLower();

   // !slg! Note:  CPU and BW Costs copied from SdpCodecFactory
   if(mMimeType.compareTo("audio") == 0)
   {
      if(mMimeSubtype.compareTo("pcmu") == 0)
      {
         setValue(SDP_CODEC_PCMU);
      }
      else if(mMimeSubtype.compareTo("pcma") == 0)
      {
         setValue(SDP_CODEC_PCMA);
      }
      else if(mMimeSubtype.compareTo("gsm") == 0)
      {
         setValue(SDP_CODEC_GSM);
         mCPUCost = SDP_CODEC_CPU_HIGH;
         mBWCost = SDP_CODEC_BANDWIDTH_LOW;
      }
      else if(mMimeSubtype.compareTo("g723") == 0)
      {
         setValue(SDP_CODEC_G723);
         mCPUCost = SDP_CODEC_CPU_HIGH;
         mBWCost = SDP_CODEC_BANDWIDTH_LOW;
      }
      else if(mMimeSubtype.compareTo("l16") == 0)
      {
         if(mNumChannels == 2)
         {
            setValue(SDP_CODEC_L16_STEREO);
         }
         else
         {
            setValue(SDP_CODEC_L16_MONO);
         }
      }
      else if(mMimeSubtype.compareTo("pcm") == 0)
      {
         setValue(SDP_CODEC_L16_8K);
      }
      else if(mMimeSubtype.compareTo("G729a") == 0)
      {
         setValue(SDP_CODEC_G729ACISCO7960);
         mCPUCost = SDP_CODEC_CPU_HIGH;
         mBWCost = SDP_CODEC_BANDWIDTH_LOW;
      }
      else if(mMimeSubtype.compareTo("g729") == 0)
      {
         if(mFormatSpecificData.compareTo("annexb=no", UtlString::ignoreCase) == 0)
         {
            setValue(SDP_CODEC_G729A);
            mCPUCost = SDP_CODEC_CPU_HIGH;
            mBWCost = SDP_CODEC_BANDWIDTH_LOW;
         }
         else
         {
            setValue(SDP_CODEC_G729AB);
            mCPUCost = SDP_CODEC_CPU_HIGH;
            mBWCost = SDP_CODEC_BANDWIDTH_LOW;
         }
      }
      else if(mMimeSubtype.compareTo("telephone-event") == 0)
      {
         setValue(SDP_CODEC_TONES);
         mBWCost = SDP_CODEC_BANDWIDTH_LOW;
      }
      else if(mMimeSubtype.compareTo("g7221") == 0)
      {
         setValue(SDP_CODEC_G7221);
      }
      else if(mMimeSubtype.compareTo("g7231") == 0)
      {
         setValue(SDP_CODEC_G7231);
      }
      else if(mMimeSubtype.compareTo("eg711a") == 0)
      {
         setValue(SDP_CODEC_GIPS_IPCMA);
      }
      else if(mMimeSubtype.compareTo("eg711u") == 0)
      {
         setValue(SDP_CODEC_GIPS_IPCMU);
      }
      else if(mMimeSubtype.compareTo("ipcmwb") == 0)
      {
         setValue(SDP_CODEC_GIPS_IPCMWB);
         mBWCost = SDP_CODEC_BANDWIDTH_HIGH;
      }
      else if(mMimeSubtype.compareTo("ilbc") == 0)
      {
         setValue(SDP_CODEC_ILBC);
         mCPUCost = SDP_CODEC_CPU_HIGH;
         mBWCost = SDP_CODEC_BANDWIDTH_LOW;
      }
      else if(mMimeSubtype.compareTo("isac") == 0)
      {
         setValue(SDP_CODEC_GIPS_ISAC);
         mCPUCost = SDP_CODEC_CPU_HIGH;
         mBWCost = SDP_CODEC_BANDWIDTH_VARIABLE;
      }
      else if(mMimeSubtype.compareTo("g726-16") == 0)
      {
         setValue(SDP_CODEC_G726_16);
      }
      else if(mMimeSubtype.compareTo("g726-24") == 0)
      {
         setValue(SDP_CODEC_G726_24);
      }
      else if(mMimeSubtype.compareTo("g726-32") == 0)
      {
         setValue(SDP_CODEC_G726_32);
      }
      else if(mMimeSubtype.compareTo("g726-40") == 0)
      {
         setValue(SDP_CODEC_G726_40);
      }
      else if(mMimeSubtype.compareTo("speex") == 0)
      {
         if(mFormatSpecificData.compareTo("mode=2", UtlString::ignoreCase) == 0)
         {
            setValue(SDP_CODEC_SPEEX_5);  // Speex Profile 1
            mBWCost = SDP_CODEC_BANDWIDTH_LOW;
         }
         else if(mFormatSpecificData.compareTo("mode=5", UtlString::ignoreCase) == 0)
         {
            setValue(SDP_CODEC_SPEEX_15);  // Speex Profile 2
            mBWCost = SDP_CODEC_BANDWIDTH_NORMAL;
         }
         else if(mFormatSpecificData.compareTo("mode=7", UtlString::ignoreCase) == 0)
         {
            setValue(SDP_CODEC_SPEEX_24);  // Speex Profile 3
            mBWCost = SDP_CODEC_BANDWIDTH_NORMAL;
         }
         else
         {
            // mode = 3
            setValue(SDP_CODEC_SPEEX);
            mBWCost = SDP_CODEC_BANDWIDTH_LOW;
         }
      }
      else if(mMimeSubtype.compareTo("speex-wb") == 0)
      {
         if(mFormatSpecificData.compareTo("mode=2", UtlString::ignoreCase) == 0)
         {
            setValue(SDP_CODEC_SPEEX_WB_5);  // Speex Profile 1
            mBWCost = SDP_CODEC_BANDWIDTH_LOW;
         }
         else if(mFormatSpecificData.compareTo("mode=5", UtlString::ignoreCase) == 0)
         {
            setValue(SDP_CODEC_SPEEX_WB_15);  // Speex Profile 2
            mBWCost = SDP_CODEC_BANDWIDTH_NORMAL;
         }
         else if(mFormatSpecificData.compareTo("mode=7", UtlString::ignoreCase) == 0)
         {
            setValue(SDP_CODEC_SPEEX_WB_24);  // Speex Profile 3
            mBWCost = SDP_CODEC_BANDWIDTH_NORMAL;
         }
         else
         {
            // mode = 3
            setValue(SDP_CODEC_SPEEX_WB);
            mBWCost = SDP_CODEC_BANDWIDTH_LOW;
         }
      }
      else if(mMimeSubtype.compareTo("speex-uwb") == 0)
      {
         if(mFormatSpecificData.compareTo("mode=2", UtlString::ignoreCase) == 0)
         {
            setValue(SDP_CODEC_SPEEX_UWB_5);  // Speex Profile 1
            mBWCost = SDP_CODEC_BANDWIDTH_LOW;
         }
         else if(mFormatSpecificData.compareTo("mode=5", UtlString::ignoreCase) == 0)
         {
            setValue(SDP_CODEC_SPEEX_UWB_15);  // Speex Profile 2
            mBWCost = SDP_CODEC_BANDWIDTH_NORMAL;
         }
         else if(mFormatSpecificData.compareTo("mode=7", UtlString::ignoreCase) == 0)
         {
            setValue(SDP_CODEC_SPEEX_UWB_24);  // Speex Profile 3
            mBWCost = SDP_CODEC_BANDWIDTH_NORMAL;
         }
         else
         {
            // mode = 3
            setValue(SDP_CODEC_SPEEX_UWB);
            mBWCost = SDP_CODEC_BANDWIDTH_LOW;
         }
      }
      else if(mMimeSubtype.compareTo("amr") == 0)
      {
         if (mFormatSpecificData.compareTo("octet-align=1", UtlString::ignoreCase) == 0)
         {
            setValue(SDP_CODEC_AMR_ALIGNED);  // Octet Aligned mode
         }
         else
         {
            setValue(SDP_CODEC_AMR);          // Bandwidth Efficient mode
         }
         mCPUCost = SDP_CODEC_CPU_HIGH;
         mBWCost = SDP_CODEC_BANDWIDTH_NORMAL;
      }
      else if(mMimeSubtype.compareTo("amr-wb") == 0)
      {
         if (mFormatSpecificData.compareTo("octet-align=1", UtlString::ignoreCase) == 0)
         {
            setValue(SDP_CODEC_AMRWB_ALIGNED);  // Octet Aligned mode
         }
         else
         {
            setValue(SDP_CODEC_AMRWB);          // Bandwidth Efficient mode
         }
         mCPUCost = SDP_CODEC_CPU_HIGH;
         mBWCost = SDP_CODEC_BANDWIDTH_NORMAL;
      }
      else
      {
         setValue(SDP_CODEC_UNKNOWN);
      }
   }
   else if(mMimeType.compareTo("video") == 0)
   {
       unsigned int index = 0;
       unsigned int start = 0;
       UtlString temp;
       const char* formatSpecificData = mFormatSpecificData.data();
       // !slg! logic taken from SdpBody.cpp - but logic in SdpBody doesn't look complete
       for(index = 0; index <= mFormatSpecificData.length(); index++)
       {
          if(formatSpecificData[index] == ' ' ||
             formatSpecificData[index] == '\t' ||
             formatSpecificData[index] == '/' ||
             formatSpecificData[index] == ':' ||
             formatSpecificData[index] == '=' ||
             formatSpecificData[index] == '\0')
          {
             if(start != index)
             {
                temp.resize(0);
                temp.append(&formatSpecificData[start], index-start);
                if (temp.compareTo("CIF", UtlString::ignoreCase) == 0)
                {
                   if(mVideoFmtp == 0)
                   {
                      mVideoFormat = SDP_VIDEO_FORMAT_CIF;
                   }
                   mVideoFmtp |= SDP_VIDEO_FORMAT_CIF;
                }
                else if (temp.compareTo("QCIF", UtlString::ignoreCase) == 0)
                {
                   if(mVideoFmtp == 0)
                   {
                      mVideoFormat = SDP_VIDEO_FORMAT_QCIF;
                   }
                   mVideoFmtp |= SDP_VIDEO_FORMAT_QCIF;
                }
                else if (temp.compareTo("SQCIF", UtlString::ignoreCase) == 0)
                {
                   if(mVideoFmtp == 0)
                   {
                      mVideoFormat = SDP_VIDEO_FORMAT_SQCIF;
                   }
                   mVideoFmtp |= SDP_VIDEO_FORMAT_SQCIF;
                }
             }
             start = index+1;
          }
       }  

      if(mMimeSubtype.compareTo("vp71") == 0)
      {
         switch(mVideoFormat)
         {
         case SDP_VIDEO_FORMAT_CIF:
            setValue(SDP_CODEC_VP71_CIF);
            break;
         case SDP_VIDEO_FORMAT_QCIF:
            setValue(SDP_CODEC_VP71_QCIF);
            break;
         case SDP_VIDEO_FORMAT_SQCIF:
            setValue(SDP_CODEC_VP71_SQCIF);
            break;
         case SDP_VIDEO_FORMAT_QVGA:
            setValue(SDP_CODEC_VP71_QVGA);
            break;
         default:
            setValue(SDP_CODEC_VP71_CIF);
            break;
         }
         mBWCost = SDP_CODEC_BANDWIDTH_NORMAL;
      }
      else if(mMimeSubtype.compareTo("iyuv") == 0)
      {
         switch(mVideoFormat)
         {
         case SDP_VIDEO_FORMAT_CIF:
            setValue(SDP_CODEC_IYUV_CIF);
            break;
         case SDP_VIDEO_FORMAT_QCIF:
            setValue(SDP_CODEC_IYUV_QCIF);
            break;
         case SDP_VIDEO_FORMAT_SQCIF:
            setValue(SDP_CODEC_IYUV_SQCIF);
            break;
         case SDP_VIDEO_FORMAT_QVGA:
            setValue(SDP_CODEC_IYUV_QVGA);
            break;
         default:
            setValue(SDP_CODEC_IYUV_CIF);
            break;
         }
         mBWCost = SDP_CODEC_BANDWIDTH_HIGH;
      }
      else if(mMimeSubtype.compareTo("i420") == 0)
      {
         switch(mVideoFormat)
         {
         case SDP_VIDEO_FORMAT_CIF:
            setValue(SDP_CODEC_I420_CIF);
            break;
         case SDP_VIDEO_FORMAT_QCIF:
            setValue(SDP_CODEC_I420_QCIF);
            break;
         case SDP_VIDEO_FORMAT_SQCIF:
            setValue(SDP_CODEC_I420_SQCIF);
            break;
         case SDP_VIDEO_FORMAT_QVGA:
            setValue(SDP_CODEC_I420_QVGA);
            break;
         default:
            setValue(SDP_CODEC_I420_CIF);
            break;
         }
         mBWCost = SDP_CODEC_BANDWIDTH_HIGH;
      }
      else if(mMimeSubtype.compareTo("rgb24") == 0)
      {
         switch(mVideoFormat)
         {
         case SDP_VIDEO_FORMAT_CIF:
            setValue(SDP_CODEC_RGB24_CIF);
            break;
         case SDP_VIDEO_FORMAT_QCIF:
            setValue(SDP_CODEC_RGB24_QCIF);
            break;
         case SDP_VIDEO_FORMAT_SQCIF:
            setValue(SDP_CODEC_RGB24_SQCIF);
            break;
         case SDP_VIDEO_FORMAT_QVGA:
            setValue(SDP_CODEC_RGB24_QVGA);
            break;
         default:
            setValue(SDP_CODEC_RGB24_CIF);
            break;
         }
         mBWCost = SDP_CODEC_BANDWIDTH_HIGH;
      }
      else if(mMimeSubtype.compareTo("h263") == 0)
      {
         switch(mVideoFormat)
         {
         case SDP_VIDEO_FORMAT_CIF:
            setValue(SDP_CODEC_H263_CIF);
            break;
         case SDP_VIDEO_FORMAT_QCIF:
            setValue(SDP_CODEC_H263_QCIF);
            break;
         case SDP_VIDEO_FORMAT_SQCIF:
            setValue(SDP_CODEC_H263_SQCIF);
            break;
         case SDP_VIDEO_FORMAT_QVGA:
            setValue(SDP_CODEC_H263_QVGA);
            break;
         default:
            setValue(SDP_CODEC_H263_CIF);
            break;
         }
         mBWCost = SDP_CODEC_BANDWIDTH_NORMAL;
      }
      else
      {
         setValue(SDP_CODEC_UNKNOWN);
      }
   }
   else
   {
      setValue(SDP_CODEC_UNKNOWN);
   }
}

// Copy constructor
SdpCodec::SdpCodec(const SdpCodec& rSdpCodec)
{
    setValue(rSdpCodec.getValue());
    mCodecPayloadFormat = rSdpCodec.mCodecPayloadFormat;
    mSampleRate = rSdpCodec.mSampleRate;
    mPacketLength = rSdpCodec.mPacketLength;
    mNumChannels = rSdpCodec.mNumChannels;
    mMimeType = rSdpCodec.mMimeType;
    mMimeSubtype = rSdpCodec.mMimeSubtype;
    mFormatSpecificData = rSdpCodec.mFormatSpecificData;
    mCPUCost  = rSdpCodec.mCPUCost;
    mBWCost  = rSdpCodec.mBWCost;
    mVideoFormat = rSdpCodec.mVideoFormat;
    mVideoFmtp = rSdpCodec.mVideoFmtp;
    mVideoFmtpString = rSdpCodec.mVideoFmtpString;
}

// Destructor
SdpCodec::~SdpCodec()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SdpCodec&
SdpCodec::operator=(const SdpCodec& rhs)
{

   if (this == &rhs)            // handle the assignment to self case
      return *this;
    setValue(rhs.getValue());
    mCodecPayloadFormat = rhs.mCodecPayloadFormat;
    mSampleRate = rhs.mSampleRate;
    mPacketLength = rhs.mPacketLength;
    mNumChannels = rhs.mNumChannels;
    mMimeType = rhs.mMimeType;
    mMimeSubtype = rhs.mMimeSubtype;
    mFormatSpecificData = rhs.mFormatSpecificData;
    mCPUCost  = rhs.mCPUCost;
    mBWCost = rhs.mBWCost;
    mVideoFormat = rhs.mVideoFormat;
    mVideoFmtp = rhs.mVideoFmtp;
    mVideoFmtpString = rhs.mVideoFmtpString;

   return *this;
}

void SdpCodec::setCodecPayloadFormat(int formatId)
{
   mCodecPayloadFormat = formatId;
}

void SdpCodec::setVideoFmtp(const int videoFmtp)
{
   mVideoFmtp = videoFmtp;
}

void SdpCodec::setVideoFmtpString(int videoFmtp)
{
   UtlString tempFmtp(NULL);

   switch (videoFmtp)
   {
   case SDP_VIDEO_FORMAT_SQCIF:
      mVideoFmtpString.append("SQCIF/");
      break;
   case SDP_VIDEO_FORMAT_QCIF:
      mVideoFmtpString.append("QCIF/");
      break;
   case SDP_VIDEO_FORMAT_CIF:
      mVideoFmtpString.append("CIF/");
      break;
   case SDP_VIDEO_FORMAT_QVGA:
      mVideoFmtpString.append("QVGA/");
      break;
   default:
      break;
   }
}

void SdpCodec::clearVideoFmtpString()
{
   mVideoFmtpString = "";
}

void SdpCodec::setPacketSize(const int packetSize)
{
   mPacketLength = packetSize;
}

/* ============================ ACCESSORS ================================= */

SdpCodec::SdpCodecTypes SdpCodec::getCodecType() const
{
   return((enum SdpCodecTypes) getValue());
}

int SdpCodec::getCodecPayloadFormat() const
{
    return(mCodecPayloadFormat);
}

void SdpCodec::getSdpFmtpField(UtlString& formatSpecificData) const
{
    formatSpecificData = mFormatSpecificData;
}

void SdpCodec::getMediaType(UtlString& mimeType) const
{
    mimeType = mMimeType;
}

void SdpCodec::getEncodingName(UtlString& mimeSubtype) const
{
    mimeSubtype = mMimeSubtype;
}

int SdpCodec::getSampleRate() const
{
    return(mSampleRate);
}

int SdpCodec::getVideoFormat() const
{
    return(mVideoFormat);
}

int SdpCodec::getPacketLength() const
{
    return(mPacketLength);
}

int SdpCodec::getNumChannels() const
{
    return(mNumChannels);
}

int SdpCodec::getVideoFmtp() const
{
    return mVideoFmtp;
}

void SdpCodec::getVideoFmtpString(UtlString& fmtpString) const
{
    fmtpString = mVideoFmtpString;
}

void SdpCodec::toString(UtlString& sdpCodecContents) const
{
    char stringBuffer[256];
    sprintf(stringBuffer,"SdpCodec: codecId=%d, payloadId=%d, mime=\'%s/%s\', rate=%d, pktLen=%d, numCh=%d, fmtData=\'%s\'\n",
            getValue(), mCodecPayloadFormat,
            mMimeType.data(), mMimeSubtype.data(),
            mSampleRate, mPacketLength, mNumChannels,
            mFormatSpecificData.data());
    sdpCodecContents = stringBuffer;
}

// Get the CPU cost for this codec.
SdpCodec::SdpCodecCPUCost SdpCodec::getCPUCost() const
{
   return mCPUCost;
}

// Get the bandwidth cost for this codec.
int SdpCodec::getBWCost() const
{
   return mBWCost;
}

/* ============================ INQUIRY =================================== */

UtlBoolean SdpCodec::isSameDefinition(SdpCodec& codec) const
{
    return(mSampleRate == codec.mSampleRate &&
           mNumChannels == codec.mNumChannels &&
           mMimeType.compareTo(codec.mMimeType, UtlString::ignoreCase) == 0 &&
           mMimeSubtype.compareTo(codec.mMimeSubtype, UtlString::ignoreCase) == 0);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
