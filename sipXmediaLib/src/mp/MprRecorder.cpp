//
// Copyright (C) 2006-2014 SIPez LLC.  All rights reserved.
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
#include <mp/MpTypes.h>
#  include <unistd.h>
#  include <fcntl.h>
#elif defined(WIN32) && !defined(WINCE) /* [ */
#  include <io.h>
#  include <fcntl.h>
#endif /* WIN32 && !WINCE ] */

// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsSysLog.h>
#include <os/OsLock.h>
#include <os/OsTask.h>
#include <mp/MpMisc.h>
#include <mp/MpBuf.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MprRecorder.h>
#include <mp/MpPackedResourceMsg.h>
#include <mp/MprnIntMsg.h>
#include <mp/MpEncoderBase.h>
#include <mp/MpResampler.h>
#include <mp/MpCodecFactory.h>

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
, mpEncoder(NULL)
, mpResampler(NULL)
{
}

// Destructor
MprRecorder::~MprRecorder()
{
   // If when we get to the destructor and our file descriptor is not set to -1
   // then close it now.
   closeFile();

   if(mpEncoder)
   {
       delete mpEncoder;
       mpEncoder = NULL;
   }

   if(mpResampler)
   {
       delete mpResampler;
       mpResampler = NULL;
   }
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

OsStatus MprRecorder::pause(const UtlString& namedResource, OsMsgQ& flowgraphQueue)
{
   MpResourceMsg message((MpResourceMsg::MpResourceMsgType)MPRM_PAUSE, namedResource);
   return(flowgraphQueue.send(message, sOperationQueueTimeout));
}

OsStatus MprRecorder::resume(const UtlString& namedResource, OsMsgQ& flowgraphQueue)
{
   MpResourceMsg message((MpResourceMsg::MpResourceMsgType)MPRM_RESUME, namedResource);
   return(flowgraphQueue.send(message, sOperationQueueTimeout));
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
   unsigned int codecSampleRate = 0;
   unsigned int flowgraphSampleRate = mpFlowGraph->getSamplesPerSec();
   OsStatus status = OS_INVALID_ARGUMENT;

   MpCodecFactory* codecFactory = MpCodecFactory::getMpCodecFactory();

   assert(codecFactory);

   if(mpEncoder)
   {
       delete mpEncoder;
       mpEncoder = NULL;
   }
   if(mpResampler)
   {
       delete mpResampler;
       mpResampler = NULL;
   }

   switch(mRecFormat)
   {
       // Encoder needed
   case MprRecorder::WAV_GSM:
       codecSampleRate = 8000;
       status = codecFactory->createEncoder(MIME_SUBTYPE_GSM_WAVE, 
                                            NULL, // FMTP
                                            codecSampleRate, // GSM 8K
                                            1, // Num channels
                                            111, // Bogus payload as we are writing to file
                                            mpEncoder);
       // Note: we could have a NULL encoder here if the codec plugin is not loaded
       if(mpEncoder == NULL)
       {
           OsSysLog::add(FAC_MP, PRI_ERR,
                         "MprRecorder::handleStartFile failed to load GSM codec.  Perhaps GSM plugin is not loaded?");
           OsSysLog::flush();
       }
       assert(mpEncoder);
       mpEncoder->initEncode();
       assert(mpEncoder->getInfo()->getSampleRate() == codecSampleRate);
       break;

   case MprRecorder::UNINITIALIZED_FORMAT:
       OsSysLog::add(FAC_MP, PRI_ERR,
                     "MprRecorder::handleStartFile unset recording format");
       OsSysLog::flush();
       assert(mRecFormat > MprRecorder::UNINITIALIZED_FORMAT);
       break;

       // Encoder not needed
   case MprRecorder::RAW_PCM_16:
   case MprRecorder::WAV_PCM_16:
       //mEncoder = NULL;
       codecSampleRate = flowgraphSampleRate;
       break;

   default:
       OsSysLog::add(FAC_MP, PRI_ERR,
           "MprRecorder::handleStartFile invalid recording format: %d",
           mRecFormat);
       OsSysLog::flush();
       assert(0);
       break;
   }

   // If the file ecoder needs a different sample rate
   if(codecSampleRate != flowgraphSampleRate)
   {
       mpResampler = MpResamplerBase::createResampler(1, flowgraphSampleRate, codecSampleRate);
   }

   // If we are creating a WAV file, write the header.
   // Otherwise we are writing raw PCM data to file.
   if (mRecFormat != MprRecorder::RAW_PCM_16)
   {
      writeWAVHeader(file, recFormat, codecSampleRate);
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
                 "MprRecorder::handleMessage(%d) id: %s", 
                 rMsg.getMsg(),
                 getName().data());
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

   case MPRM_PAUSE:
      if(mState == STATE_RECORDING)
      {
          mState = STATE_PAUSED;
          MprnIntMsg msg(MpResNotificationMsg::MPRNM_RECORDER_PAUSED,
                         getName(),
                         mSamplesRecorded);
          sendNotification(msg);
          return(TRUE);
      }
      else
      {
          OsSysLog::add(FAC_MP, PRI_ERR,
                  "Attempt to pause MprRecorder(%s) not started (%d)",
                  getName().data(), mState);
          return(TRUE);
      }
      break;

   case MPRM_RESUME:
      if(mState == STATE_PAUSED)
      {
          mState = STATE_RECORDING;
          sendNotification(MpResNotificationMsg::MPRNM_RECORDER_RESUMED);
          return(TRUE);
      }
      else
      {
          OsSysLog::add(FAC_MP, PRI_ERR,
                  "Attempt to resume MprRecorder(%s) not paused (%d)",
                  getName().data(), mState);
          return(TRUE);
      }

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
      // Any WAVE file needs header updates on closing
      if (mRecFormat != RAW_PCM_16)
      {
         updateWaveHeaderLengths(mFileDescriptor, mRecFormat);
      }
      close(mFileDescriptor);
      mFileDescriptor = -1;
   }
}


