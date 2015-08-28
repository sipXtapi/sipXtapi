//
// Copyright (C) 2006-2015 SIPez LLC.  All rights reserved.
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
, mNumFramesProcessed(0)
, mSamplesRecorded(0)
, mConsecutiveInactive(0)
, mSilenceLength(0)
, mFileDescriptor(-1)
, mRecFormat(UNINITIALIZED_FORMAT)
, mpBuffer(NULL)
, mBufferSize(0)
, mpEncoder(NULL)
, mEncodedFrames(0)
, mLastEncodedFrameSize(0)
, mpResampler(NULL)
, mpCircularBuffer(NULL)
, mRecordingBufferNotificationWatermark(0)
, mSamplesPerLastFrame(0)
, mSamplesPerSecond(0)
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

   if (mpCircularBuffer)
       mpCircularBuffer->release();
}

/* ============================ MANIPULATORS ============================== */

OsStatus MprRecorder::startFile(const UtlString& namedResource,
                                OsMsgQ& fgQ,
                                const char *filename,
                                RecordFileFormat recFormat,
                                int time,
                                int silenceLength,
                                UtlBoolean append)
{
    int fileHandle = -1;
    OsStatus result = OS_SUCCESS;

    if (NULL == filename)
    {
       result = OS_FAILED;
       OsSysLog::add(FAC_MP, PRI_ERR,
                     "MprRecorder::startFile null filename");
    }
    else
    {
        fileHandle = open(filename, O_BINARY | O_CREAT | O_RDWR |
                          // If append requested open with append mode
                          (append ? 
                           0 /* O_APPEND */: // cannot use APPEND mode as when we try to
                                             // re-write the wave file header in the beginning
                                             // it ends up appending the writes at the end of
                                             // of the file instead in the beginning where it
                                             // was positioned to via lseek (on Linux anyway)
                           O_TRUNC),
                          0640);

        OsSysLog::add(FAC_MP, PRI_DEBUG,
                "MprRecord::startFile opened file %s descriptor: %d",
                filename, fileHandle);
        if (fileHandle > -1)
        {
            if(append)
            {
              RecordFileFormat waveFileCodec;
              uint16_t fileSamplesPerSecond;
              uint16_t fileChannels;
              // open with append (O_APPEND) by default positions us at the end 
              // of the file.  We must read the header at the begining of the
              // file first.
              lseek(fileHandle, 0, SEEK_SET);
              OsStatus waveHeaderReadStatus = readWaveHeader(fileHandle, 
                                                             waveFileCodec,
                                                             fileSamplesPerSecond,
                                                             fileChannels);

              switch(waveHeaderReadStatus)
              {
              // Valid wave file header
                  case OS_SUCCESS:
                      // same codec, sample rate and channels
                      if(recFormat == waveFileCodec &&
                         // TODO: cannot check this here in static method.
                         // checking needs to occur in handleStartFile
                         //dddd == fileSamplesPerSecond &&
                         1 == fileChannels)
                      {
                          // The record format is the same as that of the file that already exists.
                          // Move to the end of the file where we will start appending
                          lseek(fileHandle, 0, SEEK_END);
                      }

                      // different recording format requested from existing file
                      // error out and close the file
                      else
                      {
                          close(fileHandle);
                          fileHandle = -1;
                          OsSysLog::add(FAC_MP, PRI_ERR, 
                                  "MprRecorder::startFile wave header read from append to file: %s "
                                  "(format: %d samples/sec: %d channels: %d), "
                                  "differs from requested wave record format: %d samples/sec: %d channels: %d",
                                  filename,
                                  waveFileCodec, fileSamplesPerSecond, fileChannels,
                                  recFormat, fileSamplesPerSecond, 1);
                          result = OS_FAILED;
                      }

                      break;

              // Not a valid wave file
                  case OS_INVALID:
                      switch(recFormat)
                      {
                          // raw audio file will not have a wave header.  So
                          // this is ok.  We can continue and append at the end
                          case RAW_PCM_16:
                              lseek(fileHandle, 0, SEEK_END);
                              break;

                          // All other file formats are wave files and if we are appending,
                          // the file must already have a valid wave header.
                          default:
                              close(fileHandle);
                              fileHandle = -1;
                              OsSysLog::add(FAC_MP, PRI_ERR, 
                                      "MprRecorder::startFile invalid wave header read from file: %s, cannot append wave format: %d",
                                      filename,
                                      recFormat);
                              result = OS_FAILED;
                              break;
                      } // end switch(recFormat)
                      break;

              // new/empty file.  Ok that file does not have wave header
                  case OS_FAILED:
                      // need to tell handleStartFile to write a WAVE header as one does
                      // not exist.
                      append = FALSE;
                      break;

              // Unhandled error case, should not get here
                  default:
                      close(fileHandle);
                      fileHandle = -1;
                      OsSysLog::add(FAC_MP, PRI_ERR, 
                              "MprRecorder::startFile wave header read failed: %d",
                              waveHeaderReadStatus);
                      result = OS_FAILED;
                      break;
              } // end switch(waveHeaderReadStatus)
            }

            // File is to be replaced, so position to begining of file
            else
            {
                lseek(fileHandle, 0, SEEK_SET);
            }

        }

        if (fileHandle > -1)
        {
          OsStatus stat;
          MpPackedResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_START_FILE,
                                  namedResource);
          UtlSerialized &msgData = msg.getData();

          stat = msgData.serialize(fileHandle);
          assert(stat == OS_SUCCESS);
          stat = msgData.serialize((int)recFormat);
          assert(stat == OS_SUCCESS);
          stat = msgData.serialize(time);
          assert(stat == OS_SUCCESS);
          stat = msgData.serialize(silenceLength);
          assert(stat == OS_SUCCESS);
          stat = msgData.serialize(append);
          assert(stat == OS_SUCCESS);
          msgData.finishSerialize();
          
          result = fgQ.send(msg, sOperationQueueTimeout);
       }
       else
       {
          OsSysLog::add(FAC_MP, PRI_ERR,
                        "MprRecorder::startFile() failed to open file %s, error code is %i",
                        filename, errno);
       }
   }

   return(result);
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

