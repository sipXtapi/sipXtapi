//  
// Copyright (C) 2005-2010 SIPez LLC.  All rights reserved.
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
#include "os/OsIntTypes.h"
#include <os/fstream>
#include <stdio.h>

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

// APPLICATION INCLUDES

#include "os/OsDefs.h"
#include "os/OsEvent.h"
#include "mp/MpTypes.h"
#include "mp/MpBuf.h"
#include "mp/MprFromFile.h"
#include "mp/MpAudioAbstract.h"
#if !defined(ANDROID)
#  include "mp/MpAudioFileOpen.h"
#  include "mp/mpau.h"
#  include "mp/MpAudioWaveFileRead.h"
#endif
#include "mp/MpAudioUtils.h"
#include "mp/MpMisc.h"
#include "mp/MpFlowGraphBase.h"
#include "os/OsSysLog.h"
#include "os/OsProtectEventMgr.h"
#include "os/OsDateTime.h"
#include "os/OsTime.h"
#include "mp/MpPackedResourceMsg.h"
#include "mp/MprnProgressMsg.h"
#include "mp/MpResampler.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS
const unsigned int MprFromFile::sFromFileReadBufferSize = 8000;

static const unsigned int MAXFILESIZE = 50000000;
static const unsigned int MINFILESIZE = 8000;
extern int      samplesPerSecond;
extern int      bitsPerSample;
extern int      samplesPerFrame;

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MprFromFile::MprFromFile(const UtlString& rName)
: MpAudioResource(rName, 0, 1, 1, 1)
, mpFileBuffer(NULL)
, mFileRepeat(FALSE)
, mState(STATE_IDLE)
, mAutoStopAfterFinish(TRUE)
, mProgressIntervalMS(0)
{
}

MprFromFile::~MprFromFile()
{
   if(mpFileBuffer) delete mpFileBuffer;
}

/* ============================ MANIPULATORS ============================== */

OsStatus MprFromFile::playBuffer(const UtlString& namedResource, OsMsgQ& fgQ, 
                                 const char* audioBuffer, unsigned long bufSize, 
                                 uint32_t inRate, uint32_t fgRate, int type, 
                                 UtlBoolean repeat, OsProtectedEvent* notify,
                                 UtlBoolean autoStopAfterFinish)
{
   UtlString* fgAudBuffer = NULL;
   OsStatus stat = 
      genericAudioBufToFGAudioBuf(fgAudBuffer, audioBuffer, bufSize, inRate, fgRate, type);

   // Tell CpCall that we've copied the data out of the buffer, so it
   // can continue processing.
   if (notify && OS_ALREADY_SIGNALED == notify->signal(0))
   {
      OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
      eventMgr->release(notify);
   }

   if(stat == OS_SUCCESS)
   {
      MpPackedResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_FROMFILE_START,
                              namedResource);
      UtlSerialized &msgData = msg.getData();
      stat = msgData.serialize(fgAudBuffer);
      assert(stat == OS_SUCCESS);
      stat = msgData.serialize(repeat);
      assert(stat == OS_SUCCESS);
      stat = msgData.serialize(autoStopAfterFinish);
      assert(stat == OS_SUCCESS);
      msgData.finishSerialize();
      stat = fgQ.send(msg, sOperationQueueTimeout);
   }
   else
   {
      MpResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_FROMFILE_ERROR,
                        namedResource);
      stat = fgQ.send(msg, sOperationQueueTimeout);
   }
   return stat;
}

