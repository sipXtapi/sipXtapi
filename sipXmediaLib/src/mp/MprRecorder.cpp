//
// Copyright (C) 2006-2020 SIPez LLC.  All rights reserved.
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
#include <os/OsDateTime.h>
#include <mp/MpMisc.h>
#include <mp/MpBuf.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MprRecorder.h>
#include <mp/MpPackedResourceMsg.h>
#include <mp/MprnIntMsg.h>
#include <mp/MpEncoderBase.h>
#include <mp/MpResampler.h>
#include <mp/MpCodecFactory.h>

//#define OPUS_FILE_RECORD_ENABLED
#ifdef OPUS_FILE_RECORD_ENABLED
#  include <opusenc.h>
#  include <opusfile.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define ETHERNET_MTU_BYTES 1500

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprRecorder::MprRecorder(const UtlString& rName)
: MpAudioResource(rName, 1, MAXIMUM_RECORDER_CHANNELS, 0, MAXIMUM_RECORDER_CHANNELS)
, mState(STATE_IDLE)
, mRecordDestination(TO_UNDEFINED)
, mChannels(1)
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
, mpOpusEncoder(NULL)
, mpOpusComments(NULL)
, mpOpusStreamObject(NULL)
, mWhenToInterlace(NO_INTERLACE)
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
    closeFile("~MprRecorder");

    if(mpEncoder)
    {
        delete mpEncoder;
        mpEncoder = NULL;
    }

    if(mpOpusEncoder)
    {
        deleteOpusEncoder();
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
                                UtlBoolean append,
                                int numChannels)
{
    int fileHandle = -1;
    OsStatus result = OS_SUCCESS;

    if(recFormat == WAV_GSM && numChannels > 1)
    {
       OsSysLog::add(FAC_MP, PRI_WARNING,
          "MprRecorder::startFile multi-channel (%d) recording with GSM not supported, assuming 1 channel",
          numChannels);
          numChannels = 1;
    }

    if(numChannels > MAXIMUM_RECORDER_CHANNELS)
    {
        OsSysLog::add(FAC_MP, PRI_ERR,
                "MprRecord::startFile number of channels requested: %d exceeds maximum, assuming maximum: %d",
                numChannels, MAXIMUM_RECORDER_CHANNELS);
        numChannels = MAXIMUM_RECORDER_CHANNELS;
    }

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
                switch(recFormat)
                {
                    case OGG_OPUS:
                    {
#ifdef OPUS_FILE_RECORD_ENABLED
                        // Check if there is at least 1 byte in the file
                        long fileSize = lseek(fileHandle, 0, SEEK_END);
                        OsSysLog::add(FAC_MP, PRI_DEBUG,
                                      "MprRecord::startFile lseek to 1 file %s descriptor: %d returned: %d",
                                      filename, fileHandle, (int)fileSize);
                        if(fileSize == 0)
                        {
                            // This is a new file.  No header to validate.
                            // Move to the begining and start appending.
                            lseek(fileHandle, 0, SEEK_SET);
                        }
                        else
                        {
                            // Opus check opus header
                            lseek(fileHandle, 0, SEEK_SET);
                            OpusHead opusHeader;
                            OsStatus opusHeaderStatus = validateOpusHeader(fileHandle, opusHeader);
                            // Valid header
                            if(opusHeaderStatus == OS_SUCCESS)
                            {
                            // Not checking channel count as each section of Opus OGG file can have different
                            // parameters including channel count.  
                                // Move to the end of the file and start appending
                                lseek(fileHandle, 0, SEEK_END);
                            }
                            else
                            {
                                OsSysLog::add(FAC_MP, PRI_ERR, 
                                        "MprRecorder::startFile invalid Opus OGG header read from append to file: %s (return: %d) closed fd: %d",
                                        filename,
                                        opusHeaderStatus,
                                        fileHandle);
                                close(fileHandle);
                                fileHandle = -1;
                                result = OS_FAILED;
                            }
                        }
#else
                        result = OS_FAILED;
#endif
                    }
                        break;

                    case RAW_PCM_16:
                        // Move to the end of the file and start appending
                        lseek(fileHandle, 0, SEEK_END);
                        break;



                    case WAV_PCM_16:
                    case WAV_ALAW:
                    case WAV_MULAW:
                    case WAV_GSM:
                    // All other file formats are wave files and if we are appending,
                    // the file must already have a valid wave header.
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
                                   numChannels == fileChannels)
                                {
                                    // The record format is the same as that of the file that already exists.
                                    // Move to the end of the file where we will start appending
                                    lseek(fileHandle, 0, SEEK_END);
                                }

                                // different recording format requested from existing file
                                // error out and close the file
                                else
                                {
                                    OsSysLog::add(FAC_MP, PRI_ERR, 
                                            "MprRecorder::startFile wave header read from append to file: %s "
                                            "(format: %d samples/sec: %d channels: %d), "
                                            "differs from requested wave record format: %d samples/sec: %d channels: %d closed fd: %d",
                                            filename,
                                            waveFileCodec, fileSamplesPerSecond, fileChannels,
                                            recFormat, fileSamplesPerSecond, numChannels,
                                            fileHandle);
                                    close(fileHandle);
                                    fileHandle = -1;
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
                                        OsSysLog::add(FAC_MP, PRI_ERR, 
                                                "MprRecorder::startFile invalid wave header read from file: %s, cannot append wave format: %d closed fd: %d",
                                                filename,
                                                recFormat,
                                                fileHandle);
                                        close(fileHandle);
                                        fileHandle = -1;
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
                                OsSysLog::add(FAC_MP, PRI_ERR, 
                                        "MprRecorder::startFile wave header read failed: %d closed fd: %d",
                                        waveHeaderReadStatus,
                                        fileHandle);
                                close(fileHandle);
                                fileHandle = -1;
                                result = OS_FAILED;
                                break;
                        } // end switch(waveHeaderReadStatus)
                    }
                        break;

                    case UNINITIALIZED_FORMAT:
                    default:
                        assert(0);  // Unexpected codec type
                        break;
                } // end switch on codec type
            }

            // File is to be replaced, so position to begining of file
            else
            {
                lseek(fileHandle, 0, SEEK_SET);
            }

        }
        else
        {
           OsSysLog::add(FAC_MP, PRI_ERR,
                         "MprRecorder::startFile() failed to open file %s, error code is %i",
                         filename, errno);
           result = OS_FILE_ACCESS_DENIED;
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
          stat = msgData.serialize(numChannels);
          assert(stat == OS_SUCCESS);
          msgData.finishSerialize();
          
          result = fgQ.send(msg, sOperationQueueTimeout);
       }
   }

   return(result);
}

