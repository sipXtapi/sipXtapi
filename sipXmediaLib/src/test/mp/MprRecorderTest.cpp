//  
// Copyright (C) 2008-2020 SIPez LLC.  All rights reserved.
//
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Daniel Petrie <dpetrie AT SIPez DOT com>
#include <os/OsIntTypes.h>
#include <sipxunittests.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsFileSystem.h>
#include <os/OsFileInfoBase.h>
#include <os/OsFileBase.h>
#include <mp/MprRecorder.h>
#include <mp/MprnIntMsg.h>
#include <mp/MpGenericResourceTest.h>

//#define OPUS_FILE_RECORD_ENABLED

MprRecorder::RecordFileFormat testFileTypes[] =
{
     MprRecorder::RAW_PCM_16,
     MprRecorder::WAV_PCM_16,
     MprRecorder::WAV_ALAW,
     MprRecorder::WAV_MULAW,
     MprRecorder::WAV_GSM
#ifdef OPUS_FILE_RECORD_ENABLED
     ,MprRecorder::OGG_OPUS
#endif
};

const char* testFileTypeStrings[] = 
{
     "RAW_PCM_16",
     "WAV_PCM_16",
     "WAV_ALAW",
     "WAV_MULAW",
     "WAV_GSM"
#ifdef OPUS_FILE_RECORD_ENABLED
     ,"OGG_OPUS"
#endif
};
    
#ifdef OPUS_FILE_RECORD_ENABLED
#include <opusenc.h>
#include <opusfile.h>

struct SipxTestOpusWriteFile
{
    int mWriteFd;
};

static int testOpusWrite(void* fileInfo, const unsigned char* data, int32_t length)
{
    int result = 0;
    CPPUNIT_ASSERT(fileInfo);
    struct SipxTestOpusWriteFile* fileHandleObject = (struct SipxTestOpusWriteFile*) fileInfo;
    int bytesWritten = write(fileHandleObject->mWriteFd, data, length);

    if(bytesWritten != length)
    {
        result = errno;
        OsSysLog::add(FAC_MP, PRI_ERR,
                      "testOpusWrite write to fd: %d failed, wrote %d of %d bytes\n", 
                      fileHandleObject->mWriteFd, bytesWritten, length);
        printf("testOpusWrite write to fd: %d failed, wrote %d of %d bytes\n", 
               fileHandleObject->mWriteFd, bytesWritten, length);
    }
    else
    {
        printf("testOpusWrite %d bytes to fd: %d\n", 
               bytesWritten, fileHandleObject->mWriteFd);
    }

    CPPUNIT_ASSERT_EQUAL(0, result);
    return(result);
}

static int testOpusCloseFile(void* fileInfo)
{
    int result = 1;
    CPPUNIT_ASSERT(fileInfo);
    struct SipxTestOpusWriteFile* fileHandleObject = (struct SipxTestOpusWriteFile*) fileInfo;
    if(fileHandleObject && fileHandleObject->mWriteFd)
    {
        result = close(fileHandleObject->mWriteFd);
        CPPUNIT_ASSERT_EQUAL(0, result);
    }

    return(result);
}

static const OpusEncCallbacks testOpusFileCallbackss =
{
    testOpusWrite,
    testOpusCloseFile
};

int openOpusFileForRecord(const char* filename, 
                          int32_t sampleRate, 
                          int channels, 
                          UtlBoolean append, 
                          const char* artist, 
                          const char* title,
                          OggOpusComments*& freeWhenDoneComments,
                          OggOpusEnc** encoder)
{
    int error = 0;

    // May need to change this to 1 or 255
    // 0 = mono or sterio
    // 1 = 1-255 channels using Vorbis channel mappings
    // 255 = 1-255 channels not using any particular channel mappings
    int family = 0;
    *encoder = NULL;
    struct SipxTestOpusWriteFile* fileObj = (struct SipxTestOpusWriteFile*) malloc(sizeof(*fileObj));
    fileObj->mWriteFd = -1;
    OggOpusComments* comments = ope_comments_create();
    ope_comments_add(comments, "ARTIST", artist);
    ope_comments_add(comments, "TITLE", title);

    int recordFileFd =  open(filename, O_BINARY | O_CREAT | O_RDWR |
                       // If append requested open with append mode
                       (append ?
                        0 /* O_APPEND */: // cannot use APPEND mode as when we try to
                                          // re-write the wave file header in the beginning
                                          // it ends up appending the writes at the end of
                                          // of the file instead in the beginning where it
                                          // was positioned to via lseek (on Linux anyway)
                        O_TRUNC),
                       0640);
    if(recordFileFd < 0)
    {
        OsSysLog::add(FAC_MP, PRI_ERR,
                      "MprRecorder::startFile() failed to open file %s, error code is %i",
                      filename, errno);
    }
    else
    {
        fileObj->mWriteFd = recordFileFd;
        OsSysLog::add(FAC_MP, PRI_ERR,
                      "MprRecorder::startFile() opened file %s as fd: %d",
                      filename, recordFileFd);

        *encoder = ope_encoder_create_callbacks(&testOpusFileCallbackss, fileObj, comments, sampleRate, channels, family, &error);
        if(error)
        {
            if(encoder) 
            {
                ope_encoder_destroy(*encoder);
                *encoder = NULL;
            }
        }
        else
        {
            printf("created encoder: %p error: %d\n", encoder, error);
            // These are safely attached to encoder.  So don't free them up until done encoding
            fileObj = NULL;
            freeWhenDoneComments = comments;
            comments = NULL;
        }

    }

    // Clean up in case of error
    if(fileObj)
    {
        testOpusCloseFile(fileObj);
        free(fileObj);
    }
    if(comments) ope_comments_destroy(comments);

    return(error);
}
#endif

class MprRecorderTest : public MpGenericResourceTest
{
    CPPUNIT_TEST_SUITE(MprRecorderTest);
#ifdef OPUS_FILE_RECORD_ENABLED
    CPPUNIT_TEST(testConvertPcmToOpusFile);
#endif
    CPPUNIT_TEST(testRecordToBadFile);
    CPPUNIT_TEST(testRecordToFile);
    CPPUNIT_TEST(testRecordToFileAppendNotExisting);
    CPPUNIT_TEST(testRecordToFileAppend);
    CPPUNIT_TEST(testRecordChannelToFileAppend);
    CPPUNIT_TEST(testRecordToPauseResumeFile);
    CPPUNIT_TEST_SUITE_END();

