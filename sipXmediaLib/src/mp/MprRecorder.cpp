//
// Copyright (C) 2006-2009 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#ifdef __pingtel_on_posix__
#  include <unistd.h>
#  include <fcntl.h>
#elif defined(WIN32) && !defined(WINCE) /* [ */
#  include <io.h>
#  include <fcntl.h>
#endif /* WIN32 && !WINCE ] */

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsSysLog.h"
#include "os/OsLock.h"
#include "os/OsTask.h"
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MpFlowGraphBase.h"
#include "mp/MprRecorder.h"
#include "mp/MpPackedResourceMsg.h"
#include "mp/MprnIntMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprRecorder::MprRecorder(const UtlString& rName)
: MpAudioResource(rName, 1, 1, 0, 1)
, mState(STATE_IDLE)
, mRecordDestination(TO_UNDEFINED)
, mFramesToRecord(0)
, mSamplesRecorded(0)
, mConsecutiveInactive(0)
, mSilenceLength(0)
, mFileDescriptor(-1)
, mRecFormat(UNINITIALIZED_FORMAT)
, mpBuffer(NULL)
, mBufferSize(0)
{
}

// Destructor
MprRecorder::~MprRecorder()
{
   // If when we get to the destructor and our file descriptor is not set to -1
   // then close it now.
   closeFile();
}

/* ============================ MANIPULATORS ============================== */

OsStatus MprRecorder::startFile(const UtlString& namedResource,
                                OsMsgQ& fgQ,
                                const char *filename,
                                RecordFileFormat recFormat,
                                int time,
                                int silenceLength)
{
   int file = -1;
   OsStatus res = OS_FAILED;

   if (NULL != filename)
   {
      file = open(filename, O_BINARY | O_CREAT | O_RDWR, 0640);
   }

   if (file > -1)
   {
      OsStatus stat;
      MpPackedResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_START_FILE,
                              namedResource);
      UtlSerialized &msgData = msg.getData();

      stat = msgData.serialize(file);
      assert(stat == OS_SUCCESS);
      stat = msgData.serialize((int)recFormat);
      assert(stat == OS_SUCCESS);
      stat = msgData.serialize(time);
      assert(stat == OS_SUCCESS);
      stat = msgData.serialize(silenceLength);
      assert(stat == OS_SUCCESS);
      msgData.finishSerialize();
      
      res = fgQ.send(msg, sOperationQueueTimeout);
   }
   else
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "MprRecorder::startFile() failed to open file %s, error code is %i",
                    filename, errno);
   }

   return res;
}

OsStatus MprRecorder::startBuffer(const UtlString& namedResource,
                                  OsMsgQ& fgQ,
                                  MpAudioSample *pBuffer,
                                  int bufferSize,
                                  int time,
                                  int silenceLength)
{
   int file = -1;
   OsStatus stat;
   MpPackedResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_START_BUFFER,
                           namedResource);
   UtlSerialized &msgData = msg.getData();

   stat = msgData.serialize(pBuffer);
   assert(stat == OS_SUCCESS);
   stat = msgData.serialize(bufferSize);
   assert(stat == OS_SUCCESS);
   stat = msgData.serialize(time);
   assert(stat == OS_SUCCESS);
   stat = msgData.serialize(silenceLength);
   assert(stat == OS_SUCCESS);
   msgData.finishSerialize();

   return fgQ.send(msg, sOperationQueueTimeout);
}

