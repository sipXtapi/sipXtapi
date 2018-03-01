//  
// Copyright (C) 2008-2018 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie < dpetrie AT sipez DOT com>

#include <os/OsIntTypes.h>
#include <sipxunittests.h>
#include <os/OsFS.h>
#include <math.h>

// Setup codec paths..
#include <../test/mp/MpTestCodecPaths.h>

#include <mp/MpFlowGraphBase.h>
#include <mp/MpMisc.h>
#include <mp/MpMediaTask.h>
#include <mp/MprFromFile.h>
#include <mp/MprEncode.h>
#include <mp/MprSplitter.h>
#include <mp/MprRecorder.h>
#include <mp/MpResampler.h>
#include <mp/MprToNet.h>
#include <mp/MpCodecFactory.h>
#include <mp/MprnRtpStreamActivityMsg.h>
#include <sdp/SdpCodec.h>
#include <sdp/SdpDefaultCodecFactory.h>

#include <os/OsTask.h>

#define TEST_SAMPLES_PER_FRAME 480
#define FROM_FILE_RESOURCE_NAME "FromFile"
#define RECORDER_RESOURCE_NAME "Recorder"
#define ENCODER_RESOURCE_NAME "Encoder"
#define MAX_TEST_SAMPLE_COUNT ((1<<16) * 2)

//#define CHECK_SILENCE 1
#ifdef CHECK_SILENCE
class MprSilenceCheck : public MpAudioResource, public MprToNet
{
    public:
    MprSilenceCheck(const UtlString resourceName)
    : MpAudioResource(resourceName, 0, 1, 0, 1),
      mProcessedFrames(0),
      mTolerance(8)
    {
    }

    UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                              MpBufPtr outBufs[],
                              int inBufsSize,
                              int outBufsSize,
                              UtlBoolean isEnabled,
                              int samplesPerFrame,
                              int samplesPerSecond)
    {
        // Pass through
        outBufs[0] = inBufs[0];
        MpAudioBufPtr output = inBufs[0];
        inBufs[0].release();

        if(output.isValid())
        {
            assert(((int)output->getSamplesNumber()) >= samplesPerFrame);
            MpAudioSample* outSamples = output->getSamplesWritePtr();

            // Dump non-silence
            for(int sampleIndex = 0; sampleIndex < samplesPerFrame; sampleIndex++)
            {
                if(outSamples[sampleIndex] > mTolerance ||
                   outSamples[sampleIndex] < -mTolerance)
                {
                    printf("%s frame[%d] sample[%d] = %d\n",
                           this->data(),
                           mProcessedFrames,
                           sampleIndex,
                           (int)outSamples[sampleIndex]);
                }
            }
        }
        else
        {
            printf("MprSilenceCheck::doProcessFrame frame[%d] invalid\n",
                   mProcessedFrames);
        }
        mProcessedFrames++;
        return(TRUE);
    }

    private:
    int mProcessedFrames;
    int mTolerance;
};
#endif

class MprEncoderDecoder : public MpAudioResource, public MprToNet
{
    public:
    MprEncoderDecoder(const UtlString resourceName)
    : MpAudioResource(resourceName, 0, 1, 0, 1),
      mpDecoder(NULL),
      mpResampler(NULL),
      mNumPacketsReceived(0),
      mProcessedFrames(0),
      mStartDecodingAtFrame(1 << 31),
      mFrameMs(0),
      mResamplerLatencyNumSamples(0),
      mNumDecodedSamples(0)
    {
    }

    virtual ~MprEncoderDecoder()
    {
        if(mpDecoder)
        {
            mpDecoder->freeDecode();
            delete mpDecoder;
            mpDecoder = NULL;
        }
        if(mpResampler)
        {
            delete mpResampler;
            mpResampler = NULL;
        }
    }

