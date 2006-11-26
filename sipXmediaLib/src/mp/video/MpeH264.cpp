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
#ifdef WIN32 // [
#  define EMULATE_INTTYPES
#endif // WIN32 ]
#include "avcodec.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

#define TIMESTAMP_FREQ 90000

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

OsStatus MpeH264::initEncode()
{


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
   mpCodecContext->bit_rate = 50000;
   // frames per second
   mpCodecContext->time_base.num = 1;
   mpCodecContext->time_base.den = 10;
   // emit one intra frame every second
   mpCodecContext->gop_size = mpCodecContext->time_base.den/mpCodecContext->time_base.num;
   mpCodecContext->max_b_frames=0;
   mpCodecContext->width = 320;
   mpCodecContext->height = 240;
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
   while (encodedDataSize>0)
   {
      int maxPacketSize = min(RTP_MTU, encodedDataSize);
      unsigned int timestamp = mpCodecContext->frame_number
                             * mpCodecContext->time_base.num 
                             * (TIMESTAMP_FREQ / mpCodecContext->time_base.den);

      // Copy encoded data to buffer payload. Other fields will be filled later.
      mpRtpWriter->writeRtp(mPayloadType, FALSE, pEncodedData, maxPacketSize,
                            timestamp, NULL);
      pEncodedData+=maxPacketSize;
      encodedDataSize-=maxPacketSize;
   }

   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

#endif // SIPX_VIDEO ]