OsStatus MprRecorder::stop(const UtlString& namedResource, OsMsgQ& fgQ)
{
   MpResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_STOP, namedResource);
   return fgQ.send(msg, sOperationQueueTimeout);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean MprRecorder::doProcessFrame(MpBufPtr inBufs[],
                                       MpBufPtr outBufs[],
                                       int inBufsSize,
                                       int outBufsSize,
                                       UtlBoolean isEnabled,
                                       int samplesPerFrame,
                                       int samplesPerSecond)
{
   MpAudioBufPtr in;

   if (inBufsSize == 0 || outBufsSize == 0)
   {
      return FALSE;
   }

   // Take data from the first input
   in.swap(inBufs[0]);

   if (!isEnabled || mState != STATE_RECORDING)
   {
      // Push data further downstream
      outBufs[0].swap(in);
      return TRUE;
   }

   // maximum record time reached or final silence timeout.
   if (  (mFramesToRecord >= 0 && mFramesToRecord-- == 0)
      || (mSilenceLength >= 0 && mConsecutiveInactive >= mSilenceLength))
   {
      OsSysLog::add(FAC_MP, PRI_INFO,
         "MprRecorder::doProcessFrame to finish recording because"
         " mFramesToRecord=%d, mSilenceLength=%d,"
         " mConsecutiveInactive=%d", mFramesToRecord,
         mSilenceLength, mConsecutiveInactive);
      finish(FINISHED_AUTO);

      // Push data further downstream
      outBufs[0].swap(in);
      return TRUE;
   }

   // Now write the buffer out
   if (!in.isValid())
   {
      // Write silence.
      int numRecorded;
      if (mRecordDestination == TO_FILE)
      {
         numRecorded = writeFileSilence(samplesPerFrame);
      }
      else if (mRecordDestination == TO_BUFFER)
      {
         numRecorded = writeBufferSilence(samplesPerFrame);
      }
      mSamplesRecorded += numRecorded;
      mConsecutiveInactive++;
      if (numRecorded != samplesPerFrame)
      {
         finish(FINISHED_ERROR);
      }
   }
   else
   {
      // Write speech data.

      if (isActiveAudio(in->getSpeechType()))
      {
         mConsecutiveInactive = 0;
      }
      else
      {
         mConsecutiveInactive++;
      }

      const MpAudioSample* input = in->getSamplesPtr();
      int numSamples = in->getSamplesNumber();
      int numRecorded;
      if (mRecordDestination == TO_FILE)
      {
         numRecorded = writeFileSpeech(input, numSamples);
      }
      else if (mRecordDestination == TO_BUFFER)
      {
         numRecorded = writeBufferSpeech(input, numSamples);
      }
      mSamplesRecorded += numRecorded;

      if (numRecorded != numSamples)
      {
         finish(FINISHED_ERROR);
      }

      // Push data further downstream
      outBufs[0].swap(in);
   }

   return TRUE;
}

UtlBoolean MprRecorder::handleStartFile(int file,
                                        RecordFileFormat recFormat,
                                        int time,
                                        int silenceLength)
{
   mFileDescriptor = file;
   mRecFormat = recFormat;
   mRecordDestination = TO_FILE;

   if (mRecFormat == MprRecorder::WAV_PCM_16)
   {
      writeWAVHeader(file, mpFlowGraph->getSamplesPerSec());
   }

   startRecording(time, silenceLength);

   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MprRecorder::handleStartFile(%d, %d, %d, %d) finished",
                 file, recFormat, time, silenceLength);
   return TRUE;
}

UtlBoolean MprRecorder::handleStartBuffer(MpAudioSample *pBuffer,
                                          int bufferSize,
                                          int time,
                                          int silenceLength)
{
   mpBuffer = pBuffer;
   mBufferSize = bufferSize;
   mRecordDestination = TO_BUFFER;

   startRecording(time, silenceLength);

   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MprRecorder::handleStartBuffer(%p, %d, %d, %d) finished",
                 pBuffer, bufferSize, time, silenceLength);
   return TRUE;
}

UtlBoolean MprRecorder::handleStop()
{
   finish(FINISHED_MANUAL);
   return TRUE;
}

UtlBoolean MprRecorder::handleDisable()
{
   if (mState == STATE_RECORDING)
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG,
                    "MprRecorder to finish recording because of resource disable.");
      finish(FINISHED_MANUAL);
   }
   MpAudioResource::handleDisable();
   return TRUE;
}