    static int getResamplerLatency(int flowgraphFrameMs,
                                   int codecSampleRate, 
                                   int flowgraphSampleRate)
    {
        assert(flowgraphFrameMs > 0);
        int numFlowgraphSamples = flowgraphSampleRate * flowgraphFrameMs / 1000;
        int numCodecSamples = codecSampleRate * flowgraphFrameMs / 1000;
        // Leave room for extra sample as conversion is not an integer number of samples
        if((codecSampleRate * flowgraphFrameMs) % 1000)
        {
            numCodecSamples++;
        }
        MpAudioSample* flowgraphSamples = new MpAudioSample[numFlowgraphSamples];
        MpAudioSample* codecSamples = new MpAudioSample[numCodecSamples];
        memset(flowgraphSamples, 0, numFlowgraphSamples * sizeof(MpAudioSample));

        // Initialize a sine wave to resample
        int sampleIndex = 0;
        for(sampleIndex = 0; sampleIndex < numFlowgraphSamples; sampleIndex++)
        {
            flowgraphSamples[sampleIndex] = (1 << 15) * // magnitude
                sin(2.0 * M_PI * ((float)sampleIndex) / 
                    ((float) (2 * flowgraphFrameMs * flowgraphSampleRate / 1000))); // period of 2 frames
                                                                                    // i.e. 50 Hz
        }

        MpResamplerBase* flowToCodecResampler = 
            MpResamplerBase::createResampler(1, // channels
                                             flowgraphSampleRate,
                                             codecSampleRate);
        MpResamplerBase* codecToFlowResampler = 
            MpResamplerBase::createResampler(1, // channels
                                             codecSampleRate, 
                                             flowgraphSampleRate);

        uint32_t consumedSamplesCount = 0;
        uint32_t resultingSamplesCount = 0;
        flowToCodecResampler->resample(0,
                                       flowgraphSamples,
                                       numFlowgraphSamples,
                                       consumedSamplesCount,
                                       codecSamples,
                                       numCodecSamples,
                                       resultingSamplesCount);
        assert((int)resultingSamplesCount == numCodecSamples);
        assert(numFlowgraphSamples == (int)consumedSamplesCount);

        codecToFlowResampler->resample(0,
                                       codecSamples,
                                       resultingSamplesCount,
                                       consumedSamplesCount,
                                       flowgraphSamples,
                                       numFlowgraphSamples,
                                       resultingSamplesCount);
        assert(numFlowgraphSamples - (int)resultingSamplesCount < 2);
        assert(numCodecSamples - (int)consumedSamplesCount < 2);

        int latency = 0;
        int tolerance = 100;
        for(sampleIndex = 0; sampleIndex < numFlowgraphSamples; sampleIndex++)
        {
            if(flowgraphSamples[sampleIndex] < tolerance &&
               flowgraphSamples[sampleIndex] > -tolerance)
            {
                //printf("resample latency flowgraphSamples[%d] = %d\n",
                //       sampleIndex,
                //       flowgraphSamples[sampleIndex]);
            }
            else
            {
                latency = sampleIndex;
                break;
            }
        }

        delete codecToFlowResampler;
        delete flowToCodecResampler;
        delete[] codecSamples;
        delete[] flowgraphSamples;

        return(latency);
    }