    long getFileSize(const UtlString recordFileName)
    {
        unsigned long fileSize = -1;
        OsFile file(recordFileName);
        OsFileInfo fileInfo;
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             file.getFileInfo(fileInfo));
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             fileInfo.getSize(fileSize));
        return(fileSize);
    }

    void validateFileSize(const UtlString recordFilename, long headerSize, long audioDataSize, long fileSizeSlop, const UtlString loopLabel)
    {
        long recordedFileSize = getFileSize(recordFilename);
        long sizeDiff = headerSize + audioDataSize - recordedFileSize;
        CPPUNIT_ASSERT_MESSAGE(loopLabel.data(), 
                               abs(sizeDiff) <=
                               abs(fileSizeSlop));
        if(headerSize + audioDataSize - recordedFileSize < -fileSizeSlop)
        {
            printf("headerSize: %d data size: %d total: %d file size: %d diff: %d slop: %d\n",
                   (int)headerSize,
                   (int)audioDataSize,
                   (int)(headerSize + audioDataSize),
                   (int)recordedFileSize,
                   (int)sizeDiff,
                   (int)fileSizeSlop);
        }
    }

    bool getFileExtension(int fileFormat, UtlString& recordFileExtension)
    {
        bool foundFileType = TRUE;
        switch(fileFormat)
        {
        case MprRecorder::RAW_PCM_16:
            recordFileExtension = "raw";
        break;

        case MprRecorder::WAV_PCM_16:
        case MprRecorder::WAV_ALAW:
        case MprRecorder::WAV_MULAW:
        case MprRecorder::WAV_GSM:
            recordFileExtension = "wav";
        break;

#ifdef OPUS_FILE_RECORD_ENABLED
        case MprRecorder::OGG_OPUS:
            recordFileExtension = "opus";
        break;
#endif
        default:
            foundFileType = FALSE;
            recordFileExtension = "unknown";
            CPPUNIT_ASSERT_MESSAGE("Unexpected recording format type", 0);
        break;
        }

    return(foundFileType);
    }

    bool getFileSizeParameters(const int fileFormat, const int numChannels, const int samplesRecorded, const int framesToProcess, const int framesPerSecond, const int sampleRate,
                               unsigned long& headerSize, unsigned long& audioDataSize, unsigned long& fileSizeSlop)
    {
        bool foundFileType = TRUE;
        headerSize = 0;
        audioDataSize = 0;
        fileSizeSlop = 0;

        switch(fileFormat)
        {
            case MprRecorder::WAV_ALAW:
            case MprRecorder::WAV_MULAW:
                headerSize = 44;
                // Always 8000 samples/second
                audioDataSize = samplesRecorded * 8000 / sampleRate * numChannels;
                break;

            case MprRecorder::WAV_PCM_16:
                headerSize = 44;
            case MprRecorder::RAW_PCM_16:
                audioDataSize = samplesRecorded *
                                sizeof(MpAudioSample) * // bytes per sample
                                numChannels;
            break;

            case MprRecorder::WAV_GSM:
                headerSize = 60;
#if 0
                        headerSize = 60;
                        // Frames (20 ms) for GSM in wave files alternate between 32 and 33 bytes.  Hense 65 for
                        // every 2 GSM frames
                        // Recorder should always record even number of frames so that
                        // there is no problem appending.  So we add one gsm frame if we
                        // had an odd number.
                        {
                            int incompleteFrames = framesToProcess % 4;
                            //printf("Extra GSM frames: %d\n", incompleteFrames);
                            // If there is only one extra media frame, we have 1/2 of an incomplete GSM frame
                            // which is not written, but there was an even number of GSM frames.
                            // This assumes 10 ms media frames, 20 ms GSM frames.  Even number of GSM frames
                            // is a multiple of 4 media frames.
                            audioDataSize = (framesToProcess +
                                             (incompleteFrames == 1 ? -1 : 0) +
                                             (incompleteFrames > 1 ? (4 - incompleteFrames) : 0)) * 65 / 4;
                        }
                        CPPUNIT_ASSERT_EQUAL(framesPerSecond, 100);
#endif
                // Frames for GSM in wave files alternate between 32 and 33 bytes.  Hense 65 for
                // every 2 GSM frames
                // This assumes 10 mSec frames
                audioDataSize = framesToProcess / 4 * 65;
                CPPUNIT_ASSERT_EQUAL(framesPerSecond, 100);
                if(framesToProcess % 4)
                {
                    audioDataSize += 65;
                }
            break;

#ifdef OPUS_FILE_RECORD_ENABLED
            case MprRecorder::OGG_OPUS:
                // Warning: Opus header and frame size is not constant or predictable.
                // Changing flowgraph frame size and rate will change the following.
                headerSize = 841;
                {
                    int opusFrames = framesToProcess / framesPerSecond; // Opus frame/page is 1 second
                    // An Opus frame/page (1 sec of audio) is about 7656 bytes for 1 channel 48000 SPS.
                    audioDataSize = 7656 * opusFrames * numChannels;
                    fileSizeSlop = opusFrames * 80 * numChannels;
                }
            break;
#endif

            default:
                foundFileType = FALSE;
                CPPUNIT_ASSERT(0);  // Unsupported record file format type
            break;
        }

        return(foundFileType);
    }

