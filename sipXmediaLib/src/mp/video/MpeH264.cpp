//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef SIPX_VIDEO // [

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/video/MpeH264.h"
#include "mp/MprToNet.h"
#include "mp/NetInTask.h"
#include "sdp/SdpCodec.h"

#ifdef WIN32 // [
#  define EMULATE_INTTYPES
#endif // WIN32 ]

extern "C" {
#  include <avcodec.h>
}

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// DEFINITIONS
#define H264_FORBIDEN_BIT_MASK 0x80  ///< 10000000 - Forbiden Bit mask
#define H264_NAL_REF_IDC_MASK  0x60  ///< 01100000 - Reference ID mask
#define H264_NAL_TYPE_MASK     0x1f  ///< 00011111 - NAL Type mask
#define H264_FU_A_HEADER_SIZE     2  ///< Size of FU-A header in bytes
#define H264_TIMESTAMP_FREQ   90000  ///< H.264 RTP Timestamp frequency is 90KHz
                                     ///< according to RFC 3984.

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpeH264::MpeH264(int payloadType, MprToNet* pRtpWriter)
: mPayloadType(payloadType)
, mpRtpWriter(pRtpWriter)
, mpCodec(NULL)
, mpCodecContext(NULL)
, mpPicture(NULL)
, mpEncodedBuffer(NULL)
, mEncodedBufferSize(0)
{
    mStreamParams.setFrameFormat(SDP_VIDEO_FORMAT_QVGA);
    mStreamParams.setFrameRate(10.f);
    mStreamParams.setStreamBitrate(70);

    // must be called before using AVcodec lib
    avcodec_init();

    // register all the codecs (you can also register only the codec
    // you wish to have smaller code
    avcodec_register_all();
}

MpeH264::~MpeH264()
{
   freeEncode();
}

OsStatus MpeH264::initEncode(const MpVideoStreamParams* params)
{
   if (NULL != params)
   {
      if (0 != params->getFrameWidth())
         mStreamParams.setFrameWidth(params->getFrameWidth());

      if (0 != params->getFrameHeight())
         mStreamParams.setFrameHeight(params->getFrameHeight());

      if (0 != params->getFrameRate())
         mStreamParams.setFrameRate(params->getFrameRate());

      if (0 != params->getStreamBitrate())
         mStreamParams.setStreamBitrate(params->getStreamBitrate());

      if (0 != params->getQuality())
         mStreamParams.setQuality(params->getQuality());
   }

   if (mpCodec == NULL)
   {
      mpCodec = avcodec_find_encoder(CODEC_ID_H264);
   }

   if (mpCodec == NULL)
   {
      osPrintf("MpeH264::initEncode: codec not found\n");
      return OS_NOT_FOUND;
   }

   if (mpCodecContext == NULL)
   {
      mpCodecContext= avcodec_alloc_context();
   }

   if (mpPicture == NULL)
   {
      mpPicture= avcodec_alloc_frame();
   }

   // put sample parameters
   mpCodecContext->bit_rate = mStreamParams.getStreamBitrate();
   AVRational ar = av_d2q(mStreamParams.getFrameRate(), H264_TIMESTAMP_FREQ);

   // frames per second
   mpCodecContext->time_base.num = ar.num;
   mpCodecContext->time_base.den = ar.den;
   // emit one intra frame every second
   mpCodecContext->gop_size = mpCodecContext->time_base.den/mpCodecContext->time_base.num;
   mpCodecContext->max_b_frames=0;
   mpCodecContext->width = mStreamParams.getFrameWidth();
   mpCodecContext->height = mStreamParams.getFrameHeight();
   mpCodecContext->pix_fmt = PIX_FMT_YUV420P;

   // open codec
   if (avcodec_open(mpCodecContext, mpCodec) < 0)
   {
      osPrintf("MpeH264::initEncode: could not open codec\n");
      freeEncode();
      return OS_FAILED;
   }

   // Allocate buffer for encoded data.
   mEncodedBufferSize = ENCODED_BUFFER_SIZE;
   mpEncodedBuffer = new UCHAR[ENCODED_BUFFER_SIZE];

   return OS_SUCCESS;
}

OsStatus MpeH264::freeEncode()
{
   if (mpCodecContext != NULL)
   {
      avcodec_close(mpCodecContext);
      av_free(mpCodecContext);
      mpCodecContext = NULL;
   }

   if (mpPicture != NULL)
   {
      av_free(mpPicture);
      mpPicture = NULL;
   }

   if (mpEncodedBuffer != NULL)
   {
      delete[] mpEncodedBuffer;
      mpEncodedBuffer = NULL;
   }

   return OS_SUCCESS;
}

/* ============================ MANIPULATORS ============================== */

