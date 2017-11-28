//
// Copyright (C) 2007-2017 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
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
#include <os/OsSysLog.h>
#include <utl/UtlNameValueTokenizer.h>
#include <utl/UtlTokenizer.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// DEFINES
//#define TEST_PRINT
#define MAXIMUM_VIDEO_SIZES 10

#define ADD_VIDEO_SIZE(maxSizes, numSizes, videoSizes, sizeParameter, status) \
    if(maxSizes > numSizes) \
    { \
        videoSizes[numSizes++] = sizeParameter; \
        status = OS_SUCCESS; \
    } \
    else \
    { \
        status = OS_LIMIT_REACHED; \
    }


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
   mMimeSubtype.SDP_MIME_SUBTYPE_TO_CASE();
   mMimeType.SDP_MIME_TO_CASE();
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
   mMimeSubtype.SDP_MIME_SUBTYPE_TO_CASE();
   mMimeType.SDP_MIME_TO_CASE();

   // !slg! Note:  CPU and BW Costs copied from SdpCodecFactory
   if(mMimeType.compareTo("audio", UtlString::ignoreCase) == 0)
   {
      if(mMimeSubtype.compareTo("pcmu", UtlString::ignoreCase) == 0)
      {
         setValue(SDP_CODEC_PCMU);
      }
      else if(mMimeSubtype.compareTo("pcma", UtlString::ignoreCase) == 0)
      {
         setValue(SDP_CODEC_PCMA);
      }
      else if(mMimeSubtype.compareTo("gsm", UtlString::ignoreCase) == 0)
      {
         setValue(SDP_CODEC_GSM);
         mCPUCost = SDP_CODEC_CPU_HIGH;
         mBWCost = SDP_CODEC_BANDWIDTH_LOW;
      }
      else if(mMimeSubtype.compareTo("g723", UtlString::ignoreCase) == 0)
      {
         setValue(SDP_CODEC_G723);
         mCPUCost = SDP_CODEC_CPU_HIGH;
         mBWCost = SDP_CODEC_BANDWIDTH_LOW;
      }
      else if(mMimeSubtype.compareTo("l16", UtlString::ignoreCase) == 0)
      {
         if(mNumChannels == 2)
         {
            switch (mSampleRate)
            {
            case 8000: setValue(SDP_CODEC_L16_8000_STEREO); break;
            case 11025: setValue(SDP_CODEC_L16_11025_STEREO); break;
            case 16000: setValue(SDP_CODEC_L16_16000_STEREO); break;
            case 22050: setValue(SDP_CODEC_L16_22050_STEREO); break;
            case 24000: setValue(SDP_CODEC_L16_24000_STEREO); break;
            case 32000: setValue(SDP_CODEC_L16_32000_STEREO); break;
            case 44100: setValue(SDP_CODEC_L16_44100_STEREO); break;
            case 48000: setValue(SDP_CODEC_L16_48000_STEREO); break;
            default:
               // Unknown samplerate.
               setValue(SDP_CODEC_UNKNOWN);
            }
         }
         else
         {
            switch (mSampleRate)
            {
            case 8000: setValue(SDP_CODEC_L16_8000_MONO); break;
            case 11025: setValue(SDP_CODEC_L16_11025_MONO); break;
            case 16000: setValue(SDP_CODEC_L16_16000_MONO); break;
            case 22050: setValue(SDP_CODEC_L16_22050_MONO); break;
            case 24000: setValue(SDP_CODEC_L16_24000_MONO); break;
            case 32000: setValue(SDP_CODEC_L16_32000_MONO); break;
            case 44100: setValue(SDP_CODEC_L16_44100_MONO); break;
            case 48000: setValue(SDP_CODEC_L16_48000_MONO); break;
            default:
               // Unknown samplerate.
               setValue(SDP_CODEC_UNKNOWN);
            }
         }
      }
      else if(mMimeSubtype.compareTo("G729a", UtlString::ignoreCase) == 0)
      {
         setValue(SDP_CODEC_G729ACISCO7960);
         mCPUCost = SDP_CODEC_CPU_HIGH;
         mBWCost = SDP_CODEC_BANDWIDTH_LOW;
      }
      else if(mMimeSubtype.compareTo("g729", UtlString::ignoreCase) == 0)
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
      else if(mMimeSubtype.compareTo("telephone-event", UtlString::ignoreCase) == 0)
      {
         setValue(SDP_CODEC_TONES);
         mBWCost = SDP_CODEC_BANDWIDTH_LOW;
      }
      else if(mMimeSubtype.compareTo("g7221", UtlString::ignoreCase) == 0)
      {
         setValue(SDP_CODEC_G7221);
      }
      else if(mMimeSubtype.compareTo("g7231", UtlString::ignoreCase) == 0)
      {
         setValue(SDP_CODEC_G7231);
      }
      else if(mMimeSubtype.compareTo("eg711a", UtlString::ignoreCase) == 0)
      {
         setValue(SDP_CODEC_GIPS_IPCMA);
      }
      else if(mMimeSubtype.compareTo("eg711u", UtlString::ignoreCase) == 0)
      {
         setValue(SDP_CODEC_GIPS_IPCMU);
      }
      else if(mMimeSubtype.compareTo("ipcmwb", UtlString::ignoreCase) == 0)
      {
         setValue(SDP_CODEC_GIPS_IPCMWB);
         mBWCost = SDP_CODEC_BANDWIDTH_HIGH;
      }
      else if(mMimeSubtype.compareTo("ilbc", UtlString::ignoreCase) == 0)
      {
         if(mFormatSpecificData.compareTo("mode=20", UtlString::ignoreCase) == 0)
         {
            setValue(SDP_CODEC_ILBC_20MS);
         }
         else
         {
            setValue(SDP_CODEC_ILBC);
         }
         mCPUCost = SDP_CODEC_CPU_HIGH;
         mBWCost = SDP_CODEC_BANDWIDTH_LOW;
      }
      else if(mMimeSubtype.compareTo("isac", UtlString::ignoreCase) == 0)
      {
         setValue(SDP_CODEC_GIPS_ISAC);
         mCPUCost = SDP_CODEC_CPU_HIGH;
         mBWCost = SDP_CODEC_BANDWIDTH_VARIABLE;
      }
      else if(mMimeSubtype.compareTo("g726-16", UtlString::ignoreCase) == 0)
      {
         setValue(SDP_CODEC_G726_16);
      }
      else if(mMimeSubtype.compareTo("g726-24", UtlString::ignoreCase) == 0)
      {
         setValue(SDP_CODEC_G726_24);
      }
      else if(mMimeSubtype.compareTo("g726-32", UtlString::ignoreCase) == 0)
      {
         setValue(SDP_CODEC_G726_32);
      }
      else if(mMimeSubtype.compareTo("g726-40", UtlString::ignoreCase) == 0)
      {
         setValue(SDP_CODEC_G726_40);
      }
      else if(mMimeSubtype.compareTo("g722", UtlString::ignoreCase) == 0)
      {
         setValue(SDP_CODEC_G722);
      }
      else if(mMimeSubtype.compareTo(MIME_SUBTYPE_OPUS, UtlString::ignoreCase) == 0)
      {
         setValue(SDP_CODEC_OPUS);
         if(mSampleRate != 48000)
         {
            OsSysLog::add(FAC_SDP, PRI_ERR,
                          "SdpCodec:xxxx OPUS codec NOT at 48000: %d", mSampleRate);
         }
         if(mNumChannels != 2)
         {
            OsSysLog::add(FAC_SDP, PRI_ERR,
                          "SdpCodec:xxxx OPUS codec not labeled 2 channel: %d", mNumChannels);
         }

         // Check for stereo

         // Parameters:
         //  rate
         //  maxplaybackrate
         //  sprop-maxcapturerate
         //  maxptime
         //  ptime
         //  minptime
         //  maxaveragebitrate
         //  stereo (receive)
         //  sprop-stereo (send)
         //  cbr
         //  useinbandfec
         //  usedtx

      }
      else if(mMimeSubtype.compareTo("speex", UtlString::ignoreCase) == 0)
      {
         if (mSampleRate == 8000)
         {
            if(mFormatSpecificData.compareTo("mode=2", UtlString::ignoreCase) == 0)
            {
               setValue(SDP_CODEC_SPEEX_5);
               mBWCost = SDP_CODEC_BANDWIDTH_LOW;
            }
            else if(mFormatSpecificData.compareTo("mode=5", UtlString::ignoreCase) == 0)
            {
               setValue(SDP_CODEC_SPEEX_15);
               mBWCost = SDP_CODEC_BANDWIDTH_NORMAL;
            }
            else if(mFormatSpecificData.compareTo("mode=7", UtlString::ignoreCase) == 0)
            {
               setValue(SDP_CODEC_SPEEX_24);
               mBWCost = SDP_CODEC_BANDWIDTH_NORMAL;
            }
            else
            {
               // mode = 3 (default)
               setValue(SDP_CODEC_SPEEX);
               mBWCost = SDP_CODEC_BANDWIDTH_LOW;
            }
         }
         else if (mSampleRate == 16000)
         {
            if(mFormatSpecificData.compareTo("mode=1", UtlString::ignoreCase) == 0)
            {
               setValue(SDP_CODEC_SPEEX_WB_5);
               mBWCost = SDP_CODEC_BANDWIDTH_LOW;
            }
            else if(mFormatSpecificData.compareTo("mode=6", UtlString::ignoreCase) == 0)
            {
               setValue(SDP_CODEC_SPEEX_WB_21);
               mBWCost = SDP_CODEC_BANDWIDTH_NORMAL;
            }
            else if(mFormatSpecificData.compareTo("mode=10", UtlString::ignoreCase) == 0)
            {
               setValue(SDP_CODEC_SPEEX_WB_42);
               mBWCost = SDP_CODEC_BANDWIDTH_NORMAL;
            }
            else
            {
               // mode = 3 (default)
               setValue(SDP_CODEC_SPEEX_WB);
               mBWCost = SDP_CODEC_BANDWIDTH_LOW;
            }
         }
         else if (mSampleRate == 32000)
         {
            if(mFormatSpecificData.compareTo("mode=1", UtlString::ignoreCase) == 0)
            {
               setValue(SDP_CODEC_SPEEX_UWB_8);
               mBWCost = SDP_CODEC_BANDWIDTH_LOW;
            }
            else if(mFormatSpecificData.compareTo("mode=6", UtlString::ignoreCase) == 0)
            {
               setValue(SDP_CODEC_SPEEX_UWB_22);
               mBWCost = SDP_CODEC_BANDWIDTH_NORMAL;
            }
            else if(mFormatSpecificData.compareTo("mode=10", UtlString::ignoreCase) == 0)
            {
               setValue(SDP_CODEC_SPEEX_UWB_44);
               mBWCost = SDP_CODEC_BANDWIDTH_NORMAL;
            }
            else
            {
               // mode = 3 (default)
               setValue(SDP_CODEC_SPEEX_UWB);
               mBWCost = SDP_CODEC_BANDWIDTH_LOW;
            }
         }
         else
         {
            // Unknown flavour of Speex
            setValue(SDP_CODEC_UNKNOWN);
         }
      }
      else if(mMimeSubtype.compareTo("amr", UtlString::ignoreCase) == 0)
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
      else if(mMimeSubtype.compareTo("amr-wb", UtlString::ignoreCase) == 0)
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
   else if(mMimeType.compareTo("video", UtlString::ignoreCase) == 0)
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

      if(mMimeSubtype.compareTo("vp71", UtlString::ignoreCase) == 0)
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
      else if(mMimeSubtype.compareTo("iyuv", UtlString::ignoreCase) == 0)
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
      else if(mMimeSubtype.compareTo("i420", UtlString::ignoreCase) == 0)
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
      else if(mMimeSubtype.compareTo("rgb24", UtlString::ignoreCase) == 0)
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
      else if(mMimeSubtype.compareTo("h263", UtlString::ignoreCase) == 0)
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

UtlBoolean SdpCodec::getFmtpParameter(const UtlString& parameterName, UtlString& parameterValue, char nameValueSeparator) const
{
    return(getFmtpParameter(mFormatSpecificData, parameterName, parameterValue, nameValueSeparator));
}
   
UtlBoolean SdpCodec::getFmtpParameter(const UtlString& fmtpField, const UtlString& parameterName, int& parameterValue, char nameValueSeparator)
{
    UtlString parameterValueString;
    UtlBoolean paramFound = getFmtpParameter(fmtpField, parameterName, parameterValueString, nameValueSeparator);
    if(!parameterValueString.isNull())
    {
        parameterValue = atoi(parameterValueString.data());
    }
    else
    {
        parameterValue = -1;
    }

    return(paramFound && !parameterValueString.isNull());
}

UtlBoolean SdpCodec::getFmtpParameter(const UtlString& fmtpField, const UtlString& parameterName, UtlString& parameterValue, char nameValueSeparator)
{
    UtlString thisTempFmpt(fmtpField);
    thisTempFmpt.replace(';', '\n'); // Tokenizer only understands newline name/value pair separators
    UtlNameValueTokenizer thisTokenizer(thisTempFmpt);
    UtlBoolean tokenPairFound = FALSE;
    UtlBoolean parameterFound = FALSE;
    UtlString thisParam;
    do
    {
        tokenPairFound = thisTokenizer.getNextPair(nameValueSeparator, &thisParam, &parameterValue);
        thisParam.strip(UtlString::both);
        parameterValue.strip(UtlString::both);

#ifdef TEST_PRINT
        OsSysLog::add(FAC_SDP, PRI_DEBUG, "SdpCodec::getFmtpParameter fmtp param: \"%s\" value: \"%s\"",
                      thisParam.data(), parameterValue.data());
#endif
        if(thisParam.compareTo(parameterName, UtlString::ignoreCase) == 0)
        {
            parameterFound = TRUE;
            break;
        }
    }
    while(tokenPairFound);

    return(parameterFound);
}

OsStatus SdpCodec::getVideoSizes(const UtlString& fmtpField, int maxSizes, int& numSizes, int videoSizes[])
{
#ifdef TEST_PRINT
    OsSysLog::add(FAC_SDP, PRI_DEBUG, "SdpCodec::getVideoSizes(fmtp=%s, maxSizes=%d", fmtpField.data(), maxSizes);
#endif
    numSizes = 0;
    OsStatus status = OS_NOT_FOUND;
    UtlTokenizer imagesizeTokenizer(fmtpField);
    UtlString videoSizeTokens;
    UtlString imagesizeToken;

    // This format is used internally in SdpDefaultCodecFactory, not user if it
    // is used in the wild.
    // a=fmtp:100 size=QCIF/SQCIF
    // Note: fmtpField should not contain "a=fmtp:100 " prefix
    // Found this example for size format in SdpBodyTest and it seemed to be supported
    // in the prior parser.  Not sure if it exists in the wild like this or if this is 
    // just a bug in the unit test, but it was simple enough to support:
    // a=fmtp:100 size:QCIF/SQCIF
    // Note: size":" not size"="
    if(getFmtpParameter(fmtpField, "size", videoSizeTokens, '=') ||
       getFmtpParameter(fmtpField, "size", videoSizeTokens, ':'))
    {
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SDP, PRI_DEBUG, "SdpCodec::getVideoSizes found size parameter: \"%s\"", videoSizeTokens.data());
#endif
        int tokenIndex = 0;
        UtlString sizeToken;
        while(UtlNameValueTokenizer::getSubField(videoSizeTokens, 
                                                 tokenIndex, // token index
                                                 " \t/:=\n\r", // seperators
                                                 &sizeToken))
        {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_SDP, PRI_DEBUG, "SdpCodec::getVideoSizes size token: \"%s\"", sizeToken.data());
#endif
            if(numSizes >= maxSizes)
            {
                status = OS_LIMIT_REACHED;
                break;
            }

            if (sizeToken.compareTo("SQCIF", UtlString::ignoreCase) == 0)
            {
                ADD_VIDEO_SIZE(maxSizes, numSizes, videoSizes, SDP_VIDEO_FORMAT_SQCIF, status);
            }
            else if (sizeToken.compareTo("QCIF", UtlString::ignoreCase) == 0)
            {
                ADD_VIDEO_SIZE(maxSizes, numSizes, videoSizes, SDP_VIDEO_FORMAT_QCIF, status);
            }
            else if (sizeToken.compareTo("CIF", UtlString::ignoreCase) == 0)
            {
                ADD_VIDEO_SIZE(maxSizes, numSizes, videoSizes, SDP_VIDEO_FORMAT_CIF, status);
            }
            else if (sizeToken.compareTo("QVGA", UtlString::ignoreCase) == 0)
            {
                ADD_VIDEO_SIZE(maxSizes, numSizes, videoSizes, SDP_VIDEO_FORMAT_QVGA, status);
            }
            else if (sizeToken.compareTo("VGA", UtlString::ignoreCase) == 0)
            {
                ADD_VIDEO_SIZE(maxSizes, numSizes, videoSizes, SDP_VIDEO_FORMAT_VGA, status);
            }
            else if (sizeToken.compareTo("CIF4", UtlString::ignoreCase) == 0)
            {
                ADD_VIDEO_SIZE(maxSizes, numSizes, videoSizes, SDP_VIDEO_FORMAT_4CIF, status);
            }
            else if (sizeToken.compareTo("CIF16", UtlString::ignoreCase) == 0)
            {
                ADD_VIDEO_SIZE(maxSizes, numSizes, videoSizes, SDP_VIDEO_FORMAT_16CIF, status);
            }

            tokenIndex++;
        }
    }

    // Not sure the source of this format, iChat perhaps???
    // a=fmtp 100 imagesize 0 rules 30:640:480:640:480:30
    // Note: fmtpField should not contain "a=fmtp:100 " prefix
    else if(imagesizeTokenizer.next(imagesizeToken, " \t\n\r") &&
            imagesizeToken.compareTo("imagesize", UtlString::ignoreCase) == 0)
    {
        UtlString imagesizeValueString;
        if(imagesizeTokenizer.next(imagesizeValueString, " \t\n\r"))
        {
            if(imagesizeValueString.compareTo("0") == 0)
            {
                ADD_VIDEO_SIZE(maxSizes, numSizes, videoSizes, SDP_VIDEO_FORMAT_QCIF, status);
            }
            else if(imagesizeValueString.compareTo("1") == 0)
            {
                ADD_VIDEO_SIZE(maxSizes, numSizes, videoSizes, SDP_VIDEO_FORMAT_CIF, status);
            }
        }
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SDP, PRI_DEBUG, "SdpCodec::getVideoSizes found imagesize token, value string: %s", imagesizeValueString.data());
#endif
    }

    // Check for RFC 4629 size format for H.261 and H.263
    // "a=fmtp 100 CIF4=2;CIF=1;QCIF=1;SQCIF=1;CUSTOM=352,240,1;CUSTOM=704,480,2;J;T"
    // Note: fmtpField should not contain "a=fmtp:100 " prefix
    else 
    {
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SDP, PRI_DEBUG, "SdpCodec::getVideoSizes looking for RFC 4629 style size format");
#endif
        if(getFmtpParameter(fmtpField, "SQCIF", videoSizeTokens, '='))
        {
            ADD_VIDEO_SIZE(maxSizes, numSizes, videoSizes, SDP_VIDEO_FORMAT_SQCIF, status);
        }
        if(getFmtpParameter(fmtpField, "QCIF", videoSizeTokens, '='))
        {
            ADD_VIDEO_SIZE(maxSizes, numSizes, videoSizes, SDP_VIDEO_FORMAT_QCIF, status);
        }
        if(getFmtpParameter(fmtpField, "CIF", videoSizeTokens, '='))
        {
            ADD_VIDEO_SIZE(maxSizes, numSizes, videoSizes, SDP_VIDEO_FORMAT_CIF, status);
        }
        if(getFmtpParameter(fmtpField, "QVGA", videoSizeTokens, '='))
        {
            ADD_VIDEO_SIZE(maxSizes, numSizes, videoSizes, SDP_VIDEO_FORMAT_QVGA, status);
        }
        if(getFmtpParameter(fmtpField, "VGA", videoSizeTokens, '='))
        {
            ADD_VIDEO_SIZE(maxSizes, numSizes, videoSizes, SDP_VIDEO_FORMAT_VGA, status);
        }
        if(getFmtpParameter(fmtpField, "CIF4", videoSizeTokens, '='))
        {
            ADD_VIDEO_SIZE(maxSizes, numSizes, videoSizes, SDP_VIDEO_FORMAT_4CIF, status);
        }
        if(getFmtpParameter(fmtpField, "CIF16", videoSizeTokens, '='))
        {
            ADD_VIDEO_SIZE(maxSizes, numSizes, videoSizes, SDP_VIDEO_FORMAT_16CIF, status);
        }
    }
