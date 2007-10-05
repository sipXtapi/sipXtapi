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
#include <stddef.h>
#include <VideoSupport/VideoFormat.h>
// APPLICATION INCLUDES
#include "mp/video/MpeH263.h"
#include "mp/video/rfc2190.h"
#include "sdp/SdpCodec.h"
#include "mp/MprToNet.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// for now we assume only Mode A packets will be generated
static const size_t h263MaxPayloadSize = RTP_MTU - sizeof(h263_header_a);

// STATIC VARIABLE INITIALIZATIONS

#ifdef WIN32 // [
#  define EMULATE_INTTYPES
#endif // WIN32 ]

extern "C" {
#  include <avcodec.h>
}

/* //////////////////////////////// PUBLIC //////////////////////////////// */

/* =============================== CREATORS =============================== */
MpeH263::MpeH263(int payloadType, MprToNet* pRtpWriter)
 : MpVideoEncoder(payloadType, pRtpWriter)
 , mpCodec(NULL)
 , mpCodecContext(NULL)
 , mpPicture(NULL)
 , mpEncodedBuffer(NULL)
 , mpPacket(NULL)
 , mEncodedBufferSize(0)
 , mPacketReady(FALSE)
 , mTimestamp(0)
 , mPacketSize(0)
 , mpHeader(NULL)
 , mAp(FALSE)
{
   mStreamParams.setFrameFormat(SDP_VIDEO_FORMAT_QCIF);
   mStreamParams.setFrameRate(10.f);
   mStreamParams.setStreamBitrate(24000);

   // must be called before using AVcodec lib
   avcodec_init();

   // register all the codecs (you can also register only the codec
   // you wish to have smaller code
   avcodec_register_all();
}

MpeH263::~MpeH263()
{
   freeEncode();
}

