//  
// Copyright (C) 2006-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

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
#include "mp/MpStringResourceMsg.h"
#include "os/OsDefs.h"
#include "os/OsSysLog.h"
#include "os/OsLock.h"
#include "os/OsNotification.h"
#include "mp/MprDejitter.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprDecode::MprDecode(const UtlString& rName, MpConnectionID connectionId,
                     const UtlString &plcName)
: MpAudioResource(rName, 0, 0, 1, 1)
, mpJB(NULL)
, mpDtmfNotication(NULL)
, mpMyDJ(NULL)
, mIsJBInitialized(FALSE)
, mpCurrentCodecs(NULL)
, mNumCurrentCodecs(0)
, mpPrevCodecs(NULL)
, mNumPrevCodecs(0)
, mConnectionId(connectionId)
{
   mpJB = new MpJitterBuffer(plcName);
   assert(mpJB != NULL);
}

// Destructor
MprDecode::~MprDecode()
{
   // Release our codecs (if any), and the array of pointers to them.
   handleDeselectCodecs();

   // Free decoder buffer.
   delete mpJB;

   // Delete the list of codecs used in the past.
   OsLock lock(mLock);
   if (mNumPrevCodecs > 0)
   {
      for (int i=0; i<mNumPrevCodecs; i++)
      {
         mpPrevCodecs[i]->freeDecode();
         delete mpPrevCodecs[i];
      }
      delete[] mpPrevCodecs;
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

OsStatus MprDecode::setPlc(const UtlString& namedResource,
                           OsMsgQ& fgQ,
                           const UtlString& plcName)
{
   MpStringResourceMsg msg(plcName, namedResource);
   return fgQ.send(msg, sOperationQueueTimeout);
}

void MprDecode::setMyDejitter(MprDejitter* pDJ)
{
   mpMyDJ = pDJ;
}

OsStatus MprDecode::pushPacket(const MpRtpBufPtr &pRtp)
{
   // Lock access to dejitter and m*Codecs data
   OsLock lock(mLock);
   int pt = pRtp->getRtpPayloadType();
   const MpCodecInfo* pDecoderInfo = mDecoderMap.mapPayloadType(pt)->getInfo();

   return getMyDejitter()->pushPacket(pRtp, pDecoderInfo->isSignalingCodec());
}

/* ============================ ACCESSORS ================================= */

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

UtlBoolean MprDecode::doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond)
{
   MpAudioBufPtr out;
   MpRtpBufPtr rtp;

   if (outBufsSize == 0)
      return FALSE;

   if (!isEnabled) {
      return TRUE;
   }

   // If mIsJBInitialized is FALSE, we haven't got selectCodecs() called.
   // This is fine, but a long we do not know anything about codecs,
   // we could not continue.
   if (!mIsJBInitialized)
   {
      return TRUE;
   }

   MprDejitter* pDej = getMyDejitter();

   // Lock access to dejitter and m*Codecs data
   OsLock lock(mLock);

   // Inform the dejitter that the next frame has happened. 
   pDej->frameIncrement(mpFlowGraph->getSamplesPerFrame());

   while ((rtp = pDej->pullPacket()).isValid()) 
   {
      int pt = rtp->getRtpPayloadType();
      MpDecoderBase* pCurDec = mDecoderMap.mapPayloadType(pt);

      // The codec is null. Do not continue.
      // TODO:: NEED ERROR HANDLING HERE.
      if (pCurDec==NULL)
         break;

      // THIS JitterBuffer is NOT the same as MprDejitter!
      // This is more of a Decode Buffer.
      OsStatus res = mpJB->pushPacket(rtp);
      if (res != OS_SUCCESS)
      {
         osPrintf("\n\n *** MprDecode::doProcessFrame() returned %d\n", res);
         osPrintf(" pt=%d, Ts=%d, Seq=%d\n\n",
                  rtp->getRtpPayloadType(),
                  rtp->getRtpTimestamp(), rtp->getRtpSequenceNumber());
      }
      if (pCurDec->getInfo()->isSignalingCodec())
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
               MprnDTMFMsg dtmfMsg(getName(), mConnectionId,
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
               MprnDTMFMsg dtmfMsg(getName(), mConnectionId,
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

   // Get next decoded frame
   outBufs[0] = mpJB->getSamples();

   return TRUE;
}

// Handle old style messages for this resource.
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

// Handle new style messages for this resource.
UtlBoolean MprDecode::handleMessage(MpResourceMsg& rMsg)
{
   UtlBoolean msgHandled = FALSE;

   switch (rMsg.getMsg()) 
   {
   case MPRM_SET_PLC:
      msgHandled = handleSetPlc(((MpStringResourceMsg*)&rMsg)->getData());
      break;

   default:
      // If we don't handle the message here, let our parent try.
      msgHandled = MpResource::handleMessage(rMsg); 
      break;
   }
   return msgHandled;
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
      pOldDecoder = mDecoderMap.mapPayloadType(payload);
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

   // Lock the m*Codecs members.
   OsLock lock(mLock);

   // If the new list is not a subset of the old list, we have to copy
   // pCodecs into mpCurrentCodecs.
   if (!allReusable)
   {
      // Delete the current codecs.
      handleDeselectCodecs(FALSE);

      mNumCurrentCodecs = 0;
      mpCurrentCodecs = new MpDecoderBase*[numCodecs];

      for (i=0; i<numCodecs; i++) {
         pCodec = pCodecs[i];
         pCodec->getEncodingName(mime);
         pCodec->getSdpFmtpField(fmtp);
         int sampleRate = pCodec->getSampleRate();
         int numChannels = pCodec->getNumChannels();
         payload = pCodec->getCodecPayloadFormat();
         ret = pFactory->createDecoder(mime, fmtp, sampleRate, numChannels,
                                       payload, pNewDecoder);
         assert(OS_SUCCESS == ret);
         assert(NULL != pNewDecoder);
         if (pNewDecoder->initDecode() == OS_SUCCESS)
         {
            // Add this codec to mpConnection's payload type decoding table.
            mDecoderMap.addPayloadType(payload, pNewDecoder);
            mpCurrentCodecs[mNumCurrentCodecs] = pNewDecoder;
            mNumCurrentCodecs++;
         }
         else
         {
            delete pNewDecoder;
         }
      }

      // Go back and add any signaling codecs to Jitter Buffer.
      for (i=0; i<mNumCurrentCodecs; i++) {
         if (mpCurrentCodecs[i]->getInfo()->isSignalingCodec()) {
            mpCurrentCodecs[i]->initDecode();
         }
      }
   }

   // If decoder buffer is not yet initialized - lets go initialize it.
   if (!mIsJBInitialized)
   {
      mpJB->init(mpFlowGraph->getSamplesPerSec(),
                 mpFlowGraph->getSamplesPerFrame());
      mIsJBInitialized= TRUE;
   }

   mpJB->setCodecList(&mDecoderMap);

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
      mDecoderMap.deletePayloadType(payload);
   }
   return TRUE;
}

UtlBoolean MprDecode::handleSetPlc(const UtlString &plcName)
{
   mpJB->setPlc(plcName);

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

   if (mIsJBInitialized)
   {
      mpJB->setCodecList(NULL);
   }

   if(shouldLock)
   {
       mLock.release();
   }
   return TRUE;
}

/* ============================ FUNCTIONS ================================= */