OsStatus MprRecorder::startBuffer(const UtlString& namedResource,
                                  OsMsgQ& fgQ,
                                  MpAudioSample *pBuffer,
                                  int bufferSize,
                                  int time,
                                  int silenceLength,
                                  int numChannels)
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
   stat = msgData.serialize(numChannels);
   assert(stat == OS_SUCCESS);
   msgData.finishSerialize();

   return fgQ.send(msg, sOperationQueueTimeout);
}

OsStatus MprRecorder::startCircularBuffer(const UtlString& namedResource,
                                          OsMsgQ& fgQ,
                                          CircularBufferPtr & buffer,
                                          RecordFileFormat recordingFormat,
                                          unsigned long recordingBufferNotificationWatermark,
                                          int numChannels)
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
    stat = msgData.serialize(numChannels);
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
   MpAudioBufPtr in[MAXIMUM_RECORDER_CHANNELS];

   if (inBufsSize == 0 || outBufsSize == 0)
   {
      return FALSE;
   }

   // Cache the last frame size
   mSamplesPerLastFrame = samplesPerFrame;
   mSamplesPerSecond = samplesPerSecond;

   // Take data from the first input
   int channelIndex;
   UtlBoolean validInput = FALSE;
   for(channelIndex = 0; channelIndex < inBufsSize; channelIndex++)
   {
       if(inBufs[channelIndex].isValid())
       {
           validInput = TRUE;
       }
       in[channelIndex].swap(inBufs[channelIndex]);
   }

   if (!isEnabled || mState != STATE_RECORDING)
   {
       for(channelIndex = 0; channelIndex < inBufsSize; channelIndex++)
       {
          // Push data further downstream
          outBufs[channelIndex].swap(in[channelIndex]);
       }
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
      for(channelIndex = 0; channelIndex < inBufsSize; channelIndex++)
      {
         outBufs[channelIndex].swap(in[channelIndex]);
      }
      return TRUE;
   }

   // Now write the buffer out
   if (!validInput)
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
         OsSysLog::add(FAC_MP, PRI_ERR,
                 "MprRecorder::doProcessFrame line %d numRecorded (%d) != numSamples (%d) channels=%d",
                 __LINE__,
                 numRecorded,
                 samplesPerFrame,
                 mChannels);
         finish(FINISHED_ERROR);
      }
   }
   else
   {
      // Write speech data.

      // Keep track of inactive audio frames for auto record stop
      mConsecutiveInactive++;
      for(channelIndex = 0; channelIndex < inBufsSize; channelIndex++)
      {
          // If any channel has active audio, reset the inactivety counter
          if (in[channelIndex].isValid() &&
              isActiveAudio(in[channelIndex]->getSpeechType()))
          {
             mConsecutiveInactive = 0;
             break;
          }
      }

      const MpAudioSample* inputSamplesPtrArray[MAXIMUM_RECORDER_CHANNELS];
      for(channelIndex = 0; channelIndex < inBufsSize; channelIndex++)
      {
          if(in[channelIndex].isValid())
          {
              inputSamplesPtrArray[channelIndex] = in[channelIndex]->getSamplesPtr();
          }
          else
          {
              inputSamplesPtrArray[channelIndex] = MpMisc.mpFgSilence->getSamplesPtr();
          }
      }
      int numSamples = in[0]->getSamplesNumber();
      int numRecorded;
      if (mRecordDestination == TO_FILE)
      {
         numRecorded = writeSamples(inputSamplesPtrArray, numSamples, &MprRecorder::writeFile);
      }
      else if (mRecordDestination == TO_BUFFER)
      {
         numRecorded = writeBufferSpeech(inputSamplesPtrArray[0], numSamples);
      }
      else if (mRecordDestination == TO_CIRCULAR_BUFFER)
      {
         numRecorded = writeSamples(inputSamplesPtrArray, numSamples, &MprRecorder::writeCircularBuffer);
      }
      mSamplesRecorded += numRecorded;

      if (numRecorded != numSamples)
      {
         OsSysLog::add(FAC_MP, PRI_ERR,
             "MprRecorder::doProcessFrame line %d numRecorded (%d) != numSamples (%d) channels=%d",
             __LINE__,
             numRecorded, 
             numSamples, 
             mChannels);

         finish(FINISHED_ERROR);
      }

      // Push data further downstream
      for(channelIndex = 0; channelIndex < inBufsSize; channelIndex++)
      {
         outBufs[channelIndex].swap(in[channelIndex]);
      }
   }

   mNumFramesProcessed++;

   return TRUE;
}