OsStatus MprRecorder::startCircularBuffer(const UtlString& namedResource,
                                          OsMsgQ& fgQ,
                                          CircularBufferPtr & buffer,
                                          RecordFileFormat recordingFormat,
                                          unsigned long recordingBufferNotificationWatermark)
{
    OsStatus stat;
    MpPackedResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_START_CIRCULAR_BUFFER,
                            namedResource);
    UtlSerialized &msgData = msg.getData();

    stat = msgData.serialize(&buffer);
    assert(stat == OS_SUCCESS);
    stat = msgData.serialize((int)recordingFormat);
    assert(stat == OS_SUCCESS);
    stat = msgData.serialize(recordingBufferNotificationWatermark);
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
   OsSysLog::add(FAC_MP, PRI_DEBUG,
           "MprRecord::stop(%s, %p)",
           namedResource.data(), &fgQ);
   MpResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_STOP, namedResource);
   OsStatus status = fgQ.send(msg, sOperationQueueTimeout);

   // To prevent a race/collision with this stop file and a subsequent startFile,
   // adding a media frame delay to be sure file gets closes.  This is a bit of a 
   // hack.  Could use a flowgraph synchronize, to reduce the delay.  The problem 
   // is most prominent when we close a record file and then immediately reopen it
   // for append.  We could end up with 2 file descriptors for the same file and
   // unpredictable stuff happens with the file seeks and writes.
   OsTask::delay(10);
   return(status);
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

   // Cache the last frame size
   mSamplesPerLastFrame = samplesPerFrame;
   mSamplesPerSecond = samplesPerSecond;

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
      else if (mRecordDestination == TO_CIRCULAR_BUFFER)
      {
          numRecorded = writeCircularBufferSilence(samplesPerFrame);
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
         numRecorded = writeSamples(input, numSamples, &MprRecorder::writeFile);
      }
      else if (mRecordDestination == TO_BUFFER)
      {
         numRecorded = writeBufferSpeech(input, numSamples);
      }
      else if (mRecordDestination == TO_CIRCULAR_BUFFER)
      {
         numRecorded = writeSamples(input, numSamples, &MprRecorder::writeCircularBuffer);
      }
      mSamplesRecorded += numRecorded;

      if (numRecorded != numSamples)
      {
         finish(FINISHED_ERROR);
      }

      // Push data further downstream
      outBufs[0].swap(in);
   }

   mNumFramesProcessed++;

   return TRUE;
}