OsStatus MpeH263::initEncode(const MpVideoStreamParams* params)
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

   // allow only SQCIF, QCIF, CIF sizes (supported by the encoder)
   VideoFormat vf;
   vf.SetSize(mStreamParams.getFrameWidth(), mStreamParams.getFrameHeight());
   VideoFormat::SizePreset s = vf.GetSize();

   int srcSize;
   switch (s) {
   case VideoFormat::size_SQCIF: srcSize = h263_src_SQCIF; break;
   case VideoFormat::size_QCIF: srcSize = h263_src_QCIF; break;
   case VideoFormat::size_CIF: srcSize = h263_src_CIF; break;
   case VideoFormat::size_4CIF: srcSize = h263_src_4CIF; break;
   case VideoFormat::size_16CIF: srcSize = h263_src_16CIF; break;
   default:
      assert(!"Unsupported frame size");
      return OS_FAILED;
   }

   if (mpCodec == NULL)
   {
      mpCodec = avcodec_find_encoder(CODEC_ID_H263);
   }

   if (mpCodec == NULL)
   {
      osPrintf("MpeH263::initEncode: codec not found\n");
      freeEncode();
      return OS_NOT_FOUND;
   }

   if (NULL == mpHeader)
      mpHeader = new h263_header_a;
   if (NULL == mpHeader)
   {
      osPrintf("MpeH263::initEncode: unable to allocate header\n");
      freeEncode();
      return OS_FAILED;
   }

   memset(mpHeader, 0, sizeof(*mpHeader));
   if (mpCodecContext == NULL)
   {
      mpCodecContext= avcodec_alloc_context();
   }

   if (mpPicture == NULL)
   {
      mpPicture= avcodec_alloc_frame();
   }

   avcodec_get_context_defaults(mpCodecContext);

   // put sample parameters
   mpCodecContext->bit_rate = mStreamParams.getStreamBitrate();
   //mpCodecContext->rc_max_rate= mpCodecContext->bit_rate * 1.25;
   //mpCodecContext->rc_min_rate = mpCodecContext->bit_rate * 0.67;

   AVRational ar = av_d2q(1. / mStreamParams.getFrameRate(), h263_timestamp_freq);
   // frames per second
   mpCodecContext->time_base.num = ar.num;
   mpCodecContext->time_base.den = ar.den;
   // emit one intra frame every second
   mpCodecContext->gop_size = mpCodecContext->time_base.den / mpCodecContext->time_base.num;

   // increase I-frame rate if bitrate above 32kbps
   if (mpCodecContext->bit_rate > 32000)
      mpCodecContext->gop_size /= 2;

   mpCodecContext->max_b_frames=0;
   mpCodecContext->width = mStreamParams.getFrameWidth();
   mpCodecContext->height = mStreamParams.getFrameHeight();
   mpCodecContext->pix_fmt = PIX_FMT_YUV420P;

   mpCodecContext->opaque = this;
   mpCodecContext->rtp_mode = 1;
   // make RTP payload size smaller than h263MaxPayloadSize, as FFmpeg is bugged and
   // will split packets AFTER macroblock exceeding the payload size, not before
   mpCodecContext->rtp_payload_size = int(h263MaxPayloadSize * 0.7);
   mpCodecContext->rtp_callback = rtpCallback;
   // according to ffmpeg docs it will split the bitstream only at GOB boundaries,
   // so the only header we need should be Mode A header.

   if (mAp)
   {
      // H.263 advanced prediction
      mpCodecContext->flags |= CODEC_FLAG_4MV;
   }

   // open codec
   if (avcodec_open(mpCodecContext, mpCodec) < 0)
   {
      osPrintf("MpeH263::initEncode: could not open codec\n");
      freeEncode();
      return OS_FAILED;
   }

   // rather impossible that encoded data overflowed buffer which 
   // can hold full 32bpp uncompressed frame
   const size_t bufferSize = vf.GetWidth() * vf.GetHeight() * 4;
   mpEncodedBuffer = (uint8_t*)realloc(mpEncodedBuffer, bufferSize);
   if (NULL == mpEncodedBuffer)
   {
      osPrintf("MpeH263::initEncode: unable to allocate buffer\n");
      freeEncode();
      return OS_FAILED;
   }

   mpPacket = (UCHAR*)realloc(mpPacket, 4 * RTP_MTU);
   if (NULL == mpPacket)
   {
      osPrintf("MpeH263::initEncode: unable to allocate buffer\n");
      freeEncode();
      return OS_FAILED;
   }
   memset(mpPacket, 0, 4 * RTP_MTU);

   mEncodedBufferSize = bufferSize;
   return OS_SUCCESS;
}

OsStatus MpeH263::freeEncode()
{
   if (mpCodecContext != NULL)
   {
      if (NULL != mpCodecContext->codec)
         avcodec_close(mpCodecContext);

      av_free(mpCodecContext);
      mpCodecContext = NULL;
   }

   if (mpPicture != NULL)
   {
      av_free(mpPicture);
      mpPicture = NULL;
   }

   if (NULL != mpEncodedBuffer)
      mpEncodedBuffer = (UCHAR*)realloc(mpEncodedBuffer, 0);

   if (NULL != mpPacket)
      mpPacket = (UCHAR*)realloc(mpPacket, 0);

   delete mpHeader;
   mpHeader = NULL;
   mEncodedBufferSize = 0;
   return OS_SUCCESS;
}