UtlBoolean MprRecorder::handleMessage(MpResourceMsg& rMsg)
{
   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MprRecorder::handleMessage(%d)", rMsg.getMsg());
   switch (rMsg.getMsg())
   {
   case MPRM_START_FILE:
      {
         OsStatus stat;
         int file;
         RecordFileFormat recFormat;
         int timeMS;
         int silenceLength;

         UtlSerialized &msgData = ((MpPackedResourceMsg*)(&rMsg))->getData();
         stat = msgData.deserialize(file);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize((int&)recFormat);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize(timeMS);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize(silenceLength);
         assert(stat == OS_SUCCESS);
         return handleStartFile(file, recFormat, timeMS, silenceLength);
      }
      break;

   case MPRM_START_BUFFER:
      {
         OsStatus stat;
         MpAudioSample *pBuffer;
         int bufferSize;
         int time;
         int silenceLength;

         UtlSerialized &msgData = ((MpPackedResourceMsg*)(&rMsg))->getData();
         stat = msgData.deserialize((void*&)pBuffer);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize(bufferSize);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize(time);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize(silenceLength);
         assert(stat == OS_SUCCESS);
         return handleStartBuffer(pBuffer, bufferSize, time, silenceLength);
      }
      break;

   case MPRM_STOP:
      return handleStop();
      break;
   }
   return MpAudioResource::handleMessage(rMsg);
}

void MprRecorder::startRecording(int time, int silenceLength)
{
   int iMsPerFrame =
      (1000 * mpFlowGraph->getSamplesPerFrame()) / mpFlowGraph->getSamplesPerSec();
   if (time > 0)
   {
      // Convert to number of frames
      mFramesToRecord = time / iMsPerFrame;
   }
   else
   {
      // Do not limit recording length by default.
      mFramesToRecord = -1;
   }

   if (silenceLength > 0)
   {
      // Convert to number of frames
      mSilenceLength = silenceLength / iMsPerFrame;
   }
   else
   {
      mSilenceLength = -1;
   }

   mSamplesRecorded = 0;
   mConsecutiveInactive = 0;
   mState = STATE_RECORDING;

   handleEnable();
   sendNotification(MpResNotificationMsg::MPRNM_RECORDER_STARTED);
}

UtlBoolean MprRecorder::finish(FinishCause cause)
{
   UtlBoolean res = FALSE;

   // Update state.
   mState = STATE_IDLE;

   if (mRecordDestination == TO_FILE)
   {
      // Update WAV-header and close file.
      closeFile();
   }
   else if (mRecordDestination == TO_BUFFER)
   {
      mpBuffer = NULL;
      mBufferSize = 0;
   }
   mRecordDestination = TO_UNDEFINED;

   // New style notification.
   switch (cause)
   {
   case FINISHED_AUTO:
      {
         MprnIntMsg msg(MpResNotificationMsg::MPRNM_RECORDER_FINISHED,
                        getName(),
                        mSamplesRecorded);
         sendNotification(msg);
      }
      break;
   case FINISHED_MANUAL:
      {
         MprnIntMsg msg(MpResNotificationMsg::MPRNM_RECORDER_STOPPED,
                        getName(),
                        mSamplesRecorded);
         sendNotification(msg);
      }
      break;
   case FINISHED_ERROR:
      sendNotification(MpResNotificationMsg::MPRNM_RECORDER_ERROR);
      break;
   }

   return res;
}

void MprRecorder::closeFile()
{
   if (mFileDescriptor > -1)
   {
      if (mRecFormat == WAV_PCM_16)
      {
         updateWaveHeaderLengths(mFileDescriptor);
      }
      close(mFileDescriptor);
      mFileDescriptor = -1;
   }
}


int MprRecorder::writeFileSilence(int numSamples)
{
   MpAudioSample silent=0;
   int i;
   for (i=0; i<numSamples; i++)
   {
      int bytesWritten = write(mFileDescriptor, (char *)&silent, sizeof(silent));
      if (bytesWritten<sizeof(silent))
      {
         // Error occurred. Probably out of space.
         break;
      }
   }

   return i;
}

