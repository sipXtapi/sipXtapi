//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: 

// APPLICATION INCLUDES
#include <mp/codecs/PlgDefsV1.h>
#include <memory.h>

#define SIGN_BIT        (0x80)          /* Sign bit for a A-law byte. */
#define QUANT_MASK      (0xf)           /* Quantization field mask. */
#define SEG_SHIFT       (4)             /* Left shift for segment number. */
#define SEG_MASK        (0x70)          /* Segment field mask. */

#define BIAS            (0x84)          /* Bias for linear code. */

typedef audio_sample_t MpAudioSample;

static MpAudioSample hzm_ULaw2linear(uint8_t u)
{
   int L;
   int seg;

   u = ~u;
   seg = (u & 0x70) >> 4;
   L = ((0x0f & u) << 3) + BIAS;
   L = (L << seg);
   if (0x80 & u)
   {
      L = BIAS - L;
   }
   else
   {
      L = L - BIAS;
   }
   return L;
}

static int ULawToLinear(MpAudioSample *Dest, const uint8_t *Source, int samples)
{
   int i;

   for (i=0; i<samples; i++)
   {
      *Dest = hzm_ULaw2linear(*Source);
      Dest++; Source++;
   }
   return samples;
}


/*
* ALaw2Linear() - Convert an A-law value to 16-bit linear PCM
*
*/
static MpAudioSample ALaw2Linear(uint8_t a_val)
{
   int t;
   int seg;

   a_val ^= 0x55;

   t = (a_val & QUANT_MASK) << 4;
   seg = ((unsigned)a_val & SEG_MASK) >> SEG_SHIFT;
   switch (seg)
   {
   case 0:
      t += 8;
      break;
   case 1:
      t += 0x108;
      break;
   default:
      t += 0x108;
      t <<= seg - 1;
   }
   return ((a_val & SIGN_BIT) ? t : -t);
}

static int ALawToLinear(MpAudioSample *Dest, const uint8_t *src, int samples)
{
   int i;

   for (i=0; i<samples; i++)
   {
      *Dest = ALaw2Linear(*src);
      Dest++; src++;
   }
   return samples;
}

static int16_t seg_end[8] = {0x00FF, 0x01FF, 0x03FF, 0x07FF,
0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF};

static int search(MpAudioSample val, int16_t *table, int size)
{
   int             i;

   for (i = 0; i < size; i++)
   {
      if (val <= *table++)
      {
         return i;
      }
   }
   return size;
}

static int G711A_Decoder(int numSamples,
                  const uint8_t* codBuff,
                  MpAudioSample* outBuff)
{
   ALawToLinear(outBuff, codBuff, numSamples);
   return 0;
}

static int G711U_Decoder(int numSamples,
                  const uint8_t* codBuff,
                  MpAudioSample* outBuff)
{
   ULawToLinear(outBuff, codBuff, numSamples);
   return 0;
}

/*
* Linear2ALaw() - Convert a 16-bit linear PCM value to 8-bit A-law
*
* Linear2ALaw() accepts an 16-bit integer and encodes it as A-law data.
*
*              Linear Input Code       Compressed Code
*      ------------------------        ---------------
*      0000000wxyza                    000wxyz
*      0000001wxyza                    001wxyz
*      000001wxyzab                    010wxyz
*      00001wxyzabc                    011wxyz
*      0001wxyzabcd                    100wxyz
*      001wxyzabcde                    101wxyz
*      01wxyzabcdef                    110wxyz
*      1wxyzabcdefg                    111wxyz
*
* For further information see John C. Bellamy's Digital Telephony, 1982,
* John Wiley & Sons, pps 98-111 and 472-476.
*/
static uint8_t Linear2ALaw(MpAudioSample pcm_val ///< 2's complement (16-bit range)
                          )
{
   int      mask;
   int      seg;
   uint8_t  aval;

   if (pcm_val >= 0)
   {
      mask = 0xD5;            /* sign (7th) bit = 1 */
   }
   else
   {
      mask = 0x55;            /* sign bit = 0 */
      pcm_val = -pcm_val - 8;
   }

   /* Convert the scaled magnitude to segment number. */
   seg = search(pcm_val, seg_end, 8);

   /* Combine the sign, segment, and quantization bits. */

   if (seg >= 8)           /* out of range, return maximum value. */
   {
      return (0x7F ^ mask);
   }
   else
   {
      aval = seg << SEG_SHIFT;
      if (seg < 2)
         aval |= (pcm_val >> 4) & QUANT_MASK;
      else
         aval |= (pcm_val >> (seg + 3)) & QUANT_MASK;
      return (aval ^ mask);
   }
}

static int LinearToALaw(uint8_t *Dest,const MpAudioSample *src, int samples)
{
   int i;

   for (i=0; i<samples; i++)
   {
      *Dest = Linear2ALaw(*src);
      Dest++; src++;
   }
   return samples;
}