/* ============================= MANIPULATORS ============================= */
OsStatus MpeH263::encode(const MpVideoBufPtr &pFrame)
{
   assert(mpCodecContext != NULL);
   assert(mpPicture != NULL);
   assert(mpEncodedBuffer != NULL);

   if (pFrame->getColorspace() != MpVideoBuf::MP_COLORSPACE_YUV420p)
   {
      osPrintf("MpeH263::encode: Not supported color space of incoming frame\n");
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

   assert(!mPacketReady);

   // encode the image
   int res = avcodec_encode_video(
                                  mpCodecContext,
                                  mpEncodedBuffer, mEncodedBufferSize,
                                  mpPicture
                                 );

   if (-1 == res)
   {
      flushReadyPacket(TRUE);
      osPrintf("MpeH263::encode: too small encode buffer for this big frame.\n");
      return OS_FAILED;
   }

   // actual packetization is done by rtpCallback()
   // executed from within avcodec_encode_video(). It will leave ready packet
   // in mpPacket buffer for send by next rtpCallback() call (leaving marker
   // unset) or the call below, which will set marker indicating that packet 
   // contains frame end.
   flushReadyPacket(TRUE);

   return OS_SUCCESS;
}

/* ============================== ACCESSORS =============================== */

/* =============================== INQUIRY ================================ */


/* ////////////////////////////// PROTECTED /////////////////////////////// */


/* /////////////////////////////// PRIVATE //////////////////////////////// */


/* ============================== FUNCTIONS =============================== */

void MpeH263::rtpCallback(AVCodecContext* avctx, void *data, int data_size, int mb_nb)
{
   assert(NULL != avctx);
   MpeH263* self = static_cast<MpeH263*>(avctx->opaque);
   assert(NULL != self);
   self->rtpCallback(data, data_size, mb_nb);
}

void MpeH263::rtpCallback(void *data, int data_size, int mb_nb)
{
   // send to the previously rendered packet (if any)
   flushReadyPacket(FALSE);

   // that is the packetization chunk we configured
   //assert(size <= h263MaxPayloadSize);
   // at least 4 bytes with start code
   assert(data_size >= 4);
   if (data_size < 4)
      return;

   // update packet header on each PSC frame
   if (h263_is_psc_payload(data, data_size))
   {
      h263_payload_header h;
      h.unpack(data, data_size);

      // confirm the header conforms to ITU-T Rec
      assert(h.ptype.bit1);
      assert(!h.ptype.bit2);
      // make sure ffmpeg doesn't create PLUSPTYPE header for 
      // passed-in frame size;
      assert(h263_src_extended != h.ptype.sf);
      
      mpHeader->i = h.ptype.pct;
      mpHeader->u = h.ptype.umv;
      mpHeader->s = h.ptype.sac;
      mpHeader->a = mAp;
      mpHeader->src = h.ptype.sf;
      // all other fields left default (0)
   }

   mpHeader->f = 0;
   mpHeader->p = 0;

   mpHeader->pack(mpPacket);
   const size_t headerSize = h263_header_length(h263_mode_a);

   if (data_size > RTP_MTU - headerSize)
      // truncate data if size exceeds RTP packet capacity
      data_size = RTP_MTU - headerSize;

   memcpy(mpPacket + headerSize, data, data_size);
   mPacketSize = headerSize + data_size;

   RtpTimestamp timestamp = mpCodecContext->frame_number
      * mpCodecContext->time_base.num 
      * (h263_timestamp_freq / mpCodecContext->time_base.den);

   // store the timestamp for sending RTP by flushReadyPacket()
   mTimestamp = timestamp;
   // indicate that packet is ready for send (by next rtpCallback(), or encode(), after
   // avcodec_encode_video() returns.
   mPacketReady = TRUE;
}

void MpeH263::flushReadyPacket(UtlBoolean lastFramePacket)
{
   if (!mPacketReady)
      return;

   mPacketReady = FALSE;

   if (mPacketSize >= RTP_MTU)
   {
      // sorry - no way around this. decoder will have to skip the frame
      osPrintf("MpeH263::flushReadyPacket: payload size %d too big to fit RTP packet\n", mPacketSize);
      return;
   }


   MprToNet* sink = getRtpSink();
   assert(NULL != sink);
   sink->writeRtp(getPayloadType(), lastFramePacket, (unsigned char*)mpPacket, mPacketSize, mTimestamp, NULL);
}

#endif // SIPX_VIDEO