OsStatus MprFromFile::playFile(const UtlString& namedResource, 
                               OsMsgQ& fgQ, 
                               uint32_t fgSampleRate,
                               const UtlString& filename, 
                               const UtlBoolean& repeat,
                               UtlBoolean autoStopAfterFinish)
{
   UtlString* audioBuffer = NULL;
   OsStatus stat = readAudioFile(fgSampleRate, audioBuffer, filename);
   if(stat == OS_SUCCESS)
   {
      MpPackedResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_FROMFILE_START,
                              namedResource);
      UtlSerialized &msgData = msg.getData();
      stat = msgData.serialize(audioBuffer);
      assert(stat == OS_SUCCESS);
      stat = msgData.serialize(repeat);
      assert(stat == OS_SUCCESS);
      stat = msgData.serialize(autoStopAfterFinish);
      assert(stat == OS_SUCCESS);
      msgData.finishSerialize();
      stat = fgQ.send(msg, sOperationQueueTimeout);
   }
   else
   {
      MpResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_FROMFILE_ERROR,
                        namedResource);
      stat = fgQ.send(msg, sOperationQueueTimeout);
   }
   return stat;
}

OsStatus MprFromFile::stopFile(const UtlString& namedResource, 
                               OsMsgQ& fgQ)
{
   MpResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_FROMFILE_STOP,
                     namedResource);
   return fgQ.send(msg, sOperationQueueTimeout);
}

OsStatus MprFromFile::pauseFile(const UtlString& namedResource, 
                                OsMsgQ& fgQ)
{
   MpResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_FROMFILE_PAUSE,
                     namedResource);
   return fgQ.send(msg, sOperationQueueTimeout);
}

OsStatus MprFromFile::resumeFile(const UtlString& namedResource,
                                 OsMsgQ& fgQ)
{
   MpResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_FROMFILE_RESUME,
                     namedResource);
   return fgQ.send(msg, sOperationQueueTimeout);
}

OsStatus MprFromFile::sendProgressPeriod(const UtlString& namedResource, 
                                         OsMsgQ& fgQ, 
                                         int32_t updatePeriodMS)
{
   MpPackedResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_FROMFILE_START,
                           namedResource);
   UtlSerialized &msgData = msg.getData();
   OsStatus stat = msgData.serialize(updatePeriodMS);
   assert(stat == OS_SUCCESS);
   msgData.finishSerialize();
   return fgQ.send(msg, sOperationQueueTimeout);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

OsStatus MprFromFile::genericAudioBufToFGAudioBuf(UtlString*& fgAudioBuf, 
                                                  const char* audioBuffer, 
                                                  unsigned long bufSize, 
                                                  uint32_t inRate,
                                                  uint32_t fgRate,
                                                  int type)
{
   OsStatus stat = OS_SUCCESS;
   char* convertedBuffer = NULL;

   assert(inRate > 0); // Assert that sample rates are not zero.
   assert(fgRate > 0);
   assert(fgAudioBuf == NULL); // assume UtlString buffer pointer is null.
   fgAudioBuf = new UtlString();

   if (fgAudioBuf)
   {
      switch(type)
      {
      case 0 : 
         {
            char* resampledBuf = NULL;
            uint32_t resampledBufSize = 0;
            if(allocateAndResample(audioBuffer, bufSize, inRate, 
                  resampledBuf, resampledBufSize, fgRate)
               == FALSE)
            {
               delete fgAudioBuf;
               return OS_FAILED;
            }
            fgAudioBuf->append(resampledBuf,resampledBufSize);
            break;
         }

      case 1 : convertedBuffer = new char[bufSize*2];
         //NOTE by Keith Kyzivat - this code was pulled directly from
         // the old implementation of playBuffer... obviously broken here.
         //TODO: actually convert the buffer
         fgAudioBuf->append(convertedBuffer,bufSize);
         delete[] convertedBuffer; 
         break;
      }
   }
   else
   {
      stat = OS_INVALID_ARGUMENT;
   }

   return stat;
}