   // Overwrite MprToNet base class implementation so
   // that we can catch RTP packets from the encoder
   virtual int writeRtp(int payloadType, 
                        UtlBoolean markerState,
                        const unsigned char* payloadData, 
                        int numPayloadOctets, 
                        unsigned int timestamp,
                        void* csrcList)
    {
        assert(mpDecoder);
        if(mpDecoder)
        {
            MpAudioSample resampleBuffer[MAX_TEST_SAMPLE_COUNT];
            MpAudioSample* decodeBufferPtr = NULL;
            int codecSampleRate = mpDecoder->getInfo()->getSampleRate();
            int flowGraphSampleRate = MpResource::mpFlowGraph->getSamplesPerSec();
            int decodeStart = 0;

            if(codecSampleRate == flowGraphSampleRate)
            {
                // We can decode straight into end of decode samples buffer
                decodeBufferPtr = mDecodedSamples;
                decodeStart = mNumDecodedSamples;
            }
            else
            {
                // Decode into temporary buffer then resample into the end of the
                // decode samples buffer
                decodeBufferPtr = resampleBuffer;
                decodeStart = 0;

                if(mpResampler == NULL)
                {
                    printf("decoder resampler from: %d to %d\n",
                           codecSampleRate, 
                           flowGraphSampleRate);
                    mpResampler = 
                        MpResamplerBase::createResampler(1, // channels
                                                         codecSampleRate, 
                                                         flowGraphSampleRate
                                                         /* quality = 0-10 low to high, 
                                                            when using speex, defaults to 3*/);
                    //mpResampler->setInputRate(codecSampleRate);

                    // Not set yet
                    if(mFrameMs == 0)
                    {
                        mFrameMs = (1000 * MpResource::mpFlowGraph->getSamplesPerFrame()) / 
                            flowGraphSampleRate;
                    }

                    mResamplerLatencyNumSamples = 
                        getResamplerLatency(mFrameMs,
                                            codecSampleRate, 
                                            flowGraphSampleRate);
                    printf("round trip latency for %d to %d and back is : %d samples (%0.5f seconds)\n",
                           flowGraphSampleRate,
                           codecSampleRate,
                           mResamplerLatencyNumSamples,
                           ((float)mResamplerLatencyNumSamples) / ((float)flowGraphSampleRate));
                }
            }

            // Create a memory based RTP packet
            if(!mRtpPacket.isValid())
            {
                mRtpPacket = MpMisc.RtpPool->getBuffer();
            }

            char* bufferPtr = mRtpPacket->getDataWritePtr();
            // TODO  setup RTP header data as needed

            // copy payload to packet
            memcpy(bufferPtr, payloadData, numPayloadOctets);
            int paddingLength = ((4 - (numPayloadOctets & 3))&3);
            if(paddingLength)
            {
                mRtpPacket->enableRtpPadding();
            }
            else
            {
                mRtpPacket->disableRtpPadding();
            }
            //mRtpPacket->setPacketSize(sizeof(RtpHeader) + numPayloadOctets + paddingLength);
            mRtpPacket->setPacketSize(numPayloadOctets);// + paddingLength);

#if 0
            printf("decode packet payload size: %d\n",
                   (int) (mRtpPacket->getPayloadSize()));
#endif
            // Decode packet
            int numDecodedSamples = 
                mpDecoder->decode(mRtpPacket,
                                  MAX_TEST_SAMPLE_COUNT - decodeStart,  // room for how many samples
                                  &decodeBufferPtr[decodeStart]);
            assert(numDecodedSamples > 0);
#ifdef CHECK_SILENCE
            // Assumes input signal is zero and prints anomalies 
            int tolerance = 8;
            for(int sampleIndex = 0; sampleIndex < numDecodedSamples; sampleIndex++)
            {
                if(decodeBufferPtr[decodeStart + sampleIndex] > tolerance ||
                   decodeBufferPtr[decodeStart + sampleIndex] < -tolerance)
                {
                    printf("frame[%d] decodeBuffer[%d] = %d\n",
                           mProcessedFrames,
                           sampleIndex,
                           (int)decodeBufferPtr[decodeStart + sampleIndex]);
                }
            }
#endif
#ifdef PRINT_FIRST_LAST
            printf("frame[%d] decodedBufferPtr[%d] = %d decodedBufferPtr[%d] = %d\n",
                   mProcessedFrames,
                   decodeStart,
                   (int)decodeBufferPtr[decodeStart],
                   decodeStart + numDecodedSamples - 1,
                   (int)decodeBufferPtr[decodeStart + numDecodedSamples - 1]);
#endif
                   
            // Resample decoded samples if needed
            if(codecSampleRate != flowGraphSampleRate)
            {
                if(mNumPacketsReceived == 0)
                {
                    // We pad out the latancy caused by the resample to a full frame.
                    // Then we drop this frame later by delaying the recordeer start
                    // by a frame.
                    int samplesPerFrame = mFrameMs * flowGraphSampleRate / 1000;
                    assert(mResamplerLatencyNumSamples < samplesPerFrame);
                    assert(mNumDecodedSamples == 0);
                    int padSamples = samplesPerFrame - mResamplerLatencyNumSamples;
                    memset(mDecodedSamples, 0, padSamples * sizeof(MpAudioSample));
                    mNumDecodedSamples = padSamples;
                }

                uint32_t samplesConvertedCount = 0;
                uint32_t resultingSamplesCount = 0;
                mpResampler->resample(0,
                                      &decodeBufferPtr[decodeStart],
                                      numDecodedSamples,
                                      samplesConvertedCount,
                                      &mDecodedSamples[mNumDecodedSamples],
                                      MAX_TEST_SAMPLE_COUNT - mNumDecodedSamples,  // room for how many samples
                                      resultingSamplesCount);
                assert(resultingSamplesCount > 0);
                // Unhandled case if it only resamples part of the input
                assert(numDecodedSamples == (int)samplesConvertedCount);
#ifdef CHECK_SILENCE
                for(int sampleIndex = 0; sampleIndex < (int)resultingSamplesCount; sampleIndex++)
                {
                    if(mDecodedSamples[mNumDecodedSamples + sampleIndex] > tolerance ||
                       mDecodedSamples[mNumDecodedSamples + sampleIndex] < -tolerance)
                    {
                        printf("frame[%d] mDecodedSamples[%d] = %d\n",
                               mProcessedFrames,
                               sampleIndex,
                               (int)mDecodedSamples[mNumDecodedSamples + sampleIndex]);
                    }
                }
#endif
#ifdef PRINT_FIRST_LAST
                printf("frame[%d] mDecodedSamples[%d] = %d mDecodedSamples[%d] = %d\n",
                       mProcessedFrames,
                       mNumDecodedSamples,
                       (int)mDecodedSamples[mNumDecodedSamples],
                       mNumDecodedSamples + resultingSamplesCount - 1,
                       (int)mDecodedSamples[mNumDecodedSamples + resultingSamplesCount - 1]);
#endif

                mNumDecodedSamples += resultingSamplesCount;
#ifdef PRINT_RESAMPLE_INFO
                printf("after resample decodeStart: %d numDecodedSamples: %d samplesConvertedCount: %d resultingSamplesCount: %d\n",
                       decodeStart,
                       numDecodedSamples,
                       samplesConvertedCount,
                       resultingSamplesCount);
                printf("Decoded buffer size: %d\n",
                       mNumDecodedSamples);
#endif
                                      
            }
            else
            {
                mNumDecodedSamples += numDecodedSamples;
            }

        }

        // First packet recieved
        if(mNumPacketsReceived == 0)
        {
            printf("MprEncoderDecoder::writeRtp got RTP packet: %d frame[%d]\n",
                   mNumPacketsReceived,
                   mProcessedFrames);

            // Start decoding on next process frame
            // We want to instroduce one frame of delay so that we can 
            // react to the RTP start notification to start recording
            // and not loose the first frame.
            // The 1 frame delay is added as latency caused by resampling and
            // calculated padding to fill a full frame.
            mStartDecodingAtFrame = mProcessedFrames;
        }
        mNumPacketsReceived++;
        return(numPayloadOctets);
    }
    

    UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                              MpBufPtr outBufs[],
                              int inBufsSize,
                              int outBufsSize,
                              UtlBoolean isEnabled,
                              int samplesPerFrame,
                              int samplesPerSecond)
    {
        if(mProcessedFrames == 0)
        {
            mFrameMs = samplesPerFrame * 1000 / samplesPerSecond;
        }
        //printf("doProcess inBufSize: %d outBufSize: %d inBufs[0] is %s\n",
        //       inBufsSize,
        //       outBufsSize,
        //       inBufs[0].isValid() ? "VALID" : "INVALID");
               
        if(outBufsSize);
        {
            if(mpDecoder)
            {
                // The decoder resource should have pushed a RTP packet to this
                // is both an MprToNet and a decoder resource
                
                if(mStartDecodingAtFrame <= mProcessedFrames)
                {
                    //printf("start decoding at frame: %d processed frames: %d\n",
                    //       mStartDecodingAtFrame,
                    //       mProcessedFrames);
                    int samplesToCopy = 0;

                    if(mNumDecodedSamples)
                    {
                        // Get reference to buffer and release the inBufs reference
                        MpAudioBufPtr output = inBufs[0];
                        if(output.isValid())
                        {
                            inBufs[0].release();
                        }
                        else
                        {
                            output = MpMisc.RawAudioPool->getBuffer();
                            assert(output.isValid());
                        }

                        // Copy a frame's worth of samples from the begining of the
                        // of the decoded samples buffer.
                        samplesToCopy = sipx_min(mNumDecodedSamples, samplesPerFrame);
                        MpAudioSample* outSamples = output->getSamplesWritePtr();
                        assert(((int)output->getSamplesNumber()) == samplesPerFrame);
                        assert(samplesToCopy > 0);
                        memcpy(outSamples, mDecodedSamples, samplesToCopy * sizeof(MpAudioSample));
#ifdef PRINT_FIRST_LAST
                        printf("Frame[%d] outSamples[%d] = %d outSamples[%d] = %d\n",
                               mProcessedFrames,
                               0,
                               outSamples[0],
                               samplesToCopy - 1,
                               outSamples[samplesToCopy - 1]);
#endif

                        // If we do not have a whole frame zero out the end of the frame
                        if(samplesToCopy < samplesPerFrame)
                        {
                            // This will only get hit for certain codecs.
                            // i.e. when codec frame is not a multiple of media frame time.
                            printf("frame[%d] padding %d samples to end of output buffer\n",
                                   mProcessedFrames,
                                   samplesPerFrame - samplesToCopy);
                            memset(&outSamples[samplesToCopy],
                                   0,
                                   (samplesPerFrame - samplesToCopy) * sizeof(MpAudioSample));

                            output->setSamplesNumber(samplesPerFrame);
                        }
                        else
                        {
                            output->setSamplesNumber(samplesToCopy);
                        }
                        
                        // Shift the remaining samples forward in the buffer
                        memmove(mDecodedSamples, 
                                &mDecodedSamples[samplesToCopy], 
                                (mNumDecodedSamples - samplesToCopy) * sizeof(MpAudioSample));

                        // Update the count for the samples removed
                        mNumDecodedSamples -= samplesToCopy;
#if 0
                        printf("Decoded buffer size: %d\n",
                               mNumDecodedSamples);
#endif

                        // Add a reference to buffer for outBufs
                        outBufs[0] = output;
                        // output's reference will be release when it goes out of scope
                        
#if 0
                        printf("passing decoded frame: %d num samples: %d\n",
                               mProcessedFrames,
                               samplesToCopy);
#endif
                    }
#if 0
                    else
                    {
                        printf("passing decoded frame: %d num samples: none decoded\n",
                               mProcessedFrames);
                    }
#endif
                }
                else
                {
                    printf("decoder doProcess skipping frame: %d\n",
                           mProcessedFrames);
                }
            }
            else
            {
                // Simple pass through, don't run it through the codec as there isn't one
                outBufs[0] = inBufs[0];
            }
        }
        mProcessedFrames++;
        return(TRUE);
    }

    OsStatus setCodec(MpCodecFactory& codecFactory,
                      const char* mimeSubtype,
                      const char* fmtp,
                      int sampleRate,
                      int channels)
    {
        assert(mpDecoder == NULL);
        if(mpDecoder)
        {
            mpDecoder->freeDecode();
            delete mpDecoder;
            mpDecoder = NULL;
        }
        OsStatus status = 
            codecFactory.createDecoder(mimeSubtype,
                                       fmtp,
                                       sampleRate,
                                       channels,
                                       0,
                                       mpDecoder);
        assert(status == OS_SUCCESS);
        assert(mpDecoder);
         
        if(mpDecoder && status == OS_SUCCESS)
        {
            status = mpDecoder->initDecode(fmtp);
            assert(OS_SUCCESS == status);
            assert(mpDecoder->getInfo());

            if (mpDecoder->getInfo() == NULL || mpDecoder->getInfo()->isSignalingCodec())
            {
                if (mpDecoder->getInfo() == NULL)
                {
                    printf("MprEncoderDecoder::setCodec(%p, \"%s\", \"%s\", %d, %d) decoder: %p NULL codecInfo\n",
                           &codecFactory,
                           mimeSubtype,
                           fmtp,
                           sampleRate,
                           channels,
                           mpDecoder);
                }
                else if(mpDecoder->getInfo()->isSignalingCodec())
                {
                    printf("MprEncoderDecoder::setCodec %s signaling codec\n",
                           mpDecoder->getInfo()->getMimeSubtype());
                }

                mpDecoder->freeDecode();
                delete mpDecoder;
                mpDecoder = NULL;
                status = OS_INVALID_ARGUMENT;
            }
            else
            {
                printf("set decoder to: %s sample rate: %d channels: %d\n",
                       mpDecoder->getInfo()->getMimeSubtype(),
                       (int)mpDecoder->getInfo()->getSampleRate(),
                       (int)mpDecoder->getInfo()->getNumChannels());
            }


        }
        assert(status == OS_SUCCESS);

        return(status);
    }

    int getNumPacketsReceived()
    {
        return(mNumPacketsReceived);
    }

    private:
    MpDecoderBase* mpDecoder;
    MpResamplerBase* mpResampler;
    int mNumPacketsReceived;
    int mProcessedFrames;
    int mStartDecodingAtFrame;
    int mFrameMs;
    int mResamplerLatencyNumSamples;
    int mNumDecodedSamples;
    MpAudioSample mDecodedSamples[MAX_TEST_SAMPLE_COUNT];
    MpRtpBufPtr mRtpPacket;
};