#ifdef OPUS_FILE_RECORD_ENABLED

    void testConvertPcmToOpusFile()
    {
        const char* inFilename = "Rec_60hz_2ch_48k.raw";
        const char* opusFilename = "Rec_60hz_2ch_48k.opus";
        const int sampleRate = 48000;
        const int channels = 2;

        int inFd =  open(inFilename, O_BINARY | O_RDONLY); 
        CPPUNIT_ASSERT(inFd >= 0);
        printf("Opened %s as fd: %d\n", inFilename, inFd);

        // 60 millisecond frame
        MpAudioSample audioBuffer[channels * sampleRate * 60 / 1000];

        UtlBoolean append = 0;
        //const char* artist = "foo";
        const char* artist = "Someone";
        //const char* title = "foos fallies";
        const char* title = "Some track";
        OggOpusComments* comments = NULL;
        OggOpusEnc* encoder = NULL;
        int result = openOpusFileForRecord(opusFilename, sampleRate, channels, append, artist, title, comments, &encoder);
        CPPUNIT_ASSERT_EQUAL(0, result);
        CPPUNIT_ASSERT(encoder);
        CPPUNIT_ASSERT(comments);

        int bytesRead = 0;
        // Loop through chunks of audio
        do
        {
            bytesRead = read(inFd, audioBuffer, sizeof(audioBuffer));
            //printf("Read %d bytes from fd: %d\n", bytesRead, inFd);
            if(bytesRead > 0)
            {
                result = ope_encoder_write(encoder, audioBuffer, bytesRead / (channels * sizeof(MpAudioSample)));
                CPPUNIT_ASSERT_EQUAL(0, result);
            }
            else
            {
                UtlString errMsg;
                errMsg.appendFormat("read from input raw file failed from fd: %d", inFd);
                perror(errMsg.data());
            }
        }
        while(bytesRead > 0);


        // Close files
        result = ope_encoder_drain(encoder);
        CPPUNIT_ASSERT_EQUAL(0, result);

        ope_encoder_destroy(encoder);
        encoder = NULL;
        ope_comments_destroy(comments);
        comments = NULL;
        close(inFd);


        // Test header for new Opus file to determine that it is valid and what we expect
        OpusHead opusHeader;
        int opusFileReadFd =  open(opusFilename, O_BINARY | O_RDONLY); 
        CPPUNIT_ASSERT(opusFileReadFd >= 0);
        OsStatus status = MprRecorder::validateOpusHeader(opusFileReadFd, opusHeader);
        CPPUNIT_ASSERT_EQUAL(status, OS_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(channels, opusHeader.channel_count);
        CPPUNIT_ASSERT_EQUAL(sampleRate, opusHeader.input_sample_rate);
        CPPUNIT_ASSERT_EQUAL(1, opusHeader.version && 0x0f);
        CPPUNIT_ASSERT_EQUAL(312, opusHeader.pre_skip);
        // May need to change this to 1 or 255
        CPPUNIT_ASSERT_EQUAL(0, opusHeader.mapping_family);
        printf("Stream count: %d\n", opusHeader.stream_count);
        printf("Coupled count: %d\n", opusHeader.coupled_count);

    }
#endif

    void testRecordToBadFile()
    {
        UtlString recorderResourceName = "MprRecorder";
        MprRecorder* recorder = new MprRecorder(recorderResourceName);
        CPPUNIT_ASSERT(recorder);

        // Build flowgraph with source, MprRecorder and sink resources
        setupFramework(recorder);

        // Add the notifier so that we get resource events
        OsMsgQ resourceEventQueue;
        OsMsgDispatcher messageDispatcher(&resourceEventQueue);
        mpFlowGraph->setNotificationDispatcher(&messageDispatcher);

        // Start recording
        CPPUNIT_ASSERT_EQUAL(OS_FILE_ACCESS_DENIED,
                             MprRecorder::startFile(recorderResourceName,
                                                    *mpFlowGraph->getMsgQ(),
                                                    "bogus/file/path/ddd",
                                                    testFileTypes[0]));


        // Enable the source resource and the recorder
        CPPUNIT_ASSERT(mpSourceResource->enable());
        CPPUNIT_ASSERT(recorder->enable());

        OsStatus frameStatus = mpFlowGraph->processNextFrame();
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, frameStatus);

        // Should be a failed to start message
        CPPUNIT_ASSERT_EQUAL(0, messageDispatcher.numMsgs());

        OsMsg* pMsg = NULL;
        while(messageDispatcher.numMsgs())
        {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, messageDispatcher.receive(pMsg, OsTime(0)));
            CPPUNIT_ASSERT(pMsg);
            MpResNotificationMsg* pNotif = (MpResNotificationMsg*) pMsg;

            switch(pNotif->getMsg())
            {
                default:
                    CPPUNIT_ASSERT(0);
                    printf("message type: %d from resource: %s\n",
                           pNotif->getMsg(),
                           pNotif->getOriginatingResourceName().data());
                    break;
            }
        }
    }

    void testRecordToFile()
    {
        int numberOfTestFileTypes = sizeof(testFileTypes) / sizeof(MprRecorder::RecordFileFormat);
        int framesPerSecond = 100; // 10 mSec frames


        for(int fileTypeIndex = 0; fileTypeIndex < numberOfTestFileTypes; fileTypeIndex++)
        {
            MprRecorder::RecordFileFormat fileFormat = testFileTypes[fileTypeIndex];
            unsigned int rateIndex;
            for(rateIndex = 0; rateIndex < sNumRates; rateIndex++)
            {
                UtlString loopLabel;
                loopLabel.appendFormat("%s(%d) rate: %d",
                                       testFileTypeStrings[fileTypeIndex],
                                       testFileTypes[fileTypeIndex],
                                       sSampleRates[rateIndex]);

                printf("Test MprRecorder file type: %d media task rate: %d samples/second\n",
                        fileFormat,
                       sSampleRates[rateIndex]);

                UtlString recordFilename;
                UtlString recordFileExtension;
                getFileExtension(fileFormat, recordFileExtension);

                recordFilename.appendFormat("testRecordToFile%d_%d.%s",
                                            sSampleRates[rateIndex],
                                            fileFormat,
                                            recordFileExtension.data());

                // Incase prior test left junk around
                tearDown();

                // Set media sample rate
                setSamplesPerSec(sSampleRates[rateIndex]);
                setSamplesPerFrame(sSampleRates[rateIndex]/framesPerSecond);
                setUp();

                int framesToProcess = 500; // 5 seconds
                UtlString recorderResourceName = "MprRecorder";
                MprRecorder* recorder = new MprRecorder(recorderResourceName);
                CPPUNIT_ASSERT(recorder);

                // Build flowgraph with source, MprRecorder and sink resources
                setupFramework(recorder);

                // Set parameters to signal to record
                for(int inputIndex = 0; inputIndex < recorder->maxInputs(); inputIndex++)
                {
                    mpSourceResource->setSignalAmplitude(inputIndex, 
                                                      (0x1 << (12 - inputIndex)));
                    mpSourceResource->setSignalPeriod(inputIndex, 
                          sSampleRates[rateIndex] / (250 * (0x1 << inputIndex)));
                }
                mpSourceResource->setOutSignalType(MpTestResource::MP_SINE);

                // Add the notifier so that we get resource events
                OsMsgQ resourceEventQueue;
                OsMsgDispatcher messageDispatcher(&resourceEventQueue);
                mpFlowGraph->setNotificationDispatcher(&messageDispatcher);

                // Start recording
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                     MprRecorder::startFile(recorderResourceName,
                                                            *mpFlowGraph->getMsgQ(),
                                                            recordFilename,
                                                            fileFormat));


                // Enable the source resource and the recorder
                CPPUNIT_ASSERT(mpSourceResource->enable());
                CPPUNIT_ASSERT(recorder->enable());

                // Process the frames
                OsStatus frameStatus;
                for(int frameIndex = 0; frameIndex < framesToProcess; frameIndex++)
                {
                    OsStatus frameStatus = mpFlowGraph->processNextFrame();
                    CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, frameStatus);

                    if(frameIndex == 0)
                    {
                        // Should be a start message
                        CPPUNIT_ASSERT_EQUAL(1, messageDispatcher.numMsgs());
                    }

                }

                int samplesRecorded = 
                    sSampleRates[rateIndex] / framesPerSecond * // samples/frame
                    framesToProcess; // frames

                // Send message to stop the recording
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                     MprRecorder::stop(recorderResourceName,
                                                       *mpFlowGraph->getMsgQ()));
                // Should be a start message
                CPPUNIT_ASSERT_EQUAL(1, messageDispatcher.numMsgs());

                // Process one more frame to be sure recording stop message is handled
                frameStatus = mpFlowGraph->processNextFrame();
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, frameStatus);

                // Start and stop notifications
                CPPUNIT_ASSERT_EQUAL(2, messageDispatcher.numMsgs());

                // Get start notification
                OsTime notificationWait(0, 1000);
                OsMsg* messagePtr = NULL;
                messageDispatcher.receive(messagePtr, notificationWait);
                CPPUNIT_ASSERT(messagePtr);
                if(messagePtr)
                {
                    CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgType(), OsMsg::MP_RES_NOTF_MSG);
                    CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgSubType(), MpResNotificationMsg::MPRNM_RECORDER_STARTED);
                }

                // Get stop notification and verify the number of samples recorded
                messageDispatcher.receive(messagePtr, notificationWait);
                CPPUNIT_ASSERT(messagePtr);
                if(messagePtr)
                {
                    CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgType(), OsMsg::MP_RES_NOTF_MSG);
                    CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgSubType(), MpResNotificationMsg::MPRNM_RECORDER_STOPPED);
                    MprnIntMsg* stopMessage = (MprnIntMsg*) messagePtr;
                    CPPUNIT_ASSERT_EQUAL(stopMessage->getValue(), samplesRecorded);
                }

                // Make sure the recorded file is the correct size
                unsigned long headerSize = 0;
                unsigned long audioDataSize = 0;
                unsigned long fileSizeSlop = 0;
                getFileSizeParameters(fileFormat, 1, /*channel*/ samplesRecorded, framesToProcess, framesPerSecond, sSampleRates[rateIndex],
                                      headerSize, audioDataSize, fileSizeSlop);

                validateFileSize(recordFilename, headerSize, audioDataSize, fileSizeSlop, loopLabel);

                // Stop flowgraph
                haltFramework();

            } // end for iteration over sample rates

        }  // end for iteration over file formats

    } // end testRecordToFile method

    void testRecordToFileAppendNotExisting()
    {
        int numberOfTestFileTypes = sizeof(testFileTypes) / sizeof(MprRecorder::RecordFileFormat);
        int framesPerSecond = 100; // 10 mSec frames


        for(int fileTypeIndex = 0; fileTypeIndex < numberOfTestFileTypes; fileTypeIndex++)
        {
            MprRecorder::RecordFileFormat fileFormat = testFileTypes[fileTypeIndex];
            unsigned int rateIndex;
            for(rateIndex = 0; rateIndex < sNumRates; rateIndex++)
            {
                UtlString loopLabel;
                loopLabel.appendFormat("%s(%d) rate: %d",
                                       testFileTypeStrings[fileTypeIndex],
                                       testFileTypes[fileTypeIndex],
                                       sSampleRates[rateIndex]);

                printf("Test MprRecorder file type: %d media task rate: %d samples/second\n",
                        fileFormat,
                       sSampleRates[rateIndex]);

                UtlString recordFilename;
                UtlString recordFileExtension;
                getFileExtension(fileFormat, recordFileExtension);
                recordFilename.appendFormat("testRecordToFile%d_%d.%s",
                                            sSampleRates[rateIndex],
                                            fileFormat,
                                            recordFileExtension.data());
                // Incase prior test left junk around
                tearDown();

                // Make sure file never exists so we can test that append is safe in
                // that case.
                // Scope to be sure no conflict/copy paste errors with similar code below
                // and file is closed.
                {
                    OsFile recordFile(recordFilename);
                    recordFile.remove(TRUE);
                    CPPUNIT_ASSERT(!recordFile.exists());
                }

                // Set media sample rate
                setSamplesPerSec(sSampleRates[rateIndex]);
                setSamplesPerFrame(sSampleRates[rateIndex]/framesPerSecond);
                setUp();

                int framesToProcess = 500; // 5 seconds
                UtlString recorderResourceName = "MprRecorder";
                MprRecorder* recorder = new MprRecorder(recorderResourceName);
                CPPUNIT_ASSERT(recorder);

                // Build flowgraph with source, MprRecorder and sink resources
                setupFramework(recorder);

                // Set parameters to signal to record
                for(int inputIndex = 0; inputIndex < recorder->maxInputs(); inputIndex++)
                {
                    mpSourceResource->setSignalAmplitude(inputIndex, 
                                                      (0x1 << (12 - inputIndex)));
                    mpSourceResource->setSignalPeriod(inputIndex, 
                          sSampleRates[rateIndex] / (250 * (0x1 << inputIndex)));
                }

                mpSourceResource->setOutSignalType(MpTestResource::MP_SINE);

                // Add the notifier so that we get resource events
                OsMsgQ resourceEventQueue;
                OsMsgDispatcher messageDispatcher(&resourceEventQueue);
                mpFlowGraph->setNotificationDispatcher(&messageDispatcher);

                // Start recording
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                     MprRecorder::startFile(recorderResourceName,
                                                            *mpFlowGraph->getMsgQ(),
                                                            recordFilename,
                                                            fileFormat,
                                                            0, // max record length = unlimited
                                                            -1, // don't check/stop after silence
                                                            TRUE)); // append = TRUE


                // Enable the source resource and the recorder
                CPPUNIT_ASSERT(mpSourceResource->enable());
                CPPUNIT_ASSERT(recorder->enable());

                // Process the frames
                OsStatus frameStatus;
                for(int frameIndex = 0; frameIndex < framesToProcess; frameIndex++)
                {
                    OsStatus frameStatus = mpFlowGraph->processNextFrame();
                    CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, frameStatus);

                    if(frameIndex == 0)
                    {
                        // Should be a start message
                        CPPUNIT_ASSERT_EQUAL(1, messageDispatcher.numMsgs());
                    }

                }

                int samplesRecorded = 
                    sSampleRates[rateIndex] / framesPerSecond * // samples/frame
                    framesToProcess; // frames

                // Send message to stop the recording
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                     MprRecorder::stop(recorderResourceName,
                                                       *mpFlowGraph->getMsgQ()));
                // Should be a start message
                CPPUNIT_ASSERT_EQUAL(1, messageDispatcher.numMsgs());

                // Process one more frame to be sure recording stop message is handled
                frameStatus = mpFlowGraph->processNextFrame();
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, frameStatus);

                // Start and stop notifications
                CPPUNIT_ASSERT_EQUAL(2, messageDispatcher.numMsgs());

                // Get start notification
                OsTime notificationWait(0, 1000);
                OsMsg* messagePtr = NULL;
                messageDispatcher.receive(messagePtr, notificationWait);
                CPPUNIT_ASSERT(messagePtr);
                if(messagePtr)
                {
                    CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgType(), OsMsg::MP_RES_NOTF_MSG);
                    CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgSubType(), MpResNotificationMsg::MPRNM_RECORDER_STARTED);
                }

                // Get stop notification and verify the number of samples recorded
                messageDispatcher.receive(messagePtr, notificationWait);
                CPPUNIT_ASSERT(messagePtr);
                if(messagePtr)
                {
                    CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgType(), OsMsg::MP_RES_NOTF_MSG);
                    CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgSubType(), MpResNotificationMsg::MPRNM_RECORDER_STOPPED);
                    MprnIntMsg* stopMessage = (MprnIntMsg*) messagePtr;
                    CPPUNIT_ASSERT_EQUAL(stopMessage->getValue(), samplesRecorded);
                }

                // Make sure the recorded file is the correct size
                unsigned long headerSize = 0;
                unsigned long audioDataSize = 0;
                unsigned long fileSizeSlop = 0;
                getFileSizeParameters(fileFormat, 1, /*channel*/ samplesRecorded, framesToProcess, framesPerSecond, sSampleRates[rateIndex],
                                      headerSize, audioDataSize, fileSizeSlop);

                validateFileSize(recordFilename, headerSize, audioDataSize, fileSizeSlop, loopLabel);

                // Stop flowgraph
                haltFramework();

            } // end for iteration over sample rates

        }  // end for iteration over file formats

    } // end testRecordToFileAppendNotExisting method

    void testRecordToFileAppend()
    {
        int numberOfTestFileTypes = sizeof(testFileTypes) / sizeof(MprRecorder::RecordFileFormat);
        int framesPerSecond = 100; // 10 mSec frames


        for(int fileTypeIndex = 0; fileTypeIndex < numberOfTestFileTypes; fileTypeIndex++)
        {
            MprRecorder::RecordFileFormat fileFormat = testFileTypes[fileTypeIndex];
            unsigned int rateIndex;
            for(rateIndex = 0; rateIndex < sNumRates; rateIndex++)
            {
                UtlString loopLabel;
                loopLabel.appendFormat("%s(%d) rate: %d",
                                       testFileTypeStrings[fileTypeIndex],
                                       testFileTypes[fileTypeIndex],
                                       sSampleRates[rateIndex]);

                printf("Test MprRecorder file type: %d media task rate: %d samples/second\n",
                        fileFormat,
                       sSampleRates[rateIndex]);

                UtlString recordFilename;
                UtlString recordFileExtension;
                getFileExtension(fileFormat, recordFileExtension);
                recordFilename.appendFormat("testRecordToFileAppend%d_%d.%s",
                                            sSampleRates[rateIndex],
                                            fileFormat,
                                            recordFileExtension.data());

                // Incase prior test left junk around
                tearDown();

                // Set media sample rate
                setSamplesPerSec(sSampleRates[rateIndex]);
                setSamplesPerFrame(sSampleRates[rateIndex]/framesPerSecond);
                setUp();

                // SHould use multiple of 2 as some codecs (e.g. GSM) have frame sizes of 20 msec.
                // In which case the encoder will not spit out half a frame.
                int framesToProcess = 503; // 5+ seconds
                UtlString recorderResourceName = "MprRecorder";
                MprRecorder* recorder = new MprRecorder(recorderResourceName);
                CPPUNIT_ASSERT(recorder);

                // Build flowgraph with source, MprRecorder and sink resources
                setupFramework(recorder);
                mpSourceResource->setSignalAmplitude(0, (0x1 << 15));
                mpSourceResource->setOutSignalType(MpTestResource::MP_SINE_SAW);


                // Add the notifier so that we get resource events
                OsMsgQ resourceEventQueue;
                OsMsgDispatcher messageDispatcher(&resourceEventQueue);
                mpFlowGraph->setNotificationDispatcher(&messageDispatcher);

                // Start recording WITHOUT append option
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                     MprRecorder::startFile(recorderResourceName,
                                                            *mpFlowGraph->getMsgQ(),
                                                            recordFilename,
                                                            fileFormat));


                // Enable the source resource and the recorder
                CPPUNIT_ASSERT(mpSourceResource->enable());
                CPPUNIT_ASSERT(recorder->enable());

                // Process the frames
                OsStatus frameStatus;
                for(int frameIndex = 0; frameIndex < framesToProcess; frameIndex++)
                {
                    OsStatus frameStatus = mpFlowGraph->processNextFrame();
                    CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, frameStatus);

                    if(frameIndex == 0)
                    {
                        // Should be a start message
                        CPPUNIT_ASSERT_EQUAL(1, messageDispatcher.numMsgs());
                    }

                }

                int samplesRecorded = 
                    sSampleRates[rateIndex] / framesPerSecond * // samples/frame
                    framesToProcess; // frames

                // Send message to stop the recording
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                     MprRecorder::stop(recorderResourceName,
                                                       *mpFlowGraph->getMsgQ()));
                // Should be a start message
                CPPUNIT_ASSERT_EQUAL(1, messageDispatcher.numMsgs());

                // Process one more frame to be sure recording stop message is handled
                frameStatus = mpFlowGraph->processNextFrame();
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, frameStatus);

                OsSysLog::add(FAC_MP, PRI_DEBUG,
                              "MprRecorderTest::testRecordToFileAppend stop 1st segment recording");

                // Start and stop notifications
                CPPUNIT_ASSERT_EQUAL(2, messageDispatcher.numMsgs());

                // Get start notification
                OsTime notificationWait(0, 1000);
                OsMsg* messagePtr = NULL;
                messageDispatcher.receive(messagePtr, notificationWait);
                CPPUNIT_ASSERT(messagePtr);
                if(messagePtr)
                {
                    CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgType(), OsMsg::MP_RES_NOTF_MSG);
                    CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgSubType(), MpResNotificationMsg::MPRNM_RECORDER_STARTED);
                }

                // Get stop notification and verify the number of samples recorded
                messagePtr = NULL;
                messageDispatcher.receive(messagePtr, notificationWait);
                CPPUNIT_ASSERT(messagePtr);
                if(messagePtr)
                {
                    CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgType(), OsMsg::MP_RES_NOTF_MSG);
                    CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgSubType(), MpResNotificationMsg::MPRNM_RECORDER_STOPPED);
                    MprnIntMsg* stopMessage = (MprnIntMsg*) messagePtr;
                    CPPUNIT_ASSERT_EQUAL(stopMessage->getValue(), samplesRecorded);
                }

                // Make sure the recorded file is the correct size
                unsigned long headerSize = 0;
                unsigned long audioDataSize = 0;
                unsigned long fileSizeSlop = 0;
                getFileSizeParameters(fileFormat, 1, /*channel*/ samplesRecorded, framesToProcess, framesPerSecond, sSampleRates[rateIndex],
                                      headerSize, audioDataSize, fileSizeSlop);

                validateFileSize(recordFilename, headerSize, audioDataSize, fileSizeSlop, loopLabel);

                mpSourceResource->setSignalAmplitude(0, (0x1 << 13));

                // Now append more audio to the end of the recording
                for(int appendFileTypeIndex = 0; appendFileTypeIndex < numberOfTestFileTypes; appendFileTypeIndex++)
                {
                    MprRecorder::RecordFileFormat appendFileFormat = testFileTypes[appendFileTypeIndex];

                    OsSysLog::add(FAC_MP, PRI_DEBUG,
                                  "MprRecorderTest::testRecordToFileAppend startFile 2nd segment recording append format: %d",
                                  appendFileFormat);

                    // Start recording WITH append option
                    OsStatus appendStatus =  MprRecorder::startFile(recorderResourceName,
                                                                    *mpFlowGraph->getMsgQ(),
                                                                    recordFilename,
                                                                    appendFileFormat,
                                                                    0, // max record length = unlimited
                                                                    -1, // don't check/stop after silence
                                                                    TRUE); // append = TRUE
                    // File of type fileType already exists.  If the append type is not
                    // the same, it should fail.
                    if(appendFileFormat != fileFormat)
                    {
                        UtlString fileError;
                        fileError.appendFormat("file: %s append of format: %d failed with status: %d",
                                                   recordFilename.data(), appendFileFormat, appendStatus);

                        // Currently, opening any file to record raw format does not fail
                        // So we must close the recording.
                        if(appendFileFormat == MprRecorder::RAW_PCM_16)
                        {
                            CPPUNIT_ASSERT_EQUAL_MESSAGE(fileError.data(), OS_SUCCESS, appendStatus);
                            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                                 MprRecorder::stop(recorderResourceName,
                                                                   *mpFlowGraph->getMsgQ()));
                        }

                        // This is a failure or negative test case and we need to clear out any messages
                        // or they will be in the queue for success cases when we verify
                        // the message types.
                        int droppedMessages = 0;
                        do
                        {
                            // Have to turn the crank and process a frame to get a 
                            // message
                            OsStatus frameStatus = mpFlowGraph->processNextFrame();
                            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, frameStatus);
                            messagePtr = NULL;
                            messageDispatcher.receive(messagePtr, notificationWait);
                            if(messagePtr != NULL) droppedMessages++;
                        } while(messagePtr != NULL);
                        if(droppedMessages)
                        {
                            printf("Append different file types %d %d, dropping %d messages\n",
                                   appendFileFormat,
                                   fileFormat,
                                   droppedMessages);
                        }


                        // TODO: raw file format could check and confirm that the existing file is
                        // not wav or ogg file type by looking at the header
                        if(appendFileFormat != MprRecorder::RAW_PCM_16)
                        {
                            CPPUNIT_ASSERT_EQUAL_MESSAGE(fileError.data(), OS_FAILED, appendStatus);
                            CPPUNIT_ASSERT_EQUAL_MESSAGE(fileError.data(), 0, droppedMessages);
                        }