int MprRecorder::writeCircularBuffer(char * data, int dataSize)
{
    OsSysLog::add(FAC_MP, PRI_INFO, "MprRecorder::doProcessFrame - TO_CIRCULAR_BUFFER, non-silence");
    
    unsigned long newSize, previousSize;
    mpCircularBuffer->write(data, dataSize, &newSize, &previousSize);

    if (previousSize < mRecordingBufferNotificationWatermark && newSize >= mRecordingBufferNotificationWatermark)
        notifyCircularBufferWatermark();

    // the circular buffer is endless, so we can say we have written all in
    return dataSize;
}

void MprRecorder::notifyCircularBufferWatermark()
{
    OsSysLog::add(FAC_MP, PRI_DEBUG, "MprRecorder::notifyCircularBufferWatermark - watermark reached");
    MprnIntMsg msg(MpResNotificationMsg::MPRNM_RECORDER_CIRCULARBUFFER_WATERMARK_REACHED,
        getName(),
        0);
    sendNotification(msg);
}

void MprRecorder::createEncoder(const char * mimeSubtype, unsigned int codecSampleRate)
{
    OsStatus status = OS_INVALID_ARGUMENT;

    MpCodecFactory* codecFactory = MpCodecFactory::getMpCodecFactory();
    assert(codecFactory);

    status = codecFactory->createEncoder(mimeSubtype,
        NULL, // FMTP
        codecSampleRate, // GSM 8K
        1, // Num channels
        111, // Bogus payload as we are writing to file
        mpEncoder);
    // Note: we could have a NULL encoder here if the codec plugin is not loaded
    if (mpEncoder == NULL)
    {
        OsSysLog::add(FAC_MP, PRI_ERR,
            "MprRecorder::createEncoder failed to load the codec for MIME subtype '%s'. Perhaps the plugin is not loaded?",
            mimeSubtype);
        OsSysLog::flush();
    }
    assert(mpEncoder);
    mpEncoder->initEncode();
    assert(mpEncoder->getInfo()->getSampleRate() == codecSampleRate);
}