int MprRecorder::writeFileSpeech(const MpAudioSample *pBuffer, int numSamples)
{
   return write(mFileDescriptor, (char *)pBuffer, numSamples * sizeof(MpAudioSample))
          / sizeof(MpAudioSample);
}

int MprRecorder::writeBufferSilence(int numSamples)
{
   int toWrite = sipx_min(mBufferSize-mSamplesRecorded, numSamples);
   memset(&mpBuffer[mSamplesRecorded], 0, toWrite*sizeof(MpAudioSample));
   return toWrite;
}

int MprRecorder::writeBufferSpeech(const MpAudioSample *pBuffer, int numSamples)
{
   int toWrite = sipx_min(mBufferSize-mSamplesRecorded, numSamples);
   memcpy(&mpBuffer[mSamplesRecorded], pBuffer, toWrite*sizeof(MpAudioSample));
   return toWrite;
}

UtlBoolean MprRecorder::writeWAVHeader(int handle, uint32_t samplesPerSecond)
{
   UtlBoolean retCode = FALSE;
   char tmpbuf[80];
   int16_t sampleSize = 2; //sizeof(MpAudioSample);
   int16_t bitsPerSample = sampleSize*8;

   int16_t compressionCode = 1; //PCM
   int16_t numChannels = 1;
   uint32_t averageSamplePerSec = samplesPerSecond*sampleSize;
   int16_t blockAlign = sampleSize*numChannels;
   unsigned long bytesWritten = 0;

   //write RIFF & length
   //8 bytes written
   strcpy(tmpbuf,"RIFF");
   uint32_t length = 0; // actual value is filled in on close
   bytesWritten += write(handle, tmpbuf, (unsigned)strlen(tmpbuf));
   bytesWritten += write(handle, (char*)&length, sizeof(length));

   //write WAVE & length
   //8 bytes written
   strcpy(tmpbuf,"WAVE");
   bytesWritten += write(handle, tmpbuf, (unsigned)strlen(tmpbuf));

   //write fmt & length
   //8 bytes written
   strcpy(tmpbuf,"fmt ");
   length = 16; // size of the format header
   bytesWritten += write(handle, tmpbuf, (unsigned)strlen(tmpbuf));
   bytesWritten += write(handle, (char*)&length,sizeof(length));

   //now write each piece of the format
   //16 bytes written
   bytesWritten += write(handle, (char*)&compressionCode, sizeof(compressionCode));
   bytesWritten += write(handle, (char*)&numChannels, sizeof(numChannels));
   bytesWritten += write(handle, (char*)&samplesPerSecond, sizeof(samplesPerSecond));
   bytesWritten += write(handle, (char*)&averageSamplePerSec, sizeof(averageSamplePerSec));
   bytesWritten += write(handle, (char*)&blockAlign, sizeof(blockAlign));
   bytesWritten += write(handle, (char*)&bitsPerSample, sizeof(bitsPerSample));


   //write data and length
   strcpy(tmpbuf,"data");
   length = 0;  // actual value is filled in on close
   bytesWritten += write(handle, tmpbuf, (unsigned)strlen(tmpbuf));
   bytesWritten += write(handle, (char*)&length, sizeof(length));

   //total length at this point should be 44 bytes
   if (bytesWritten == 44)
      retCode = TRUE;

   return retCode;
}

UtlBoolean MprRecorder::updateWaveHeaderLengths(int handle)
{
   UtlBoolean retCode = FALSE;

   //find out how many bytes were written so far
   uint32_t length = lseek(handle,0,SEEK_END);

   //now go back to beginning
   lseek(handle,4,SEEK_SET);

   //and update the RIFF length
   uint32_t rifflength = length-8;
   write(handle, (char*)&rifflength,sizeof(length));

   //now seek to the data length
   lseek(handle,40,SEEK_SET);

   //this should be the length of just the data
   uint32_t datalength = length-44;
   write(handle, (char*)&datalength,sizeof(datalength));

   return retCode;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