class MpCodecsQualityTest : public SIPX_UNIT_BASE_CLASS
{
   CPPUNIT_TEST_SUITE(MpCodecsQualityTest);
   CPPUNIT_TEST(testEncodeDecodeToFile);
   CPPUNIT_TEST_SUITE_END();

public:
   MpCodecsQualityTest() :
   mpFlowGraph(NULL),
   mprFromFile(NULL),
   mprEncoder(NULL),
   mprEncoderDecoder(NULL),
   mprSplitter(NULL),
   mprRecorder(NULL) 
   {
   }

   // This function will be called before every test to setup framework.
   void setUp()
   {
      // Setup media task
      // We do not need the media task, but the buffer pools, codecs
      // and other infrastructure that get setup. We will manually 
      // manage the the flowgraph.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpStartUp(TEST_SAMPLES_PER_FRAME * 100, // sample rate
                                     TEST_SAMPLES_PER_FRAME,
                                     3*100, // buffers allocated in pool
                                     NULL, // no configDB
                                     sNumCodecPaths, sCodecPaths));

      // Create flowgraph
      mpFlowGraph = new MpFlowGraphBase(TEST_SAMPLES_PER_FRAME,
                                        TEST_SAMPLES_PER_FRAME * 100); // sample rate
      CPPUNIT_ASSERT(mpFlowGraph != NULL);

      // Turn on notifications
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->setNotificationsEnabled(true));

      // Create a resource to read wav files in
      CPPUNIT_ASSERT(mprFromFile == NULL);
      mprFromFile = new MprFromFile(FROM_FILE_RESOURCE_NAME);
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpFlowGraph->addResource(*mprFromFile));

//#define CHECK_SILENCE 1
#ifdef CHECK_SILENCE
      // Create a non-silence detector to debug where noise is getting injected
      // into recording of silence
      MprSilenceCheck* preEncodeSilenceCheck = new MprSilenceCheck("PreEncode_SilenceCheck");
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpFlowGraph->addResource(*preEncodeSilenceCheck));
#endif

      // Create an encoder resource
      CPPUNIT_ASSERT(mprEncoder == NULL);
      mprEncoder = new MprEncode(ENCODER_RESOURCE_NAME);
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpFlowGraph->addResource(*mprEncoder));

      // Create resource to use codec to decode audio
      CPPUNIT_ASSERT(mprEncoderDecoder == NULL);
      mprEncoderDecoder = new MprEncoderDecoder("EncoderDecoder");
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpFlowGraph->addResource(*mprEncoderDecoder));
      mprEncoder->setMyToNet(mprEncoderDecoder);
      
      // Create a splitter to turn mono into 2 channel input to recorder
      CPPUNIT_ASSERT(mprSplitter == NULL);
      mprSplitter = new MprSplitter("Splitter",
                                    2); // outputs
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpFlowGraph->addResource(*mprSplitter));

      // Create a resource to write files out
      CPPUNIT_ASSERT(mprRecorder == NULL);
      mprRecorder = new MprRecorder(RECORDER_RESOURCE_NAME);
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpFlowGraph->addResource(*mprRecorder));
     
      // Link/connect the resource outputs to inputs 
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpFlowGraph->addLink(*mprFromFile, 0,   // output 0 goes to
#ifdef CHECK_SILENCE 
                                                *preEncodeSilenceCheck, 0)); // input 0
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpFlowGraph->addLink(*preEncodeSilenceCheck, 0,   // output 0 goes to
                                                *mprEncoder, 0)); // input 0
#else
                                                *mprEncoder, 0)); // input 0
