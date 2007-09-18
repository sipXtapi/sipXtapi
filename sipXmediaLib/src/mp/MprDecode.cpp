//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
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
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MpRtpInputAudioConnection.h"
#include "mp/MprDecode.h"
#include "mp/MprDejitter.h"
#include "mp/MpDecoderBase.h"
#include "mp/NetInTask.h"
#include "mp/dmaTask.h"
#include "mp/MpMediaTask.h"
#include "mp/MpCodecFactory.h"
#include "mp/MpJitterBuffer.h"
#include "mp/MpFlowGraphBase.h"
#include "mp/MprnDTMFMsg.h"
#include "os/OsDefs.h"
#include "os/OsSysLog.h"
#include "os/OsLock.h"
#include "os/OsNotification.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprDecode::MprDecode(const UtlString& rName, MpRtpInputAudioConnection* pConn,
                     int samplesPerFrame, int samplesPerSec)
:  MpAudioResource(rName, 0, 0, 1, 1, samplesPerFrame, samplesPerSec),
   mpJB(NULL),
   mpDtmfNotication(NULL),
   mpMyDJ(NULL),
   mpCurrentCodecs(NULL),
   mNumCurrentCodecs(0),
   mpPrevCodecs(NULL),
   mNumPrevCodecs(0),
   mpConnection(pConn)
{
}