int MprRecorder::writeFileSilence(int numSamples)
{
    assert(((int)MpMisc.mpFgSilence->getSamplesNumber()) >= numSamples);
    const MpAudioSample* silence = MpMisc.mpFgSilence->getSamplesPtr();
    return(writeFileSpeech(silence, numSamples));
}

int MprRecorder::writeFileSpeech(const MpAudioSample *pBuffer, int numSamples)
{
#ifdef TEST_PRINT
    OsSysLog::add(FAC_MP, PRI_DEBUG,
            "MprRecorder::writeFileSpeech(pBuffer: %p, numSamples: %d)",
            pBuffer, numSamples);
#endif
    const MpAudioSample* resampledBufferPtr = NULL;
    const int localBufferSize = 1500; // No audio codecs exceed an MTU
    MpAudioSample localBuffer[localBufferSize];
    OsStatus status = OS_FAILED;

    // If the resampler exists, we resample
    uint32_t samplesConsumed;
    uint32_t numResampled = 0;
    if(mpResampler)
    {
        resampledBufferPtr = localBuffer;
        status = mpResampler->resample(0, 
                                       pBuffer, 
                                       numSamples, 
                                       samplesConsumed,
                                       localBuffer, 
                                       localBufferSize, 
                                       numResampled);
        if(status != OS_SUCCESS)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "MprRecoder::writeFileSpeech resample returned: %d",
                          status);
        }
        assert(samplesConsumed == (uint32_t) numSamples);
    }

    // No resampler, pass it straight through
    else
    {
        numResampled = numSamples;
        resampledBufferPtr = pBuffer;
    }

    // Some encoders are frame based and do not create a complete
    const MpAudioSample* encodedSamplesPtr = NULL;
    MpAudioSample localEncodeBuffer[localBufferSize]; // Should never get larger after encoding

    // If there is an encoder, encode
    int dataSize = 0;
    int numSamplesEncoded = 0;
    UtlBoolean isEndOfFrame = FALSE;
    UtlBoolean isPacketSilent = FALSE;
    UtlBoolean shouldSetMarker = FALSE;
    if(mpEncoder)
    {
        encodedSamplesPtr = localEncodeBuffer;
        status = mpEncoder->encode(resampledBufferPtr, 
                                   numResampled, 
                                   numSamplesEncoded, 
                                   (unsigned char*)localEncodeBuffer, 
                                   numResampled * sizeof(MpAudioSample), 
                                   dataSize, 
                                   isEndOfFrame,
                                   isPacketSilent,
                                   shouldSetMarker);
        if(status != OS_SUCCESS)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                "MprRecorder::writeFileSpeech encode returned: %d",
                status);
        }
        else
        {
            // The upper lay does not know or care about resampling.
            // So we tell it that we encoded all of the samples passed in.
            if(numSamplesEncoded == (int)numResampled)
            {
                numSamplesEncoded = numSamples;
            }
        }
    }

    //  No encoder, pass it straight through
    else
    {
        dataSize = numResampled * sizeof(MpAudioSample);
        // The upper layer does not know or care about resampling.
        // So we tell it that we encoded all of the samples passed in.
        //numSamplesEncoded = numResampled;
        numSamplesEncoded = numSamples;
        encodedSamplesPtr = resampledBufferPtr;
    }
   
    // Depending upon the encoder framing, there may not always be stuff to write
    if(dataSize)
    {
        int bytesWritten = 
            write(mFileDescriptor, (char *)encodedSamplesPtr, dataSize);

        if(bytesWritten != dataSize)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "MprRecorder::writeFileSpeech wrote %d of %d bytes",
                          bytesWritten, dataSize);
        }
    }

