//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#define DEBUG_DECODING
#undef DEBUG_DECODING

// SYSTEM INCLUDES
#include <assert.h>

#ifdef WIN32 /* [ */
#include <winsock2.h>
#endif /* WIN32 ] */

#if defined(_VXWORKS) || defined(__pingtel_on_posix__) /* [ */
#include <sys/types.h>
#include <netinet/in.h>
#endif /* _VXWORKS || __pingtel_on_posix__ ] */

#include <string.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsSysLog.h"
#include "os/OsLock.h"
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MpConnection.h"
#include "mp/MprDecode.h"
#include "mp/MprDejitter.h"
#include "mp/MpDecoderBase.h"
#include "mp/NetInTask.h"
#include "mp/dmaTask.h"
#include "mp/MpMediaTask.h"
#include "mp/MpCodecFactory.h"
#include "mp/JB/JB_API.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprDecode::MprDecode(const UtlString& rName, MpConnection* pConn,
                           int samplesPerFrame, int samplesPerSec)
:  MpAudioResource(rName, 1, 1, 1, 1, samplesPerFrame, samplesPerSec),
   mPreloading(1),
   mpMyDJ(NULL),
   mpCurrentCodecs(NULL),
   mNumCurrentCodecs(0),
   mpPrevCodecs(NULL),
   mNumPrevCodecs(0),
   mpConnection(pConn),
   mLock(OsMutex::Q_PRIORITY|OsMutex::INVERSION_SAFE)
{

   // no work required
}

// Destructor
MprDecode::~MprDecode()
{
   // Clean up decoder object
   int i;

   // Release our codecs (if any), and the array of pointers to them
   handleDeselectCodecs();

   // Delete the list of codecs used in the past.
   {
      OsLock lock(mLock);
      if (mNumPrevCodecs > 0) {
         for (i=0; i<mNumPrevCodecs; i++) {
            mpPrevCodecs[i]->freeDecode();
            delete mpPrevCodecs[i];
         }
         delete[] mpPrevCodecs;
      }
   }
}

/* ============================ MANIPULATORS ============================== */

OsStatus MprDecode::selectCodecs(SdpCodec* codecs[], int numCodecs)
{
   OsStatus ret = OS_SUCCESS;
   SdpCodec** codecArray;
   int i;
   MpFlowGraphMsg msg(SELECT_CODECS, this, NULL, NULL, 0, 0);

   codecArray = new SdpCodec*[numCodecs];

   for (i=0; i<numCodecs; i++) {
      codecArray[i] = new SdpCodec(*codecs[i]);
   }

   msg.setPtr1(codecArray);
   msg.setInt1(numCodecs);
   ret = postMessage(msg);

   return ret;
}

OsStatus MprDecode::deselectCodec()
{
   MpFlowGraphMsg msg(DESELECT_CODECS, this, NULL, NULL, 0, 0);
   OsStatus ret = OS_SUCCESS;

//   osPrintf("MprDecode::deselectCodec\n");
   ret = postMessage(msg);

   return ret;
}