// WARNING: This allocates a buffer for the whole file -- thus,
//          files read in should not be huge.  This does occur
//          outside of the media task, so the time it takes to 
//          resample should not incur any latency, but memory 
//          utilization with large files may become an issue.
// TODO: Replace this whole file reading code with some that 
//       happens in a separate file-reading thread that can 
//       happen while mediaTask is going on, so files of 
//       extremely large length can be used.
OsStatus MprFromFile::readAudioFile(uint32_t fgSampleRate,
                                    UtlString*& audioBuffer,
                                    const char* audioFileName)
{
   char* charBuffer = NULL;
   FILE* audioFilePtr = NULL;
   int iTotalChannels = 1;
   uint32_t filesize;
   uint32_t trueFilesize;
   int samplesReaded;
   int compressionType = 0;
   int channelsMin = 1, channelsMax = 2, channelsPreferred = 0;
   long rateMin = 8000, rateMax = 44100, ratePreferred = 22050;
   UtlBoolean bDetectedFormatIsOk = TRUE;
   MpAudioAbstract *audioFile = NULL;
   OsStatus result = OS_SUCCESS;

   // Assume audioBuffer passed in is NULL..
   assert(audioBuffer == NULL);
   audioBuffer = NULL;

   // Create a temporary pointer for resampled data, and var for size.
   char* resampledBuf = NULL;
   uint32_t resampledBufSz = 0;


   if (!audioFileName)
      return OS_INVALID_ARGUMENT;

// Stub out on Android temporarily as it does not support ifstream
#ifdef ANDROID
   return OS_FILE_NOT_FOUND;
#else

   ifstream inputFile(audioFileName,ios::in|ios::binary);

   if (!inputFile.good())
   {
      return OS_FILE_NOT_FOUND;
   }

   //get file size
   inputFile.seekg(0, ios::end);
   filesize = trueFilesize = inputFile.tellg();
   inputFile.seekg(0);

   //we have to have at least one sample to play
   if (trueFilesize < sizeof(AudioSample))  
   {
      osPrintf("WARNING: %s contains less than one sample to play. "
         "Skipping play.\n", audioFileName);
      return OS_INVALID_ARGUMENT;
   }

   if (trueFilesize > MAXFILESIZE)
   {
      osPrintf("playFile('%s') WARNING:\n"
         "    length (%u) exceeds size limit (%d)\n",
         audioFileName, trueFilesize, MAXFILESIZE);
      filesize = MAXFILESIZE;
   }

   if (trueFilesize < MINFILESIZE)
   {
      osPrintf("playFile('%s') WARNING:\n"
         "    length (%u) is suspiciously short!\n",
         audioFileName, trueFilesize);
   }


   audioFile = MpOpenFormat(inputFile);
   //if we have an audioFile object, then it must be a known file type
   //otherwise, lets treat it as RAW
   if (audioFile)
   {
      if (audioFile->isOk())
      {
         audioFile->minMaxChannels(&channelsMin,
            &channelsMax, &channelsPreferred);

         if (channelsMin > channelsMax) 
         {
            osPrintf("Couldn't negotiate channels.\n");
            bDetectedFormatIsOk = FALSE;
         }

         audioFile->minMaxSamplingRate(&rateMin,&rateMax,&ratePreferred);
         if (rateMin > rateMax) 
         {
            osPrintf("Couldn't negotiate rate.\n");
            bDetectedFormatIsOk = FALSE;
         }
      }
      else
         bDetectedFormatIsOk = FALSE;

      if (bDetectedFormatIsOk)
      {
         iTotalChannels = channelsPreferred;
         compressionType = audioFile->getDecompressionType();
      }
      else
      {
         osPrintf("\nERROR: Could not detect format correctly. "
            "Should be AU or WAV or RAW\n");
      }

      // First, figure out which kind of file it is
      if (bDetectedFormatIsOk && 
         audioFile->getAudioFormat() == AUDIO_FORMAT_WAV)
      {

         // Get actual data size without header.
         filesize = audioFile->getBytesSize();

         switch(compressionType) 
         {
         case MpAudioWaveFileRead::DePcm8Unsigned: //8
            // We'll convert it to 16 bit
            filesize *= sizeof(AudioSample);
            charBuffer = (char*)malloc(filesize);
            samplesReaded = audioFile->getSamples((AudioSample*)charBuffer,
                                                  filesize);

            if (samplesReaded) 
            {
               assert(samplesReaded*sizeof(AudioSample) == filesize);

               // Convert to mono if needed
               if (channelsPreferred > 1)
                  filesize = mergeChannels(charBuffer, filesize, iTotalChannels);

               // resampledBuf will point to a buffer holding the resampled
               // data and resampledBufSz will hold the new buffer size
               // after this call.
               if(allocateAndResample(charBuffer, filesize, ratePreferred, 
                     resampledBuf, resampledBufSz, fgSampleRate) == FALSE)
               {
                  result = OS_FAILED;
                  break;
               }
               else
               {
                  // We want resampledBuf to replace charBuffer, so we'll free
                  // charBuffer, and store resampledBuf to it. (updating size too)
                  free(charBuffer);
                  charBuffer = resampledBuf;
                  filesize = resampledBufSz;
               }
            }
            else
            {
               result = OS_FAILED;
            }
            break;

         case MpAudioWaveFileRead::DePcm16LsbSigned: // 16
            charBuffer = (char*)malloc(filesize);
            samplesReaded = audioFile->getSamples((AudioSample*)charBuffer,
                                                  filesize/sizeof(AudioSample));
            if (samplesReaded)
            {
               assert(samplesReaded*sizeof(AudioSample) == filesize);

               // Convert to mono if needed
               if (iTotalChannels > 1)
                  filesize = mergeChannels(charBuffer, filesize, iTotalChannels);

               // resampledBuf will point to a buffer holding the resampled
               // data and resampledBufSz will hold the new buffer size
               // after this call.
               if(allocateAndResample(charBuffer, filesize, ratePreferred, 
                     resampledBuf, resampledBufSz, fgSampleRate) == FALSE)
               {
                  result = OS_FAILED;
                  break;
               }
               else
               {
                  // We want resampledBuf to replace charBuffer, so we'll free
                  // charBuffer, and store resampledBuf to it. (updating size too)
                  free(charBuffer);
                  charBuffer = resampledBuf;
                  filesize = resampledBufSz;
               }
            }
            else
            {
               result = OS_FAILED;
            }
            break;
         }
      }
      else
      {
         if (bDetectedFormatIsOk && 
            audioFile->getAudioFormat() == AUDIO_FORMAT_AU)
         {

            // Get actual data size without header.
            filesize = audioFile->getBytesSize();

            switch(compressionType)
            {
            case MpAuRead::DePcm8Unsigned:
               break; //do nothing for this format

            case MpAuRead::DeG711MuLaw:
               charBuffer = (char*)malloc(filesize*2);
               samplesReaded = audioFile->getSamples((AudioSample*)charBuffer, filesize);
               if (samplesReaded) 
               {

                  //it's now 16 bit so it's twice as long
                  filesize *= sizeof(AudioSample);

                  // Convert to mono if needed
                  if (channelsPreferred > 1)
                     filesize = mergeChannels(charBuffer, filesize, iTotalChannels);

                  // resampledBuf will point to a buffer holding the resampled
                  // data and resampledBufSz will hold the new buffer size
                  // after this call.
                  if(allocateAndResample(charBuffer, filesize, ratePreferred, 
                        resampledBuf, resampledBufSz, fgSampleRate) == FALSE)
                  {
                     result = OS_FAILED;
                     break;
                  }
                  else
                  {
                     // We want resampledBuf to replace charBuffer, so we'll free
                     // charBuffer, and store resampledBuf to it. (updating size too)
                     free(charBuffer);
                     charBuffer = resampledBuf;
                     filesize = resampledBufSz;
                  }
               }
               else
               {
                  result = OS_FAILED;
               }
               break;

            case MpAuRead::DePcm16MsbSigned:
               charBuffer = (char*)malloc(filesize);
               samplesReaded = audioFile->getSamples((AudioSample*)charBuffer,
                                                     filesize/sizeof(AudioSample));
               if (samplesReaded) 
               {
                  assert(samplesReaded*sizeof(AudioSample) == filesize);

                  // Convert to mono if needed
                  if (channelsPreferred > 1)
                     filesize = mergeChannels(charBuffer, filesize, iTotalChannels);

                  // resampledBuf will point to a buffer holding the resampled
                  // data and resampledBufSz will hold the new buffer size
                  // after this call.
                  if(allocateAndResample(charBuffer, filesize, ratePreferred, 
                        resampledBuf, resampledBufSz, fgSampleRate) == FALSE)
                  {
                     result = OS_FAILED;
                     break;
                  }
                  else
                  {
                     // We want resampledBuf to replace charBuffer, so we'll free
                     // charBuffer, and store resampledBuf to it. (updating size too)
                     free(charBuffer);
                     charBuffer = resampledBuf;
                     filesize = resampledBufSz;
                  }
               }
               else
               {
                  result = OS_FAILED;
               }
               break;
            }
         } 
         else 
         {
            OsSysLog::add(FAC_MP, PRI_ERR, 
               "ERROR: Detected audio file is bad.  "
               "Must be MONO, 16bit signed wav or u-law au");
         }
      }

      //remove object used to determine rate, compression, etc.
      delete audioFile;
      audioFile = NULL;
   }
   else
   {
#if 0
      osPrintf("AudioFile: raw file\n");
#endif

      // if we cannot determine the format of the audio file,
      // and if the ext of the file is .ulaw, we assume it is a no-header
      // raw file of ulaw audio, 8 bit, 8kHZ.
      if (strstr(audioFileName, ".ulaw"))
      {
         ratePreferred = 8000;
         channelsPreferred = 1;
         audioFile = new MpAuRead(inputFile, 1);
         if (audioFile)
         {
            filesize *= sizeof(AudioSample);
            charBuffer = (char*)malloc(filesize);

            samplesReaded = audioFile->getSamples((AudioSample*)charBuffer,
                                                  filesize/sizeof(AudioSample));
            if (!samplesReaded) 
            {
               result = OS_FAILED;
            }
         }
      }
      else // the file extension is not .ulaw ... 
      {
         if (0 != (audioFilePtr = fopen(audioFileName, "rb")))
         {
            unsigned int cbIdx = 0;
            int bytesRead = 0;
            // Extra slop in case the real filesize changed since we checked its size
            charBuffer = (char*)malloc(filesize + sFromFileReadBufferSize);
            assert(charBuffer != NULL); // Assume malloc succeeds.

            // Read in the unknown audio file a chunk at a time.
            // (specified by sFromFileReadBufferSize)
            while((cbIdx < filesize) &&
               ((bytesRead = fread(charBuffer+cbIdx, 1, 
               sFromFileReadBufferSize, 
               audioFilePtr)) > 0))
            {
               cbIdx += bytesRead;
            }

            // Now that we're done with the unknown raw audio file
            // close it up.
            fclose(audioFilePtr);
         }
      }
   }

   // Now we copy over the char buffer data to UtlString for use in
   // messages.
   if(charBuffer != NULL)
   {
      audioBuffer = new UtlString();
      if (audioBuffer)
      {
         audioBuffer->append(charBuffer, filesize);
#if 0
         osPrintf("Audio Buffer length: %d\n", audioBuffer->length());
#endif
      }
      free(charBuffer);
   }

   return result;
#endif /* non-ANDROID */
}

