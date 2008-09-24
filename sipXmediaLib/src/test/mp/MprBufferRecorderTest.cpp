//  
// Copyright (C) 2008 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#include <mp/MpMediaTask.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MpTestResource.h>
#include <mp/MpMisc.h>
#include <mp/MprBufferRecorder.h>
#include <mp/MpBufferMsg.h>

#include "mp/MpGenericResourceTest.h"

///  Unit test for MprBufferRecorder
class MprBufferRecorderTest : public MpGenericResourceTest
{
   CPPUNIT_TEST_SUB_SUITE(MprBufferRecorderTest, MpGenericResourceTest);
   CPPUNIT_TEST(testCreators);
   CPPUNIT_TEST(testDisabled);
   CPPUNIT_TEST(testEnabled);
   CPPUNIT_TEST_SUITE_END();

public:

   void testCreators()
   {
      MprBufferRecorder*  pBufRec = NULL;

      // when we have a flow graph that contains resources and links,
      // verify that destroying the flow graph also gets rid of the resources
      // and links.
      pBufRec = new MprBufferRecorder("MprBufferRecorder");

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addResource(*pBufRec));
   }

   void testDisabled()
   {
      MprBufferRecorder* pBufRec   = NULL;

      pBufRec = new MprBufferRecorder("MprBufferRecorder");
      CPPUNIT_ASSERT(pBufRec != NULL);

      setupFramework(pBufRec);

      // TESTCASE 1:
      // pBufRec disabled, there are no buffers on the input 0.
      CPPUNIT_ASSERT(mpSourceResource->disable());
      CPPUNIT_ASSERT(pBufRec->disable(MprBufferRecorder::RECORD_STOPPED));

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->processNextFrame());

      // We did not generated any buffers
      CPPUNIT_ASSERT(  !mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                    && !mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid());

      // TESTCASE 2:
      // pBufRec disabled, there are buffers on the input 0.
      CPPUNIT_ASSERT(mpSourceResource->enable());
      CPPUNIT_ASSERT(pBufRec->disable(MprBufferRecorder::RECORD_STOPPED));

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->processNextFrame());

      // Buffer should be passed through
      CPPUNIT_ASSERT(  mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                    && (  mpSourceResource->mLastDoProcessArgs.outBufs[0]
                       == mpSinkResource->mLastDoProcessArgs.inBufs[0]));

         // Stop flowgraph
         haltFramework();
   }

   void testEnabled()
   {
      for (size_t i = 0; i < sNumRates; i++)	 
      {	 
         printf("Test sample rate %d\n", sSampleRates[i]);	 
         // For this test, we want to modify the sample rate and samples per frame	 
         // so we need to de-inititialize what has already been initialized for us	 
         // by cppunit, or by a previous loop.	 
         tearDown();	 

         // Set the sample rates	 
         setSamplesPerSec(sSampleRates[i]);	 
         setSamplesPerFrame(sSampleRates[i]/100);	 
         setUp();

         testEnabledHelper();
      }

   }

protected:

   void testEnabledHelper()
   {
      const UtlString     recorderName = "MprBufferRecorder";
      MprBufferRecorder*  pBufRec   = NULL;
      UtlString           buffer;
      MpAudioBufPtr       pReferenceBuf;
      const MpAudioSample *referenceSamples;
      const MpAudioSample *bufferSamples;
      unsigned            frameSize;
      unsigned            frameSizeMs;
      unsigned            i;

      pBufRec = new MprBufferRecorder(recorderName);
      CPPUNIT_ASSERT(pBufRec != NULL);

      setupFramework(pBufRec);
      frameSize = mpFlowGraph->getSamplesPerFrame();
      frameSizeMs = frameSize*1000/mpFlowGraph->getSamplesPerSec();

      // We'll record 2.5 frames of audio. First will be silent, and next ones
      // will be filled with squre waves.
      CPPUNIT_ASSERT(MprBufferRecorder::startRecording(recorderName,
                                                       *mpFlowGraph->getMsgQ(),
                                                       (frameSizeMs*5)/2,
                                                       &buffer));

      //
      // TEST 1: Do not pass buffer to input, a frame of silence should be recorded.
      //
      CPPUNIT_ASSERT(mpSourceResource->disable());

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->processNextFrame());

      // We did not generated any buffers
      CPPUNIT_ASSERT(  !mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                    && !mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid());

      // Check that we recorded a frame of silence
      CPPUNIT_ASSERT_EQUAL(frameSize*sizeof(MpAudioSample), buffer.length());
      for (i=0; i < frameSize; i++)
      {
         CPPUNIT_ASSERT_EQUAL(MpAudioSample(0), ((MpAudioSample*)buffer.data())[i]);
      }

      //
      // TEST 2: Buffer with square waves will passed to recorder and should be
      //         recorded.
      //
      mpSourceResource->setOutSignalType(MpTestResource::MP_TEST_SIGNAL_SQUARE);
      mpSourceResource->setSignalPeriod(0, 2);
      mpSourceResource->setSignalAmplitude(0, 100);
      CPPUNIT_ASSERT(mpSourceResource->enable());

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->processNextFrame());

      // Buffer should be passed through
      CPPUNIT_ASSERT(  mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                    && (  mpSourceResource->mLastDoProcessArgs.outBufs[0]
                       == mpSinkResource->mLastDoProcessArgs.inBufs[0]));

      // Check that we recorded frame data correctly
      CPPUNIT_ASSERT_EQUAL(frameSize*sizeof(MpAudioSample)*2, buffer.length());
      pReferenceBuf = mpSourceResource->mLastDoProcessArgs.outBufs[0];
      referenceSamples = pReferenceBuf->getSamplesPtr();
      bufferSamples = (MpAudioSample*)buffer.data() + frameSize;
      for (i=0; i < frameSize; i++)
      {
         CPPUNIT_ASSERT_EQUAL(referenceSamples[i], bufferSamples[i]);
      }

      //
      // TEST 3: Buffer with square waves will passed to recorder, but recorder
      //         do not have enough space for it. Only half of it should be
      //         recorded and recorder should stop after this.
      //
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->processNextFrame());

      // Buffer should be passed through
      CPPUNIT_ASSERT(  mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                    && (  mpSourceResource->mLastDoProcessArgs.outBufs[0]
                       == mpSinkResource->mLastDoProcessArgs.inBufs[0]));

      // Check that we recorded frame data correctly
      CPPUNIT_ASSERT_EQUAL((frameSize*sizeof(MpAudioSample)*5)/2, buffer.length());
      pReferenceBuf = mpSourceResource->mLastDoProcessArgs.outBufs[0];
      referenceSamples = pReferenceBuf->getSamplesPtr();
      bufferSamples = (MpAudioSample*)buffer.data() + 2*frameSize;
      for (i=0; i < frameSize/2; i++)
      {
         CPPUNIT_ASSERT_EQUAL(referenceSamples[i], bufferSamples[i]);
      }

      // Recorder should be stopped.
      CPPUNIT_ASSERT_EQUAL(MprBufferRecorder::RECORD_FINISHED, pBufRec->getState());

      // Stop flowgraph
      haltFramework();
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(MprBufferRecorderTest);
