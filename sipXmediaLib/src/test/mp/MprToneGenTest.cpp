//
// Copyright (C) 2006-2017 SIPez LLC. All rights reserved.
//  
// Copyright (C) 2007-2008 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>

// SYSTEM INCLUDES
#include <os/OsIntTypes.h>
#include <math.h>

// APPLICATION INCLUDES
#include <mp/MpMediaTask.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MpTestResource.h>
#include <mp/MprToneGen.h>
#include <mp/MpDTMFDetector.h>
#include "mp/MpGenericResourceTest.h"

///  Unit test for MprToneGen
class MprToneGenTest : public MpGenericResourceTest
{
   CPPUNIT_TEST_SUB_SUITE(MprToneGenTest, MpGenericResourceTest);
   CPPUNIT_TEST(testToneAuthenticity);
   CPPUNIT_TEST_SUITE_END();

public:
   void testToneAuthenticity()
   {
      MprToneGen* pToneGen = NULL;
      OsStatus res;

      int framesToProcess = 3;

      // Create the Goertzel DTMF detector -- sample rate and goertzel_n doesn't
      // really matter here, since we'll be resetting it in the loop.
      // But to illustrate, here is what should be used for 8khz
      MpDtmfDetector dtmfDetector(8000, framesToProcess*80);

      size_t i;
      for(i = 0; i < sNumRates; i++)
      {
         //printf("Testing frequency authenticity at %d Hz\n", sSampleRates[i]);

         // For this test, we want to modify the sample rate and samples per frame
         // so we need to de-inititialize what has already been initialized for us
         // by cppunit, or by a previous loop.
         tearDown();

         // Set the sample rates
         setSamplesPerSec(sSampleRates[i]);
         setSamplesPerFrame(sSampleRates[i]/100);
         dtmfDetector.setSamplesPerSec(sSampleRates[i]);
         dtmfDetector.setNumProcessSamples(framesToProcess*getSamplesPerFrame());
         setUp();

         pToneGen = new MprToneGen("MprToneGen", "");
         CPPUNIT_ASSERT(pToneGen != NULL);

         // This sets up to have some input to our resource, and something to collect
         // output at the end.  In case of tone gen, the input doesn't matter,
         // as it only uses the input when it is in a disabled state (passing
         // the input directly to output - no changes).
         setupFramework(pToneGen);

         // pToneGen enabled, there are buffers on the input 0
         CPPUNIT_ASSERT(mpSourceResource->enable());
         CPPUNIT_ASSERT(pToneGen->enable());

         // Tell our MprToneGen to generate a '4' DTMF tone playing.
         MprToneGen::startTone("MprToneGen", *(mpFlowGraph->getMsgQ()), '4');

         int j;
         for(j = 0; j < framesToProcess; j++)
         {
            // Process a frame.
            res = mpFlowGraph->processNextFrame();
            CPPUNIT_ASSERT(res == OS_SUCCESS);

            // Now analyze the newly processed output.
            // A new buffer should be generated.
            CPPUNIT_ASSERT(mpSourceResource->mLastDoProcessArgs.outBufs[0] != 
                           mpSinkResource->mLastDoProcessArgs.inBufs[0]);

            MpAudioBufPtr paBuf = mpSinkResource->mLastDoProcessArgs.inBufs[0];
            CPPUNIT_ASSERT(paBuf.isValid());

            // SANITY CHECK: Make sure that the number of samples in the 
            // tone gen output frame is equal to the samples per frame that 
            // we set during this run.
            CPPUNIT_ASSERT_EQUAL(getSamplesPerFrame(), paBuf->getSamplesNumber());

            UtlBoolean dtmfDetected = FALSE;
            // Now grab audio samples and run them through the dtmf detector.
            unsigned k;
            for(k = 0; k < paBuf->getSamplesNumber(); k++)
            {
               const MpAudioSample* pSamples = paBuf->getSamplesPtr();
               dtmfDetected = dtmfDetector.processSample(pSamples[k]);

               // If we are at the last sample we will process, then, based on how
               // we configured the dtmf detector, a tone should have been detected.
               if( k == paBuf->getSamplesNumber()-1 && j == framesToProcess-1)
               {
                  CPPUNIT_ASSERT_EQUAL(TRUE, dtmfDetected);
               }
               else
               {
                  CPPUNIT_ASSERT_EQUAL(FALSE, dtmfDetected);
               }
            }

            // Free up buffers..
            paBuf.release();
         }

         // Now for the real test that we were building up for --
         // the DTMF detector should have detected a '4' tone.
         // If it didn't then something went wrong -- one thing that could
         // have gone wrong in that case, is that the generation was assuming
         // a different sample rate than I.
         char detectedDTMF = dtmfDetector.getLastDetectedDTMF();
         CPPUNIT_ASSERT_EQUAL('4', detectedDTMF);

         // Stop flowgraph
         haltFramework();

         // No need to delete tone gen, as haltFramework deletes all resources
         // in the flowgraph.
      }
   }

private:

};

CPPUNIT_TEST_SUITE_REGISTRATION(MprToneGenTest);