int MprRecorder::writeCircularBuffer(const char* channelData[], int dataSize)
{
    OsSysLog::add(FAC_MP, PRI_INFO, "MprRecorder::doProcessFrame - TO_CIRCULAR_BUFFER, non-silence");
    
    unsigned long newSize, previousSize, iterPreviousSize;
    int bytesPerSample = getBytesPerSample(mRecFormat);
    assert(bytesPerSample > 0);

    int dataIndex;
    int channelIndex;
    if(bytesPerSample > 0)
    {
        for(dataIndex = 0; dataIndex < dataSize; dataIndex += bytesPerSample)
        {
            for(channelIndex = 0; channelIndex < mChannels; channelIndex++)
            {
                mpCircularBuffer->write(&((channelData[channelIndex])[dataIndex]), bytesPerSample, &newSize, &iterPreviousSize);
                if(dataIndex == 0)
                {
                    previousSize = iterPreviousSize;
                }
            }
        }
    }

    if (previousSize < mRecordingBufferNotificationWatermark && newSize >= mRecordingBufferNotificationWatermark)
        notifyCircularBufferWatermark();

    // the circular buffer is endless, so we can say we have written all in
    return dataSize * mChannels;
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
            "MprRecorder::createEncoder failed (status = %d) to load the codec for MIME subtype '%s'. Perhaps the plugin is not loaded?",
            status,
            mimeSubtype);
        OsSysLog::flush();
    }
    if(mpEncoder)
    {
        mpEncoder->initEncode();
        assert(mpEncoder->getInfo()->getSampleRate() == codecSampleRate);
    }
}

#ifdef OPUS_FILE_RECORD_ENABLED
#if 0
struct SipxOpusWriteObject
{
    int mWriteFd;
};

static int SipxOpusWriteFile(void* fileInfo, const unsigned char* data, int32_t length)
{
    int result = 0;
    assert(fileInfo);
    struct SipxOpusWriteObject* fileHandleObject = (struct SipxOpusWriteObject*) fileInfo;
    int bytesWritten = write(fileHandleObject->mWriteFd, data, length);

    if(bytesWritten != length)
    {
        result = errno;
        OsSysLog::add(FAC_MP, PRI_ERR,
                      "SipxOpusWrite write to fd: %d failed, wrote %d of %d bytes\n", 
                      fileHandleObject->mWriteFd, bytesWritten, length);
    }
    else
    {
        OsSysLog::add(FAC_MP, PRI_DEBUG,
                     "SipxOpusWrite %d bytes to fd: %d\n", 
                     bytesWritten, fileHandleObject->mWriteFd);
    }

    return(result);
}

static int SipxOpusCloseFile(void* fileInfo)
{
    int result = 1;
    assert(fileInfo);
    struct SipxOpusWriteObject* fileHandleObject = (struct SipxOpusWriteObject*) fileInfo;
    if(fileHandleObject && fileHandleObject->mWriteFd)
    {
        OsSysLog::add(FAC_MP, PRI_DEBUG,
                      "SipxOpusCloseFile(fd=%d) not closing file",
                      fileHandleObject->mWriteFd);
        // We do not close here as file open and close is managed external to Opus utils
        //result = close(fileHandleObject->mWriteFd);
    }

    return(result);
}

static const OpusEncCallbacks opusFileCallbacks =
{
    SipxOpusWriteFile,
    SipxOpusCloseFile
};

#else

struct SipxOpusWriteObject
{
    int mBytesWritten;
    int mBufferMaximum;
    char* mpBuffer;
};

static int SipxOpusWriteBuffer(void* bufferInfo, const unsigned char* data, int32_t length)
{
    int result = 1;
    struct SipxOpusWriteObject* bufferHandleObject = (struct SipxOpusWriteObject*) bufferInfo;
    if(length + bufferHandleObject->mBytesWritten > bufferHandleObject->mBufferMaximum)
    {
        OsSysLog::add(FAC_MP, PRI_ERR,
                      "MprRecorder SipxOpusWriteBuffer Buffer: %d too small.  Written: %d length to add: %d",
                      bufferHandleObject->mBufferMaximum, bufferHandleObject->mBytesWritten, length);
        OsSysLog::flush();
    }
    assert(length + bufferHandleObject->mBytesWritten <= bufferHandleObject->mBufferMaximum);

    if(length + bufferHandleObject->mBytesWritten <= bufferHandleObject->mBufferMaximum)
    {
        memcpy(&(bufferHandleObject->mpBuffer[bufferHandleObject->mBytesWritten]),
               data,
               length);
        result = 0;
    }
    bufferHandleObject->mBytesWritten += length;

    return(result);
}

