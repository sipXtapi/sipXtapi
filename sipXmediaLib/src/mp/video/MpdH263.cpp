//  
// Copyright (C) 2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2007 Wirtualna Polska S.A. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Andrzej Ciarkowski <andrzejc AT wp-sa DOT pl>

#ifdef SIPX_VIDEO

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/video/MpdH263.h"
#include "mp/video/rfc2190.h"

#include "mp/NetInTask.h"  // for RTP_MTU

#include <VideoSupport/VideoFormat.h>

#ifdef WIN32 // [
#  define EMULATE_INTTYPES
#endif // WIN32 ]

extern "C" {
#  include <avcodec.h>
#  include <bitstream.h>
}


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////////// PUBLIC //////////////////////////////// */

/* =============================== CREATORS =============================== */
MpdH263::MpdH263(int payloadType, MpBufPool *pVideoBufPool)
 : MpVideoDecoder(payloadType, pVideoBufPool)
 , mpCodec(NULL)
 , mpCodecContext(NULL)
 , mpFrame(NULL)
 , mpHeaderA(NULL)
 , mpHeaderB(NULL)
 , mPreviousSeqNum(0)
 , mPreviousTimeStamp(0)
 , mpDecodeBuffer(NULL)
 , mDecodeBufferSize(0)
 , mAccuPayloadBits(0)
 , mpDecodePos(NULL)
 , mDecodeLeft(0)
 //, mpDecodePosition(NULL)
{
    // must be called before using AVcodec lib
    avcodec_init();

    // register all the codecs (you can also register only the codec
    // you wish to have smaller code
    avcodec_register_all();
}

MpdH263::~MpdH263()
{
   freeDecode();
}

OsStatus MpdH263::initDecode()
{
   if (mpCodec == NULL)
   {
      mpCodec = avcodec_find_decoder(CODEC_ID_H263);
   }

   if (mpCodec == NULL)
   {
      osPrintf("MpdH263::initDecode: codec not found\n");
      return OS_NOT_FOUND;
   }

   if (mpCodecContext == NULL)
   {
      mpCodecContext = avcodec_alloc_context();
   }

   if (mpCodecContext == NULL)
   {
      osPrintf("MpdH263::initDecode: unable to allocate codec context\n");
      return OS_NO_MEMORY;
   }

   if (mpFrame == NULL)
   {
      mpFrame = avcodec_alloc_frame();
   }

   if (mpFrame == NULL)
   {
      osPrintf("MpdH263::initDecode: unable to allocate frame\n");
      return OS_NO_MEMORY;
   }

   // We could receive incomplete frames
   if (mpCodec->capabilities & CODEC_CAP_TRUNCATED)
   {
      mpCodecContext->flags |= CODEC_FLAG_TRUNCATED;
   }

   mpCodecContext->error_resilience = FF_ER_COMPLIANT;
   mpCodecContext->error_concealment = FF_EC_DEBLOCK;

   // Open codec
   if (avcodec_open(mpCodecContext, mpCodec) < 0)
   {
      osPrintf("MpdH263::initDecode: could not open codec\n");
      freeDecode();
      return OS_FAILED;
   }

   // ensure space for padding used by ffmpeg bitstream reader
   const size_t bufferSize = VideoFormat::width_CIF * VideoFormat::height_CIF * 4;
   mpDecodeBuffer = (UCHAR*)realloc(mpDecodeBuffer, bufferSize);
   if (NULL == mpDecodeBuffer)
   {
      osPrintf("MpdH263::initDecode: unable to allocate decode buffer\n");
      freeDecode();
      return OS_NO_MEMORY;
   }

   mDecodeBufferSize = bufferSize;
   mpDecodePos = mpDecodeBuffer;
   mDecodeLeft = 0;

   mpHeaderA = new h263_header_a;
   mpHeaderB = new h263_header_b;
   if (NULL == mpHeaderA || NULL == mpHeaderB)
   {
      osPrintf("MpdH263::initDecode: unable to allocate H263 headers\n");
      freeDecode();
      return OS_NO_MEMORY;
   }
   return OS_SUCCESS;
}

OsStatus MpdH263::freeDecode()
{
   if (NULL != mpDecodeBuffer)
      mpDecodeBuffer = (UCHAR*)realloc(mpDecodeBuffer, 0);

   mDecodeBufferSize = 0;

   delete mpHeaderA;
   mpHeaderA = NULL;

   delete mpHeaderB;
   mpHeaderB = NULL;

   if (mpCodecContext != NULL)
   {
      if (NULL != mpCodecContext->codec)
         avcodec_close(mpCodecContext);

      av_free(mpCodecContext);
      mpCodecContext = NULL;
   }

   if (mpFrame != NULL)
   {
      av_free(mpFrame);
      mpFrame = NULL;
   }

   mpCodec = NULL;
   return OS_SUCCESS;
}