// Destructor
MprDecode::~MprDecode()
{
   // Clean up decoder object
   int i;

   // Release our codecs (if any), and the array of pointers to them.
   // Jitter buffer instance (mpJB) is also deleted here.
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
   int audioCodecsNum=0;
   UtlString codecMediaType;
   MpFlowGraphMsg msg(SELECT_CODECS, this, NULL, NULL, 0, 0);

   codecArray = new SdpCodec*[numCodecs];

   // Copy all audio codecs to new array
   for (i=0; i<numCodecs; i++) {
      codecs[i]->getMediaType(codecMediaType);
      if (codecMediaType.compareTo("audio") == 0)
      {
         codecArray[audioCodecsNum] = new SdpCodec(*codecs[i]);
         audioCodecsNum++;
      }
   }

   msg.setPtr1(codecArray);
   msg.setInt1(audioCodecsNum);
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

MpJitterBuffer* MprDecode::getJBinst(UtlBoolean optional)
{
   if ((NULL == mpJB) && (!optional)) {
      mpJB = new MpJitterBuffer();
      assert(NULL != mpJB);
   }
   return mpJB;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean MprDecode::handleSetDtmfNotify(OsNotification* pNotify)
{
   OsLock lock(mLock);

   mpDtmfNotication = pNotify;

   return TRUE;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

MprDejitter* MprDecode::getMyDejitter(void)
{
   assert(NULL != mpMyDJ);
   return mpMyDJ;
}

#ifdef SIPX_DEBUG /* [ */
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

UtlBoolean MprDecode::doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond)
{
   MpAudioBufPtr out;
   MpAudioSample* pSamples;

   if (outBufsSize == 0)
      return FALSE;

   if (!isEnabled) {
      return TRUE;
   }

   MprDejitter* pDej = getMyDejitter();

   // Not sure this is a good idea to do in doProcessFrame, 
   // but the use of this lock is meaningless
   // unless we lock all access of the mpCurrentCodecs
   OsLock lock(mLock);

   // Inform the dejitter that the next frame has happened. 
   pDej->frameIncrement();

   // Cycle through all decoders and process frames for them.
   for (int i = 0; i < mNumCurrentCodecs; i++) 
   {
      int pt = mpCurrentCodecs[i]->getPayloadType();
      MpDecoderBase* pCurDec=mpConnection->mapPayloadType(pt);
      MpRtpBufPtr rtp;

      // The codec is null. Do not continue.
      // TODO:: NEED ERROR HANDLING HERE.
      if (pCurDec==NULL)
         continue;

      UtlBoolean signalingCodec = pCurDec->getInfo()->isSignalingCodec();

      while ((rtp = pDej->pullPacket(pt, (bool)signalingCodec)).isValid()) 
      {
         // THIS JitterBuffer is NOT the same as MprDejitter!
         // This is more of a Decode Buffer.
         MpJitterBuffer* pJBState = getJBinst();

         OsStatus res = pJBState->pushPacket(rtp);
         if (res != OS_SUCCESS) {
            osPrintf("\n\n *** MprDecode::doProcessFrame() returned %d\n", res);
            osPrintf(" pt=%d, Ts=%d, Seq=%d\n\n",
                     rtp->getRtpPayloadType(),
                     rtp->getRtpTimestamp(), rtp->getRtpSequenceNumber());
         }
         if (signalingCodec)
         {
            uint8_t event;
            UtlBoolean isStarted;
            UtlBoolean isStopped;
            uint16_t duration;
            OsStatus sigRes;

            do {
               sigRes = pCurDec->getSignalingData(event,
                                                  isStarted,
                                                  isStopped,
                                                  duration);
               assert(sigRes != OS_NOT_SUPPORTED);
               if (sigRes == OS_SUCCESS && isStarted)
               {
                  // Post DTMF notification message to indicate key down.
                  MprnDTMFMsg dtmfMsg(getName(), mpConnection->getConnectionId(),
                                      (MprnDTMFMsg::KeyCode)event,
                                      MprnDTMFMsg::KEY_DOWN);
                  sendNotification(dtmfMsg);

                  // Old way to indicate DTMF event. Will be removed soon, I hope.
                  if (mpDtmfNotication)
                  {
                     mpDtmfNotication->signal( duration
                                             | (uint32_t)(event) << 16
                                             | 0<<31);
                  }
               }

               if (sigRes == OS_SUCCESS && isStopped)
               {
                  // Post DTMF notification message to indicate key up.
                  MprnDTMFMsg dtmfMsg(getName(), mpConnection->getConnectionId(),
                                      (MprnDTMFMsg::KeyCode)event,
                                      MprnDTMFMsg::KEY_UP, duration);
                  sendNotification(dtmfMsg);

                  // Old way to indicate DTMF event. Will be removed soon, I hope.
                  if (mpDtmfNotication)
                  {
                     mpDtmfNotication->signal( duration
                                             | (uint32_t)(event) << 16
                                             | 1<<31);
                  }
               }
            } while (sigRes == OS_SUCCESS);
         }
      }
   }

   // Get new audio buffer for decoded sound
   out = MpMisc.RawAudioPool->getBuffer();
   if (!out.isValid())
   {
      return FALSE;
   }
   out->setSamplesNumber(samplesPerFrame);
   pSamples = out->getSamplesWritePtr();
   memset(pSamples, 0, out->getSamplesNumber() * sizeof(MpAudioSample));
   out->setSpeechType(MpAudioBuf::MP_SPEECH_SILENT);

   // Decode one packet from Jitter Buffer
   MpJitterBuffer* pJBState = getJBinst();
   int decodedAPacket = FALSE;
   if (pJBState) {
      // This should be a JB_something or other.  However the only
      // current choices is a short or long equivalent and this needs
      // to be a plain old int:
      int bufLength=samplesPerFrame;
      int res;
      res = pJBState->getSamples(pSamples, bufLength);
      assert(bufLength == (int)out->getSamplesNumber());
      out->setSpeechType(MpAudioBuf::MP_SPEECH_UNKNOWN);
      decodedAPacket = TRUE;
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
   UtlString mime;
   UtlString fmtp;
   OsStatus ret;
   MpDecoderBase* pNewDecoder;
   MpDecoderBase* pOldDecoder;
   MpCodecFactory* pFactory = MpCodecFactory::getMpCodecFactory();
   int allReusable = 0;
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

#if 0 // [
   // Check to see if all codecs in pCodecs can be handled by codecs
   // in mpCurrentCodecs.
   for (i=0; i<numCodecs; i++) {
      pCodec = pCodecs[i];
      pCodec->getEncodingName(mime);
      pCodec->getSdpFmtpField(fmtp);
      payload = pCodec->getCodecPayloadFormat();
#if 0
      osPrintf("  #%d: New=0x%X/i:%d/x:%d, ",
         i, (int)pCodec, ourCodec, payload);
#endif
      pOldDecoder = mpConnection->mapPayloadType(payload);
      if (NULL != pOldDecoder) {
         UtlString oldMime;
         UtlString oldFmtp;
         pOldDecoder->getInfo()->getCodecType();
#if 0
         osPrintf("  Old=0x%X/i:%d", (int)pOldDecoder, oldSdpType);
#endif
         canReuse = (ourCodec == oldSdpType);
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
#endif // 0 ]

   // If the new list is not a subset of the old list, we have to copy
   // pCodecs into mpCurrentCodecs.
   if (!allReusable) {
      // Lock the m*Codecs members.
      OsLock lock(mLock);

      // Delete the current codecs.
      handleDeselectCodecs(FALSE);

      mNumCurrentCodecs = numCodecs;
      mpCurrentCodecs = new MpDecoderBase*[numCodecs];

      for (i=0; i<numCodecs; i++) {
         pCodec = pCodecs[i];
         pCodec->getEncodingName(mime);
         pCodec->getSdpFmtpField(fmtp);
         payload = pCodec->getCodecPayloadFormat();
         ret = pFactory->createDecoder(mime, fmtp, payload, pNewDecoder);
         assert(OS_SUCCESS == ret);
         assert(NULL != pNewDecoder);
         pNewDecoder->initDecode();
         // Add this codec to mpConnection's payload type decoding table.
         mpConnection->addPayloadType(payload, pNewDecoder);
         mpCurrentCodecs[i] = pNewDecoder;
      }

      // Go back and add any signaling codecs to Jitter Buffer.
      for (i=0; i<numCodecs; i++) {
         if (mpCurrentCodecs[i]->getInfo()->isSignalingCodec()) {
            mpCurrentCodecs[i]->initDecode();
         }
      }
   }

   MpJitterBuffer* pJBState = getJBinst();   
   pJBState->setCodecList(mpCurrentCodecs,numCodecs);

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

UtlBoolean MprDecode::handleDeselectCodecs(UtlBoolean shouldLock)
{
   int i;
   MpDecoderBase** pCurrentCodecs;
   MpDecoderBase** pPrevCodecs;
   int newN;
   if(shouldLock)
   {
       mLock.acquire();
   }
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

   delete mpJB;
   mpJB = NULL;

   if(shouldLock)
   {
       mLock.release();
   }
   return TRUE;
}

/* ============================ FUNCTIONS ================================= */