#ifdef TEST_PRINT
    OsSysLog::add(FAC_SDP, PRI_DEBUG, "imagesizeToken: %s", imagesizeToken.data());
#endif
    // Not sure we should default
    if(numSizes == 0)
    {
        // Default to QCIF if no size is present
        ADD_VIDEO_SIZE(maxSizes, numSizes, videoSizes, SDP_VIDEO_FORMAT_QCIF, status);
        status = OS_NOT_FOUND;
    }

    return(status);
}

void SdpCodec::setSdpFmtpField(const UtlString& formatSpecificData)
{
    mFormatSpecificData = formatSpecificData;
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

UtlBoolean SdpCodec::isSameDefinition(const SdpCodec& codec) const
{
    UtlBoolean isSame = FALSE;

    if(mSampleRate == codec.mSampleRate &&
       (
           mNumChannels == codec.mNumChannels ||
           (mNumChannels == -1 && codec.mNumChannels == 1) ||
           (mNumChannels == 1 && codec.mNumChannels == -1)
       ) &&
       mMimeType.compareTo(codec.mMimeType, UtlString::ignoreCase) == 0 &&
       mMimeSubtype.compareTo(codec.mMimeSubtype, UtlString::ignoreCase) == 0)
    {
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SDP, PRI_DEBUG, "SdpCodec::isSameDefinition mime subtype: %s", 
            mMimeSubtype.data());
#endif
        int compares;
        isSame = compareFmtp(codec.mFormatSpecificData, compares);

    }

    return(isSame);
}


