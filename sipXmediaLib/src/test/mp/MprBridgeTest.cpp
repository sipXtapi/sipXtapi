//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <mp/MpMediaTask.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MpTestResource.h>
#include <mp/MpMisc.h>
#include <mp/MprBridge.h>
#include <mp/MpBufferMsg.h>

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
       MpBufPtr          pBuf;
       OsStatus          res;
       int               i;

       pBridge = new MprBridge("MprBridge", 10,
                               TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);
       CPPUNIT_ASSERT(pBridge != NULL);

       setupFramework(pBridge);

       // TESTCASE 1:
       // pBridge disabled, there are no buffers on the input 0.
       CPPUNIT_ASSERT(mpSourceResource->disable());
       CPPUNIT_ASSERT(pBridge->disable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // We did not generated any buffers on remote parts
       for (i=1; i<pBridge->maxOutputs(); i++)
          CPPUNIT_ASSERT(!mpSinkResource->mLastDoProcessArgs.inBufs[i].isValid());

       // Mixer does not generate data on the local output, cause it can't find
       // any active audio.
       CPPUNIT_ASSERT(!mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid());

       // TESTCASE 2:
       // pBridge disabled, there are buffers on all inputs.
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pBridge->disable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Local microphone data (input 0) should be copied to all remote parts
       // (outputs 1..MAX_BRIDGE_PORTS)
       pBuf = mpSourceResource->mLastDoProcessArgs.outBufs[0];
       for (i=1; i<pBridge->maxOutputs(); i++)
          CPPUNIT_ASSERT(mpSinkResource->mLastDoProcessArgs.inBufs[i] == pBuf);

       // Mixer generate data on the local output, cause it try to do mixing.
       CPPUNIT_ASSERT(mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid());

       // Free stored buffer
       pBuf.release();

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
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MprBridgeTest);
