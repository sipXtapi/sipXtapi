//
// Copyright (C) 2007-2011 SIPez LLC.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
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
//#define TEST_PRINT
#ifdef TEST_PRINT
#    include <os/OsSysLog.h>
#endif
#include <utl/UtlNameValueTokenizer.h>

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
      else if(mMimeSubtype.compareTo("g722") == 0)
      {
         setValue(SDP_CODEC_G722);
      }
      else if(mMimeSubtype.compareTo("speex") == 0)
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

UtlBoolean SdpCodec::getFmtpParameter(const UtlString& parameterName, UtlString& parameterValue) const
{
    return(getFmtpParameter(mFormatSpecificData, parameterName, parameterValue));
}
   
UtlBoolean SdpCodec::getFmtpParameter(const UtlString& fmtpField, const UtlString& parameterName, UtlString& parameterValue)
{
    UtlString thisTempFmpt(fmtpField);
    thisTempFmpt.replace(';', '\n'); // Tokenizer only understands newline name/value pair separators
    UtlNameValueTokenizer thisTokenizer(thisTempFmpt);
    UtlBoolean tokenPairFound = FALSE;
    UtlBoolean parameterFound = FALSE;
    UtlString thisParam;
    do
    {
        tokenPairFound = thisTokenizer.getNextPair('=', &thisParam, &parameterValue);
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

        // Need to compare more for H.264
        if(mMimeSubtype.compareTo(MIME_SUBTYPE_H264, UtlString::ignoreCase) == 0)
        {
            isSame = isFmtpParameterSame(codec, "packetization-mode", "0");

            // Also need to compare profile-level-id parameter in codec fmtp fields
            if(isSame)
            {
                UtlString parameterName("profile-level-id");
                UtlString thisProfileId;
                UtlString thatProfileId;
                getFmtpParameter(parameterName, thisProfileId);
                codec.getFmtpParameter(parameterName, thatProfileId);
                // The profile_idc & profile-iop is just the first 4 of 6 characters in the
                // profile-level-id parameter.  We ignore the level_idc for now.
                thisProfileId.remove(4);
                thatProfileId.remove(4);
                if(thisProfileId.compareTo(thatProfileId, UtlString::ignoreCase))
                {
                    isSame = FALSE;
                }
            }
        }
        else
        {
            isSame = TRUE;
        }
    }

    return(isSame);
}

UtlBoolean SdpCodec::isFmtpParameterSame(const SdpCodec& codec, const UtlString& fmtpParameterName, const UtlString& fmtpParameterDefaultValue) const
{
    return(isFmtpParameterSame(codec.mFormatSpecificData, fmtpParameterName, fmtpParameterDefaultValue));
}

UtlBoolean SdpCodec::isFmtpParameterSame(const UtlString& fmtp, const UtlString& fmtpParameterName, const UtlString& fmtpParameterDefaultValue) const
{
    UtlBoolean isSame = FALSE;
#ifdef TEST_PRINT
    OsSysLog::add(FAC_SDP, PRI_DEBUG, "SdpCodec::isFmtpParameterSame(mFormatSpecificData=\"%s\" fmpt=\"%s\", fmtpParameterName=\"%s\"",
        mFormatSpecificData.data(), fmtp.data(), fmtpParameterName.data());
#endif

    // Need to compare the parameter value of the two codecs
    UtlString thisValue;
    UtlString thatParam;
    UtlString thatValue;
    UtlBoolean thisParameterFound = getFmtpParameter(fmtpParameterName, thisValue);

    // If fmtp parameter is not present, it is assumed to be default
    if(!thisParameterFound)
    {
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SDP, PRI_DEBUG, "SdpCodec::isFmtpParameterSame parameter: %s not found for this codec, assuming: \"%s\"",
                      fmtpParameterName.data(), fmtpParameterDefaultValue.data());
#endif
        thisValue = fmtpParameterDefaultValue;
    }

    UtlBoolean thatParameterFound = getFmtpParameter(fmtp, fmtpParameterName, thatValue);

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