void MprRecorder::prepareEncoder(RecordFileFormat recFormat, unsigned int & codecSampleRate)
{
    codecSampleRate = 0;
    OsSysLog::add(FAC_MP, PRI_DEBUG,
            "MprRecorder::prepareEncoder format: %d media frame size: %d sample rate: %d processed frames: %d encoded frames: %d last encoded size: %d",
            mRecFormat, mSamplesPerLastFrame, mSamplesPerSecond, mNumFramesProcessed, mEncodedFrames, mLastEncodedFrameSize);
    assert(mpFlowGraph);
    unsigned int flowgraphSampleRate = mpFlowGraph->getSamplesPerSec();

    if (mpEncoder)
    {
        // Should always be even number of GSM frames (alternating between 32 and 33 bytes).
        // If last frame written is not 33, something went wrong.
        if(mRecFormat == WAV_GSM && 
           (mEncodedFrames % 2 || (mLastEncodedFrameSize != 0 && mLastEncodedFrameSize != 33)))
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                    "MprRecord::prepareEncoder deleting GSM codec, last of %d frames written was: %d bytes, should be 33",
                    mEncodedFrames, mLastEncodedFrameSize);
        }

        delete mpEncoder;
        mpEncoder = NULL;
    }

    mNumFramesProcessed = 0;
    mRecFormat = recFormat;
    mEncodedFrames = 0;
    mLastEncodedFrameSize = 0;
    if (mpResampler)
    {
        delete mpResampler;
        mpResampler = NULL;
    }

    switch (mRecFormat)
    {
        // Encoder needed
    case MprRecorder::WAV_GSM:
        codecSampleRate = 8000;
        createEncoder(MIME_SUBTYPE_GSM_WAVE, codecSampleRate);
        break;

    case MprRecorder::WAV_ALAW:
        codecSampleRate = 8000;
        createEncoder(MIME_SUBTYPE_PCMA, codecSampleRate);
        break;

    case MprRecorder::WAV_MULAW:
        codecSampleRate = 8000;
        createEncoder(MIME_SUBTYPE_PCMU, codecSampleRate);
        break;

    case MprRecorder::UNINITIALIZED_FORMAT:
        OsSysLog::add(FAC_MP, PRI_ERR,
            "MprRecorder::prepareEncoder unset recording format");
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
            "MprRecorder::prepareEncoder invalid recording format: %d",
            mRecFormat);
        OsSysLog::flush();
        assert(0);
        break;
    }

    // If the file ecoder needs a different sample rate
    if (codecSampleRate != flowgraphSampleRate)
    {
        mpResampler = MpResamplerBase::createResampler(1, flowgraphSampleRate, codecSampleRate);
    }
}

UtlBoolean MprRecorder::handleStartFile(int file,
                                        RecordFileFormat recFormat,
                                        int time,
                                        int silenceLength, 
                                        UtlBoolean append)
{
    // If the file descriptor is already set, its busy already recording.
    if(mFileDescriptor > -1)
    {
        OsSysLog::add(FAC_MP, PRI_ERR,
                "MprRecord::handleFileStart file already set: %d new: %d",
                mFileDescriptor, file);
    }

   mFileDescriptor = file;
   mRecordDestination = TO_FILE;

   unsigned int codecSampleRate;
   prepareEncoder(recFormat, codecSampleRate);

   // If we are creating a WAV file, write the header.
   // Otherwise we are writing raw PCM data to file.
   // If we are appending, the wave file header already exists
   if (mRecFormat != MprRecorder::RAW_PCM_16 && !append)
   {
      writeWaveHeader(file, recFormat, codecSampleRate);
   }

   startRecording(time, silenceLength);

   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MprRecorder::handleStartFile(%d, %d, %d, %d, %s) finished",
                 file, recFormat, time, silenceLength, (append ? "TRUE" : "FALSE"));
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