#ifdef TEST_PRINT
                        // Variable scope
                        {
                            long fileSize = getFileSize(recordFilename);
                            printf("file: %s size: %d\n", recordFilename.data(), (int)fileSize);
                        }
#endif

                    }

                    // Same format as original file, should succeed
                    else
                    {
                        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, appendStatus);

                        // Record some more by Processing the frames
                        OsStatus frameStatus;
                        for(int frameIndex = 0; frameIndex < framesToProcess; frameIndex++)
                        {
                            OsStatus frameStatus = mpFlowGraph->processNextFrame();
                            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, frameStatus);

                            if(frameIndex == 0)
                            {
                                // Should be a start message
                                CPPUNIT_ASSERT_EQUAL(1, messageDispatcher.numMsgs());
                            }

#ifdef TEST_PRINT
                            if(frameIndex % 100 == 0 || frameIndex == framesToProcess - 1)
                            {
                                long fileSize = getFileSize(recordFilename);
                                printf("frame: %d file: %s size: %d\n", frameIndex, recordFilename.data(), (int)fileSize);
                            }
#endif
                        }

                        // Send message to stop the recording
                        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                             MprRecorder::stop(recorderResourceName,
                                                               *mpFlowGraph->getMsgQ()));
                        // Should be a start message
                        CPPUNIT_ASSERT_EQUAL(1, messageDispatcher.numMsgs());

                        // Process one more frame to be sure recording stop message is handled
                        frameStatus = mpFlowGraph->processNextFrame();
                        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, frameStatus);

                        OsSysLog::add(FAC_MP, PRI_DEBUG,
                                      "MprRecorderTest::testRecordToFileAppend stop 2nd segment recording");

                        // Start and stop notifications
                        CPPUNIT_ASSERT_EQUAL(2, messageDispatcher.numMsgs());

                        // Get start notification
                        messagePtr = NULL;
                        messageDispatcher.receive(messagePtr, notificationWait);
                        CPPUNIT_ASSERT(messagePtr);
                        if(messagePtr)
                        {
                            CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgType(), OsMsg::MP_RES_NOTF_MSG);
                            CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgSubType(), MpResNotificationMsg::MPRNM_RECORDER_STARTED);
                        }

                        // Get stop notification and verify the number of samples recorded
                        messagePtr = NULL;
                        messageDispatcher.receive(messagePtr, notificationWait);
                        CPPUNIT_ASSERT(messagePtr);
                        if(messagePtr)
                        {
                            CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgType(), OsMsg::MP_RES_NOTF_MSG);
                            CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgSubType(), MpResNotificationMsg::MPRNM_RECORDER_STOPPED);
                            MprnIntMsg* stopMessage = (MprnIntMsg*) messagePtr;
                            CPPUNIT_ASSERT_EQUAL(stopMessage->getValue(), samplesRecorded);
                        }

                        // Verify the file size
                        audioDataSize += audioDataSize;

                        // Each OGG append has an additional header and slop increases with number of frames
                        if(fileFormat == MprRecorder::OGG_OPUS)
                        {
                            headerSize += headerSize;
                            fileSizeSlop += fileSizeSlop;
                        }

                        validateFileSize(recordFilename, headerSize, audioDataSize, fileSizeSlop, loopLabel);

                        OsSysLog::add(FAC_MP, PRI_DEBUG,
                                      "MprRecorderTest::testRecordToFileAppend startFile 3nd segment recording append format: %d",
                                      appendFileFormat);

                        // Append a 2nd time.
                        appendStatus =  MprRecorder::startFile(recorderResourceName,
                                                               *mpFlowGraph->getMsgQ(),
                                                               recordFilename,
                                                               appendFileFormat,
                                                               0, // max record length = unlimited
                                                               -1, // don't check/stop after silence
                                                               TRUE); // append = TRUE

                        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, appendStatus);

                        // Record some more by Processing the frames
                        for(int frameIndex = 0; frameIndex < framesToProcess; frameIndex++)
                        {
                            OsStatus frameStatus = mpFlowGraph->processNextFrame();
                            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, frameStatus);

                            if(frameIndex == 0)
                            {
                                // Should be a start message
                                CPPUNIT_ASSERT_EQUAL(1, messageDispatcher.numMsgs());
                            }