UtlBoolean SdpCodec::compareFmtp(const SdpCodec& codec, int& compares) const
{
    return(compareFmtp(codec.mFormatSpecificData, compares));
}

UtlBoolean SdpCodec::compareFmtp(const UtlString& fmtp, int& compares) const
{
    return(compareFmtp(mMimeType, mMimeSubtype, mFormatSpecificData, fmtp, compares));
}

UtlBoolean SdpCodec::compareFmtp(const UtlString& mimeType, const UtlString& mimeSubtype, const UtlString& fmtp1, const UtlString& fmtp2, int& compares)
{
//#ifdef TEST_PRINT
    OsSysLog::add(FAC_SDP, PRI_DEBUG,
            "SdpCodec::compareFmtp(%s, %s, %s, %s,",
            mimeType.data(), mimeSubtype.data(), fmtp1.data(), fmtp2.data());
//#endif

    UtlBoolean isCompatible = FALSE;
    if(mimeType.compareTo(MIME_TYPE_VIDEO, UtlString::ignoreCase) == 0)
    {
        // Need to compare more for H.264
        if(mimeSubtype.compareTo(MIME_SUBTYPE_H264, UtlString::ignoreCase) == 0)
        {
            isCompatible = isFmtpParameterSame(fmtp1, fmtp2, "packetization-mode", "0");

            // Also need to compare profile-level-id parameter in codec fmtp fields
            if(isCompatible)
            {
                UtlString parameterName("profile-level-id");
                UtlString thisProfileId;
                UtlString thatProfileId;
                getFmtpParameter(fmtp1, parameterName, thisProfileId);
                getFmtpParameter(fmtp2, parameterName, thatProfileId);
                // The profile_idc is just the first 2 of 6 characters in the
                // profile-level-id parameter.  We ignore the profile-iop & level_idc for now.
                thisProfileId.remove(2);
                thatProfileId.remove(2);
                if(thisProfileId.compareTo(thatProfileId, UtlString::ignoreCase))
                {
                    isCompatible = FALSE;
                }
            }
            // TODO rank other parameters (e.g. size and bandwidth)
        }
        // Other video types
        else
        {
            // Just need to compare size
            int thisNumSizes;
            int thatNumSizes;
            int thisVideoSizes[MAXIMUM_VIDEO_SIZES];
            int thatVideoSizes[MAXIMUM_VIDEO_SIZES];
            getVideoSizes(fmtp1, MAXIMUM_VIDEO_SIZES, thisNumSizes, thisVideoSizes);
            getVideoSizes(fmtp2, MAXIMUM_VIDEO_SIZES, thatNumSizes, thatVideoSizes);
#ifdef TEST_PRINT
            OsSysLog::add(FAC_SDP, PRI_DEBUG, "SdpCodec::compareFmtp comparing video sizes this: %d that: %d",
                    thisNumSizes, thatNumSizes);
#endif

            // Default if size is not set
            // TODO: Should probably use mVideoFormat as the default size, but as this is static
            // we do not have access to it.  
            if(thisNumSizes < 1)
            {
                thisNumSizes = 1;
                thisVideoSizes[0] = SDP_VIDEO_FORMAT_QCIF;
            }
            if(thatNumSizes < 1)
            {
                thatNumSizes = 1;
                thatVideoSizes[0] = SDP_VIDEO_FORMAT_QCIF;
            }

            int thisMaxSize = thisVideoSizes[0];
            int thatMaxSize = thatVideoSizes[0];
            for(int thisIndex = 0; thisIndex < thisNumSizes; thisIndex++)
            {
                if(thisMaxSize < thisVideoSizes[thisIndex])
                {
                    thisMaxSize = thisVideoSizes[thisIndex];
                }

                for(int thatIndex = 0; thatIndex < thatNumSizes; thatIndex++)
                {
                    if(thatMaxSize < thatVideoSizes[thatIndex])
                    {
                        thatMaxSize = thatVideoSizes[thatIndex];
                    }

                    // If any of the sizes match, they are compatible
                    if(thisVideoSizes[thisIndex] == thatVideoSizes[thatIndex])
                    {
                        isCompatible = TRUE;
                    }
                }
            }

            // Size matters, the biggest is the best
            if(thisMaxSize > thatMaxSize)
            {
                compares = 1;
            }
            else if(thisMaxSize == thatMaxSize)
            {
                compares = 0;
            }
            else
            {
                compares = -1;
            }
        }
    }
    else if(mimeType.compareTo(MIME_TYPE_AUDIO, UtlString::ignoreCase) == 0)
    {
        // TODO: need unit tests
        // iLBC need to compare modes
        if(mimeSubtype.compareTo(MIME_SUBTYPE_ILBC, UtlString::ignoreCase) == 0)
        {
            int mode1;
            int mode2;
            UtlBoolean foundMode1 = getFmtpParameter(fmtp1, "mode", mode1);
            UtlBoolean foundMode2 = getFmtpParameter(fmtp2, "mode", mode2);
            if(
               (!foundMode1 && !foundMode2) ||
               (mode1 == mode2)
              )
            {
                isCompatible = TRUE;
                compares = 1;
            }

            else if(
               (!foundMode1 && mode2 == 30) ||
               (!foundMode2 && mode1 == 30) ||
               (mode1 == 20 && mode2 == 0)  ||
               (mode2 == 20 && mode1 == 0)
              )
            {
                isCompatible = TRUE;
                compares = 0;
            }
            OsSysLog::add(FAC_SDP, PRI_DEBUG,
                          "SdpCodec::compareFmtp ILBC fmtp1=%s fmtp2=%s mode1=%d mode2=%d compatible: %s",
                          fmtp1.data(),
                          fmtp2.data(),
                          mode1,
                          mode2,
                          isCompatible ? "TRUE" : "FALSE");
        }

        else if(mimeSubtype.compareTo(MIME_SUBTYPE_G7221, UtlString::ignoreCase) == 0)
        {
            int bitrate1;
            int bitrate2;
            UtlBoolean foundBitRate1 = getFmtpParameter(fmtp1, "bitrate", bitrate1);
            UtlBoolean foundBitRate2 = getFmtpParameter(fmtp2, "bitrate", bitrate2);
            // Bit rate is manditory
            if(foundBitRate1 && foundBitRate2 && bitrate1 == bitrate2)
            {
                isCompatible = TRUE;
                compares = 1;
            }
        }
        // telephone-events theoretically need to compare event type codes, but we just assume they overlap
        else if(mimeSubtype.compareTo(MIME_SUBTYPE_DTMF_TONES, UtlString::ignoreCase) == 0)
        {
#ifdef SDP_RFC4733_STRICT_FMTP_CHECK // [
            // TODO: this has not been tested since the re-factoring

            // Workaround for RFC4733. Refer to RFC4733 section 7.1.1.
            // paragraph optional "Optional parameters" and
            // section 2.4.1 for details.
            if (  (fmtp1.isNull() || fmtp1 == "0-15")
                && (fmtp2.isNull() || fmtp2 == "0-15"))
            {
                // we found a match
                isCompatible = TRUE;
                compares = 0;
            }
#else // SDP_RFC4733_STRICT_FMTP_CHECK ][
            // Match any fmtp for RFC4733 DTMFs.
            // There are quite a few implementation which use
            // different fmtp strings in their SDP and we should be
            // interoperable with them. Simplest way is to accept
            // everything and ignore unknown codes later.
            // Examples of fmtp strings seen in the field:
            // "0-16" (e.g. Snom phones), "0-11".
            isCompatible = TRUE;
#endif // !SDP_RFC4733_STRICT_FMTP_CHECK ]
        }

        else if((mimeSubtype.compareTo(MIME_SUBTYPE_AMR, UtlString::ignoreCase) == 0) ||
                (mimeSubtype.compareTo(MIME_SUBTYPE_AMRWB, UtlString::ignoreCase) == 0))
        {
            if(fmtp1 == fmtp2)
            {
                isCompatible = TRUE;
                compares = 1;
            }
            else if((fmtp1 == "" && fmtp2 == "octet-align=0")  ||
                    (fmtp2 == "" && fmtp1 == "octet-align=0"))
            {
                isCompatible = TRUE;
                compares = 0;
            }
        }

        else if(mimeSubtype.compareTo(MIME_SUBTYPE_SPEEX, UtlString::ignoreCase) == 0)
        {
            if(fmtp1 == fmtp2)
            {
                isCompatible = TRUE;
                compares = 1;
            }
            // TODO:  are softer compares needed?
        }

        // All other Audio Mime subtypes:
        else
        {
            isCompatible = TRUE;
            compares = 0;
        }
    }
    else
    {
        OsSysLog::add(FAC_SDP, PRI_ERR, "SdpCodec::compareFmtp not implemented for mime type: %s/%s",
                mimeType.data(), mimeSubtype.data());
    }

    return(isCompatible);
}