#endif
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpFlowGraph->addLink(*mprEncoder, 0,   // output 0 goes to
                                                *mprEncoderDecoder, 0)); // input 0
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpFlowGraph->addLink(*mprEncoderDecoder, 0,   // output 0 goes to
                                                *mprSplitter, 0)); // input 0
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpFlowGraph->addLink(*mprSplitter, 0,   // output 0 goes to
                                                *mprRecorder, 0)); // input 0
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpFlowGraph->addLink(*mprSplitter, 1,   // output 1 goes to
                                                *mprRecorder, 1)); // input 1
   }

   // This function will be called after every test to clean up framework.
   void tearDown()
   {
      // This should normally be done by haltFramework, but if we aborted due
      // to an assertion the flowgraph will need to be shutdown here
      if (mpFlowGraph && mpFlowGraph->isStarted())
      {
         //osPrintf("WARNING: flowgraph found still running, shutting down\n");

         // ignore the result and keep going
         mpFlowGraph->stop();

         // Request processing of another frame so that the STOP_FLOWGRAPH
         // message gets handled
         mpFlowGraph->processNextFrame();
      }

      // Free flowgraph
      delete mpFlowGraph;
      mpFlowGraph = NULL;

      // Should have been deleted with the flowgraph
      mprFromFile = NULL;
      mprEncoder = NULL;
      mprEncoderDecoder = NULL;
      mprSplitter = NULL;
      mprRecorder = NULL;

      // Clear all media processing data and delete MpMediaTask instance.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpShutdown());
   }

    void testEncodeDecodeToFile()
    {
        //  Create dir where results/recorded files will be stored
        OsFileSystem::createDir("codecQualityResult");
 
        // Get list of loaded codecs
        unsigned codecInfoNum;
        // Scope of codecFactory
        {
            const MppCodecInfoV1_1** codecInfoArray;
            MpCodecFactory* codecFactory = MpCodecFactory::getMpCodecFactory();
            CPPUNIT_ASSERT(codecFactory != NULL);
 
            // Get array here, just to get the codec count
            codecFactory->getCodecInfoArray(codecInfoNum, codecInfoArray);
        }
 
        // Loop over codecs
        UtlString mimeSubtype;
        UtlString fmtp;
        int codecSampleRate;
        int codecChannels;
        int codecIndex = 0;
        int codecFmtpIndex = 0;
        int codecFmtpCount = 0;
        while(codecIndex < (int)codecInfoNum)
        {
            // Scope codecInfo as we reset the media subsystem and it goes away
            {
                // Need to do this inside the codec loop as we reset the media system 
                // when looping over each file
                const MppCodecInfoV1_1** codecInfoArray;
                MpCodecFactory* codecFactory = MpCodecFactory::getMpCodecFactory();
                CPPUNIT_ASSERT(codecFactory != NULL);
 
                codecFactory->getCodecInfoArray(codecInfoNum, codecInfoArray);
                CPPUNIT_ASSERT(codecInfoNum > 0);

                const MppCodecInfoV1_1* codecInfo = codecInfoArray[codecIndex];
                CPPUNIT_ASSERT(codecInfo);

                mimeSubtype = codecInfo->mimeSubtype;
                if("telephone-event" == mimeSubtype)
                {
                    printf("Skipping codec: %s\n",
                           mimeSubtype.data());
                    codecIndex++;
                    continue;
                }
                mimeSubtype.toLower();
                fmtp = codecInfo->fmtpsNum ? codecInfo->fmtps[codecFmtpIndex] : "";
                codecFmtpCount = codecInfo->fmtpsNum;
                codecSampleRate = codecInfo->sampleRate;
                codecChannels = codecInfo->numChannels;
            }

            printf("codec[%d] mime: %s sample rate: %dk channels: %d fmtp[%d]: %s\n",
                   codecIndex,
                   mimeSubtype.data(),
                   codecSampleRate / 1000,
                   codecChannels,
                   codecFmtpIndex,
                   fmtp.data());

            // Setup PCM 8000 mono
            //mimeSubtype = "L16";
            //fmtp = "";
            //codecSampleRate = 16000;
            //codecChannels = 1;

            UtlString sourcePath("codecQualitySource/");
            UtlString recordPath("codecQualityResult/");

            OsFileIterator sourceRecordingIterator(sourcePath);
            OsPathBase aSourceRecording;
            OsStatus foundRecordingStatus = sourceRecordingIterator.findFirst(aSourceRecording);
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, foundRecordingStatus);
                   
            // Loop over file names in directory
            while(foundRecordingStatus == OS_SUCCESS)
            {
                // Make sure prior run was cleaned up, could have aborted without cleanup
                tearDown(); 
                setUp();

                UtlString sourceFileBaseName("GREFCLA.WAV");
                //UtlString sourceFileName("100hz.wav");
                //UtlString sourceFileName("silence.wav");
                int nameLength = aSourceRecording.length();
                if(nameLength > 4 && 
                   strcasecmp(&aSourceRecording.data()[nameLength - 4], ".wav") == 0)
                {
                    printf("Found recording: %s\n",
                           aSourceRecording.data());
                    sourceFileBaseName = aSourceRecording;
                }
                else
                {
                    printf("Ignoring file: %s\n",
                           aSourceRecording.data());
                    // Get next file in directory
                    foundRecordingStatus = sourceRecordingIterator.findNext(aSourceRecording);
                    continue;
                }


                UtlString sourceFileName = sourcePath + sourceFileBaseName;
                UtlString recordFileName = recordPath + sourceFileBaseName;

                // Make the codec definition, part of the recorded file name
                UtlString codecSuffix;
                codecSuffix.appendFormat("_%s_%dk_%dc",
                                         mimeSubtype.data(),
                                         codecSampleRate / 1000,
                                         codecChannels);
                if(fmtp.length())
                {
                    UtlString fileSafeFmtp(fmtp);
                    fileSafeFmtp.replace('=', '_');
                    fileSafeFmtp.replace(' ', '_');
                    fileSafeFmtp.replace(';', '_');
                    codecSuffix.appendFormat("_%s", fileSafeFmtp.data());
                }
                recordFileName.insert(recordFileName.length() - 4,
                                      codecSuffix);
                
                // Get codec
                {
                   UtlString loopMessage;
                   loopMessage.appendFormat("source: %s codec: %s FMTP: %s sample rate: %d channels: %d", 
                                            sourceFileName.data(), 
                                            mimeSubtype.data(),
                                            fmtp.data(),
                                            codecSampleRate,
                                            codecChannels);

                    SdpCodec::SdpCodecTypes sdpCodecId;
                    OsStatus getCodecStatus = SdpDefaultCodecFactory::getCodecType(mimeSubtype,
                                                                                   codecSampleRate,
                                                                                   codecChannels,
                                                                                   fmtp,
                                                                                   sdpCodecId);
                    CPPUNIT_ASSERT_EQUAL_MESSAGE(loopMessage,
                                                 OS_SUCCESS,
                                                 getCodecStatus);
                    if(getCodecStatus != OS_SUCCESS)
                    {
                        // For some reason we cannot look up this codec
                        break;
                    }
                    printf("codec mime subtype: %s id: %d\n",
                           mimeSubtype.data(),
                           (int)sdpCodecId);
                   SdpCodec primaryCodec = SdpDefaultCodecFactory::getCodec(sdpCodecId);
                   UtlString sdpMimeSubtype;
                   primaryCodec.getEncodingName(sdpMimeSubtype);
                   sdpMimeSubtype.toLower();
                   CPPUNIT_ASSERT_EQUAL_MESSAGE(loopMessage, mimeSubtype, sdpMimeSubtype);
                   CPPUNIT_ASSERT_EQUAL_MESSAGE(loopMessage, codecSampleRate, primaryCodec.getSampleRate());
                   CPPUNIT_ASSERT_EQUAL_MESSAGE(loopMessage, codecChannels, primaryCodec.getNumChannels());
                   UtlString sdpFmtp;
                   primaryCodec.getSdpFmtpField(sdpFmtp);
                   // If fmtp is empty string, we could get back the explicit parameters in sdpFmtp
                   if("" != fmtp)
                   {
                       CPPUNIT_ASSERT_EQUAL_MESSAGE(loopMessage, fmtp, sdpFmtp);
                   } 
                   // Create a queue to receive notifications from the resources
                   OsMsg* pMsg;
                   OsMsgDispatcher notifDispatcher;
 
                   // Now we enable the flowgraph..  Which should enable resources.
                   CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->enable());
                   CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->start());
 
                   mpFlowGraph->setNotificationDispatcher(&notifDispatcher);

                   // Set the encoder
                   CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                        MprEncode::selectCodecs(ENCODER_RESOURCE_NAME,
                                        *mpFlowGraph->getMsgQ(),
                                        &primaryCodec,
                                        NULL)); // DTMF codec

                   // Setup the decoder
                   // Should never do this in production code.  Hwever we can call a direct
                   // method on resource here as we know the flowgraph is not being processed.
                   CPPUNIT_ASSERT(mprEncoderDecoder);
                   // Scope for codec factory
                   {
                       MpCodecFactory* codecFactory = MpCodecFactory::getMpCodecFactory();
                       CPPUNIT_ASSERT(codecFactory != NULL);
 
                       mprEncoderDecoder->setCodec(*codecFactory,
                                                   mimeSubtype,
                                                   sdpFmtp, // use the explicit FMTP if set
                                                   codecSampleRate,
                                                   codecChannels);
                   }
 
                   // Start reading in the source file
                   CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                        MprFromFile::playFile(FROM_FILE_RESOURCE_NAME,
                                                              *mpFlowGraph->getMsgQ(),
                                                              mpFlowGraph->getSamplesPerSec(),
                                                              sourceFileName,
                                                              FALSE)); // Do not repeat at end of file
 
 
                   // Process frames until we get an end of playout of the input file 
                   int frameCount = 0;
                   int numFromFileFrames = -1;
                   int startDecodingFrame = -1;
                   int stillReadingFile = TRUE;
                   while(stillReadingFile)
                   {
                      UtlString frameLoopMessage(loopMessage);
                      frameLoopMessage.appendFormat(" frame: %d", frameCount);
 
                      mpFlowGraph->processNextFrame();
                      while(notifDispatcher.numMsgs())
                      {
                          CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, notifDispatcher.receive(pMsg, OsTime(0)));
                          CPPUNIT_ASSERT(pMsg);
                          MpResNotificationMsg* pNotif = (MpResNotificationMsg*) pMsg;
                          UtlString notifyingResourceName = pNotif->getOriginatingResourceName();
 
                          switch(pNotif->getMsg())
                          {
                          case MpResNotificationMsg::MPRNM_FROMFILE_STARTED: // 1
                              printf("Frame[%d]: started reading: %s\n", 
                                     frameCount,
                                     sourceFileName.data());
                              break;
                          case MpResNotificationMsg::MPRNM_FROMFILE_STOPPED: // 4
                              printf("Frame[%d]: stopped reading: %s\n", 
                                     frameCount,
                                     sourceFileName.data());
                              break;
                          case MpResNotificationMsg::MPRNM_FROMFILE_FINISHED: // 5
                          {
                                  // Keep pushing through frames of silence so that we can
                                  // get one more RTP packet through.  This silence will 
                                  // push the latent samples in the resampler through so
                                  // the last frame is complete in the recording.
                                  MpAudioSample silence[TEST_SAMPLES_PER_FRAME];
                                  memset(silence, 0, TEST_SAMPLES_PER_FRAME * sizeof(MpAudioSample));
                                  CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                                       MprFromFile::playBuffer(FROM_FILE_RESOURCE_NAME,
                                                                               *mpFlowGraph->getMsgQ(),
                                                                               (char*) silence,
                                                                               TEST_SAMPLES_PER_FRAME * sizeof(MpAudioSample),
                                                                               mpFlowGraph->getSamplesPerSec(),
                                                                               mpFlowGraph->getSamplesPerSec(),
                                                                               0, // type: raw PCM
                                                                               TRUE, // repeat
                                                                               NULL)); // no event/callback
                          }        
                              printf("Frame[%d]: finished reading: %s\n", 
                                     frameCount,
                                     sourceFileName.data());
                              numFromFileFrames = frameCount;
 
                              break;
 
                          case MpResNotificationMsg::MPRNM_FROMFILE_ERROR: // 6
                              CPPUNIT_ASSERT_MESSAGE((frameLoopMessage + UtlString(" file open error")).data(),0);
                              stillReadingFile = FALSE;
                              break;
 
                          case MpResNotificationMsg::MPRNM_RECORDER_STARTED: // 8
                              printf("Frame[%d]: Started recording to file: %s\n", 
                                     frameCount,
                                     recordFileName.data());
                              startDecodingFrame = frameCount;
                              break;
 
                          case MpResNotificationMsg::MPRNM_RECORDER_STOPPED: // 11
                              printf("Frame[%d]: Stopped recording to file: %s\n", 
                                     frameCount,
                                     recordFileName.data());
                              stillReadingFile = FALSE;
                              break;
 
                          case MpResNotificationMsg::MPRNM_RX_STREAM_ACTIVITY: // 18
                          {
                              MprnRtpStreamActivityMsg* streamNotif = (MprnRtpStreamActivityMsg*)pNotif;
                              UtlString streamActivity;
                              switch(streamNotif->getState())
                              {
                              case MprnRtpStreamActivityMsg::STREAM_START:
                              {
                                  streamActivity = "start";

                                  // The encoder wrote the first RTP packet.
                                  // Start recording now.
                                  OsStatus recordStartStatus = 
                                      MprRecorder::startFile(RECORDER_RESOURCE_NAME,
                                                             *mpFlowGraph->getMsgQ(),
                                                             recordFileName,
                                                             MprRecorder::WAV_PCM_16,
                                                             0, // no time limit
                                                             -1, // don't stop after silence
                                                             FALSE, // don't append file
                                                             2); // channels
                                  CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                        recordStartStatus);

                                  // No sense in continuing if we cannot record
                                  stillReadingFile = recordStartStatus == OS_SUCCESS ? TRUE : FALSE;
                                  break;
                              }
                              case MprnRtpStreamActivityMsg::STREAM_STOP:
                                  streamActivity = "stop";
                                  break;

                              case MprnRtpStreamActivityMsg::STREAM_CHANGE:
                                  streamActivity = "change";
                                  break;
                              } // end switch
                              printf("Frame[%d]: %s RTP (%s)\n", 
                                     frameCount,
                                     streamActivity.data(),
                                     notifyingResourceName.data());
                          }
                              break;
 
                          default:
                              CPPUNIT_ASSERT(0);
                              printf("Frame[%d]: message type: %d from resource: %s\n",
                                     frameCount,
                                     pNotif->getMsg(),
                                     pNotif->getOriginatingResourceName().data());
                              stillReadingFile = FALSE;
                              break;
                          }
                      }
                      if(frameCount == startDecodingFrame + numFromFileFrames)
                      {
                          // Send message to stop the recording
                          CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                               MprRecorder::stop(RECORDER_RESOURCE_NAME,
                                                                 *mpFlowGraph->getMsgQ()));
                      }
                      frameCount++;
                   }
 
                   printf("Processed %d frames\n", 
                          frameCount);
 
                   // If the analyze environmental variable is defined, call the analyse programe
                   // with the source, result records and the codec info