UtlBoolean MprFromFile::allocateAndResample(const char* audBuf,
                                            const uint32_t audBufSz,
                                            const uint32_t inRate,
                                            char*& outAudBuf, 
                                            uint32_t& outAudBufSz, 
                                            const uint32_t outRate)
{
   // Malloc up a new chunk of memory for resampling to.
   outAudBufSz = (uint32_t)(((uint64_t)audBufSz * (uint64_t)outRate) / (uint64_t)inRate);
   outAudBuf = (char*)malloc(outAudBufSz);
   if(outAudBuf == NULL)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, 
                    "ERROR: Failed to allocate a buffer to resample to.");
      return FALSE;
   }

   // Check if the rates match -- if so, no need to resample - it's already done!
   if(inRate == outRate)
   {
      // Still have to copy over the audBuf to the newly allocated outAudBuf..
      assert(audBufSz == outAudBufSz);
      memcpy(outAudBuf, audBuf, audBufSz);
      return TRUE;
   }

   uint32_t inSamplesProcessed = 0;
   uint32_t outSamplesWritten = 0;
   OsStatus resampleStat = OS_SUCCESS;
   MpResamplerBase* pResampler = MpResamplerBase::createResampler(1, inRate, outRate);
   if (!pResampler)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "MprFromFile::allocateAndResample(): Can't create resampler");
      return FALSE;
   }
   resampleStat = pResampler->resample(0, (const MpAudioSample*)audBuf, audBufSz/sizeof(MpAudioSample), 
                                       inSamplesProcessed, 
                                       (MpAudioSample*)outAudBuf, outAudBufSz/sizeof(MpAudioSample), 
                                       outSamplesWritten);
   delete pResampler;
   if(resampleStat != OS_SUCCESS)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "ERROR: Resampler failed with status: %d", 
                    resampleStat);
      return FALSE;
   }

   return TRUE;
}