void MprDecode::setMyDejitter(MprDejitter* pDJ)
{
   mpMyDJ = pDJ;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean MprDecode::handleSetDtmfNotify(OsNotification* pNotify)
{
   MpDecoderBase** pMDB;
   UtlBoolean ret = TRUE;
   int i;
   OsLock lock(mLock);

   pMDB = mpCurrentCodecs;
   for (i=0; i<mNumCurrentCodecs; i++) {
      if (((*pMDB)->getInfo())->isSignalingCodec()) {
         (*pMDB)->handleSetDtmfNotify(pNotify);
      }
      pMDB++;
   }
   return ret;
}

UtlBoolean MprDecode::setDtmfTerm(MprRecorder *pRecorder)
{
   MpDecoderBase** pMDB;
   UtlBoolean ret = TRUE;
   int i;
   OsLock lock(mLock);

   pMDB = mpCurrentCodecs;
   for (i=0; i<mNumCurrentCodecs; i++) {
      if (((*pMDB)->getInfo())->isSignalingCodec()) {
         (*pMDB)->setDtmfTerm(pRecorder);
      }
      pMDB++;
   }
   return ret;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

MprDejitter* MprDecode::getMyDejitter(void)
{
   assert(NULL != mpMyDJ);
   return mpMyDJ;
}

#ifdef DEBUG /* [ */
static void showRtpPacket(MpRtpBufPtr rtp)
{
   struct RtpHeader &rh = rtp->getRtpHeader();
   int len;

   rh.vpxcc = rp->vpxcc;
   rh.mpt = rp->mpt;
   rh.seq = ntohs(rp->seq);
   rh.timestamp = ntohl(rp->timestamp);
   rh.ssrc = ntohl(rp->ssrc);
   len = rtp->getPayloadSize();
   Zprintf("RcvRTP: %02X, %02X, %d, %d, %08X, and %d bytes of data\n",
      rh.vpxcc, rh.mpt, rh.seq, rh.timestamp, rh.ssrc, len);
}
#endif /* DEBUG ] */

void MprDecode::pushIntoJitterBuffer(MpRtpBufPtr &pPacket, int packetLen)
{
   int res;

   JB_inst* pJBState = mpConnection->getJBinst();

   res = JB_RecIn(pJBState, pPacket);
   if (res != 0) {
      osPrintf(
         "\n\n *** JB_RecIn(0x%X, %d) returned %d\n",
         (int) pJBState, packetLen, res);
      osPrintf(" pt=%d, Ts=%d, Seq=%d\n\n",
         pPacket->getRtpPayloadType(),
         pPacket->getRtpTimestamp(), pPacket->getRtpSequenceNumber());
   }
}

UtlBoolean MprDecode::doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond)
{
#ifdef DEBUG_DECODING /* [ */
static int numFramesForWarnings = 0;
static int numWarnings = 0;
#endif /* DEBUG_DECODING ] */

   MpRtpBufPtr rtp;
   MpAudioBufPtr out;

#ifdef DEBUG_DECODING /* [ */
   numFramesForWarnings++;
#endif /* DEBUG_DECODING ] */

   MpDecoderBase* pCurDec;
   MpAudioSample* pSamples;

   if (outBufsSize == 0)
      return FALSE;

   if (!isEnabled) {
      mPreloading = 1;
      return TRUE;
   }

   // Push all incoming packets (from Dejitter) to Jitter Buffer
   {
      MprDejitter* pDej = getMyDejitter();
      int packetLen;
      int pt;

      while ((rtp = pDej->pullPacket()).isValid()) {
         pt = rtp->getRtpPayloadType();
         pCurDec = mpConnection->mapPayloadType(pt);
         if (pCurDec != NULL) {
            packetLen = pCurDec->decodeIn(rtp);
            if (packetLen > 0) {
               pushIntoJitterBuffer(rtp, packetLen);
            }
         }
         rtp.release();
      }
   }

   // Get new audio buffer for decoded sound
   out = MpMisc.UcbPool->obtainBuffer();
   if (!out.isValid())
   {
      return FALSE;
   }
   out->setSamplesNumber(samplesPerFrame);
   pSamples = out->getSamples();
   memset(pSamples, 0, out->getSamplesNumber() * sizeof(MpAudioSample));
   out->setSpeechType(MpAudioBuf::MP_SPEECH_SILENT);

   // Decode one packet from Jitter Buffer
   JB_inst* pJBState = mpConnection->getJBinst();
   if (pJBState) {
      // This should be a JB_something or other.  However the only
      // current choices is a short or long equivalant and this needs
      // to be a plain old int:
      int outLen;
      int res;
      res = JB_RecOut(pJBState, pSamples, &outLen);
      assert(outLen == out->getSamplesNumber());
      out->setSpeechType(MpAudioBuf::MP_SPEECH_UNKNOWN);
   }

   // Push decoded audio packet downstream
   outBufs[0] = out;

   return TRUE;
}

// Handle messages for this resource.
UtlBoolean MprDecode::handleMessage(MpFlowGraphMsg& rMsg)
{
   UtlBoolean ret = FALSE;

   switch (rMsg.getMsg()) {
   case DESELECT_CODECS:
      handleDeselectCodecs();
      ret = TRUE;
      break;
   case SELECT_CODECS:
      handleSelectCodecs((SdpCodec**) rMsg.getPtr1(), rMsg.getInt1());
      ret = TRUE;
      break;
   default:
      ret = MpAudioResource::handleMessage(rMsg);
      break;
   }
   return ret;
}

UtlBoolean MprDecode::handleSelectCodecs(SdpCodec* pCodecs[], int numCodecs)
{
   int i;
   SdpCodec* pCodec;
   int payload;
   SdpCodec::SdpCodecTypes ourCodec;
   SdpCodec::SdpCodecTypes oldSdpType = SdpCodec::SDP_CODEC_UNKNOWN;
   OsStatus ret;
   MpDecoderBase* pNewDecoder;
   MpDecoderBase* pOldDecoder;
   MpCodecFactory* pFactory = MpCodecFactory::getMpCodecFactory();
   int allReusable = 1;
   int canReuse;
#if 0
   osPrintf("MprDecode::handleSelectCodecs(%d codec%s):\n",
      numCodecs, ((1 == numCodecs) ? "" : "s"));
#endif
   if (OsSysLog::willLog(FAC_MP, PRI_DEBUG))
   {
      for (i=0; i<numCodecs; i++) {
         pCodec = pCodecs[i];
         OsSysLog::add(FAC_MP, PRI_DEBUG,
                       "MprDecode::handleSelectCodecs "
                       "pCodecs[%d]->getCodecType() = %d, "
                       "pCodecs[%d]->getCodecPayloadFormat() = %d",
                       i, pCodec->getCodecType(),
                       i, pCodec->getCodecPayloadFormat());
            }
   }

   // Check to see if all codecs in pCodecs can be handled by codecs
   // in mpCurrentCodecs.
   for (i=0; i<numCodecs; i++) {
      pCodec = pCodecs[i];
      ourCodec = pCodec->getCodecType();
      payload = pCodec->getCodecPayloadFormat();
#if 0
      osPrintf("  #%d: New=0x%X/i:%d/x:%d, ",
         i, (int)pCodec, ourCodec, payload);
#endif
      pOldDecoder = mpConnection->mapPayloadType(payload);
      if (NULL != pOldDecoder) {
         oldSdpType = pOldDecoder->getInfo()->getCodecType();
#if 0
         osPrintf("  Old=0x%X/i:%d", (int)pOldDecoder, oldSdpType);
#endif
         canReuse = (ourCodec == oldSdpType)
            || ((SdpCodec::SDP_CODEC_G729AB == ourCodec)
                            && (SdpCodec::SDP_CODEC_G729A == oldSdpType))
            || ((SdpCodec::SDP_CODEC_G729A == ourCodec)
                            && (SdpCodec::SDP_CODEC_G729AB == oldSdpType));
      } else {
         // osPrintf("  no Old");
         canReuse = 0;
      }
      allReusable &= canReuse;
#if 0
      osPrintf(" i:%d/x:%d (%sreusable%s)\n", ourCodec, payload,
         (canReuse ? "" : "not "),
         (canReuse && (ourCodec != oldSdpType) ? "[*]" : ""));
#endif
   }

   // If the new list is not a subset of the old list, we have to copy
   // pCodecs into mpCurrentCodecs.
   if (!allReusable) {
      // Lock the m*Codecs members.
      OsLock lock(mLock);

      // Delete the current codecs.
      handleDeselectCodecs();

      mNumCurrentCodecs = numCodecs;
      mpCurrentCodecs = new MpDecoderBase*[numCodecs];

      for (i=0; i<numCodecs; i++) {
         pCodec = pCodecs[i];
         ourCodec = pCodec->getCodecType();
         payload = pCodec->getCodecPayloadFormat();
         ret = pFactory->createDecoder(ourCodec, payload, pNewDecoder);
         assert(OS_SUCCESS == ret);
         assert(NULL != pNewDecoder);
         pNewDecoder->initDecode(mpConnection);
         // Add this codec to mpConnection's payload type decoding table.
         mpConnection->addPayloadType(payload, pNewDecoder);
         mpCurrentCodecs[i] = pNewDecoder;
      }

      // Go back and add any signaling codecs to Jitter Buffer.
      for (i=0; i<numCodecs; i++) {
         if (mpCurrentCodecs[i]->getInfo()->isSignalingCodec()) {
            mpCurrentCodecs[i]->initDecode(mpConnection);
         }
      }
   }

   // Delete the list pCodecs.
   for (i=0; i<numCodecs; i++) {
      delete pCodecs[i];
   }
   delete[] pCodecs;
   return TRUE;
}

UtlBoolean MprDecode::handleDeselectCodec(MpDecoderBase* pDecoder)
{
   int payload;

   if (NULL != pDecoder) {
      payload = pDecoder->getPayloadType();
      mpConnection->deletePayloadType(payload);
   }
   return TRUE;
}

UtlBoolean MprDecode::handleDeselectCodecs()
{
   int i;
   MpDecoderBase** pCurrentCodecs;
   MpDecoderBase** pPrevCodecs;
   int newN;
   OsLock lock(mLock);

   if (0 < mNumCurrentCodecs) {

      newN = mNumCurrentCodecs + mNumPrevCodecs;
      pPrevCodecs = new MpDecoderBase*[newN];
#if 0
      osPrintf("|handleDeselectCodecs(0x%X): (0x%X,%d) -> (0x%X,%d) (+%d)\n",
         (int) this, (int) mpPrevCodecs, mNumPrevCodecs, (int) pPrevCodecs,
         newN, mNumCurrentCodecs);
#endif
      if (mNumPrevCodecs > 0) {
         for (i=0; i<mNumPrevCodecs; i++) {
            pPrevCodecs[i] = mpPrevCodecs[i];
         }
         delete[] mpPrevCodecs;
      }

      i = mNumCurrentCodecs;
      mNumCurrentCodecs = 0;
      pCurrentCodecs = mpCurrentCodecs;
      mpCurrentCodecs = NULL;
      while (i>0) {
         i--;
         handleDeselectCodec(pCurrentCodecs[i]);
         pPrevCodecs[i+mNumPrevCodecs] = pCurrentCodecs[i];
         pCurrentCodecs[i] = NULL;
      }
      delete[] pCurrentCodecs;
      mpPrevCodecs = pPrevCodecs;
      mNumPrevCodecs = newN;
   }
   return TRUE;
}

/* ============================ FUNCTIONS ================================= */