static int G711A_Encoder(int numSamples,
                  const MpAudioSample* inBuff,
                  uint8_t* outBuf)
{
   LinearToALaw(outBuf, inBuff, numSamples);
   return 0;
}

static uint8_t hzm_Linear2ULaw(MpAudioSample L)
{
   int seg;
   uint8_t signmask;

   if (0 > L) {
      L = BIAS - L;
      signmask = 0x7f;
   } else {
      signmask = 0xff;
      L = BIAS + L;
   }
   if (L > 32767) L = 32767;
   if (0x7800 & L) {
      seg = (4<<4);
   } else {
      seg = 0;
      L = L << 4;
   }
   if (0x6000 & L) {
      seg += (2<<4);
   } else {
      L = L << 2;
   }
   if (0x4000 & L) {
      seg += (1<<4);
   } else {
      L = L << 1;
   }
   return ((seg | ((0x3C00 & L) >> 10)) ^ signmask);
}

static int LinearToULaw(uint8_t *Dest, const MpAudioSample *src, int samples)
{
   int i;

   for (i=0; i<samples; i++) {
      *Dest = hzm_Linear2ULaw(*src);
      Dest++; src++;
   }
   return samples;
}

static int G711U_Encoder(int numSamples,
                  const MpAudioSample* inBuff,
                  uint8_t* outBuf)
{
   LinearToULaw(outBuf, inBuff, numSamples);
   return 0;
}




//////////////////////////////////////////////////////////////////////////

#define DECODER_HANDLE     ((void*)1)
#define ENCODER_HANDLE     ((void*)2)

const char codecPcmaMIMEsubtype[] = "pcma";

struct plgCodecInfoV1 sipxCodecInfoPCMA = 
{
   sizeof(struct plgCodecInfoV1),   //cbSize
 //  SdpCodec::SDP_CODEC_PCMA,        //codecSDPType
   codecPcmaMIMEsubtype,
   "sipxPcma",                      //codecName
   "SIPfoundry 1.0",                //codecVersion
   8000,                            //samplingRate
   8,                               //fmtAndBitsPerSample
   1,                               //numChannels
   160,                             //interleaveBlockSize
   64000,                           //bitRate
   1280,                            //minPacketBits
   1280,                            //avgPacketBits
   1280,                            //maxPacketBits
   160,                             //numSamplesPerFrame
   3                                //preCodecJitterBufferSize
};

CODEC_API int PLG_ENUM_V1(sipxPcma)(const char** mimeSubtype, unsigned int* pModesCount, const char*** modes)
{
   if (mimeSubtype) {
      *mimeSubtype = codecPcmaMIMEsubtype;
   }
   if (pModesCount) {
      *pModesCount = 0;
   }
   if (modes) {
      *modes = NULL;
   }
   return RPLG_SUCCESS;
}

/*
int PLG_PREPARE_V1(sipxPcma)(void* handle, int bDecoder)
{
   return RPLG_SUCCESS;
}

int PLG_UNPREPARE_V1(sipxPcma)(void* handle, int bDecoder)
{
   return RPLG_SUCCESS;
}
*/
//typedef int   (*dlPlgEnumSDPAndModesV1)(const char** mimeSubtype, unsigned int* pModesCount, const struct sdpModeInfo** modes);
//typedef void* (*dlPlgInitializeV1)(const char* fmt, int bDecoder, struct plgCodecInfoV1* pCodecInfo);

CODEC_API void *PLG_INIT_V1(sipxPcma)(const char* fmt, int bDecoder, struct plgCodecInfoV1* pCodecInfo)
{
   if (pCodecInfo == NULL) {
      return NULL;
   }
   memcpy(pCodecInfo, &sipxCodecInfoPCMA, sizeof(struct plgCodecInfoV1));
   if (bDecoder)
      return DECODER_HANDLE;
   else
      return ENCODER_HANDLE;
}

CODEC_API int PLG_FREE_V1(sipxPcma)(void* handle)
{
   return RPLG_SUCCESS;
}

CODEC_API int PLG_DECODE_V1(sipxPcma)(void* handle, const void* pCodedData, unsigned cbCodedPacketSize, void* pAudioBuffer, unsigned cbBufferSize, unsigned *pcbCodedSize, const struct RtpHeader* pRtpHeader)
{
   int samples;
   if (handle != DECODER_HANDLE)
      return RPLG_BAD_HANDLE;

   // Assert that available buffer size is enough for the packet.
   if (cbCodedPacketSize > cbBufferSize)
   {
      //osPrintf("sipxPcma::decode: Jitter buffer overloaded. Glitch!\n");
      return RPLG_INVALID_ARGUMENT;
   }

   if (cbBufferSize == 0)
      return RPLG_INVALID_ARGUMENT;

   samples = min(cbCodedPacketSize, cbBufferSize);
   G711A_Decoder(samples, (uint8_t*)pCodedData, (MpAudioSample *)pAudioBuffer);
   *pcbCodedSize = samples;

   return RPLG_SUCCESS;
}