static int SipxOpusCloseBuffer(void* bufferInfo)
{
    return(0);
}

static const OpusEncCallbacks opusFileCallbacks =
{
    SipxOpusWriteBuffer,
    SipxOpusCloseBuffer
};

OsStatus MprRecorder::validateOpusHeader(int inFileFd, OpusHead& opusHeader)
{
    OsStatus status = OS_FILE_INVALID_HANDLE;
    unsigned char headerBuffer[57];
    // Opus file can be appended.  So headers can exist anywhere in the file.
    // Keep the current position so that we can restore it after validating the
    // header.
    uint64_t currentPos = lseek(inFileFd, 0, SEEK_CUR);
    if(currentPos < 0)
    {
        status = OS_FILE_SEEK_ERROR;
    }

    else
    {
        int bytesRead = read(inFileFd, headerBuffer, sizeof(headerBuffer));
        if(bytesRead < 47)
        {
            status = OS_INVALID_LENGTH;
        }

        if(bytesRead > 0)
        {
            int result = op_test(&opusHeader, headerBuffer, bytesRead);
            if(result == 0)
            {
                status = OS_SUCCESS;
            }
            else
            {
                OsSysLog::add(FAC_MP, PRI_ERR,
                              "MprRecorder::validateOpusHeader opus_test returned: %d",
                              result);
            }

            // Reposition back where we were so there is not impact on file position
            uint64_t resetPos = lseek(inFileFd, currentPos, SEEK_SET);
            if(resetPos < 0)
            {
                status = OS_FILE_SEEK_ERROR;
            }
        }    
    }
    return(status);
}

#endif

#endif

OsStatus MprRecorder::createOpusEncoder(int channels,
                                        const char* artist, 
                                        const char* title)
{
#ifdef OPUS_FILE_RECORD_ENABLED
    OsStatus status = OS_SUCCESS;
    //  We always upsample to 48000 external to the codec
    int sampleRate = 48000;

    // May need to change this to 1 or 255
    // 0 = mono or stereo
    // 1 = 1-255 channels using Vorbis channel mappings
    // 255 = 1-255 channels not using any particular channel mappings
    int family = channels <= 2 ? 0 : 255;
    deleteOpusEncoder();
    SipxOpusWriteObject* opusStreamObject = (struct SipxOpusWriteObject*) malloc(sizeof(*mpOpusStreamObject));
#if 0
    opusStreamObject->mWriteFd = -1;
#else
    opusStreamObject->mBytesWritten = 0;
    // Opus data pushed out when draining can be 3 frames over 7KB each.
    opusStreamObject->mBufferMaximum = (0x1 << 15) /* 32KB */  * channels; 
    opusStreamObject->mpBuffer = (char*)malloc(opusStreamObject->mBufferMaximum);
#endif
    OggOpusComments* comments = ope_comments_create();
    ope_comments_add(comments, "ARTIST", artist);
    ope_comments_add(comments, "TITLE", title);

    if(mFileDescriptor < 0)
    {
        status = OS_INVALID_STATE;
        OsSysLog::add(FAC_MP, PRI_ERR,
                      "MprRecorder::createOpusEncoder() recieved invalid file descriptor: %d",
                      mFileDescriptor);
    }
    else
    {
#if 0
        fileObj->mWriteFd = mFileDescriptor;
#endif
        OsSysLog::add(FAC_MP, PRI_DEBUG,
                      "MprRecorder::createOpusEncoder() file fd: %d",
                      mFileDescriptor);

        int error = 0;
        mpOpusEncoder = ope_encoder_create_callbacks(&opusFileCallbacks, opusStreamObject, comments, sampleRate, channels, family, &error);
        if(error)
        {
            OsSysLog::add(FAC_MP, PRI_DEBUG,
                          "MprRecorder::createOpusEncoder() failed encoder: %p error: %d\n", 
                          mpOpusEncoder, error);
            status = OS_FAILED;
            if(mpOpusEncoder) 
            {
                ope_encoder_destroy((OggOpusEnc*)mpOpusEncoder);
                mpOpusEncoder = NULL;
            }
        }
        else
        {
            OsSysLog::add(FAC_MP, PRI_DEBUG,
                          "MprRecorder::createOpusEncoder() created encoder: %p", 
                          mpOpusEncoder);
            // These are safely attached to encoder.  So don't free them up until done encoding
            mpOpusComments = comments;
            comments = NULL;
            mpOpusStreamObject = opusStreamObject;
            opusStreamObject = NULL;
        }

    }

    // Clean up in case of error
    if(comments) 
    {
        ope_comments_destroy(comments);
        // Do not close the record file here.  That should be managed outside the codec.
        //OpusCloseFile(fileObj);
        if(opusStreamObject)
        {
            free(opusStreamObject->mpBuffer);
            free(opusStreamObject);
        }
    }
#else
    OsStatus status = OS_NOT_YET_IMPLEMENTED;
    OsSysLog::add(FAC_MP, PRI_ERR,
                  "MprRecorder::createOpusEncoder() Opus file operations DISABLED.  OPUS_FILE_RECORD_ENABLED not defined");
#endif

    return(status);
}