/* ============================= MANIPULATORS ============================= */

OsStatus MpdH263::initStream(const MpRtpBufPtr &pPacket)
{
   // Initialize sequence number counter
   mPreviousSeqNum = pPacket->getRtpSequenceNumber();

   return OS_SUCCESS;
}


static short RtpSeqDiff(RtpSeq now, RtpSeq prev)
{
   if (now >= prev)
      return now - prev;
   else
      return USHRT_MAX - prev + now + 1;
}

MpVideoBufPtr MpdH263::decode(const MpRtpBufPtr &pPacket, bool &packetConsumed, bool forceFlag)
{
   MpVideoBufPtr pFrame;
   packetConsumed = true;

   // this would probably be a bug?
   if (!pPacket.isValid())
      return pFrame;

   const RtpSeq seqNum = pPacket->getRtpSequenceNumber();
   // check if the packet is obsolete, we should skip it then (?)
   if (compare(mPreviousSeqNum, seqNum) > 0)
   {
      osPrintf("MpdH263::decode: obsolete frame %d rejected\n", seqNum);
      return pFrame;
   }

   size_t sbit = 0, ebit = 0, payloadSize = 0;
   const void* payload = NULL;
   bool headerParsed = false;
   bool hasStart = false;
   bool hasEnd = pPacket->isRtpMarker();

   short diff = RtpSeqDiff(seqNum, mPreviousSeqNum);
   if (diff > 1)
   {
      osPrintf("MpdH263::decode: frame diff %d\n", long(diff));
      if (mPreviousTimeStamp != pPacket->getRtpTimestamp())
         flushDecoder();
   }

   //{
   //   osPrintf("MpdH263::decode: frame diff %d\n", long(diff));
   //   // if packet is not the next (expected) one, we should not process it now,
   //   // but also indicate that packet is not consumed, so that we have chance to get it 
   //   // again when the "hole" is filled. unless the frame difference is increasing,
   //   // then we should dump the current decoder state and start decoding from fresh 
   //   // frame
   //   if (diff < 5 && !forceFlag)
   //   {
   //      packetConsumed = false;
   //      return pFrame;
   //   }

   //   if (!parseHeader(pPacket, payload, payloadSize, sbit, ebit))
   //   {
   //      osPrintf("MpdH263::decode: parse header failed\n");
   //      return pFrame;
   //   }

   //   headerParsed = true;
   //   hasStart = h263_is_psc_payload(payload, payloadSize);
   //}

   // extract proper payload from the packet
   if (!headerParsed)
   {
      if (!parseHeader(pPacket, payload, payloadSize, sbit, ebit))
      {
         osPrintf("MpdH263::decode: parse header failed\n");
         return pFrame;
      }

      headerParsed = true;
      hasStart = h263_is_psc_payload(payload, payloadSize);
   }

   mPreviousSeqNum = seqNum;
   mPreviousTimeStamp = pPacket->getRtpTimestamp();

   if ((mpDecodePos - mpDecodeBuffer) + mDecodeLeft + payloadSize > mDecodeBufferSize)
   {
      osPrintf("MpdH263::decode: payload would overwrite decode buffer, flushing its contents\n");
      // avoid overwriting decode buffer
      flushDecoder();
   }

   accumulatePayload(payload, payloadSize, sbit, ebit);

   int gotPicture = FALSE;
   while (0 != mDecodeLeft && 0 == gotPicture)
   {
      int len = avcodec_decode_video(mpCodecContext, mpFrame, &gotPicture, mpDecodePos, mDecodeLeft);
      if (len < 0)
      {
         osPrintf("MpdH263::decode: Error while decoding frame %d\n", mpCodecContext->frame_number);
         flushDecoder();
         break;
      }

      mDecodeLeft -= len;
      if (0 == mDecodeLeft)
         flushDecoder();
      else
         mpDecodePos += len;
   }

   if (0 != gotPicture)
      pFrame = renderVideoBuffer();

   return pFrame;
}