OsStatus MpeH264::encode(const MpVideoBufPtr &pFrame)
{
   int     encodedFrameSize; ///< Encoded frame size returned by codec
   UCHAR  *pEncodedData;     ///< Pointer to not packetized encoded data
   int     encodedDataSize;  ///< Size of data available in pEncodedData
   int     nalUnitType;      ///< NAL Unit Type
   int     nalRefIdc;        ///< NAL Reference Indicator
   UCHAR   packetData[RTP_MTU];     ///< Current packet data
   int     frameFBit;

   assert(mpCodecContext != NULL);
   assert(mpPicture != NULL);
   assert(mpEncodedBuffer != NULL);

   if (pFrame->getColorspace() != MpVideoBuf::MP_COLORSPACE_YUV420p)
   {
      osPrintf("MpeH264::encode: Not supported color space of incoming frame\n");
      return OS_FAILED;
   }

   // resolution must be a multiple of two
   mpCodecContext->width = pFrame->getFrameWidth();
   mpCodecContext->height = pFrame->getFrameHeight();
   mpCodecContext->pix_fmt = PIX_FMT_YUV420P;

   // Setup color planes
   mpPicture->data[0] = (uint8_t*)pFrame->getPlanePointer(0);
   mpPicture->data[1] = (uint8_t*)pFrame->getPlanePointer(1);
   mpPicture->data[2] = (uint8_t*)pFrame->getPlanePointer(2);
   mpPicture->linesize[0] = pFrame->getPlaneStep(0);
   mpPicture->linesize[1] = pFrame->getPlaneStep(1);
   mpPicture->linesize[2] = pFrame->getPlaneStep(2);

   // encode the image
   encodedFrameSize = avcodec_encode_video(mpCodecContext,
                                           (uint8_t*)mpEncodedBuffer, mEncodedBufferSize,
                                           mpPicture);

   if (encodedFrameSize == -1)
   {
      osPrintf("MpeH264::encode: too small encode buffer for this big frame.\n");
      return OS_FAILED;
   }

   pEncodedData=mpEncodedBuffer;
   encodedDataSize=encodedFrameSize;

   while ( ( encodedDataSize>5 ) &&
          !( (pEncodedData[0] == 0x00) && (pEncodedData[1] == 0x00) && (pEncodedData[2] == 0x01) &&
             ((pEncodedData[3]&H264_NAL_TYPE_MASK) >= 1) && ((pEncodedData[3]&H264_NAL_TYPE_MASK) <= 23 )  ) )
   {
      pEncodedData++;
      encodedDataSize--;
   }

   if (encodedDataSize < 5)
      return OS_SUCCESS;

   // Skip startcode
   pEncodedData    += 3;
   encodedDataSize -= 3;

   frameFBit   = pEncodedData[0]&H264_FORBIDEN_BIT_MASK;
   nalRefIdc   = pEncodedData[0]&H264_NAL_REF_IDC_MASK;
   nalUnitType = pEncodedData[0]&H264_NAL_TYPE_MASK;

   // TODO: skip nalTypes 7 and 8?

   unsigned int timestamp = mpCodecContext->frame_number
      * mpCodecContext->time_base.num 
      * (H264_TIMESTAMP_FREQ / mpCodecContext->time_base.den);

   if (encodedDataSize <= RTP_MTU)
   {
      // Single NAL unit mode

      int payloadSize = min(RTP_MTU, encodedDataSize);

      // We could just pass pEncodedData pointer directly to RtpWriter and not
      // copy it to intermediate buffer, so comment this:
//      memcpy(packetData, pEncodedData, payloadSize);

      // Copy encoded data to buffer payload. Other fields will be filled later.
      mpRtpWriter->writeRtp(mPayloadType, TRUE, pEncodedData, payloadSize, timestamp, NULL);
   }
   else
   {
      // Non-interleaved mode

      // Skip NAL Unit Octet
      pEncodedData++;
      encodedDataSize--;

      int startBit = 1;
      int endBit   = 0;
      while (!endBit)
      {
         int payloadSize = min(RTP_MTU-H264_FU_A_HEADER_SIZE, encodedDataSize);
         if (payloadSize==encodedDataSize)
            endBit = 1;

         packetData[0] = frameFBit|nalRefIdc|28; // FU Indicator. 28 - FU-A payload type
         packetData[1] = (startBit<<7)|(endBit<<6)|nalUnitType; // FU Header

         memcpy(&packetData[H264_FU_A_HEADER_SIZE], pEncodedData, payloadSize);

         startBit = 0;
         pEncodedData += payloadSize;
         encodedDataSize -= payloadSize;

         // Copy encoded data to buffer payload. Other fields will be filled later.
         mpRtpWriter->writeRtp(mPayloadType, endBit, packetData, payloadSize+H264_FU_A_HEADER_SIZE, timestamp, NULL);
      }

   }

   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

#endif // SIPX_VIDEO ]