UtlBoolean SdpCodec::isFmtpParameterSame(const SdpCodec& codec, const UtlString& fmtpParameterName, const UtlString& fmtpParameterDefaultValue,
        const char nameValueSeperator) const
{
    return(isFmtpParameterSame(codec.mFormatSpecificData, fmtpParameterName, fmtpParameterDefaultValue, nameValueSeperator));
}

UtlBoolean SdpCodec::isFmtpParameterSame(const UtlString& fmtp, const UtlString& fmtpParameterName, const UtlString& fmtpParameterDefaultValue,
        const char nameValueSeperator) const
{
    return(isFmtpParameterSame(mFormatSpecificData, fmtp, fmtpParameterName, fmtpParameterDefaultValue, nameValueSeperator));
}

UtlBoolean SdpCodec::isFmtpParameterSame(const UtlString& fmtp1, const UtlString& fmtp2, const UtlString& fmtpParameterName, 
                                         const UtlString& fmtpParameterDefaultValue,
                                         const char nameValueSeperator)
{
    UtlBoolean isSame = FALSE;
#ifdef TEST_PRINT
    OsSysLog::add(FAC_SDP, PRI_DEBUG, "SdpCodec::isFmtpParameterSame(fmtp1=\"%s\" fmtp2=\"%s\", fmtpParameterName=\"%s\"",
        fmtp1.data(), fmtp2.data(), fmtpParameterName.data());
#endif

    // Need to compare the parameter value of the two codecs
    UtlString thisValue;
    UtlString thatParam;
    UtlString thatValue;
    UtlBoolean thisParameterFound = getFmtpParameter(fmtp1, fmtpParameterName, thisValue, nameValueSeperator);

    // If fmtp parameter is not present, it is assumed to be default
    if(!thisParameterFound)
    {
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SDP, PRI_DEBUG, "SdpCodec::isFmtpParameterSame parameter: %s not found for this codec, assuming: \"%s\"",
                      fmtpParameterName.data(), fmtpParameterDefaultValue.data());
#endif
        thisValue = fmtpParameterDefaultValue;
    }

    UtlBoolean thatParameterFound = getFmtpParameter(fmtp2, fmtpParameterName, thatValue);

    // If packetization-mode is not present, it is assumed to be 0
    if(!thatParameterFound)
    {
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SDP, PRI_DEBUG, "SdpCodec::isFmtpParameterSame parameter: %s not found for that codec, assuming: \"%s\"",
                      fmtpParameterName.data(), fmtpParameterDefaultValue.data());
#endif
        thatValue = fmtpParameterDefaultValue;
    }

    if(thisValue.compareTo(thatValue, UtlString::ignoreCase) == 0)
    {
        isSame = TRUE;
    }
#ifdef TEST_PRINT
    OsSysLog::add(FAC_SDP, PRI_DEBUG, "SdpCodec::isFmtpParameterSame parameter %s this: \"%s\" that: \"%s\" isSame: %d",
                  fmtpParameterName.data(), thisValue.data(), thatValue.data(), isSame);
#endif

    return(isSame);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