UtlBoolean MprRecorder::handleStartCircularBuffer(CircularBufferPtr * buffer, 
                                                  RecordFileFormat recordingFormat,
                                                  unsigned long recordingBufferNotificationWatermark)
{
   if (mpCircularBuffer)
       mpCircularBuffer->release();

   mpCircularBuffer = buffer;
   mRecordingBufferNotificationWatermark = recordingBufferNotificationWatermark;
   mRecordDestination = TO_CIRCULAR_BUFFER;

   unsigned int codecSampleRate;
   prepareEncoder(recordingFormat, codecSampleRate);

   startRecording(0, 0);

   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MprRecorder::handleStartCircularBuffer() finished");
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
         UtlBoolean append;

         UtlSerialized &msgData = ((MpPackedResourceMsg*)(&rMsg))->getData();
         stat = msgData.deserialize(file);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize((int&)recFormat);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize(timeMS);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize(silenceLength);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize(append);
         assert(stat == OS_SUCCESS);
         return handleStartFile(file, recFormat, timeMS, silenceLength, append);
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

   case MPRM_START_CIRCULAR_BUFFER:
      {
         OsStatus stat;
         CircularBufferPtr * buffer;
         RecordFileFormat recordingFormat;
         unsigned long recordingBufferNotificationWatermark;

         UtlSerialized &msgData = ((MpPackedResourceMsg*)(&rMsg))->getData();
         stat = msgData.deserialize((void*&)buffer);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize((int&)recordingFormat);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize(recordingBufferNotificationWatermark);
         assert(stat == OS_SUCCESS);
         return handleStartCircularBuffer(buffer, recordingFormat, recordingBufferNotificationWatermark);
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
   assert(mpFlowGraph);
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
           // Some codecs require a specific multiple of frames such that we can append
           // properly.
           switch(mRecFormat)
           {
               OsSysLog::add(FAC_MP, PRI_DEBUG,
                       "MprRecorder::closeFile format: %d media frame size: %d sample rate: %d processed frames: %d",
                       mRecFormat, mSamplesPerLastFrame, mSamplesPerSecond, mNumFramesProcessed);
               // GSM requires every other frame to be a different size.
               // So we ensure even number of frames to be sure the append works ok.
               case MprRecorder::WAV_GSM:
                   if(mEncodedFrames % 2)
                   {
                       // Should not take more than 3 media frames to fill 1 more GSM frame in the encoder
                       for(int extraFrameIndex = 0; (mEncodedFrames % 2) && extraFrameIndex < 3; extraFrameIndex++)
                       {
                           OsSysLog::add(FAC_MP, PRI_DEBUG,
                                   "MprRecorder::closeFile adding even numbered GSM frame (%d + 1) added %d media frames, last frame size: %d",
                                   mEncodedFrames, extraFrameIndex + 1, mLastEncodedFrameSize);
                           // Add an extra frame of silence
                           writeFileSilence(mSamplesPerLastFrame ? mSamplesPerLastFrame : 80);
                           OsSysLog::add(FAC_MP, PRI_DEBUG,
                                   "MprRecorder::closeFile added %d samples, total GSM frames: %d, last frame size: %d",
                                   (mSamplesPerLastFrame ? mSamplesPerLastFrame : 80), mEncodedFrames, mLastEncodedFrameSize);
                       }
                   }
#ifdef TEST_PRINT
                   else
                   {
                       OsSysLog::add(FAC_MP, PRI_DEBUG,
                               "MprRecorder::closeFile even number of GSM frames: %d",
                               mEncodedFrames);
                   }
#endif

                   break;

               default:
                   break;
           }

           updateWaveHeaderLengths(mFileDescriptor, mRecFormat);
           if(mRecFormat == WAV_GSM && mLastEncodedFrameSize != 33)
           {
                   OsSysLog::add(FAC_MP, PRI_ERR,
                           "MprRecord::updateWaveHeaderLength last GSM frame written was: %d bytes, should be 33",
                           mLastEncodedFrameSize);
           }
        }
        close(mFileDescriptor);
        mFileDescriptor = -1;
    }
}


int MprRecorder::writeFileSilence(int numSamples)
{
    assert(((int)MpMisc.mpFgSilence->getSamplesNumber()) >= numSamples);
    const MpAudioSample* silence = MpMisc.mpFgSilence->getSamplesPtr();
    return(writeSamples(silence, numSamples, &MprRecorder::writeFile));
}

int MprRecorder::writeSamples(const MpAudioSample *pBuffer, int numSamples, WriteMethod writeMethod)
{
#ifdef TEST_PRINT
    OsSysLog::add(FAC_MP, PRI_DEBUG,
            "MprRecorder::writeSamples(pBuffer: %p, numSamples: %d)",
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

            // Only count when we have a fully encoded frame for the 
            // specific codec
            if(dataSize > 0)
            {
#ifdef TEST_PRINT
                OsSysLog::add(FAC_MP, PRI_DEBUG,
                        "MprRecord:writeSamples incrementing encoded frames(%d)  Datasize: %d",
                        mEncodedFrames, dataSize);
#endif
                mEncodedFrames++;
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
        if(mRecFormat == WAV_GSM)
        {
            if(mLastEncodedFrameSize == 32 && dataSize == 32)
            {
                OsSysLog::add(FAC_MP, PRI_ERR,
                              "MprRecorder::writeSamples GSM dataSize: %d last frame was: %d bytes",
                              dataSize, mLastEncodedFrameSize);

            }
            else if((mLastEncodedFrameSize == 0 || mLastEncodedFrameSize == 33) && dataSize == 33)
            {
                OsSysLog::add(FAC_MP, PRI_ERR,
                              "MprRecorder::writeSamples GSM dataSize: %d last frame was: %d bytes",
                              dataSize, mLastEncodedFrameSize);

            }
        }
        mLastEncodedFrameSize = dataSize;

        int bytesWritten = 
            (this->*writeMethod)((char *)encodedSamplesPtr, dataSize);

        if(bytesWritten != dataSize)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "MprRecorder::writeSamples wrote %d of %d bytes",
                          bytesWritten, dataSize);
        }
    }