UtlBoolean MprFromFile::doProcessFrame(MpBufPtr inBufs[],
                                       MpBufPtr outBufs[],
                                       int inBufsSize,
                                       int outBufsSize,
                                       UtlBoolean isEnabled,
                                       int samplesPerFrame,
                                       int samplesPerSecond)
{
   MpAudioBufPtr out;
   MpAudioSample *outbuf;
   int count;
   int bytesLeft;

   // There's nothing to do if the output buffers or the number
   // of samples per frame are zero, so just return.
   if (outBufsSize == 0 || samplesPerFrame == 0)
   {
       return FALSE;
   }

   // If we're enabled and not paused, then do playback,
   // otherwise pass through.
   if (isEnabled && mState == STATE_PLAYING)
   {
      if (mpFileBuffer)
      {
         // Get new buffer
         out = MpMisc.RawAudioPool->getBuffer();
         if (!out.isValid())
         {
            return FALSE;
         }
         out->setSpeechType(MP_SPEECH_TONE);
         out->setSamplesNumber(samplesPerFrame);
         count = out->getSamplesNumber();
         outbuf = out->getSamplesWritePtr();

         int bytesPerFrame = count * sizeof(MpAudioSample);
         int bufferLength = mpFileBuffer->length();
         int totalBytesRead = 0;

         if(mFileBufferIndex < bufferLength)
         {
            totalBytesRead = bufferLength - mFileBufferIndex;
            totalBytesRead = sipx_min(totalBytesRead, bytesPerFrame);
            memcpy(outbuf, &(mpFileBuffer->data()[mFileBufferIndex]),
                   totalBytesRead);
            mFileBufferIndex += totalBytesRead;
         }

         if (mFileRepeat) 
         {
            bytesLeft = 1;
            while((totalBytesRead < bytesPerFrame) && (bytesLeft > 0))
            {
               mFileBufferIndex = 0;
               bytesLeft = sipx_min(bufferLength - mFileBufferIndex,
                               bytesPerFrame - totalBytesRead);
               memcpy(&outbuf[(totalBytesRead/sizeof(MpAudioSample))],
                      &(mpFileBuffer->data()[mFileBufferIndex]), bytesLeft);
               totalBytesRead += bytesLeft;
               mFileBufferIndex += bytesLeft;
            }
         } else 
         {
            if (mFileBufferIndex >= bufferLength) 
            {
               // We're done playing..
               // zero out the remaining bytes in the frame after the end
               // of the real data before sending it off - it could be garbage.
               bytesLeft = bytesPerFrame - totalBytesRead;
               memset(&outbuf[(totalBytesRead/sizeof(MpAudioSample))], 0, bytesLeft);

               // Set state and emit signals.
               handleFinish();
            }
         }

         // Check to see if we need to make a progress update.
         OsTime curTime;
         OsDateTime::getCurTime(curTime);
         // Info about clock rollover here -- subtraction of OsTimes -- this will
         // not have issue with rollover -- issues with rollover would happen in 2038 
         // (2^31 seconds)
         // Comparison rollover -- cvtToMsecs() is a long - 2^31 msecs - 24.85 days
         // So, given that specification of interval is with int32_t, that means
         // interval will never be > than this rollover, so no need to worry,
         // and we can assume doProcessFrame will be executed in a timely fashion.
         if(mProgressIntervalMS > 0 &&
            (mLastProgressUpdate - curTime).cvtToMsecs() >= mProgressIntervalMS)
         {
            // We get here if there *is* a progress interval, and if the # ms
            // passed since the last progress update is >= the progress interval.
            // In this case, we need to send out a progress notification message.
            unsigned amountPlayedMS = 
               mFileBufferIndex / sizeof(MpAudioSample) / samplesPerSecond;
            unsigned totalBufferMS = 
               mpFileBuffer->length() / sizeof(MpAudioSample) / samplesPerSecond;

            MprnProgressMsg progressMsg(MpResNotificationMsg::MPRNM_FROMFILE_PROGRESS,
                                        getName(), amountPlayedMS, totalBufferMS);
            sendNotification(progressMsg);
         }
      }
   }
   else
   {
      // Resource is disabled. Passthrough input data
      out.swap(inBufs[0]);
   }

   // Push audio data downstream
   outBufs[0] = out;

   return TRUE;
}