#ifdef TEST_PRINT
                            if(frameIndex % 100 == 0 || frameIndex == framesToProcess - 1)
                            {
                                long fileSize = getFileSize(recordFilename);
                                printf("frame: %d file: %s size: %d\n", frameIndex, recordFilename.data(), (int)fileSize);
                            }
#endif
                        }

                        // Should be a start message
                        CPPUNIT_ASSERT_EQUAL(1, messageDispatcher.numMsgs());

                        // Get start notification
                        messagePtr = NULL;
                        messageDispatcher.receive(messagePtr, notificationWait);
                        CPPUNIT_ASSERT(messagePtr);
                        if(messagePtr)
                        {
                            CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgType(), OsMsg::MP_RES_NOTF_MSG);
                            CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgSubType(), MpResNotificationMsg::MPRNM_RECORDER_STARTED);
                        }

                        // Cannot test MprRecord destructor to close the file here.
                        // Have to send message to stop the recording, as we are in a loop and
                        // potentially would leave files open.
                        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                             MprRecorder::stop(recorderResourceName,
                                                               *mpFlowGraph->getMsgQ()));
                        frameStatus = mpFlowGraph->processNextFrame();
                        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, frameStatus);

                        // Should be a stop message
                        CPPUNIT_ASSERT_EQUAL(1, messageDispatcher.numMsgs());

                        // Get stop notification
                        messagePtr = NULL;
                        messageDispatcher.receive(messagePtr, notificationWait);
                        CPPUNIT_ASSERT(messagePtr);
                        if(messagePtr)
                        {
                            CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgType(), OsMsg::MP_RES_NOTF_MSG);
                            CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgSubType(), MpResNotificationMsg::MPRNM_RECORDER_STOPPED);
                        }

                        // Verify the file size
                        audioDataSize += (audioDataSize / 2);

                        // Each OGG append has an additional header
                        if(fileFormat == MprRecorder::OGG_OPUS)
                        {
                            headerSize += (headerSize / 2);
                            fileSizeSlop += (fileSizeSlop / 2);
                        }

                    }
                }

                // Stop flowgraph
                haltFramework();

                // This should force the recorder to close the file.
                tearDown();

                OsSysLog::add(FAC_MP, PRI_DEBUG,
                              "MprRecorderTest::testRecordToFileAppend tearDown");

                validateFileSize(recordFilename, headerSize, audioDataSize, fileSizeSlop, loopLabel);

            } // end for iteration over sample rates

        }  // end for iteration over file formats

    } // end testRecordToFileAppend method

    void testRecordChannelToFileAppend()
    {
        int numberOfTestFileTypes = sizeof(testFileTypes) / sizeof(MprRecorder::RecordFileFormat);
        int framesPerSecond = 100; // 10 mSec frames

        for(int channelCount = 1; channelCount < MAXIMUM_RECORDER_CHANNELS + 1; channelCount++)
        {
            for(int fileTypeIndex = 0; fileTypeIndex < numberOfTestFileTypes; fileTypeIndex++)
            {
                MprRecorder::RecordFileFormat fileFormat = testFileTypes[fileTypeIndex];
                unsigned int rateIndex;
                for(rateIndex = 0; rateIndex < sNumRates; rateIndex++)
                {
                    UtlString loopLabel;
                    loopLabel.appendFormat("%s(%d) rate: %d channels: %d",
                                           testFileTypeStrings[fileTypeIndex],
                                           testFileTypes[fileTypeIndex],
                                           sSampleRates[rateIndex],
                                           channelCount);

                    printf("Test MprRecorder file type: %d channels: %d media task rate: %d samples/second\n",
                           fileFormat,
                           channelCount,
                           sSampleRates[rateIndex]);

                    UtlString recordFilename;
                    UtlString recordFileExtension;
                    CPPUNIT_ASSERT(getFileExtension(fileFormat, recordFileExtension));
                    recordFilename.appendFormat("testRecordChannelToFileAppend%d_ch%d_%d.%s",
                                                sSampleRates[rateIndex],
                                                channelCount,
                                                fileFormat,
                                                recordFileExtension.data());
                    // Incase prior test left junk around
                    tearDown();

                    // Set media sample rate
                    setSamplesPerSec(sSampleRates[rateIndex]);
                    setSamplesPerFrame(sSampleRates[rateIndex]/framesPerSecond);
                    setUp();

                    // SHould use multiple of 2 as some codecs (e.g. GSM) have frame sizes of 20 msec.
                    // In which case the encoder will not spit out half a frame.
                    int framesToProcess = 503; // 5+ seconds
                    UtlString recorderResourceName = "MprRecorder";
                    MprRecorder* recorder = new MprRecorder(recorderResourceName);
                    CPPUNIT_ASSERT(recorder);

                    // Build flowgraph with source, MprRecorder and sink resources
                    setupFramework(recorder);

                    for(int inputIndex = 0; inputIndex < recorder->maxInputs(); inputIndex++)
                    {
                        mpSourceResource->setSignalAmplitude(inputIndex, 
                                                          (0x1 << (12 - inputIndex)));
                        mpSourceResource->setSignalPeriod(inputIndex, 
                              sSampleRates[rateIndex] / (250 * (0x1 << inputIndex)));
                    }

                    mpSourceResource->setOutSignalType(MpTestResource::MP_SINE);


                    // Add the notifier so that we get resource events
                    OsMsgQ resourceEventQueue;
                    OsMsgDispatcher messageDispatcher(&resourceEventQueue);
                    mpFlowGraph->setNotificationDispatcher(&messageDispatcher);

                    // Start recording WITHOUT append option
                    CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                         MprRecorder::startFile(recorderResourceName,
                                                                *mpFlowGraph->getMsgQ(),
                                                                recordFilename,
                                                                fileFormat,
                                                                0, // max record length = unlimited
                                                                -1, // don't check/stop after silence
                                                                FALSE, // don't append
                                                                channelCount)); // number of channels


                    // Enable the source resource and the recorder
                    CPPUNIT_ASSERT(mpSourceResource->enable());
                    CPPUNIT_ASSERT(recorder->enable());

                    // Process the frames
                    OsStatus frameStatus;
                    for(int frameIndex = 0; frameIndex < framesToProcess; frameIndex++)
                    {
                        OsStatus frameStatus = mpFlowGraph->processNextFrame();
                        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, frameStatus);

                        if(frameIndex == 0)
                        {
                            // Should be a start message
                            CPPUNIT_ASSERT_EQUAL(1, messageDispatcher.numMsgs());
                        }

                    }

                    int samplesRecorded = 
                        sSampleRates[rateIndex] / framesPerSecond * // samples/frame
                        framesToProcess; // frames

                    // Send message to stop the recording
                    CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                         MprRecorder::stop(recorderResourceName,
                                                           *mpFlowGraph->getMsgQ()));
                    // Should be a start message
                    CPPUNIT_ASSERT_EQUAL(1, messageDispatcher.numMsgs());

                    // Process one more frame to be sure recording stop message is handled
                    frameStatus = mpFlowGraph->processNextFrame();
                    CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, frameStatus);

                    // Start and stop notifications
                    CPPUNIT_ASSERT_EQUAL(2, messageDispatcher.numMsgs());

                    // Get start notification
                    OsTime notificationWait(0, 1000);
                    OsMsg* messagePtr = NULL;
                    messageDispatcher.receive(messagePtr, notificationWait);
                    CPPUNIT_ASSERT(messagePtr);
                    if(messagePtr)
                    {
                        CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgType(), OsMsg::MP_RES_NOTF_MSG);
                        CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgSubType(), MpResNotificationMsg::MPRNM_RECORDER_STARTED);
                    }

                    // Get stop notification and verify the number of samples recorded
                    messagePtr = NULL;
                    messageDispatcher.receive(messagePtr, notificationWait);
                    CPPUNIT_ASSERT(messagePtr);
                    if(messagePtr)
                    {
                        CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgType(), OsMsg::MP_RES_NOTF_MSG);
                        CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgSubType(), MpResNotificationMsg::MPRNM_RECORDER_STOPPED);
                        MprnIntMsg* stopMessage = (MprnIntMsg*) messagePtr;
                        CPPUNIT_ASSERT_EQUAL(stopMessage->getValue(), samplesRecorded);
                    }

                    // Make sure the recorded file is the correct size
                    unsigned long headerSize = 0;
                    unsigned long audioDataSize = 0;
                    unsigned long fileSizeSlop = 0;
                    getFileSizeParameters(fileFormat, channelCount, samplesRecorded, framesToProcess, framesPerSecond, sSampleRates[rateIndex],
                                          headerSize, audioDataSize, fileSizeSlop);

                    validateFileSize(recordFilename, headerSize, audioDataSize, fileSizeSlop, loopLabel);

                    for(int inputIndex = 0; inputIndex < recorder->maxInputs(); inputIndex++)
                    {
                        mpSourceResource->setSignalAmplitude(inputIndex, 
                                                          (0x1 << (10 - inputIndex)));
                    }

                    // Now append more audio to the end of the recording
                    for(int appendFileTypeIndex = 0; appendFileTypeIndex < numberOfTestFileTypes; appendFileTypeIndex++)
                    {
                        MprRecorder::RecordFileFormat appendFileFormat = testFileTypes[appendFileTypeIndex];

                        // Start recording WITH append option
                        OsStatus appendStatus =  MprRecorder::startFile(recorderResourceName,
                                                                        *mpFlowGraph->getMsgQ(),
                                                                        recordFilename,
                                                                        appendFileFormat,
                                                                        0, // max record length = unlimited
                                                                        -1, // don't check/stop after silence
                                                                        TRUE, // append = TRUE
                                                                        channelCount);
                        // File of type fileType already exists.  If the append type is not
                        // the same, it should fail.
                        if(appendFileFormat != fileFormat)
                        {
                            UtlString fileError;
                            fileError.appendFormat("file: %s startFile record of format: %d failed with status: %d",
                                                       recordFilename.data(), appendFileFormat, appendStatus);

                            // Currently, opening any file to record raw format does not fail
                            // So we must close the recording.
                            if(appendFileFormat == MprRecorder::RAW_PCM_16)
                            {
                                CPPUNIT_ASSERT_EQUAL_MESSAGE(fileError.data(), OS_SUCCESS, appendStatus);
                                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                                     MprRecorder::stop(recorderResourceName,
                                                                       *mpFlowGraph->getMsgQ()));
                            }

                            // This is a failure or negative test case and we need to clear out any messages
                            // or they will be in the queue for success cases when we verify
                            // the message types.
                            int droppedMessages = 0;
                            do
                            {
                                // Have to turn the crank and process a frame to get a 
                                // message
                                OsStatus frameStatus = mpFlowGraph->processNextFrame();
                                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, frameStatus);
                                messagePtr = NULL;
                                messageDispatcher.receive(messagePtr, notificationWait);
                                if(messagePtr != NULL) droppedMessages++;
                            } while(messagePtr != NULL);
                            if(droppedMessages)
                            {
                                printf("Append different file types %d %d, dropping %d messages\n",
                                       appendFileFormat,
                                       fileFormat,
                                       droppedMessages);
                            }


                            // TODO: raw file format could check and confirm that the existing file is
                            // not wav or ogg file type by looking at the header
                            if(appendFileFormat != MprRecorder::RAW_PCM_16)
                            {
                                CPPUNIT_ASSERT_EQUAL_MESSAGE(fileError.data(), OS_FAILED, appendStatus);
                                CPPUNIT_ASSERT_EQUAL_MESSAGE(fileError.data(), 0, droppedMessages);
                            }