#ifdef TEST_PRINT
    OsSysLog::add(FAC_MP, PRI_DEBUG,
            "MprRecorder::writeFileSpeech returning: %d (numSamplesEncoded)",
            numSamplesEncoded);
#endif
    return(numSamplesEncoded);
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

UtlBoolean MprRecorder::writeWAVHeader(int handle, 
                                       RecordFileFormat format,
                                       uint32_t samplesPerSecond)
{
   UtlBoolean retCode = FALSE;
   char tmpbuf[80];
   int16_t sampleSize = 2; //sizeof(MpAudioSample);
   int16_t bitsPerSample = 0;
   int32_t formatLength = 0;

   int16_t compressionCode = (int16_t) format;
   int16_t numChannels = 1;
   uint32_t averageBytesPerSecond = 0;
   int16_t blockAlign = 0;
   unsigned long bytesWritten = 0;
   uint32_t totalHeaderSize = 0;

   switch(format)
   {
       case MprRecorder::WAV_GSM:
           averageBytesPerSecond = 1625;
           blockAlign = 65;
           bitsPerSample = 0;
           formatLength = 20;
           totalHeaderSize = 60;
           break;

       case MprRecorder::WAV_PCM_16:
           averageBytesPerSecond = samplesPerSecond*sampleSize;
           blockAlign = sampleSize*numChannels;
           bitsPerSample = sampleSize*8;
           formatLength = 16;
           totalHeaderSize = 44;
           break;

       default:
           assert(0);
           break;
   }
  
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
   length = formatLength; // size of the format header
   bytesWritten += write(handle, tmpbuf, (unsigned)strlen(tmpbuf));
   bytesWritten += write(handle, (char*)&length,sizeof(length));

   //now write each piece of the format
   //16 bytes written
   bytesWritten += write(handle, (char*)&compressionCode, sizeof(compressionCode));
   bytesWritten += write(handle, (char*)&numChannels, sizeof(numChannels));
   bytesWritten += write(handle, (char*)&samplesPerSecond, sizeof(samplesPerSecond));
   bytesWritten += write(handle, (char*)&averageBytesPerSecond, sizeof(averageBytesPerSecond));
   bytesWritten += write(handle, (char*)&blockAlign, sizeof(blockAlign));
   bytesWritten += write(handle, (char*)&bitsPerSample, sizeof(bitsPerSample));

   // GSM specific part of fmt header
   if(MprRecorder::WAV_GSM)
   {
       int16_t extraFormat = 320; // magic number
       int16_t extraFormatBytes = sizeof(extraFormat);
       bytesWritten += write(handle, (char*)&extraFormatBytes, sizeof(extraFormatBytes));
       bytesWritten += write(handle, (char*)&extraFormat, sizeof(extraFormat));

       int32_t factNumberOfSamples = 0;
       int32_t factLength = sizeof(factNumberOfSamples);
       bytesWritten += write(handle, "fact", 4);
       bytesWritten += write(handle, (char*)&factLength, sizeof(factLength));
       bytesWritten += write(handle, (char*)&factNumberOfSamples, sizeof(factNumberOfSamples));
   }

   //write data and length
   strcpy(tmpbuf,"data");
   length = 0;  // actual value is filled in on close
   bytesWritten += write(handle, tmpbuf, (unsigned)strlen(tmpbuf));
   bytesWritten += write(handle, (char*)&length, sizeof(length));

   //total length at this point should be 44 or 60 bytes
   if (bytesWritten == totalHeaderSize)
      retCode = TRUE;

   return retCode;
}

UtlBoolean MprRecorder::updateWaveHeaderLengths(int handle, RecordFileFormat format)
{
   UtlBoolean retCode = FALSE;

   //find out how many bytes were written so far
   uint32_t length = lseek(handle,0,SEEK_END);

   //now go back to beginning
   lseek(handle,4,SEEK_SET);

   //and update the RIFF length
   uint32_t rifflength = length-8;
   write(handle, (char*)&rifflength,sizeof(length));


   //this should be the length of just the data
   uint16_t totalWaveHeaderLength = 0;
   switch(format)
   {
       case MprRecorder::WAV_GSM:
           //now seek to the data length
           lseek(handle,56,SEEK_SET);
           totalWaveHeaderLength = 60;
           break;

       case MprRecorder::WAV_PCM_16:
           //now seek to the data length
           lseek(handle,40,SEEK_SET);
           totalWaveHeaderLength = 44;
           break;

       default:
           assert(0);
           break;
   }

   uint32_t datalength = length - totalWaveHeaderLength;
   write(handle, (char*)&datalength,sizeof(datalength));

   return(retCode);
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