void MprRecorder::deleteOpusEncoder()
{
#ifdef OPUS_FILE_RECORD_ENABLED
    if(mpOpusEncoder)
    {
        // Drain here. What if we have already closed the file?
        // Is it better that we don't drain as we likely only have silence anyway?
        // Also this is not much different than other codecs where there may be samples still
        // in buffers somewhere.
        int result = ope_encoder_drain((OggOpusEnc*)mpOpusEncoder);
        if(result)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "MprRecorder::deleteOpusEncoder ope_encoder_drain returned: %d",
                          result);

            // If we could not drain codec, we still want to clear the buffer
            mpOpusStreamObject->mBytesWritten = 0;
        }
        // Still have buffered, encoded data that needs to be written to the file
        else if(mpOpusStreamObject && mpOpusStreamObject->mBytesWritten > 0)
        {
            int dataSize = mpOpusStreamObject->mBytesWritten;
            const char* channelData[1];
            channelData[0] = (const char*)mpOpusStreamObject->mpBuffer;
            int bytesWritten = writeFile(channelData, dataSize);

            if(bytesWritten != dataSize * mChannels)
            {
                OsSysLog::add(FAC_MP, PRI_ERR,
                              "MprRecorder::deleteOpusEncoder wrote %d of %d drained Opus encoder bytes",
                              bytesWritten, dataSize);
            }
            else
            {
                OsSysLog::add(FAC_MP, PRI_DEBUG,
                              "MprRecorder::deleteOpusEncoder wrote %d drained Opus encoder bytes",
                              bytesWritten);
            }
            mpOpusStreamObject->mBytesWritten = 0;
        }
        else
        {
            OsSysLog::add(FAC_MP, PRI_DEBUG,
                          "MprRecorder::deleteOpusEncoder no data to drain");
        }
        
        ope_encoder_destroy((OggOpusEnc*)mpOpusEncoder);
        mpOpusEncoder = NULL;
    }
    if(mpOpusComments) ope_comments_destroy((OggOpusComments*)mpOpusComments);
    mpOpusComments = NULL;

    if(mpOpusStreamObject)
    {
        free(mpOpusStreamObject->mpBuffer);
        free(mpOpusStreamObject);
        mpOpusStreamObject = NULL;
    }
#endif
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

    if(mpOpusEncoder || mpOpusComments)
    {
        deleteOpusEncoder();
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
        mWhenToInterlace = NO_INTERLACE;
        createEncoder(MIME_SUBTYPE_GSM_WAVE, codecSampleRate);
        break;

    case MprRecorder::WAV_ALAW:
        codecSampleRate = 8000;
        mWhenToInterlace = (mChannels > 1) ? POST_ENCODE_INTERLACE : NO_INTERLACE;
        createEncoder(MIME_SUBTYPE_PCMA, codecSampleRate);
        break;

    case MprRecorder::WAV_MULAW:
        codecSampleRate = 8000;
        mWhenToInterlace = (mChannels > 1) ? POST_ENCODE_INTERLACE : NO_INTERLACE;
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
        mWhenToInterlace = (mChannels > 1) ? POST_ENCODE_INTERLACE : NO_INTERLACE;
        break;

    case MprRecorder::OGG_OPUS:
        codecSampleRate = 48000;
        mWhenToInterlace = (mChannels > 1) ? PRE_ENCODE_INTERLACE : NO_INTERLACE;
        {
            UtlString trackName;
            // TODO get artist as local URI
            //      track name as remote URI and date
            //trackName.appendFormat("");
            OsDateTime::getLocalTimeString(trackName);
            createOpusEncoder(mChannels, "sipX", trackName.data());
        }
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
// TODO we need a resampler for each channel????
    if (codecSampleRate != flowgraphSampleRate)
    {
        OsSysLog::add(FAC_MP, PRI_ERR,
                      "MprRecorder::prepareDecoder creating resampler from: %d to: %d SPS",
                      flowgraphSampleRate, codecSampleRate);
        mpResampler = MpResamplerBase::createResampler(1, flowgraphSampleRate, codecSampleRate);
    }
}

