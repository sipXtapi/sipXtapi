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

extern "C" {
#  include <avcodec.h>
}

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
, mPreviousSeqNum(0)
, mPreviousTimeStamp(0)
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
   mPreviousSeqNum = 0;
   mPreviousTimeStamp = 0;

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

OsStatus MpdH264::initStream(const MpRtpBufPtr &pPacket)
{
   // Initialize sequence number counter
   mPreviousSeqNum = pPacket->getRtpSequenceNumber();

   return OS_SUCCESS;
}

MpVideoBufPtr MpdH264::decode(const MpRtpBufPtr &pPacket, bool &packetConsumed, bool forceFlag)
{
   int      gotPicture;    // Does we got picture from decoder?
   uint8_t *inbuf_ptr;     // Pointer to encoded data
   int      encodedSize;   // Size of encoded data
   int      nalUnitType;
   int      nalRefIdc;
   int      packetStatus;
   int      startBit;
   int      endBit;
   int      packetFBit;
   MpVideoBufPtr pFrame; // Buffer for decoded data   

   assert(mpCodecContext != NULL);
   assert(mpPicture != NULL);

   // Initialize internal variables
   packetStatus=PACKET_COLLECT;
   startBit = 0;
   endBit = 0;
   packetFBit = 0;

   // By default we consume this packet.
   packetConsumed = true;

   if (!pPacket.isValid())
      return MpVideoBufPtr();

   inbuf_ptr = (uint8_t*)pPacket->getDataPtr();
   encodedSize = pPacket->getPayloadSize();

   // If this is a past (missed) packet - set packetStatus to PACKET_DROP.
   if (compare(mPreviousSeqNum, pPacket->getRtpSequenceNumber()) > 0) {
      packetStatus=PACKET_DROP;
   }

   // Packet preprocessor
   if (packetStatus==PACKET_COLLECT)
   {
      if (mpDecFrameBuf == NULL)
      {
         packetFBit  = inbuf_ptr[0]&H264_FORBIDEN_BIT_MASK;
         nalRefIdc   = inbuf_ptr[0]&H264_NAL_REF_IDC_MASK;
         nalUnitType = inbuf_ptr[0]&H264_NAL_TYPE_MASK;

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
            packetStatus=PACKET_DROP;
         }

         // ErrorChecker
         if (packetStatus==PACKET_COLLECT && mDecFrameBufSize == 0 && !startBit)
         {
            // Stream error: not starting packet on zero Frame Buffer
            osPrintf("MpdH264::decode: Stream error: not starting packet on zero Frame Buffer\n");
            packetFBit=1;
            startBit=1;
         }
         if (  packetStatus==PACKET_COLLECT && mDecFrameBufSize > 0
            && (pPacket->getRtpTimestamp() != mPreviousTimeStamp
               || (nalRefIdc|nalUnitType) != mNALuOctet // When this or next
               || startBit                      // condition is true, something
               )                                // very bad happened with RTP stream.
            )
         {
            // Stream error: this is packet of the next frame
            osPrintf("MpdH264::decode: Stream error: this is packet of the next frame\n");
            packetFBit=1;
            endBit=1;
            packetStatus=PACKET_REPEAT;
         }
         if (mPreviousSeqNum+1 != pPacket->getRtpSequenceNumber() && !startBit)
            packetFBit=1;
      }
      else
      {
         packetStatus=PACKET_REPEAT;
      }
   }

   // Packet collector
   if (packetStatus==PACKET_COLLECT)
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
      mPreviousSeqNum=pPacket->getRtpSequenceNumber();
      mPreviousTimeStamp=pPacket->getRtpTimestamp();
   }
   if (endBit || forceFlag)
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
         int decodedLen = avcodec_decode_video(mpCodecContext,
                                               mpPicture, &gotPicture,
                                               mpDecFrameBuf, mDecFrameBufSize);
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
         int   planeWidth  = pFrame->getPlaneWidth(plane);
         int   planeHeight = pFrame->getPlaneHeight(plane);
         char *src      = (char*)mpPicture->data[plane];
         int   srcStep  = mpPicture->linesize[plane];
         for (int i=0; i<planeHeight; i++)
         {
            memcpy(dest, src, planeWidth);
            dest += destStep;
            src += srcStep;
         }
      }
   }

   packetConsumed = (packetStatus != PACKET_REPEAT);

   return pFrame;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

#endif // SIPX_VIDEO ]