CODEC_API int PLG_ENCODE_V1(sipxPcma)(void* handle, const void* pAudioBuffer, unsigned cbAudioSamples, int* rSamplesConsumed, void* pCodedData,
                               unsigned cbMaxCodedData, int* pcbCodedSize, unsigned* pbSendNow)
{
   if (handle != ENCODER_HANDLE)
      return RPLG_BAD_HANDLE;

   G711A_Encoder(cbAudioSamples, (MpAudioSample *)pAudioBuffer, (uint8_t*)pCodedData);
   *pcbCodedSize = cbAudioSamples;

   *pbSendNow = FALSE;
   *rSamplesConsumed = cbAudioSamples;

   return RPLG_SUCCESS;
}


const char codecPcmuMIMEsubtype[] = "pcmu";

struct plgCodecInfoV1 sipxCodecInfoPCMU = 
{
   sizeof(struct plgCodecInfoV1),   //cbSize
   /*SdpCodec::SDP_CODEC_PCMU,        //codecSDPType*/
   codecPcmuMIMEsubtype,
   "sipxPcmu",                      //codecName
   "SIPfoundry 1.0",                //codecVersion
   8000,                            //samplingRate
   8,                               //fmtAndBitsPerSample
   1,                               //numChannels
   160,                             //interleaveBlockSize
   64000,                           //bitRate
   1280,                            //minPacketBits
   1280,                            //avgPacketBits
   1280,                            //maxPacketBits
   160,                             //numSamplesPerFrame
   3                                //preCodecJitterBufferSize
};
/*
const struct sdpModeInfo modesPCMU[] = {
   {SdpCodec::SDP_CODEC_PCMU, ""}
};
*/
CODEC_API int PLG_ENUM_V1(sipxPcmu)(const char** mimeSubtype, unsigned int* pModesCount, const char*** modes)
{
   if (mimeSubtype) {
      *mimeSubtype = codecPcmuMIMEsubtype;
   }
   if (pModesCount) {
      *pModesCount = 0;
   }
   if (modes) {
      *modes = NULL;
   }
   return RPLG_SUCCESS;
}

CODEC_API void *PLG_INIT_V1(sipxPcmu)(const char* fmt, int bDecoder, struct plgCodecInfoV1* pCodecInfo)
{
   if (pCodecInfo == NULL) {
      return NULL;
   }
   memcpy(pCodecInfo, &sipxCodecInfoPCMU, sizeof(struct plgCodecInfoV1));
   if (bDecoder)
      return DECODER_HANDLE;
   else
      return ENCODER_HANDLE;
}

CODEC_API int PLG_FREE_V1(sipxPcmu)(void* handle)
{
   return 0;
}

CODEC_API int PLG_DECODE_V1(sipxPcmu)(void* handle, const void* pCodedData, unsigned cbCodedPacketSize, void* pAudioBuffer, unsigned cbBufferSize, unsigned *pcbCodedSize, const struct RtpHeader* pRtpHeader)
{
   int samples;

   if (handle != DECODER_HANDLE)
      return RPLG_INVALID_SEQUENCE_CALL;

   // Assert that available buffer size is enough for the packet.
   if (cbCodedPacketSize > cbBufferSize)
   {
      //osPrintf("sipxPcma::decode: Jitter buffer overloaded. Glitch!\n");
      return RPLG_INVALID_ARGUMENT;
   }

   if (cbBufferSize == 0)
      return RPLG_INVALID_ARGUMENT;

   samples = min(cbCodedPacketSize, cbBufferSize);
   G711U_Decoder(samples, (uint8_t*)pCodedData, (MpAudioSample *)pAudioBuffer);
   *pcbCodedSize = samples;

   return RPLG_SUCCESS;
}


CODEC_API int PLG_ENCODE_V1(sipxPcmu)(void* handle, const void* pAudioBuffer, unsigned cbAudioSamples, int* rSamplesConsumed, void* pCodedData,
                            unsigned cbMaxCodedData, int* pcbCodedSize, unsigned* pbSendNow)
{
   if (handle != ENCODER_HANDLE)
      return RPLG_INVALID_SEQUENCE_CALL;

   G711U_Encoder(cbAudioSamples, (MpAudioSample *)pAudioBuffer, (uint8_t*)pCodedData);
   *pcbCodedSize = cbAudioSamples;

   *pbSendNow = FALSE;
   *rSamplesConsumed = cbAudioSamples;

   return RPLG_SUCCESS;
}


PLG_DOUBLE_CODECS(sipxPcmu, sipxPcma);

#ifdef STATIC_CODEC
#include <mp/MpPlgStaffV1.h>
const char* stub_function_to_do() { return __FILE__; }
DECLARE_MP_STATIC_PLUGIN_CODEC_V1(sipxPcmu);
DECLARE_MP_STATIC_PLUGIN_CODEC_V1(sipxPcma);
#endif