UtlBoolean MprRecorder::handleStartFile(int file,
                                        RecordFileFormat recFormat,
                                        int time,
                                        int silenceLength, 
                                        UtlBoolean append,
                                        int numChannels)
{
    OsSysLog::add(FAC_MP, PRI_DEBUG,
        "MprRecorder::handleStartFile numChannels: %d MAXIMUM_RECORDER_CHANNELS: %d",
        numChannels,
        MAXIMUM_RECORDER_CHANNELS);
    // If the file descriptor is already set, its busy already recording.
    if(mFileDescriptor > -1)
    {
        OsSysLog::add(FAC_MP, PRI_ERR,
                "MprRecord::handleFileStart this: %p file already set: %d new: %d",
                this, mFileDescriptor, file);
    }
    mFileDescriptor = file;
    mRecordDestination = TO_FILE;

    mChannels = numChannels;
    if(numChannels < 1)
    {
        mChannels = 1;
        OsSysLog::add(FAC_MP, PRI_ERR,
                "MprRecord::handleFileStart number of channels requested: %d, assuming 1",
                numChannels);
    }

    if(mChannels > MAXIMUM_RECORDER_CHANNELS)
    {
        mChannels = MAXIMUM_RECORDER_CHANNELS;
        OsSysLog::add(FAC_MP, PRI_ERR,
                "MprRecord::handleFileStart number of channels requested: %d exceeds maximum, assuming maximum: %d",
                numChannels, MAXIMUM_RECORDER_CHANNELS);
    }
   unsigned int codecSampleRate;
   prepareEncoder(recFormat, codecSampleRate);

   // If we are creating a WAV file, write the header.
   // Otherwise we are writing raw PCM data to file.
   // If we are appending, the wave file header already exists
   if (mRecFormat != MprRecorder::RAW_PCM_16 && 
       mRecFormat != MprRecorder::OGG_OPUS && 
       !append)
   {
      writeWaveHeader(file, recFormat, codecSampleRate, numChannels);
   }

   startRecording(time, silenceLength);

   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MprRecorder::handleStartFile(%d, %d, %d, %d, %s, %d) this: %p finished",
                 file, recFormat, time, silenceLength, (append ? "TRUE" : "FALSE"), numChannels, this);
   return TRUE;
}

UtlBoolean MprRecorder::handleStartBuffer(MpAudioSample *pBuffer,
                                          int bufferSize,
                                          int time,
                                          int silenceLength,
                                          int numChannels)
{
   mpBuffer = pBuffer;
   mBufferSize = bufferSize;
   mRecordDestination = TO_BUFFER;
   mChannels = numChannels < 1 ? 1 : numChannels;

   if(mChannels != 1)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
         "Multi-channel (%d) record not supported for record to buffer",
         mChannels);
   }

   startRecording(time, silenceLength);

   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MprRecorder::handleStartBuffer(%p, %d, %d, %d) finished",
                 pBuffer, bufferSize, time, silenceLength);
   return TRUE;
}

