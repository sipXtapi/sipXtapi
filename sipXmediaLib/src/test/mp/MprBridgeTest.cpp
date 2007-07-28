//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#include <mp/MpMediaTask.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MpTestResource.h>
#include <mp/MpMisc.h>
#include <mp/MprBridge.h>
#include <mp/MpBufferMsg.h>
#include <os/OsDateTime.h>

#include "mp/MpGenericResourceTest.h"

#ifdef RTL_ENABLED // [
#  include "rtl_macro.h"
#else  // RTL_ENABLED ][
#  define RTL_WRITE(x)
#  define RTL_BLOCK(x)
#  define RTL_START(x)
#  define RTL_STOP
#endif // RTL_ENABLED ]

///  Unit test for MprBridge
class MprBridgeTest : public MpGenericResourceTest
{
    CPPUNIT_TEST_SUB_SUITE(MprBridgeTest, MpGenericResourceTest);
    CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST(testDisabled);
    CPPUNIT_TEST(testEnabledNoData);
    CPPUNIT_TEST(testEnabledWithOneActiveInput);
    CPPUNIT_TEST(testEnabledWithManyActiveInputs);
    CPPUNIT_TEST(testSideBar);
    CPPUNIT_TEST(testMixNormalWeights);
    CPPUNIT_TEST(testSimpleMixPerformance);
    CPPUNIT_TEST_SUITE_END();

public:

   void testCreators()
   {
       MprBridge*        pBridge    = NULL;
       OsStatus          res;

       // when we have a flow graph that contains resources and links,
       // verify that destroying the flow graph also gets rid of the resources
       // and links.
       pBridge = new MprBridge("MprBridge", 10,
                               TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);

       res = mpFlowGraph->addResource(*pBridge);
       CPPUNIT_ASSERT(res == OS_SUCCESS);
   }

   void testDisabled()
   {
       MprBridge*        pBridge    = NULL;
       OsStatus          res;
       int               i;

       pBridge = new MprBridge("MprBridge", 10,
                               TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);
       CPPUNIT_ASSERT(pBridge != NULL);

       setupFramework(pBridge);

       // pBridge disabled, there are buffers on all inputs.
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pBridge->disable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // No data should be generated.
       for (i=1; i<pBridge->maxOutputs(); i++)
          CPPUNIT_ASSERT(!mpSinkResource->mLastDoProcessArgs.inBufs[i].isValid());

       // Stop flowgraph
       haltFramework();
   }

   void testEnabledNoData()
   {
       MprBridge*        pBridge    = NULL;
       OsStatus          res;

       pBridge = new MprBridge("MprBridge", 10,
                               TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);
       CPPUNIT_ASSERT(pBridge != NULL);

       setupFramework(pBridge);

       // pBridge enabled, there are no buffers on the input 0
       CPPUNIT_ASSERT(mpSourceResource->disable());
       CPPUNIT_ASSERT(pBridge->enable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // We did not generated any buffers on remote parts
       for (int i=1; i<pBridge->maxOutputs(); i++)
          CPPUNIT_ASSERT(!mpSinkResource->mLastDoProcessArgs.inBufs[i].isValid());

       // Mixer does not generate data on the local output, cause it can't find
       // any active audio.
       CPPUNIT_ASSERT(!mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid());

       // Stop flowgraph
       haltFramework();
   }

   void testEnabledWithOneActiveInput()
   {
       MprBridge*        pBridge    = NULL;
       MpBufPtr          pBuf;
       OsStatus          res;

       pBridge = new MprBridge("MprBridge", 10,
                               TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);
       CPPUNIT_ASSERT(pBridge != NULL);

       setupFramework(pBridge);

       // pBridge enabled, there are buffers on the input 0
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pBridge->enable());

       // Generate buffers on outputs 0 (simulate local mic) and 1 (simulate one
       // remote party).
       mpSourceResource->setGenOutBufMask(0x03);

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Local microphone data (input 0) should be copied to the first remote
       // connection (output 1)
       CPPUNIT_ASSERT(mpSinkResource->mLastDoProcessArgs.inBufs[1] ==
                      mpSourceResource->mLastDoProcessArgs.outBufs[0]);

       // Data from the input 1 (first remote connection) should be copied to
       // the local speaker.
       CPPUNIT_ASSERT(mpSinkResource->mLastDoProcessArgs.inBufs[0] ==
                      mpSourceResource->mLastDoProcessArgs.outBufs[1]);

       // Other remote connections (outputs 2..MAX_BRIDGE_PORTS) should receive
       // mixed audio.
       for (int i=2; i<pBridge->maxOutputs(); i++)
          CPPUNIT_ASSERT(mpSinkResource->mLastDoProcessArgs.inBufs[i].isValid());

       // Free stored buffer
       pBuf.release();

       // Stop flowgraph
       haltFramework();
   }