#ifdef TEST_PRINT
                            // Variable scope
                            {
                                long fileSize = getFileSize(recordFilename);
                                printf("file: %s size: %d\n", recordFilename.data(), (int)fileSize);
                            }
#endif

                        }

                        // Same format as original file, should succeed
                        else
                        {
                            CPPUNIT_ASSERT_EQUAL_MESSAGE(loopLabel.data(), OS_SUCCESS, appendStatus);

                            // Record some more by Processing the frames
                            OsStatus frameStatus;
                            for(int frameIndex = 0; frameIndex < framesToProcess; frameIndex++)
                            {
                                OsStatus frameStatus = mpFlowGraph->processNextFrame();
                                CPPUNIT_ASSERT_EQUAL_MESSAGE(loopLabel.data(), OS_SUCCESS, frameStatus);

                                if(frameIndex == 0)
                                {
                                    // Should be a start message
                                    CPPUNIT_ASSERT_EQUAL_MESSAGE(loopLabel.data(), 1, messageDispatcher.numMsgs());
                                }

                            }

                            // Send message to stop the recording
                            CPPUNIT_ASSERT_EQUAL_MESSAGE(loopLabel.data(),
                                                         OS_SUCCESS,
                                                         MprRecorder::stop(recorderResourceName,
                                                                           *mpFlowGraph->getMsgQ()));
                            // Should be a start message
                            CPPUNIT_ASSERT_EQUAL_MESSAGE(loopLabel.data(), 1, messageDispatcher.numMsgs());

                            // Process one more frame to be sure recording stop message is handled
                            frameStatus = mpFlowGraph->processNextFrame();
                            CPPUNIT_ASSERT_EQUAL_MESSAGE(loopLabel.data(), OS_SUCCESS, frameStatus);

                            // Start and stop notifications
                            CPPUNIT_ASSERT_EQUAL_MESSAGE(loopLabel.data(), 2, messageDispatcher.numMsgs());

                            // Get start notification
                            messagePtr = NULL;
                            messageDispatcher.receive(messagePtr, notificationWait);
                            CPPUNIT_ASSERT(messagePtr);
                            if(messagePtr)
                            {
                                CPPUNIT_ASSERT_EQUAL_MESSAGE(loopLabel.data(), messagePtr->getMsgType(), OsMsg::MP_RES_NOTF_MSG);
                                CPPUNIT_ASSERT_EQUAL_MESSAGE(loopLabel.data(), messagePtr->getMsgSubType(), MpResNotificationMsg::MPRNM_RECORDER_STARTED);
                            }

                            // Get stop notification and verify the number of samples recorded
                            messagePtr = NULL;
                            messageDispatcher.receive(messagePtr, notificationWait);
                            CPPUNIT_ASSERT(messagePtr);
                            if(messagePtr)
                            {
                                CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgType(), OsMsg::MP_RES_NOTF_MSG);
                                CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgSubType(), MpResNotificationMsg::MPRNM_RECORDER_STOPPED);
                                MprnIntMsg* stopMessage = (MprnIntMsg*) messagePtr;
                                CPPUNIT_ASSERT_EQUAL(stopMessage->getValue(), samplesRecorded);
                            }

                            // Verify the file size
                            audioDataSize += audioDataSize;

                            // Each OGG append has an additional header and slop increases with number of frames
                            if(fileFormat == MprRecorder::OGG_OPUS)
                            {
                                headerSize += headerSize;
                                fileSizeSlop += fileSizeSlop;
                            }

                            validateFileSize(recordFilename, headerSize, audioDataSize, fileSizeSlop, loopLabel);

                            // Append a 2nd time.
                            appendStatus =  MprRecorder::startFile(recorderResourceName,
                                                                   *mpFlowGraph->getMsgQ(),
                                                                   recordFilename,
                                                                   appendFileFormat,
                                                                   0, // max record length = unlimited
                                                                   -1, // don't check/stop after silence
                                                                   TRUE, // append = TRUE
                                                                   channelCount);

                            CPPUNIT_ASSERT_EQUAL_MESSAGE(loopLabel.data(), OS_SUCCESS, appendStatus);

                            // Record some more by Processing the frames
                            for(int frameIndex = 0; frameIndex < framesToProcess; frameIndex++)
                            {
                                OsStatus frameStatus = mpFlowGraph->processNextFrame();
                                CPPUNIT_ASSERT_EQUAL_MESSAGE(loopLabel.data(), OS_SUCCESS, frameStatus);

                                if(frameIndex == 0)
                                {
                                    // Should be a start message
                                    CPPUNIT_ASSERT_EQUAL_MESSAGE(loopLabel.data(), 1, messageDispatcher.numMsgs());
                                }

                            }

                            // Should be a start message
                            CPPUNIT_ASSERT_EQUAL_MESSAGE(loopLabel.data(), 1, messageDispatcher.numMsgs());

                            // Get start notification
                            messagePtr = NULL;
                            messageDispatcher.receive(messagePtr, notificationWait);
                            CPPUNIT_ASSERT(messagePtr);
                            if(messagePtr)
                            {
                                CPPUNIT_ASSERT_EQUAL_MESSAGE(loopLabel.data(), messagePtr->getMsgType(), OsMsg::MP_RES_NOTF_MSG);
                                CPPUNIT_ASSERT_EQUAL_MESSAGE(loopLabel.data(), messagePtr->getMsgSubType(), MpResNotificationMsg::MPRNM_RECORDER_STARTED);
                            }

                            // Cannot test MprRecord destructor to close the file here.
                            // Have to send message to stop the recording, as we are in a loop and
                            // potentially would leave files open.
                            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                                 MprRecorder::stop(recorderResourceName,
                                                                   *mpFlowGraph->getMsgQ()));
                            frameStatus = mpFlowGraph->processNextFrame();
                            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, frameStatus);

                            // Should be a stop message
                            CPPUNIT_ASSERT_EQUAL(1, messageDispatcher.numMsgs());

                            // Get stop notification
                            messagePtr = NULL;
                            messageDispatcher.receive(messagePtr, notificationWait);
                            CPPUNIT_ASSERT(messagePtr);
                            if(messagePtr)
                            {
                                CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgType(), OsMsg::MP_RES_NOTF_MSG);
                                CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgSubType(), MpResNotificationMsg::MPRNM_RECORDER_STOPPED);
                            }

                            // Verify the file size
                            audioDataSize += (audioDataSize / 2);

                            // Each OGG append has an additional header
                            if(fileFormat == MprRecorder::OGG_OPUS)
                            {
                                headerSize += (headerSize / 2);
                                fileSizeSlop += (fileSizeSlop / 2);
                            }
                        }
                    }

                    // Stop flowgraph
                    haltFramework();

                    // This should force the recorder to close the file.
                    tearDown();

                    validateFileSize(recordFilename, headerSize, audioDataSize, fileSizeSlop, loopLabel);

                } // end for iteration over sample rates

            }  // end for iteration over file formats

        } // End for channel count iteration

    } // end testRecordChannelToFileAppend method

    // TODO: append functionality not yet tested
    // Append to non-existing file

    void testRecordToPauseResumeFile()
    {
        int numberOfTestFileTypes = sizeof(testFileTypes) / sizeof(MprRecorder::RecordFileFormat);
        int framesPerSecond = 100; // 10 mSec frames


        for(int fileTypeIndex = 0; fileTypeIndex < numberOfTestFileTypes; fileTypeIndex++)
        {
            MprRecorder::RecordFileFormat fileFormat = testFileTypes[fileTypeIndex];
            unsigned int rateIndex;
            for(rateIndex = 0; rateIndex < sNumRates; rateIndex++)
            {
                UtlString loopLabel;
                loopLabel.appendFormat("%s(%d) rate: %d",
                                       testFileTypeStrings[fileTypeIndex],
                                       testFileTypes[fileTypeIndex],
                                       sSampleRates[rateIndex]);

                printf("Test MprRecorder file type: %d media task rate: %d samples/second\n",
                        fileFormat,
                       sSampleRates[rateIndex]);

                UtlString recordFilename;
                UtlString recordFileExtension;
                CPPUNIT_ASSERT(getFileExtension(fileFormat, recordFileExtension));
                recordFilename.appendFormat("testRecordToFilePauseResume%d_%d.%s",
                                            sSampleRates[rateIndex],
                                            fileFormat,
                                            recordFileExtension.data());
                // Incase prior test left junk around
                tearDown();

                // Set media sample rate
                setSamplesPerSec(sSampleRates[rateIndex]);
                setSamplesPerFrame(sSampleRates[rateIndex]/framesPerSecond);
                setUp();

                int framesToProcess = 200; // 2 seconds
                UtlString recorderResourceName = "MprRecorder";
                MprRecorder* recorder = new MprRecorder(recorderResourceName);
                CPPUNIT_ASSERT(recorder);

                // Build flowgraph with source, MprRecorder and sink resources
                setupFramework(recorder);

                // Set parameters to signal to record
                for(int inputIndex = 0; inputIndex < recorder->maxInputs(); inputIndex++)
                {
                    mpSourceResource->setSignalAmplitude(inputIndex, 
                                                      (0x1 << (12 - inputIndex)));
                    mpSourceResource->setSignalPeriod(inputIndex, 
                          sSampleRates[rateIndex] / (250 * (0x1 << inputIndex)));
                }
                mpSourceResource->setOutSignalType(MpTestResource::MP_SINE);

                // Add the notifier so that we get resource events
                OsMsgQ resourceEventQueue;
                OsMsgDispatcher messageDispatcher(&resourceEventQueue);
                mpFlowGraph->setNotificationDispatcher(&messageDispatcher);

                // Start recording
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                     MprRecorder::startFile(recorderResourceName,
                                                            *mpFlowGraph->getMsgQ(),
                                                            recordFilename,
                                                            fileFormat));


                // Enable the source resource and the recorder
                CPPUNIT_ASSERT(mpSourceResource->enable());
                CPPUNIT_ASSERT(recorder->enable());

                // Process the frames
                OsStatus frameStatus;
                for(int frameIndex = 0; frameIndex < framesToProcess; frameIndex++)
                {
                    OsStatus frameStatus = mpFlowGraph->processNextFrame();
                    CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, frameStatus);

                    if(frameIndex == 0)
                    {
                        // Should be a start message
                        CPPUNIT_ASSERT_EQUAL(1, messageDispatcher.numMsgs());
                    }

                }

                int samplesRecorded = 
                    sSampleRates[rateIndex] / framesPerSecond * // samples/frame
                    framesToProcess; // frames

                // Send message to pause the recording
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                     MprRecorder::pause(recorderResourceName,
                                                        *mpFlowGraph->getMsgQ()));
                // Should be a start message, but not yet a pause message
                CPPUNIT_ASSERT_EQUAL(1, messageDispatcher.numMsgs());

                // Process more messages while paused.  Should not add to file size
                for(int frameIndex = 0; frameIndex < framesToProcess; frameIndex++)
                {
                    OsStatus frameStatus = mpFlowGraph->processNextFrame();
                    CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, frameStatus);

                    if(frameIndex == 0)
                    {
                        // Should be a start and pause message
                        CPPUNIT_ASSERT_EQUAL(2, messageDispatcher.numMsgs());
                    }

                }

                // Send message to resume the recording
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                     MprRecorder::resume(recorderResourceName,
                                                         *mpFlowGraph->getMsgQ()));
                // Should be a start and pause message, but not yet the resume message
                CPPUNIT_ASSERT_EQUAL(2, messageDispatcher.numMsgs());

                // Process more frames while resumed, should add to file size
                for(int frameIndex = 0; frameIndex < framesToProcess; frameIndex++)
                {
                    OsStatus frameStatus = mpFlowGraph->processNextFrame();
                    CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, frameStatus);

                    if(frameIndex == 0)
                    {
                        // Should be a start, pause and resume messages
                        CPPUNIT_ASSERT_EQUAL(3, messageDispatcher.numMsgs());
                    }

                }

                // Send message to stop the recording
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                     MprRecorder::stop(recorderResourceName,
                                                       *mpFlowGraph->getMsgQ()));
                // Should be a start, pause and resume messages, but not yet the stop message
                CPPUNIT_ASSERT_EQUAL(3, messageDispatcher.numMsgs());

                // Process one more frame to be sure recording stop message is handled
                frameStatus = mpFlowGraph->processNextFrame();
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, frameStatus);

                // Start, pause, resume and stop notifications
                CPPUNIT_ASSERT_EQUAL(4, messageDispatcher.numMsgs());

                // Get start notification
                OsTime notificationWait(0, 1000);
                OsMsg* messagePtr = NULL;
                messageDispatcher.receive(messagePtr, notificationWait);
                CPPUNIT_ASSERT(messagePtr);
                if(messagePtr)
                {
                    CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgType(), OsMsg::MP_RES_NOTF_MSG);
                    CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgSubType(), MpResNotificationMsg::MPRNM_RECORDER_STARTED);
                }

                // Get pause notification and verify the number of samples recorded
                messageDispatcher.receive(messagePtr, notificationWait);
                CPPUNIT_ASSERT(messagePtr);
                if(messagePtr)
                {
                    CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgType(), OsMsg::MP_RES_NOTF_MSG);
                    CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgSubType(), MpResNotificationMsg::MPRNM_RECORDER_PAUSED);
                    MprnIntMsg* stopMessage = (MprnIntMsg*) messagePtr;
                    CPPUNIT_ASSERT_EQUAL(stopMessage->getValue(), samplesRecorded);
                }

                // Get resumed notification
                messageDispatcher.receive(messagePtr, notificationWait);
                CPPUNIT_ASSERT(messagePtr);
                if(messagePtr)
                {
                    CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgType(), OsMsg::MP_RES_NOTF_MSG);
                    CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgSubType(), MpResNotificationMsg::MPRNM_RECORDER_RESUMED);
                }

                // Get stop notification and verify the number of samples recorded
                messageDispatcher.receive(messagePtr, notificationWait);
                CPPUNIT_ASSERT(messagePtr);
                if(messagePtr)
                {
                    CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgType(), OsMsg::MP_RES_NOTF_MSG);
                    CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgSubType(), MpResNotificationMsg::MPRNM_RECORDER_STOPPED);
                    MprnIntMsg* stopMessage = (MprnIntMsg*) messagePtr;
                    CPPUNIT_ASSERT_EQUAL(stopMessage->getValue(), samplesRecorded * 2);
                }

                // Make sure the recorded file is the correct size
                unsigned long headerSize = 0;
                unsigned long audioDataSize = 0;
                unsigned long fileSizeSlop = 0;
                getFileSizeParameters(fileFormat, 1, /*channel*/ samplesRecorded * 2, framesToProcess * 2, framesPerSecond, sSampleRates[rateIndex],
                                      headerSize, audioDataSize, fileSizeSlop);

                validateFileSize(recordFilename, headerSize, audioDataSize, fileSizeSlop, loopLabel);

                // Stop flowgraph
                haltFramework();

            } // end for iteration over sample rates

        }  // end for iteration over file formats

    } // end testRecordToPauseResumeFile method

}; // end MprRecorderTest class
           

CPPUNIT_TEST_SUITE_REGISTRATION(MprRecorderTest);