#ifdef TEST_PRINT
    OsSysLog::add(FAC_MP, PRI_DEBUG,
            "MprRecorder::writeSamples returning: %d (numSamplesEncoded)",
            numSamplesEncoded);
#endif
    return(numSamplesEncoded);
}

int MprRecorder::writeFile(char * data, int dataSize)
{
    return write(mFileDescriptor, data, dataSize);
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

int MprRecorder::writeCircularBufferSilence(int numSamples)
{
    assert(((int)MpMisc.mpFgSilence->getSamplesNumber()) >= numSamples);
    const MpAudioSample* silence = MpMisc.mpFgSilence->getSamplesPtr();
    return writeSamples(silence, numSamples, &MprRecorder::writeCircularBuffer);
}

int16_t MprRecorder::getBitsPerSample(RecordFileFormat format)
{
    switch (format)
    {
    case MprRecorder::WAV_GSM:
        return 0;

    case MprRecorder::WAV_PCM_16:
        return 2; //sizeof(MpAudioSample);

    case MprRecorder::WAV_ALAW:
        // fall-through
    case MprRecorder::WAV_MULAW:
        return 1;

    default:
        return 2; //sizeof(MpAudioSample);
    }
}

// TODO refactor this out of recorder into separate class
OsStatus MprRecorder::readWaveHeader(int fileHandle,
                                     RecordFileFormat& format,
                                     uint16_t& samplesPerSecond,
                                     uint16_t& channels)
{
    OsStatus status = OS_INVALID;
    int totalBytesRead = 0;
    int bytesRead = 0;
    char riffLabel[5];
    riffLabel[4] = '\0';
    char waveLabel[5];
    waveLabel[4] = '\0';
    char fmtLabel[5];
    fmtLabel[4] = '\0';
    uint32_t length = 0;
    uint32_t fmtLength = 0;
    uint16_t compressionCode = 0;
    if((bytesRead = read(fileHandle, &riffLabel, 4)) == 4 &&
       (totalBytesRead += bytesRead) &&
       strcmp(riffLabel, "RIFF") == 0 &&
       (bytesRead = read(fileHandle, (char*) &length, sizeof(length))) == sizeof(length) &&
       (totalBytesRead += bytesRead) &&
       (bytesRead = read(fileHandle, &waveLabel, 4)) == 4 &&
       (totalBytesRead += bytesRead) &&
       strcmp(waveLabel, "WAVE") == 0 &&
       (bytesRead = read(fileHandle, &fmtLabel, 4)) == 4 &&
       (totalBytesRead += bytesRead) &&
       strcmp(fmtLabel, "fmt ") == 0 &&
       (bytesRead = read(fileHandle, (char*) &fmtLength, sizeof(fmtLength))) == sizeof(fmtLength) &&
       (totalBytesRead += bytesRead) &&
       // TODO: check codec specific fmt length
       (bytesRead = read(fileHandle, (char*) &compressionCode, sizeof(compressionCode))) == sizeof(compressionCode) &&
       (totalBytesRead += bytesRead) &&
       (bytesRead = read(fileHandle, (char*) &channels, sizeof(channels))) == sizeof(channels) &&
       (totalBytesRead += bytesRead) &&
       (bytesRead = read(fileHandle, (char*) &samplesPerSecond, sizeof(samplesPerSecond))) == sizeof(samplesPerSecond) &&
       (totalBytesRead += bytesRead))
    {
        status = OS_SUCCESS;
    }

    // Empty/new file
    else if(totalBytesRead == 0)
    {
        status = OS_FAILED;
    }

    format = (MprRecorder::RecordFileFormat) compressionCode;
    return(status);
}

// TODO refactor this out of recorder into separate class
UtlBoolean MprRecorder::writeWaveHeader(int handle, 
                                        RecordFileFormat format,
                                        uint32_t samplesPerSecond)
{
   UtlBoolean retCode = FALSE;
   char tmpbuf[80];
   int16_t sampleSize = getBitsPerSample(format);
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
           // fall-through
       case MprRecorder::WAV_ALAW:
           // fall-through
       case MprRecorder::WAV_MULAW:
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
   if (format == MprRecorder::WAV_GSM)
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
   uint32_t length = lseek(handle, 0, SEEK_END);
   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MprRecorder::updateWaveHeaderLengths wave file length: %d",
                 (int) length);

   //now go back to beginning
   uint32_t waveHeaderLengthOffset = lseek(handle, 4, SEEK_SET);
   if(waveHeaderLengthOffset != 4)
   {
       OsSysLog::add(FAC_MP, PRI_ERR,
               "MprRecorder::updateWaveHeaderLengths could not index to wave file header length offset (requested 4) got: %d (errno: %d) total length: %d.",
               (int) waveHeaderLengthOffset, errno, (int) length);
   }

   //and update the RIFF length
   uint32_t rifflength = length-8;
   write(handle, (char*)&rifflength,sizeof(length));


   //this should be the length of just the data
   uint16_t totalWaveHeaderLength = 0;
   switch(format)
   {
       case MprRecorder::WAV_GSM:
           {
           //now seek to the data length
               uint32_t waveDataLengthOffset = lseek(handle, 56, SEEK_SET);
               if(waveDataLengthOffset != 56)
               {
                   OsSysLog::add(FAC_MP, PRI_ERR,
                           "MprRecorder::updateWaveHeaderLengths could not index to wave header GSM data length offset (requested 56) got: %d (errno: %d) total length: %d.",
                           (int) waveDataLengthOffset, errno, (int) length);
               }
           }
           if((length - 60) % 65)
           {
                   OsSysLog::add(FAC_MP, PRI_ERR,
                           "MprRecord::updateWaveHeaderLength should be even number GSM frames written was: %d frames + %d bytes",
                           (length - 60) / 65 * 2, (length - 60) % 65);
           }

           totalWaveHeaderLength = 60;
           break;

       case MprRecorder::WAV_PCM_16:
           // fall-through
       case MprRecorder::WAV_ALAW:
           // fall-through
       case MprRecorder::WAV_MULAW:
           //now seek to the data length
           {
               uint32_t waveDataLengthOffset = lseek(handle, 40, SEEK_SET);
               if(waveDataLengthOffset != 40)
               {
                   OsSysLog::add(FAC_MP, PRI_ERR,
                           "MprRecorder::updateWaveHeaderLengths could not index to wave header data length offset (requested 40) got: %d (errno: %d) total length: %d.",
                           (int) waveDataLengthOffset, errno, (int) length);
               }
           }
           totalWaveHeaderLength = 44;
           break;

       default:
           assert(0);
           break;
   }

   uint32_t datalength = length - totalWaveHeaderLength;
   write(handle, (char*)&datalength,sizeof(datalength));

   uint32_t newLength = lseek(handle, 0, SEEK_END);
   if(length != newLength)
   {
       // Something has gone wrong, length should not change we
       // are supposed to updating the wave file header at the begining of the file
       OsSysLog::add(FAC_MP, PRI_ERR,
               "MprRecorder::updateWaveHeaderLengths file length changed from: %d to: %d",
               length, newLength);
   }
   return(retCode);
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