UtlBoolean MpdH263::parseHeader(const MpRtpBufPtr &pPacket, const void*& payload, size_t& payloadSize, size_t& sbit, size_t& ebit)
{
   const void* data = pPacket->getDataPtr();
   const size_t dataSize = pPacket->getPayloadSize();
   // theoretically empty payload is possible, I don't know if this would not 
   // make the packet invalid, but it's better to check than to crash
   if (0 == dataSize)
      return FALSE;

   const h263_mode packetMode = h263_query_mode(data);
   if (h263_mode_c == packetMode)
      // we're not required to handle Mode C packets; actually I haven't seen
      // a RT application which would render these
      return FALSE;

   const size_t headerSize = h263_header_length(packetMode);
   if (dataSize < headerSize)
      // check if we have enough data to read the header
      return FALSE;

   // unpack header and check stuffing
   if (h263_mode_a == packetMode)
   {
      mpHeaderA->unpack(data);
      sbit = mpHeaderA->sbit;
      ebit = mpHeaderA->ebit;
   }
   else
   {
      mpHeaderB->unpack(data);
      sbit = mpHeaderB->sbit;
      ebit = mpHeaderB->ebit;
      if (mAccuPayloadBits == 0)
      {
         // can't correctly decode frame starting with Mode B header
         return FALSE;
      }
   }

   payloadSize = dataSize - headerSize;
   if (sbit + ebit > 8 * payloadSize)
      // check if indicated stuffing is not larger than left payload, this would 
      // indicate corrupted payload or header.
      return FALSE;

   payload = (const UCHAR*)data + headerSize;
   return TRUE;
}

UtlBoolean MpdH263::accumulatePayload(const void* payload, size_t payloadSize, size_t sbit, size_t ebit)
{
   ldiv_t d = ldiv(mAccuPayloadBits, 8);
   if (0 == d.rem && 0 == sbit)
   {
      // trivial optimization: don't even try bit-shifting if buffers are byte-aligned
      void* dst = mpDecodeBuffer + d.quot;
      memcpy(dst, payload, payloadSize);
      mAccuPayloadBits += payloadSize * 8 - ebit;
      mpDecodeBuffer[mAccuPayloadBits / 8] = 0;
   }
   else
   {
      // use ffmpeg's bitstream utilities for wicked bit-shifting wizardry
      PutBitContext pb;
      init_put_bits(&pb, mpDecodeBuffer, mDecodeBufferSize * 8);

      if (0 != mAccuPayloadBits)
      {
         // skip already-accumulated bits
         skip_put_bits(&pb, mAccuPayloadBits);
         //const int pbInit = put_bits_count(&pb);
         //assert(pbInit == mAccuPayloadBits);
      }

      GetBitContext gb;
      init_get_bits(&gb, (const uint8_t*)payload, payloadSize * 8);
      // skip start bits
      skip_bits(&gb, sbit);
      // the number of payload bits we actually would like to have copied
      const size_t payloadBits = payloadSize * 8 - sbit - ebit;
      // calculate number of full 32-bit words that fit in payloadBits
      d = ldiv(payloadBits, 32);
      // transfer bits in 32-bit word chunks
      uint32_t w;
      for (long i = 0; i < d.quot; ++i)
      {
         w = get_bits_long(&gb, 32);
         put_bits(&pb, 32, w);
      }

      if (0 != d.rem)
      {
         // transfer the trailing remainder
         w = get_bits_long(&gb, d.rem);
         put_bits(&pb, d.rem, w);
      }

      // bump accumulated bits count
      mAccuPayloadBits += payloadBits;
      //const int pbBits = put_bits_count(&pb);
      //assert(pbBits == mAccuPayloadBits);
      // insert null-termination expected by avcodec_decode_video()
      put_bits(&pb, 32, 0);
   }

   size_t decodePos = mpDecodePos - mpDecodeBuffer;
   d = ldiv(mAccuPayloadBits, 8);
   mDecodeLeft = d.quot;
   if (0 != d.rem)
      ++mDecodeLeft;

   mDecodeLeft -= decodePos;
   return TRUE;
}

void MpdH263::flushDecoder()
{
   mAccuPayloadBits = 0;
   mpDecodePos = mpDecodeBuffer;
   mDecodeLeft = 0;
}

MpVideoBufPtr MpdH263::renderVideoBuffer()
{
   MpBufPool* pool = getVideoBufferPool();
   assert(NULL != pool);

   // Get buffer for captured data
   MpVideoBufPtr pFrame = pool->getBuffer();
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
      char *src      = (char*)mpFrame->data[plane];
      int   srcStep  = mpFrame->linesize[plane];
      for (int i=0; i<planeHeight; i++)
      {
         memcpy(dest, src, planeWidth);
         dest += destStep;
         src += srcStep;
      }
   }

   return pFrame;
}

/* ============================== ACCESSORS =============================== */

/* =============================== INQUIRY ================================ */


/* ////////////////////////////// PROTECTED /////////////////////////////// */


/* /////////////////////////////// PRIVATE //////////////////////////////// */


/* ============================== FUNCTIONS =============================== */

#endif // SIPX_VIDEO