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
#include "mp/MpFlowGraphMsg.h" // For notifying CallFlowgraph about enable/disable.
#include "mp/MprRecorder.h"
#include "mp/MpPackedResourceMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprRecorder::MprRecorder(const UtlString& rName)
:  MpAudioResource(rName, 1, 1, 0, 1),
   mFileDescriptor(-1),
   mRecFormat(UNINITIALIZED_FORMAT),
   mTotalBytesWritten(0),
   mTotalSamplesWritten(0),
   mConsecutiveInactive(0),
   mSilenceLength(0),
   mpEvent(NULL),
   mFramesToRecord(0),
   mState(STATE_IDLE)
{
}

// Destructor
MprRecorder::~MprRecorder()
{
   if (mFileDescriptor != -1)
   {
       // If when we get to the destructor and our file descriptor is not set to -1
       // then close it now.

       if (mRecFormat == WAV_PCM_16)
       {
          updateWaveHeaderLengths(mFileDescriptor);
       }
       close(mFileDescriptor);
   }
}

/* ============================ MANIPULATORS ============================== */

OsStatus MprRecorder::start(const UtlString& namedResource,
                            OsMsgQ& fgQ,
                            const char *filename,
                            RecordFileFormat recFormat,
                            int time,
                            int silenceLength,
                            OsEvent* event)
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
      MpPackedResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_START,
                              namedResource);
      UtlSerialized &msgData(msg.getData());

      stat = msgData.serialize(file);
      assert(stat == OS_SUCCESS);
      stat = msgData.serialize((int)recFormat);
      assert(stat == OS_SUCCESS);
      stat = msgData.serialize(time);
      assert(stat == OS_SUCCESS);
      stat = msgData.serialize(silenceLength);
      assert(stat == OS_SUCCESS);
      stat = msgData.serialize((void*)event);
      assert(stat == OS_SUCCESS);
      msgData.finishSerialize();
      
      res = fgQ.send(msg, sOperationQueueTimeout);
   }
   else
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "MprRecorder::setup() failed to open file %s, error code is %i",
                    filename, errno);
   }

   return res;
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

   if (mFileDescriptor < 0)
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG,
                    "MprRecorder::doProcessFrame to finish recording because mFileDescriptor=%d < 0",
                    mFileDescriptor);
      finish(FINISHED_ERROR);

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
      // Silence. Write zeros.

      MpAudioSample silent=0;
      for (int i=0; i<samplesPerFrame; i++)
      {
         int bytesWritten = write(mFileDescriptor, (char *)&silent, sizeof(silent));
         if (bytesWritten != sizeof(silent))
         {
            finish(FINISHED_ERROR);
         }
         else
         {
            mTotalBytesWritten += sizeof(silent);
            mTotalSamplesWritten++;
         }
      }
      mConsecutiveInactive++;
   }
   else
   {
      // Active voice. Write it.

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
      int numBytes = numSamples * sizeof(MpAudioSample);
      int bytesWritten = write(mFileDescriptor, (char *)input, numBytes);

      if (bytesWritten != numBytes)
      {
         finish(FINISHED_ERROR);
      }
      else
      {
         mTotalBytesWritten += numBytes;
         mTotalSamplesWritten += samplesPerFrame;
      }

      // Push data further downstream
      outBufs[0].swap(in);
   }

   return TRUE;
}

// Handle messages for this resource.
UtlBoolean MprRecorder::handleStart(int file,
                                    RecordFileFormat recFormat,
                                    int timeMS,
                                    int silenceLength,
                                    OsEvent* event)
{
   int iMsPerFrame =
      (1000 * mpFlowGraph->getSamplesPerFrame()) / mpFlowGraph->getSamplesPerSec();
   if (timeMS > 0)
   {
      // Convert to number of frames
      mFramesToRecord = timeMS / iMsPerFrame;
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

   mFileDescriptor = file;
   mpEvent = event;
   mTotalBytesWritten = 0;
   mTotalSamplesWritten = 0;
   mConsecutiveInactive = 0;
   mState = STATE_RECORDING;
   mRecFormat = recFormat;

   if (mRecFormat == MprRecorder::WAV_PCM_16)
   {
      writeWAVHeader(file, mpFlowGraph->getSamplesPerSec());
   }

   handleEnable();

   sendNotification(MpResNotificationMsg::MPRNM_RECORDER_STARTED);

   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MprRecorder::handleStart(%d, %d, %p) finished #frames=%d",
                 file, timeMS, event, mFramesToRecord);
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
   case MPRM_START:
      {
         OsStatus stat;
         int file;
         RecordFileFormat recFormat;
         int timeMS;
         int silenceLength;
         OsEvent* pEvent;

         UtlSerialized &msgData(((MpPackedResourceMsg*)(&rMsg))->getData());
         stat = msgData.deserialize(file);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize((int&)recFormat);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize(timeMS);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize(silenceLength);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize((void*&)pEvent);
         assert(stat == OS_SUCCESS);
         return handleStart(file, recFormat, timeMS, silenceLength, pEvent);
      }
      break;

   case MPRM_STOP:
      return handleStop();
      break;
   }
   return MpAudioResource::handleMessage(rMsg);
}