// This is used in both old and new messaging schemes to initialize everything
// and start playing a buffer, when a play is requested.
UtlBoolean MprFromFile::handlePlay(UtlString* pBuffer, UtlBoolean repeat,
                                   UtlBoolean autoStopAfterFinish)
{
   // Stop previous playback if still playing it.
   if (mState != STATE_IDLE)
   {
      handleStop();
   }
   // We must be in STATE_IDLE at this point.
   assert(mState == STATE_IDLE);

   if (mpFileBuffer)
   {
      delete mpFileBuffer;
   }
   mpFileBuffer = pBuffer;
   if (mpFileBuffer) 
   {
      mFileBufferIndex = 0;
      mFileRepeat = repeat;
   }
   mAutoStopAfterFinish = autoStopAfterFinish;
   mState = STATE_PLAYING;

   // Notify about our startup.
   sendNotification(MpResNotificationMsg::MPRNM_FROMFILE_STARTED);

   return TRUE;
}

UtlBoolean MprFromFile::handleFinish()
{
   // Finish only if playing or paused
   if (mState == STATE_PLAYING || mState == STATE_PAUSED)
   {
      // Send a notification.
      sendNotification(MpResNotificationMsg::MPRNM_FROMFILE_FINISHED);

      // Cleanup.
      if (mpFileBuffer)
      {
         delete mpFileBuffer;
         mpFileBuffer = NULL;
         mFileBufferIndex = 0;
      }

      // Set state.
      mState = STATE_FINISHED;

      // Perform auto-stop if requested
      if (mAutoStopAfterFinish)
      {
         handleStop();
      }
   }
   else
   {
      assert(!"MprFromFile::handleFinish() called when FromFile is in wrong state");
   }

   return TRUE;
}