UtlBoolean MprRecorder::handleStartCircularBuffer(CircularBufferPtr * buffer, 
                                                  RecordFileFormat recordingFormat,
                                                  unsigned long recordingBufferNotificationWatermark,
                                                  int numChannels)
{
   if (mpCircularBuffer)
       mpCircularBuffer->release();

   mpCircularBuffer = buffer;
   mRecordingBufferNotificationWatermark = recordingBufferNotificationWatermark;
   mRecordDestination = TO_CIRCULAR_BUFFER;
   mChannels = numChannels < 1 ? 1 : numChannels;

   if(mChannels != 1)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
         "Multi-channel (%d) record not supported for record to buffer",
         mChannels);
   }

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
         int numChannels;

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
         stat = msgData.deserialize(numChannels);
         assert(stat == OS_SUCCESS);
         return handleStartFile(file, recFormat, timeMS, silenceLength, append, numChannels);
      }
      break;

   case MPRM_START_BUFFER:
      {
         OsStatus stat;
         MpAudioSample *pBuffer;
         int bufferSize;
         int time;
         int silenceLength;
         int numChannels;

         UtlSerialized &msgData = ((MpPackedResourceMsg*)(&rMsg))->getData();
         stat = msgData.deserialize((void*&)pBuffer);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize(bufferSize);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize(time);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize(silenceLength);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize(numChannels);
         assert(stat == OS_SUCCESS);
         return handleStartBuffer(pBuffer, bufferSize, time, silenceLength, numChannels);
      }
      break;

   case MPRM_START_CIRCULAR_BUFFER:
      {
         OsStatus stat;
         CircularBufferPtr * buffer;
         RecordFileFormat recordingFormat;
         unsigned long recordingBufferNotificationWatermark;
         int numChannels;

         UtlSerialized &msgData = ((MpPackedResourceMsg*)(&rMsg))->getData();
         stat = msgData.deserialize((void*&)buffer);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize((int&)recordingFormat);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize(recordingBufferNotificationWatermark);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize(numChannels);
         assert(stat == OS_SUCCESS);
         return handleStartCircularBuffer(buffer, recordingFormat, recordingBufferNotificationWatermark, numChannels);
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
      closeFile("finish");
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

void MprRecorder::closeFile(const char* fromWhereLabel)
{
    if (mFileDescriptor > -1)
    {
        OsSysLog::add(FAC_MP, PRI_DEBUG,
                "MprRecorder::closeFile(%s) this: %p fd: %d format: %d channels: %d media frame size: %d sample rate: %d processed frames: %d",
                fromWhereLabel, this, mFileDescriptor,  mRecFormat, mChannels, mSamplesPerLastFrame, mSamplesPerSecond, mNumFramesProcessed);

        if (mRecFormat == RAW_PCM_16)
        {
        }
        else if (mRecFormat == OGG_OPUS)
        {
            deleteOpusEncoder();
        }
        // Any WAVE file needs header updates on closing
        else
        {
           // Some codecs require a specific multiple of frames such that we can append
           // properly.
           switch(mRecFormat)
           {
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
        OsSysLog::add(FAC_MP, PRI_ERR,
                      "MprRecord::updateWaveHeaderLength closed fd: %d",
                      mFileDescriptor);
        close(mFileDescriptor);
        mFileDescriptor = -1;
    }

    // Invalid file descriptor
    else
    {
        OsSysLog::add(FAC_MP, PRI_DEBUG,
                "MprRecorder::closeFile(%s) this: %p fd: %d already closed",
                fromWhereLabel, this, mFileDescriptor);
    }

}


int MprRecorder::writeFileSilence(int numSamples)
{
    assert(((int)MpMisc.mpFgSilence->getSamplesNumber()) >= numSamples);
    const MpAudioSample* silence[MAXIMUM_RECORDER_CHANNELS];
    int channelIndex;
    for(channelIndex = 0; channelIndex < mChannels; channelIndex++)
    {
       silence[channelIndex] = MpMisc.mpFgSilence->getSamplesPtr();
    }
    return(writeSamples(silence, numSamples, &MprRecorder::writeFile));
}

int MprRecorder::writeSamples(const MpAudioSample *pBuffers[], int numSamples, WriteMethod writeMethod)
{
#ifdef TEST_PRINT
    OsSysLog::add(FAC_MP, PRI_DEBUG,
            "MprRecorder::writeSamples(pBuffer: %p, numSamples: %d)",
            pBuffers[0], numSamples);
#endif
    const MpAudioSample* resampledBufferPtrArray[MAXIMUM_RECORDER_CHANNELS];
    const int localBufferSize = ETHERNET_MTU_BYTES; // No audio codecs exceed an MTU
    MpAudioSample localBuffer[localBufferSize * MAXIMUM_RECORDER_CHANNELS];
    OsStatus status = OS_FAILED;

    // If the resampler exists, we resample
    uint32_t samplesConsumed;
    uint32_t numResampled = 0;
    int channelIndex;
    if(mpResampler)
    {
        for(channelIndex = 0; channelIndex < mChannels; channelIndex++)
        {
            resampledBufferPtrArray[channelIndex] = &localBuffer[localBufferSize * channelIndex];
// TODO: use channelIndex not 0
            status = mpResampler->resample(0, 
                                           pBuffers[channelIndex], 
                                           numSamples, 
                                           samplesConsumed,
                                           (MpAudioSample*)resampledBufferPtrArray[channelIndex], 
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
    }

    // No resampler, pass it straight through
    else
    {
        numResampled = numSamples;
        for(channelIndex = 0; channelIndex < mChannels; channelIndex++)
        {
            resampledBufferPtrArray[channelIndex] = pBuffers[channelIndex];
        }
    }

    // Some encoders are frame based and do not create a complete frame every time
    const MpAudioSample* encodedSamplesPtrArray[MAXIMUM_RECORDER_CHANNELS];
    MpAudioSample localEncodeBuffer[localBufferSize * MAXIMUM_RECORDER_CHANNELS]; // Should never get larger after encoding

    // If there is an encoder, encode
    int dataSize = 0;
    int numSamplesEncoded = 0;
    UtlBoolean isEndOfFrame = FALSE;
    UtlBoolean isPacketSilent = FALSE;
    UtlBoolean shouldSetMarker = FALSE;
    if(mpEncoder)
    {
        for(channelIndex = 0; channelIndex < mChannels; channelIndex++)
        {
            encodedSamplesPtrArray[channelIndex] = &localEncodeBuffer[channelIndex * localBufferSize ];
            status = mpEncoder->encode(resampledBufferPtrArray[channelIndex], 
                                       numResampled, 
                                       numSamplesEncoded, 
                                       (unsigned char*) encodedSamplesPtrArray[channelIndex],
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
                // The upper layer does not know or care about resampling.
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
    }

    else if(mpOpusEncoder)
    {
#ifdef OPUS_FILE_RECORD_ENABLED
        // Interlace samples as the codec expects PCM with interlaced channel samples

        MpAudioSample interlacedBuffer[localBufferSize * MAXIMUM_RECORDER_CHANNELS];
        assert(sizeof(interlacedBuffer) >= (numResampled * sizeof(MpAudioSample) * mChannels));
        int interlacedSize = interlaceSamples((const char**)resampledBufferPtrArray, numResampled, sizeof(MpAudioSample), mChannels, (char*)interlacedBuffer, sizeof(interlacedBuffer));

        assert(interlacedSize == (int)( numResampled * sizeof(MpAudioSample) * mChannels));

        int opusResult = ope_encoder_write((OggOpusEnc*)mpOpusEncoder, interlacedBuffer, numResampled);
        if(opusResult)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "MprRecorder::writeSamples ope_encoder_write returned: %d",
                          opusResult);
        }
        else
        {
            // Opus encoder also does up/down sampling.  So we need
            // to correct the sample count for up/down sampling.  Opus
            // encodes at 48000.
            numSamplesEncoded = numResampled * mpFlowGraph->getSamplesPerSec() / 48000;
        }

        if(mpOpusStreamObject && mpOpusStreamObject->mBytesWritten > 0)
        {
            encodedSamplesPtrArray[0] = (MpAudioSample*)mpOpusStreamObject->mpBuffer;
            dataSize = mpOpusStreamObject->mBytesWritten;
            mpOpusStreamObject->mBytesWritten = 0;
            OsSysLog::add(FAC_MP, PRI_DEBUG,
                          "MprRecorder::writeSamples ope_encoder_write encoded: %d resetting buffer to zero: %d",
                          dataSize, mpOpusStreamObject->mBytesWritten);
        }
#else
        assert(0);
#endif
    }

    //  No encoder, pass it straight through
    else
    {
        dataSize = numResampled * sizeof(MpAudioSample);
        // The upper layer does not know or care about resampling.
        // So we tell it that we encoded all of the samples passed in.
        //numSamplesEncoded = numResampled;
        numSamplesEncoded = numSamples;
        for(channelIndex = 0; channelIndex < mChannels; channelIndex++)
        {
            encodedSamplesPtrArray[channelIndex] = resampledBufferPtrArray[channelIndex];
        }
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
            (this->*writeMethod)((const char**)encodedSamplesPtrArray, dataSize);

        if(bytesWritten != dataSize * mChannels)
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

int MprRecorder::writeFile(const char* channelData[], int dataSize)
{
    int bytesWritten = 0;
    int bytesPerSample = getBytesPerSample(mRecFormat);

    OsSysLog::add(FAC_MP, PRI_DEBUG,
            "MprRecorder::writeFile %d record format: %d mChannels: %d dataSize: %d bytes/sample: %d bytesPerSample",
             mFileDescriptor,
             mRecFormat,
             mChannels,
             dataSize,
             bytesPerSample);

    if(mWhenToInterlace == POST_ENCODE_INTERLACE)
    {
        assert(bytesPerSample > 0 || mChannels == 1);
        char interlacedBuffer[1 << 14];
        assert(((int)sizeof(interlacedBuffer)) >= (dataSize * mChannels));

        // Interlace a sample from each channel
        int interlacedSize = interlaceSamples(channelData, dataSize / bytesPerSample , bytesPerSample, mChannels, interlacedBuffer, sizeof(interlacedBuffer));


        bytesWritten = write(mFileDescriptor, interlacedBuffer, interlacedSize);
        OsSysLog::add(FAC_MP, PRI_DEBUG,
                      "MprRecorder::writeFile write fd: %d returned: %d (interlaced)",
                      mFileDescriptor, 
                      bytesWritten);
    }
    else
    {
        bytesWritten = write(mFileDescriptor, channelData[0], dataSize);
        OsSysLog::add(FAC_MP, PRI_DEBUG,
                      "MprRecorder::writeFile write fd: %d returned: %d (non-interlaced)",
                      mFileDescriptor, 
                      bytesWritten);
        if(bytesWritten < 0)
        {
            OsSysLog::add(FAC_MP, PRI_DEBUG,
                          "MprRecorder::writeFile write fd: %d returned: %d (non-interlaced) errno: %d (%s)",
                      mFileDescriptor, 
                      bytesWritten,
                      errno,
                      strerror(errno));
        }
    }

    return(bytesWritten);
}

int MprRecorder::interlaceSamples(const char* samplesArrays[], int samplesPerChannel, int bytesPerSample, int channels, char* interlacedChannelSamplesArray, int interlacedArrayMaximum)
{
    int totalWritten = 0;

    if(bytesPerSample > 0)
    {
        // Optimization:
        if(channels == 1)
        {
            // Cause a single memcpy for the whole chunk
            bytesPerSample = samplesPerChannel * bytesPerSample;
            samplesPerChannel = 1;
        }

        // Interlace a sample from each channel
        for(int dataIndex = 0; dataIndex < samplesPerChannel * bytesPerSample; dataIndex += bytesPerSample)
        {
            //  Need to write a sample from all channels or none
            if(totalWritten + channels * bytesPerSample > interlacedArrayMaximum)
            {
                break;
            }

            for(int channelIndex = 0; channelIndex < channels; channelIndex++)
            {
                memcpy(&(interlacedChannelSamplesArray[totalWritten]), &((samplesArrays[channelIndex])[dataIndex]), bytesPerSample);
                totalWritten += bytesPerSample;
            }
        }
    }

    return(totalWritten);
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
    const MpAudioSample* silence[MAXIMUM_RECORDER_CHANNELS];
    int channelIndex;
    for(channelIndex = 0; channelIndex < mChannels; channelIndex++)
    {
       silence[channelIndex] = MpMisc.mpFgSilence->getSamplesPtr();
    }
    return writeSamples(silence, numSamples, &MprRecorder::writeCircularBuffer);
}

int16_t MprRecorder::getBytesPerSample(RecordFileFormat format)
{
    switch (format)
    {
    case MprRecorder::WAV_GSM:
        // fall-through
    case MprRecorder::OGG_OPUS:
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
                                        uint32_t samplesPerSecond,
                                        int16_t numChannels)
{
   UtlBoolean retCode = FALSE;
   char tmpbuf[80];
   int16_t sampleSize = getBytesPerSample(format);
   int16_t bitsPerSample = 0;
   int32_t formatLength = 0;

   int16_t compressionCode = (int16_t) format;
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
           averageBytesPerSecond = samplesPerSecond * numChannels * sampleSize;
           blockAlign = sampleSize * numChannels;
           bitsPerSample = sampleSize * 8;
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


