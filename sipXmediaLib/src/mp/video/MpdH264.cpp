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
#include "mp/video/MpdH264.h"
#ifdef WIN32 // [
#  define EMULATE_INTTYPES
#endif // WIN32 ]
#include "avcodec.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpdH264::MpdH264(int payloadType, MpBufPool *pVideoBufPool)
: mpVideoBufPool(pVideoBufPool)
, mpCodec(NULL)
, mpCodecContext(NULL)
, mpPicture(NULL)
{
    // must be called before using AVcodec lib
    avcodec_init();

    // register all the codecs (you can also register only the codec
    // you wish to have smaller code
    avcodec_register_all();
}

MpdH264::~MpdH264()
{
   freeDecode();
}

OsStatus MpdH264::initDecode()
{
   // Find video decoder
   if (mpCodec == NULL)
   {
      mpCodec = avcodec_find_decoder(CODEC_ID_H264);
   }

   if (mpCodec == NULL)
   {
      osPrintf("MpdH264::initDecode: codec not found\n");
      return OS_NOT_FOUND;
   }

   if (mpCodecContext == NULL)
   {
      mpCodecContext = avcodec_alloc_context();
   }

   if (mpPicture == NULL)
   {
      mpPicture = avcodec_alloc_frame();
   }

   // We could receive incomplete frames
   if (mpCodec->capabilities & CODEC_CAP_TRUNCATED)
   {
      mpCodecContext->flags |= CODEC_FLAG_TRUNCATED;
   }

   // Open codec
   if (avcodec_open(mpCodecContext, mpCodec) < 0)
   {
      osPrintf("MpdH264::initDecode: could not open codec\n");
      freeDecode();
      return OS_FAILED;
   }

   return OS_SUCCESS;
}

OsStatus MpdH264::freeDecode()
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

   return OS_SUCCESS;
}

// ============================ MANIPULATORS ==============================

MpVideoBufPtr MpdH264::decode(const MpRtpBufPtr &pPacket)
{
   int      gotPicture;
   uint8_t *inbuf_ptr;
   int      encodedSize;
   MpVideoBufPtr pFrame; // Buffer for decoded data

   assert(mpCodecContext != NULL);
   assert(mpPicture != NULL);

   inbuf_ptr = (uint8_t*)pPacket->getDataPtr();
   encodedSize = pPacket->getPayloadSize();
   while (encodedSize > 0) {
      int decodedLen = avcodec_decode_video(mpCodecContext, mpPicture,
                                           &gotPicture, inbuf_ptr, encodedSize);
      if (decodedLen < 0) {
         osPrintf("MpdH264::decode: Error while decoding frame %d\n",
                  mpCodecContext->frame_number);
         break;
      }
      if (gotPicture) {
         // Get buffer for captured data
         pFrame = mpVideoBufPool->getBuffer();
         assert(pFrame.isValid());

         // Set frame params
         pFrame->setFrameSize(mpCodecContext->width, mpCodecContext->height);
         pFrame->setColorspace(MpVideoBuf::MP_COLORSPACE_YUV420p);

         // Copy decoded data to our frame
         for (int plane=0; plane<3; plane++)
         {
            char *dest     = (char*)pFrame->getWritePlanePointer(plane);
            int   destStep = pFrame->getPlaneStep(plane);
            char *src      = (char*)mpPicture->data[plane];
            int   srcStep  = mpPicture->linesize[plane];
            for (int i=0; i<mpCodecContext->height; i++)
            {
               memcpy(dest, src, mpCodecContext->width);
               dest += destStep;
               src += srcStep;
            }
         }
         break;
      }
      encodedSize -= decodedLen;
      inbuf_ptr += decodedLen;
   }

   return pFrame;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

#endif // SIPX_VIDEO ]