UtlBoolean MprRecorder::finish(FinishCause cause)
{
   UtlBoolean res = FALSE;

   // Update state.
   mState = STATE_IDLE;

   // Update WAV-header and close file.
   if (mFileDescriptor > -1)
   {
      if (mRecFormat == WAV_PCM_16)
      {
         updateWaveHeaderLengths(mFileDescriptor);
      }
      close(mFileDescriptor);
      mFileDescriptor = -1;
   }

   // New style notification.
   switch (cause)
   {
   case FINISHED_AUTO:
      sendNotification(MpResNotificationMsg::MPRNM_RECORDER_FINISHED);
      break;
   case FINISHED_MANUAL:
      sendNotification(MpResNotificationMsg::MPRNM_RECORDER_STOPPED);
      break;
   case FINISHED_ERROR:
      sendNotification(MpResNotificationMsg::MPRNM_RECORDER_ERROR);
      break;
   }

   // Old style notification (for MpCallFlowGraph::ezRecord() only!)
   if (mpEvent != NULL)
   {
      OsStatus ret = mpEvent->signal(mTotalSamplesWritten);
      if (OS_SUCCESS != ret)
      {
         OsSysLog::add(FAC_MP, PRI_WARNING,
                       "MprRecorder::progressReport signal failed, returned %d",
                       (int)ret);
      }
      mpEvent = NULL;
   }

   return res;
}

UtlBoolean MprRecorder::writeWAVHeader(int handle, unsigned long samplesPerSecond)
{
   UtlBoolean retCode = FALSE;
   char tmpbuf[80];
   short sampleSize = sizeof(MpAudioSample);
   short bitsPerSample = sampleSize*8;

   short compressionCode = 1; //PCM
   short numChannels = 1;
   unsigned long averageSamplePerSec = samplesPerSecond*sampleSize;
   short blockAlign = sampleSize*numChannels;
   unsigned long bytesWritten = 0;

   //write RIFF & length
   //8 bytes written
   strcpy(tmpbuf,"RIFF");
   unsigned long length = 0;
   bytesWritten += write(handle, tmpbuf, (unsigned)strlen(tmpbuf));
   bytesWritten += write(handle, (char*)&length, sizeof(length)); //filled in on close

   //write WAVE & length
   //8 bytes written
   strcpy(tmpbuf,"WAVE");
   bytesWritten += write(handle, tmpbuf, (unsigned)strlen(tmpbuf));
   //    bytesWritten += write(handle,&length, sizeof(length)); //filled in on close

   //write fmt & length
   //8 bytes written
   strcpy(tmpbuf,"fmt ");
   length = 16;
   bytesWritten += write(handle, tmpbuf, (unsigned)strlen(tmpbuf));
   bytesWritten += write(handle, (char*)&length,sizeof(length)); //filled in on close

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
   length = 0;
   bytesWritten += write(handle, tmpbuf, (unsigned)strlen(tmpbuf));
   bytesWritten += write(handle, (char*)&length, sizeof(length)); //filled in on close

   //total length at this point should be 44 bytes
   if (bytesWritten == 44)
      retCode = TRUE;

   return retCode;
}

UtlBoolean MprRecorder::updateWaveHeaderLengths(int handle)
{
   UtlBoolean retCode = FALSE;

   //find out how many bytes were written so far
   unsigned long length = lseek(handle,0,SEEK_END);

   //now go back to beginning
   lseek(handle,4,SEEK_SET);

   //and update the RIFF length
   unsigned long rifflength = length-8;
   write(handle, (char*)&rifflength,sizeof(length));

   //now seek to the data length
   lseek(handle,40,SEEK_SET);

   //this should be the length of just the data
   unsigned long datalength = length-44;
   write(handle, (char*)&datalength,sizeof(datalength));

   return retCode;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