   void testEnabledWithManyActiveInputs()
   {
       MprBridge*        pBridge    = NULL;
       MpBufPtr          pBuf;
       OsStatus          res;

       pBridge = new MprBridge("MprBridge", 10,
                               TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);
       CPPUNIT_ASSERT(pBridge != NULL);

       setupFramework(pBridge);

       // pBridge enabled, there are buffers on the input 0
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pBridge->enable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Store output buffer for convenience
       pBuf = mpSinkResource->mLastDoProcessArgs.inBufs[0];

       // We did not generated any buffers on remote parts
       for (int i=1; i<pBridge->maxOutputs(); i++)
          CPPUNIT_ASSERT(mpSinkResource->mLastDoProcessArgs.inBufs[i].isValid());

       // Mixer does not generate data on the local output, cause it can't find
       // any active audio.
       CPPUNIT_ASSERT(mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid());

       // Free stored buffer
       pBuf.release();

       // Stop flowgraph
       haltFramework();
   }

   void testSideBar()
   {
       RTL_START(1000000);

       const int         numParticipants = 4;
       MprBridge*        pBridge    = NULL;

       pBridge = new MprBridge("MprBridge", numParticipants,
                               TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);
       CPPUNIT_ASSERT(pBridge != NULL);

       setupFramework(pBridge);

       // Generate square samples on all bridge inputs. Square period of data
       // on input N is 2*N:
       //   input 0:  -_-_-_-_-_-_-_-_
       //   input 1:  --__--__--__--__
       //   input 2:  ---___---___---_
       //   input 3:  ----____----____
       // Amplitude of generated signal is 100*N.
       CPPUNIT_ASSERT(mpSourceResource->enable());
       int outIndex;
       mpSourceResource->setOutSignalType(MpTestResource::MP_TEST_SIGNAL_SQUARE);
       for(outIndex = 0; outIndex < numParticipants; outIndex++)
       {
          mpSourceResource->setSignalPeriod(outIndex, (outIndex+1) * 2);
          mpSourceResource->setSignalAmplitude(outIndex, (outIndex+1) * 100);
       }

       // Set the weights such that we have a side bar conversation
       // between input 1 and 3 such that 1 & 3 hear 0 & 2 at a lower
       // volume and 0 & 2 do not hear 1 & 3 at all.  The matrix for the
       // weights is shown below, where x is the decrease in volume that
       // 1 & 3 hear 0 & 2 at.
       // 0 0 1 0
       // x 0 x 1
       // 1 0 0 0
       // x 1 x 0
       
       // Use x = 0.75. If fixed point enabled, this value MUST BE convertible
       // to selected fixed point format without data lose for this test to
       // succeed. 
       float quiterFloat = 0.75f;
       MpBridgeGain quieter = MPF_BRIDGE_FLOAT(quiterFloat);
       MpBridgeGain gainsOut[numParticipants][numParticipants] = {
               {0, 0, MP_BRIDGE_GAIN_PASSTHROUGH, 0},
               {quieter, 0, quieter, MP_BRIDGE_GAIN_PASSTHROUGH},
               {MP_BRIDGE_GAIN_PASSTHROUGH, 0, 0, 0},
               {quieter, MP_BRIDGE_GAIN_PASSTHROUGH, quieter, 0}};
       OsMsgQ* flowgraphQueue = mpFlowGraph->getMsgQ();
       CPPUNIT_ASSERT(flowgraphQueue != NULL);
       for (int i=0; i<numParticipants; i++)
       {
          CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                               MprBridge::setMixWeightsForOutput("MprBridge",
                                                                 *flowgraphQueue,
                                                                 i,
                                                                 numParticipants,
                                                                 gainsOut[i]));
       }
       CPPUNIT_ASSERT(pBridge->enable());

       CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                            mpFlowGraph->processNextFrame());

       RTL_WRITE("testSideBar.rtl");
       RTL_STOP

       // Test samples, generated for output 0
       {
          MpAudioBufPtr pBuf = mpSinkResource->mLastDoProcessArgs.inBufs[0];
          CPPUNIT_ASSERT(pBuf.isValid());
          const MpAudioSample *pSamples = pBuf->getSamplesPtr();

          for (unsigned i=0; i<pBuf->getSamplesNumber(); i++)
          {
             MpAudioSample curSample =
                mpSourceResource->getSquareSampleValue(2, i);

             CPPUNIT_ASSERT_EQUAL(curSample, pSamples[i]);
          }
       }

       // Test samples, generated for output 1
       {
          MpAudioBufPtr pBuf = mpSinkResource->mLastDoProcessArgs.inBufs[1];
          CPPUNIT_ASSERT(pBuf.isValid());
          const MpAudioSample *pSamples = pBuf->getSamplesPtr();

          for (unsigned i=0; i<pBuf->getSamplesNumber(); i++)
          {
             MpAudioSample curSample = (MpAudioSample)(
                mpSourceResource->getSquareSampleValue(0, i)*quiterFloat +
                mpSourceResource->getSquareSampleValue(2, i)*quiterFloat +
                mpSourceResource->getSquareSampleValue(3, i));

             CPPUNIT_ASSERT_EQUAL(curSample, pSamples[i]);
          }
       }

       // Test samples, generated for output 2
       {
          MpAudioBufPtr pBuf = mpSinkResource->mLastDoProcessArgs.inBufs[2];
          CPPUNIT_ASSERT(pBuf.isValid());
          const MpAudioSample *pSamples = pBuf->getSamplesPtr();

          for (unsigned i=0; i<pBuf->getSamplesNumber(); i++)
          {
             MpAudioSample curSample =
                mpSourceResource->getSquareSampleValue(0, i);

             CPPUNIT_ASSERT_EQUAL(curSample, pSamples[i]);
          }
       }

       // Test samples, generated for output 3
       {
          MpAudioBufPtr pBuf = mpSinkResource->mLastDoProcessArgs.inBufs[3];
          CPPUNIT_ASSERT(pBuf.isValid());
          const MpAudioSample *pSamples = pBuf->getSamplesPtr();

          for (unsigned i=0; i<pBuf->getSamplesNumber(); i++)
          {
             MpAudioSample curSample = (MpAudioSample)(
                mpSourceResource->getSquareSampleValue(0, i)*quiterFloat +
                mpSourceResource->getSquareSampleValue(1, i) +
                mpSourceResource->getSquareSampleValue(2, i)*quiterFloat);

             CPPUNIT_ASSERT_EQUAL(curSample, pSamples[i]);
          }
       }

       // Stop flowgraph
       haltFramework();
   } // End testSideBar()


   void testMixNormalWeights()
   {
       const int         numParticipants = 15;
       MprBridge*        pBridge    = NULL;

       CPPUNIT_ASSERT(numParticipants < 16);
       pBridge = new MprBridge("MprBridge", numParticipants,
                               TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);
       CPPUNIT_ASSERT(pBridge != NULL);

       setupFramework(pBridge);

       CPPUNIT_ASSERT(mpSourceResource->enable());
       int outIndex;
       mpSourceResource->setOutSignalType(MpTestResource::MP_TEST_SIGNAL_SQUARE);
       // Each input is 2**N where N is port index with a fixed period of 2 samples
       // (i.e. oscilating from peak to -peak every sample)  This has the advantage
       // of being the same as a bit mask of input which contribute to any output.
       int peak = 1;
       for(outIndex = 0; outIndex < numParticipants; outIndex++)
       {
          mpSourceResource->setSignalPeriod(outIndex, 2);
          mpSourceResource->setSignalAmplitude(outIndex, peak);
          peak = peak * 2;
       }
       int generationMask = (1<<numParticipants)-1;
       mpSourceResource->setGenOutBufMask(generationMask);

       const MpBridgeGain I = MP_BRIDGE_GAIN_PASSTHROUGH;
       MpBridgeGain gainsOut[numParticipants][numParticipants] = {
          {0, I, I, I, 0, 0, I, 0, 0, 0, I, 0, 0, 0, I},
          {I, 0, I, I, I, 0, I, I, I, 0, I, I, I, 0, I},
          {I, 0, 0, 0, I, 0, 0, 0, I, 0, 0, 0, I, 0, 0},
          {0, 0, I, 0, 0, 0, I, 0, 0, 0, I, 0, 0, 0, I},
          {0, 0, I, 0, 0, 0, I, 0, 0, 0, I, 0, 0, 0, I},
          {I, 0, I, I, I, 0, I, I, I, 0, I, I, I, 0, I},
          {I, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // This output and next one
          {I, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // just copy data from one input
          {0, 0, I, 0, 0, 0, I, 0, 0, 0, I, 0, 0, 0, I},
          {I, 0, I, I, I, 0, I, I, I, 0, I, I, I, 0, I},
          {I, 0, 0, 0, I, 0, 0, 0, I, 0, 0, 0, I, 0, 0},
          {0, 0, I, 0, 0, 0, I, 0, 0, 0, I, 0, 0, 0, I},
          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // Muted output
          {I, 0, I, I, I, 0, I, I, I, 0, I, I, I, 0, I},
          {I, 0, 0, 0, I, 0, 0, 0, I, 0, 0, 0, I, 0, 0}};
       // Uncomment following code to set gain matrix to simple conference case.
/*       int row, column;
       for(row = 0; row < numParticipants; row++)
       {
          for(column = 0; column < numParticipants; column++)
          {
             if(row == column)
             {
                gainsOut[row][column] = 0;
             }
             else
             {
                gainsOut[row][column] = MP_BRIDGE_GAIN_PASSTHROUGH;
             }
          }
       }
*/

       OsMsgQ* flowgraphQueue = mpFlowGraph->getMsgQ();
       CPPUNIT_ASSERT(flowgraphQueue != NULL);

       int i;
       for (i=0; i < numParticipants; i++)
       {
          CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                               MprBridge::setMixWeightsForOutput("MprBridge",
                                                                 *flowgraphQueue,
                                                                 i,
                                                                 numParticipants,
                                                                 gainsOut[i]));
       }
       CPPUNIT_ASSERT(pBridge->enable());

       CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                            mpFlowGraph->processNextFrame());

       // For each output check the frame of samples
       int outputIndex;
       for(outputIndex = 0; outputIndex < numParticipants; outputIndex++)
       {
          int inputIndex;
          MpAudioSample magnitude = 0;
          for(inputIndex = 0; inputIndex < numParticipants; inputIndex++)
          {
             if(mpSourceResource->mLastDoProcessArgs.outBufs[inputIndex].isValid() &&
                gainsOut[outputIndex][inputIndex])
             {
                magnitude += (1 << inputIndex);
             }
          }
          printf("Expected output[%d] magnitude: %d (0x%X)\n", outputIndex, magnitude, magnitude);

          if (magnitude == 0)
          {
             // Output should be silent.
             CPPUNIT_ASSERT(!mpSinkResource->mLastDoProcessArgs.inBufs[outputIndex].isValid());
          }
          else
          {
             // Active (not silent) output.

             // Get samples from produced buffer.
             MpAudioBufPtr pBuf = mpSinkResource->mLastDoProcessArgs.inBufs[outputIndex];
             CPPUNIT_ASSERT(pBuf.isValid());
             const MpAudioSample* samplePtr = pBuf->getSamplesPtr();
             int numSamples = pBuf->getSamplesNumber();

             // The even numbered samples should be equal to the magnitude of the
             // odd numbered ones.  The odd numbered should be negative
             int sampleIndex;
             for(sampleIndex = 0; sampleIndex < numSamples; sampleIndex+=2)
             {
                CPPUNIT_ASSERT_EQUAL(magnitude, samplePtr[sampleIndex]);
                CPPUNIT_ASSERT_EQUAL((MpAudioSample)(0 - samplePtr[sampleIndex+1]),
                   samplePtr[sampleIndex]);
                CPPUNIT_ASSERT(samplePtr[sampleIndex] > 0);
             }
          }
       }

       // Stop flowgraph
       haltFramework();

   } // end testMixNormalWeights()

   void testSimpleMixPerformance()
   {
       const int         numParticipants = 8;
       MprBridge*        pBridge    = NULL;

       CPPUNIT_ASSERT(numParticipants < 16);
       pBridge = new MprBridge("MprBridge", numParticipants,
                               TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);
       CPPUNIT_ASSERT(pBridge != NULL);

       setupFramework(pBridge);

       CPPUNIT_ASSERT(mpSourceResource->enable());
       int outIndex;
       mpSourceResource->setOutSignalType(MpTestResource::MP_TEST_SIGNAL_SQUARE);
       // Each input is 2**N where N is port index with a fixed period of 2 samples
       // (i.e. oscilating from peak to -peak every sample)  This has the advantage
       // of being the same as a bit mask of input which contribute to any output.
       int peak = 1;
       for(outIndex = 0; outIndex < numParticipants; outIndex++)
       {
          mpSourceResource->setSignalPeriod(outIndex, 2);
          mpSourceResource->setSignalAmplitude(outIndex, peak);
          peak = peak * 2;
       }

       MpBridgeGain gainsOut[numParticipants][numParticipants];
       int row, column;
       for(row = 0; row < numParticipants; row++)
       {
          for(column = 0; column < numParticipants; column++)
          {
             if(row == column)
             {
                gainsOut[row][column] = 0;
             }
             else
             {
                gainsOut[row][column] = MPF_BRIDGE_FLOAT(2.0f);
             }
          }
       }

       OsMsgQ* flowgraphQueue = mpFlowGraph->getMsgQ();
       CPPUNIT_ASSERT(flowgraphQueue != NULL);

       int i;
       for (i=0; i < numParticipants; i++)
       {
          CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                               MprBridge::setMixWeightsForOutput("MprBridge",
                                                                 *flowgraphQueue,
                                                                 i,
                                                                 numParticipants,
                                                                 gainsOut[i]));
       }
       CPPUNIT_ASSERT(pBridge->enable());

       // Let flowgraph process all messages before we'll start
       CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                            mpFlowGraph->processNextFrame());

       int framesToProcess = 10000;
       int frameCount;
       OsTime start;
       OsTime end;
       OsDateTime::getCurTime(start);

       for(frameCount = 0; frameCount < framesToProcess; frameCount++)
       {
          CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                               mpFlowGraph->processNextFrame());
       }
       OsDateTime::getCurTime(end);

       OsTime lapse = end - start;
       printf("process %d bridge frames: %ld.%06ld\n", 
              framesToProcess,
              lapse.seconds(),
              lapse.usecs());

       // Stop flowgraph
       haltFramework();

   } // end testSimpleMixPerformance()

};


CPPUNIT_TEST_SUITE_REGISTRATION(MprBridgeTest);