#ifdef __pingtel_on_posix__
                   const char* analyseProgramName = getenv("CODEC_QUALITY_ANALYSE_PROGRAM");
                   if(analyseProgramName && analyseProgramName[0])
                   {
                       UtlString command;
                       // <analyse_command> <source_recording> <result_recording> <codec_mime_subtype> <codec_sample_rate> <codec_channels> "<codec_fmtp>"
                       command.appendFormat("%s %s %s %s %d %d \"%s\"",
                                            analyseProgramName,
                                            sourceFileName.data(),
                                            recordFileName.data(),
                                            mimeSubtype.data(),
                                            codecSampleRate,
                                            codecChannels,
                                            fmtp.data());
                       int analyseResult = system(command.data());
                       CPPUNIT_ASSERT_EQUAL_MESSAGE(loopMessage, 0, analyseResult);
                   }
                   else
#endif
                   {
                       printf("CODEC_QUALITY_ANALYSE_PROGRAM not defined, skipping analysis of codec recording: %s\n",
                              loopMessage.data());
                   }

                }


                // Get next file in directory
                foundRecordingStatus = sourceRecordingIterator.findNext(aSourceRecording);
            } // End loop over source recording files

            // Step to next fmtp or codec
            if(codecFmtpIndex + 1 >= codecFmtpCount)
            {
                codecFmtpIndex = 0;
                codecIndex++;
            }
            else
            {
                codecFmtpIndex++;
            }
        } // end loop over codecs
    }

private:
   MpFlowGraphBase* mpFlowGraph; 
   MprFromFile* mprFromFile;
   MprEncode* mprEncoder;
   MprEncoderDecoder* mprEncoderDecoder;
   MprSplitter* mprSplitter;
   MprRecorder* mprRecorder;
};

CPPUNIT_TEST_SUITE_REGISTRATION(MpCodecsQualityTest);