UtlBoolean MprFromFile::handleStop()
{
   // Stop only if not idle.
   if (mState != STATE_IDLE)
   {
      // Cleanup if not done yet.
      if (mpFileBuffer)
      {
         delete mpFileBuffer;
         mpFileBuffer = NULL;
         mFileBufferIndex = 0;
      }

      // Set state.
      mState = STATE_IDLE;
   }

   // Send notification even if playback is already stopped to notify caller
   // that request has been processed.
   sendNotification(MpResNotificationMsg::MPRNM_FROMFILE_STOPPED);

   return TRUE;
}

UtlBoolean MprFromFile::handlePause()
{
   if (mState == STATE_PLAYING || mState == STATE_PAUSED)
   {
      // Success if playing or already paused
      mState = STATE_PAUSED;
      sendNotification(MpResNotificationMsg::MPRNM_FROMFILE_PAUSED);
   }
   else // (STATE_IDLE or STATE_FINISHED)
   {
      // Failure if stopped.
      sendNotification(MpResNotificationMsg::MPRNM_FROMFILE_ERROR);
   }
   return TRUE;
}

UtlBoolean MprFromFile::handleResume()
{
   if(mState == STATE_PAUSED || mState == STATE_PLAYING)
   {
      // Success if paused or already playing
      mState = STATE_PLAYING;
      sendNotification(MpResNotificationMsg::MPRNM_FROMFILE_RESUMED);
   }
   else // (STATE_IDLE or STATE_FINISHED)
   {
      // Failure if nothing to resume.
      sendNotification(MpResNotificationMsg::MPRNM_FROMFILE_ERROR);
   }
   return TRUE;
}

