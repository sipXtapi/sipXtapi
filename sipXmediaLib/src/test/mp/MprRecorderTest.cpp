//  
// Copyright (C) 2008-2017 SIPez LLC.  All rights reserved.
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

class MprRecorderTest : public MpGenericResourceTest
{
    CPPUNIT_TEST_SUITE(MprRecorderTest);
    CPPUNIT_TEST(testRecordToFile);
    CPPUNIT_TEST(testRecordToFileAppendNotExisting);
    CPPUNIT_TEST(testRecordToFileAppend);
    CPPUNIT_TEST(testRecordChannelToFileAppend);
    CPPUNIT_TEST(testRecordToPauseResumeFile);
    CPPUNIT_TEST_SUITE_END();

    void testRecordToFile()
    {
        MprRecorder::RecordFileFormat testFileTypes[] =
        {
             MprRecorder::RAW_PCM_16,
             MprRecorder::WAV_PCM_16,
             MprRecorder::WAV_ALAW,
             MprRecorder::WAV_MULAW,
             MprRecorder::WAV_GSM
        };
        int numberOfTestFileTypes = sizeof(testFileTypes) / sizeof(MprRecorder::RecordFileFormat);
        int framesPerSecond = 100; // 10 mSec frames


        for(int fileTypeIndex = 0; fileTypeIndex < numberOfTestFileTypes; fileTypeIndex++)
        {
            MprRecorder::RecordFileFormat fileFormat = testFileTypes[fileTypeIndex];
            unsigned int rateIndex;
            for(rateIndex = 0; rateIndex < sNumRates; rateIndex++)
            {
                printf("Test MprRecorder file type: %d media task rate: %d samples/second\n",
                        fileFormat,
                       sSampleRates[rateIndex]);

                UtlString recordFilename;
                recordFilename.appendFormat("testRecordToFile%d_%d.%s",
                                            sSampleRates[rateIndex],
                                            fileFormat,
                                            fileFormat == MprRecorder::RAW_PCM_16 ? "raw" : "wav");
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
                switch(fileFormat)
                {
                    case MprRecorder::WAV_ALAW:
                    case MprRecorder::WAV_MULAW:
                        headerSize = 44;
                        // Always 8000 samples/second
                        audioDataSize = samplesRecorded * 8000 / sSampleRates[rateIndex];
                        break;

                    case MprRecorder::WAV_PCM_16:
                        headerSize = 44;
                    case MprRecorder::RAW_PCM_16:
                        audioDataSize = samplesRecorded *
                                        sizeof(MpAudioSample); // bytes per sample
                    break;

                    case MprRecorder::WAV_GSM:
                        headerSize = 60;
                        // Frames for GSM in wave files alternate between 32 and 33 bytes.  Hense 65 for
                        // every 2 GSM frames
                        // This assumes 10 mSec frames
                        audioDataSize = framesToProcess / 4 * 65;
                        CPPUNIT_ASSERT_EQUAL(framesPerSecond, 100);
                        if(framesToProcess % 2)
                        {
                            // Not sure if this should be 32 or 33
                            audioDataSize += 32;
                        }
                    break;

                    default:
                        CPPUNIT_ASSERT(0);  // Unsupported record file format type
                    break;
                }

                OsFile recordFile(recordFilename);
                OsFileInfo fileInfo;
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                     recordFile.getFileInfo(fileInfo));
                unsigned long recordedFileSize;
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                     fileInfo.getSize(recordedFileSize));
                CPPUNIT_ASSERT_EQUAL(headerSize + audioDataSize,
                                     recordedFileSize);

                // Stop flowgraph
                haltFramework();

            } // end for iteration over sample rates

        }  // end for iteration over file formats

    } // end testRecordToFile method

    void testRecordToFileAppendNotExisting()
    {
        MprRecorder::RecordFileFormat testFileTypes[] =
        {
             MprRecorder::RAW_PCM_16,
             MprRecorder::WAV_PCM_16,
             MprRecorder::WAV_ALAW,
             MprRecorder::WAV_MULAW,
             MprRecorder::WAV_GSM
        };
        int numberOfTestFileTypes = sizeof(testFileTypes) / sizeof(MprRecorder::RecordFileFormat);
        int framesPerSecond = 100; // 10 mSec frames


        for(int fileTypeIndex = 0; fileTypeIndex < numberOfTestFileTypes; fileTypeIndex++)
        {
            MprRecorder::RecordFileFormat fileFormat = testFileTypes[fileTypeIndex];
            unsigned int rateIndex;
            for(rateIndex = 0; rateIndex < sNumRates; rateIndex++)
            {
                printf("Test MprRecorder file type: %d media task rate: %d samples/second\n",
                        fileFormat,
                       sSampleRates[rateIndex]);

                UtlString recordFilename;
                recordFilename.appendFormat("testRecordToFile%d_%d.%s",
                                            sSampleRates[rateIndex],
                                            fileFormat,
                                            fileFormat == MprRecorder::RAW_PCM_16 ? "raw" : "wav");
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
                switch(fileFormat)
                {
                    case MprRecorder::WAV_ALAW:
                    case MprRecorder::WAV_MULAW:
                        headerSize = 44;
                        // Always 8000 samples/second
                        audioDataSize = samplesRecorded * 8000 / sSampleRates[rateIndex];
                        break;

                    case MprRecorder::WAV_PCM_16:
                        headerSize = 44;
                    case MprRecorder::RAW_PCM_16:
                        audioDataSize = samplesRecorded *
                                        sizeof(MpAudioSample); // bytes per sample
                    break;

                    case MprRecorder::WAV_GSM:
                        headerSize = 60;
                        // Frames for GSM in wave files alternate between 32 and 33 bytes.  Hense 65 for
                        // every 2 GSM frames
                        // This assumes 10 mSec frames
                        audioDataSize = framesToProcess / 4 * 65;
                        CPPUNIT_ASSERT_EQUAL(framesPerSecond, 100);
                        if(framesToProcess % 2)
                        {
                            // Not sure if this should be 32 or 33
                            audioDataSize += 32;
                        }
                    break;

                    default:
                        CPPUNIT_ASSERT(0);  // Unsupported record file format type
                    break;
                }

                OsFile recordFile(recordFilename);
                OsFileInfo fileInfo;
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                     recordFile.getFileInfo(fileInfo));
                unsigned long recordedFileSize;
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                     fileInfo.getSize(recordedFileSize));
                CPPUNIT_ASSERT_EQUAL(headerSize + audioDataSize,
                                     recordedFileSize);

                // Stop flowgraph
                haltFramework();

            } // end for iteration over sample rates

        }  // end for iteration over file formats

    } // end testRecordToFile method

    void testRecordToFileAppend()
    {
        MprRecorder::RecordFileFormat testFileTypes[] =
        {
             MprRecorder::RAW_PCM_16,
             MprRecorder::WAV_PCM_16,
             MprRecorder::WAV_ALAW,
             MprRecorder::WAV_MULAW,
             MprRecorder::WAV_GSM
        };
        int numberOfTestFileTypes = sizeof(testFileTypes) / sizeof(MprRecorder::RecordFileFormat);
        int framesPerSecond = 100; // 10 mSec frames


        for(int fileTypeIndex = 0; fileTypeIndex < numberOfTestFileTypes; fileTypeIndex++)
        {
            MprRecorder::RecordFileFormat fileFormat = testFileTypes[fileTypeIndex];
            unsigned int rateIndex;
            for(rateIndex = 0; rateIndex < sNumRates; rateIndex++)
            {
                printf("Test MprRecorder file type: %d media task rate: %d samples/second\n",
                        fileFormat,
                       sSampleRates[rateIndex]);

                UtlString recordFilename;
                recordFilename.appendFormat("testRecordToFileAppend%d_%d.%s",
                                            sSampleRates[rateIndex],
                                            fileFormat,
                                            fileFormat == MprRecorder::RAW_PCM_16 ? "raw" : "wav");
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
                switch(fileFormat)
                {
                    case MprRecorder::WAV_ALAW:
                    case MprRecorder::WAV_MULAW:
                        headerSize = 44;
                        audioDataSize = samplesRecorded * 8000 / sSampleRates[rateIndex];
                        break;

                    case MprRecorder::WAV_PCM_16:
                        headerSize = 44;
                    case MprRecorder::RAW_PCM_16:
                        audioDataSize = samplesRecorded *
                                        sizeof(MpAudioSample); // bytes per sample
                    break;

                    case MprRecorder::WAV_GSM:
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
                    break;

                    default:
                        CPPUNIT_ASSERT(0);  // Unsupported record file format type
                    break;
                }

                // Scope to be sure no conflict/copy paste errors with similar check below
                // and file is closed.
                {
                    OsFile recordFile(recordFilename);
                    OsFileInfo fileInfo;
                    CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                         recordFile.getFileInfo(fileInfo));
                    unsigned long recordedFileSize;
                    CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                         fileInfo.getSize(recordedFileSize));
                    CPPUNIT_ASSERT_EQUAL(headerSize + audioDataSize,
                                         recordedFileSize);
                }

                mpSourceResource->setSignalAmplitude(0, (0x1 << 13));

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
                                                                    TRUE); // append = TRUE
                    // File of type fileType already exists.  If the append type is not
                    // the same, it should fail.
                    if(appendFileFormat != fileFormat)
                    {
                        CPPUNIT_ASSERT_EQUAL(OS_FAILED, appendStatus);
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

                        // Scope to be sure no conflict/copy paste errors with similar check above
                        // and file is closed.
                        {
                            OsFile appendRecordFile(recordFilename);
                            OsFileInfo appendFileInfo;
                            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                                 appendRecordFile.getFileInfo(appendFileInfo));
                            unsigned long appendRecordedFileSize;
                            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                                 appendFileInfo.getSize(appendRecordedFileSize));
                            CPPUNIT_ASSERT_EQUAL(headerSize + audioDataSize,
                                                 appendRecordedFileSize);
                        }

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

                        // No stop notification as we want the MprRecord destructor to close the file

                        // Verify the file size
                        audioDataSize += (audioDataSize / 2);

                    }
                }

                // Stop flowgraph
                haltFramework();

                // This should force the recorder to close the file.
                tearDown();

                // Scope to be sure no conflict/copy paste errors with similar check above
                // and file is closed.
                {
                    OsFile appendRecordFile(recordFilename);
                    OsFileInfo appendFileInfo;
                    CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                         appendRecordFile.getFileInfo(appendFileInfo));
                    unsigned long appendRecordedFileSize;
                    CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                         appendFileInfo.getSize(appendRecordedFileSize));
                    CPPUNIT_ASSERT_EQUAL(headerSize + audioDataSize,
                                         appendRecordedFileSize);
                }

            } // end for iteration over sample rates

        }  // end for iteration over file formats

    } // end testRecordToFileAppend method

    void testRecordChannelToFileAppend()
    {
        MprRecorder::RecordFileFormat testFileTypes[] =
        {
             MprRecorder::RAW_PCM_16,
             MprRecorder::WAV_PCM_16,
             MprRecorder::WAV_ALAW,
             MprRecorder::WAV_MULAW,
             MprRecorder::WAV_GSM
        };
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
                    printf("Test MprRecorder file type: %d media task rate: %d samples/second\n",
                            fileFormat,
                           sSampleRates[rateIndex]);

                    UtlString recordFilename;
                    recordFilename.appendFormat("testRecordChannelToFileAppend%d_ch%d_%d.%s",
                                                sSampleRates[rateIndex],
                                                channelCount,
                                                fileFormat,
                                                fileFormat == MprRecorder::RAW_PCM_16 ? "raw" : "wav");
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
                    switch(fileFormat)
                    {
                        case MprRecorder::WAV_ALAW:
                        case MprRecorder::WAV_MULAW:
                            headerSize = 44;
                            audioDataSize = samplesRecorded * 8000 / sSampleRates[rateIndex] * channelCount;
                            break;

                        case MprRecorder::WAV_PCM_16:
                            headerSize = 44;
                        case MprRecorder::RAW_PCM_16:
                            audioDataSize = samplesRecorded *
                                            sizeof(MpAudioSample) * channelCount; // bytes per sample
                        break;

                        case MprRecorder::WAV_GSM:
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
                        break;

                        default:
                            CPPUNIT_ASSERT(0);  // Unsupported record file format type
                        break;
                    }

                    // Scope to be sure no conflict/copy paste errors with similar check below
                    // and file is closed.
                    {
                        OsFile recordFile(recordFilename);
                        OsFileInfo fileInfo;
                        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                             recordFile.getFileInfo(fileInfo));
                        unsigned long recordedFileSize;
                        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                             fileInfo.getSize(recordedFileSize));
                        CPPUNIT_ASSERT_EQUAL(headerSize + audioDataSize,
                                             recordedFileSize);
                    }

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
                            CPPUNIT_ASSERT_EQUAL(OS_FAILED, appendStatus);
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

                            // Scope to be sure no conflict/copy paste errors with similar check above
                            // and file is closed.
                            {
                                OsFile appendRecordFile(recordFilename);
                                OsFileInfo appendFileInfo;
                                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                                     appendRecordFile.getFileInfo(appendFileInfo));
                                unsigned long appendRecordedFileSize;
                                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                                     appendFileInfo.getSize(appendRecordedFileSize));
                                CPPUNIT_ASSERT_EQUAL(headerSize + audioDataSize,
                                                     appendRecordedFileSize);
                            }

                            // Append a 2nd time.
                            appendStatus =  MprRecorder::startFile(recorderResourceName,
                                                                   *mpFlowGraph->getMsgQ(),
                                                                   recordFilename,
                                                                   appendFileFormat,
                                                                   0, // max record length = unlimited
                                                                   -1, // don't check/stop after silence
                                                                   TRUE, // append = TRUE
                                                                   channelCount);

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

                            // No stop notification as we want the MprRecord destructor to close the file

                            // Verify the file size
                            audioDataSize += (audioDataSize / 2);

                        }
                    }

                    // Stop flowgraph
                    haltFramework();

                    // This should force the recorder to close the file.
                    tearDown();

                    // Scope to be sure no conflict/copy paste errors with similar check above
                    // and file is closed.
                    {
                        OsFile appendRecordFile(recordFilename);
                        OsFileInfo appendFileInfo;
                        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                             appendRecordFile.getFileInfo(appendFileInfo));
                        unsigned long appendRecordedFileSize;
                        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                             appendFileInfo.getSize(appendRecordedFileSize));
                        CPPUNIT_ASSERT_EQUAL(headerSize + audioDataSize,
                                             appendRecordedFileSize);
                    }

                } // end for iteration over sample rates

            }  // end for iteration over file formats

        } // End for channel count iteration

    } // end testRecordChannelToFileAppend method

    // TODO: append functionality not yet tested
    // Append to non-existing file

    void testRecordToPauseResumeFile()
    {
        MprRecorder::RecordFileFormat testFileTypes[] =
        {
             MprRecorder::RAW_PCM_16,
             MprRecorder::WAV_PCM_16,
             MprRecorder::WAV_ALAW,
             MprRecorder::WAV_MULAW,
             MprRecorder::WAV_GSM
        };
        int numberOfTestFileTypes = sizeof(testFileTypes) / sizeof(MprRecorder::RecordFileFormat);
        int framesPerSecond = 100; // 10 mSec frames


        for(int fileTypeIndex = 0; fileTypeIndex < numberOfTestFileTypes; fileTypeIndex++)
        {
            MprRecorder::RecordFileFormat fileFormat = testFileTypes[fileTypeIndex];
            unsigned int rateIndex;
            for(rateIndex = 0; rateIndex < sNumRates; rateIndex++)
            {
                printf("Test MprRecorder file type: %d media task rate: %d samples/second\n",
                        fileFormat,
                       sSampleRates[rateIndex]);

                UtlString recordFilename;
                recordFilename.appendFormat("testRecordToFilePauseResume%d_%d.%s",
                                            sSampleRates[rateIndex],
                                            fileFormat,
                                            fileFormat == MprRecorder::RAW_PCM_16 ? "raw" : "wav");
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
                switch(fileFormat)
                {
                    case MprRecorder::WAV_ALAW:
                    case MprRecorder::WAV_MULAW:
                        headerSize = 44;
                        audioDataSize = samplesRecorded * 2 * 8000 / sSampleRates[rateIndex];
                        break;

                    case MprRecorder::WAV_PCM_16:
                        headerSize = 44;
                    case MprRecorder::RAW_PCM_16:
                        audioDataSize = samplesRecorded * 2 *
                                        sizeof(MpAudioSample); // bytes per sample
                    break;

                    case MprRecorder::WAV_GSM:
                        headerSize = 60;
                        // Frames for GSM in wave files alternate between 32 and 33 bytes.  Hense 65 for
                        // every 2 GSM frames
                        // This assumes 10 mSec frames
                        audioDataSize = framesToProcess * 2 / 4 * 65;
                        CPPUNIT_ASSERT_EQUAL(framesPerSecond, 100);
                        if(framesToProcess % 2)
                        {
                            // Not sure if this should be 32 or 33
                            audioDataSize += 32;
                        }
                    break;

                    default:
                        CPPUNIT_ASSERT(0);  // Unsupported record file format type
                    break;
                }

                OsFile recordFile(recordFilename);
                OsFileInfo fileInfo;
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                     recordFile.getFileInfo(fileInfo));
                unsigned long recordedFileSize;
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                     fileInfo.getSize(recordedFileSize));
                CPPUNIT_ASSERT_EQUAL(headerSize + audioDataSize,
                                     recordedFileSize);

                // Stop flowgraph
                haltFramework();

            } // end for iteration over sample rates

        }  // end for iteration over file formats

    } // end testRecordToPauseResumeFile method

}; // end MprRecorderTest class
           

CPPUNIT_TEST_SUITE_REGISTRATION(MprRecorderTest);




