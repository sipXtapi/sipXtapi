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
// DEFINES
#define H264_NAL_TYPE_MASK     0x1f  ///< 00011111
#define H264_NAL_REF_IDC_MASK  0x60  ///< 01100000
#define H264_START_BIT_MASK    0x80  ///< 10000000
#define H264_FORBIDEN_BIT_MASK 0x80  ///< 10000000
#define H264_END_BIT_MASK      0x40  ///< 01000000

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpdH264::MpdH264(int payloadType, MpBufPool *pVideoBufPool)
: mpVideoBufPool(pVideoBufPool)
, mpCodec(NULL)
, mpCodecContext(NULL)
, mpPicture(NULL)
, mpDecFrameBuf(NULL)
, mDecFrameBufSize(0)
, mNALuOctet(0)
, mFrameFBit(0)
, mpFrameBuf(NULL)
, mpFrameBufEnd(NULL)
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

   if (mpFrameBuf != NULL)
   {
      delete[] mpFrameBuf;
      mpFrameBuf = NULL;
   }

   if (mpFrameBuf == NULL)
   {
      mpFrameBuf = new UCHAR[ENCODED_BUFFER_SIZE];
      mpFrameBufEnd = mpFrameBuf;
   }

   // Initialize RTP depacketizer variables.
   mpDecFrameBuf = NULL;
   mDecFrameBufSize = 0;
   mNALuOctet = 0;
   mFrameFBit = 0;
   mpFrameBufEnd = mpFrameBuf;

   return OS_SUCCESS;
}

OsStatus MpdH264::freeDecode()
{
   if (mpFrameBuf != NULL)
   {
      delete[] mpFrameBuf;
      mpFrameBuf = NULL;
   }

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

MpVideoBufPtr MpdH264::decode(const MpRtpBufPtr &pPacket, bool &packetConsumed)
{
   int      gotPicture;    // Does we got picture from decoder?
   uint8_t *inbuf_ptr;     // Pointer to encoded data
   int      encodedSize;   // Size of encoded data
   int      nalUnitType;
   int      nalRefIdc;
   int      packetDropFlag;
   int      startBit;
   int      endBit;
   int      packetFBit;
   MpVideoBufPtr pFrame; // Buffer for decoded data

   assert(mpCodecContext != NULL);
   assert(mpPicture != NULL);

   inbuf_ptr = (uint8_t*)pPacket->getDataPtr();
   encodedSize = pPacket->getPayloadSize();

   // Initialize internal variables
   packetDropFlag = FALSE;
   startBit = 0;
   endBit = 0;
   packetFBit = 0;

   // By default we consume this packet.
   packetConsumed = true;

   // Packet preprocessor
   if (mpDecFrameBuf == NULL)
   {
      nalUnitType = inbuf_ptr[0]&H264_NAL_TYPE_MASK;
      nalRefIdc = inbuf_ptr[0]&H264_NAL_REF_IDC_MASK;
      packetFBit = inbuf_ptr[0]&H264_FORBIDEN_BIT_MASK;

      // Packet analyze
      if (nalUnitType >= 1 && nalUnitType <= 23)
      {
         // Single NAL unit mode
         startBit=1;
         endBit=1;
         inbuf_ptr++;
         encodedSize--;
      }
      else if (nalUnitType == 28)
      {
         // Non-interleaved mode - FU-A
         nalUnitType = inbuf_ptr[1]&H264_NAL_TYPE_MASK;
         startBit = !!(inbuf_ptr[1]&H264_START_BIT_MASK);
         endBit = !!(inbuf_ptr[1]&H264_END_BIT_MASK);
         inbuf_ptr+=2;
         encodedSize-=2;
      }
      else
      {
         // Unexpected NALu type
         osPrintf("MpdH264::decode: Unexpected NALu type %d\n", nalUnitType);
         packetDropFlag=TRUE;
      }

      // ErrorChecker
      if (mDecFrameBufSize == 0 && !startBit && !packetDropFlag)
      {
         // Stream error: not starting packet on zero Frame Buffer
         osPrintf("MpdH264::decode: Stream error: not starting packet on zero Frame Buffer\n");
         packetFBit=1;
         startBit=1;
      }
      if (mDecFrameBufSize > 0 && ((nalRefIdc|nalUnitType) != mNALuOctet || startBit) && !packetDropFlag)
      {
         // Stream error: this is packet of the next frame
         osPrintf("MpdH264::decode: Stream error: this is packet of the next frame\n");
         packetFBit=1;
         endBit=1;
         packetConsumed=false;
      }
   }
   else
   {
      packetConsumed=false;
   }

   // Packet collector
   if (packetConsumed && !packetDropFlag)
   {
      if (startBit)
      {
         mNALuOctet=nalRefIdc|nalUnitType;
         mpFrameBuf[0]=0x00;
         mpFrameBuf[1]=0x00;
         mpFrameBuf[2]=0x01;
         // pFrameBuf[3] - NALu Octet.
         mpFrameBufEnd+=4;
         mDecFrameBufSize=4;

         mFrameFBit=0;
      }
      memcpy(mpFrameBufEnd, inbuf_ptr, encodedSize);
      mDecFrameBufSize+=encodedSize;
      mpFrameBufEnd+=encodedSize;

      mFrameFBit|=packetFBit;
   }
   if (endBit)
   {
      mpFrameBuf[3] = (packetFBit ? H264_FORBIDEN_BIT_MASK : 0) | mNALuOctet;
      mpDecFrameBuf=mpFrameBuf;
   }

   // Decoder
   gotPicture=FALSE;
   if (mpDecFrameBuf != NULL)
   {
      while (mDecFrameBufSize > 0 && !gotPicture)
      {
         int decodedLen = avcodec_decode_video(mpCodecContext, mpPicture,
                                             &gotPicture, mpDecFrameBuf, mDecFrameBufSize);
         if (decodedLen < 0)
         {
            mDecFrameBufSize=0;
            osPrintf("MpdH264::decode: Error while decoding frame %d\n",
                     mpCodecContext->frame_number);
            break;
         }

         mDecFrameBufSize -= decodedLen;
         mpDecFrameBuf += decodedLen;
      }

      if (mDecFrameBufSize == 0)
      {
         mpDecFrameBuf=NULL;
         mpFrameBufEnd=mpFrameBuf;
      }
   }

   if (gotPicture)
   {
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
   }

   return pFrame;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

#endif // SIPX_VIDEO ]