UtlBoolean MprFromFile::handleSetUpdatePeriod(int32_t periodMS)
{
   // Set the 'last progress update' to now, since there wasn't one before.
   OsDateTime::getCurTime(mLastProgressUpdate);
   // and set the period to the new one provided.
   mProgressIntervalMS = periodMS;
   return TRUE;
}

// New resource message handling.  This is part of the new
// messaging infrastructure (2007).
UtlBoolean MprFromFile::handleMessage(MpResourceMsg& rMsg)
{
   UtlBoolean msgHandled = FALSE;

   switch (rMsg.getMsg()) 
   {
   case MPRM_FROMFILE_START:
      {
         OsStatus stat;
         UtlString *pAudioBuffer;
         UtlBoolean isRepeating;
         UtlBoolean autoStopAfterFinish;

         UtlSerialized &msgData = ((MpPackedResourceMsg*)(&rMsg))->getData();
         stat = msgData.deserialize((void*&)pAudioBuffer);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize(isRepeating);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize(autoStopAfterFinish);
         assert(stat == OS_SUCCESS);

         msgHandled = handlePlay(pAudioBuffer, isRepeating, autoStopAfterFinish);
      }
      break;

   case MPRM_FROMFILE_STOP:
      msgHandled = handleStop();
      break;

   case MPRM_FROMFILE_PAUSE:
      msgHandled = handlePause();
      break;

   case MPRM_FROMFILE_RESUME:
      msgHandled = handleResume();
      break;

   case MPRM_FROMFILE_SEND_PROGRESS:
      {
         OsStatus stat;
         int32_t updatePeriodMS;

         UtlSerialized &msgData = ((MpPackedResourceMsg*)(&rMsg))->getData();
         // Explicit cast to char& is hack to fix for broken VS6 compiler.
         stat = msgData.deserialize((char&)updatePeriodMS);
         assert(stat == OS_SUCCESS);

         msgHandled = handleSetUpdatePeriod(updatePeriodMS);
      }
      break;      

   case MPRM_FROMFILE_ERROR:
      sendNotification(MpResNotificationMsg::MPRNM_FROMFILE_ERROR);
      msgHandled = TRUE;
      break;

   default:
      // If we don't handle the message here, let our parent try.
      msgHandled = MpResource::handleMessage(rMsg); 
      break;
   }
   return msgHandled;
}

/* ============================ FUNCTIONS ================================= */


