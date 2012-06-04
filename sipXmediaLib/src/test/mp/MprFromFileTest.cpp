//  
// Copyright (C) 2008-2012 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>
#include <os/OsIntTypes.h>
#include <sipxunittests.h>
#include <sipxunit/TestUtilities.h>

#include <mp/MprFromFile.h>
#include <mp/MpDTMFDetector.h>
#include "mp/MpGenericResourceTest.h"

// Include static wave data headers.
// These were generated from 16bit 48kHz mono raw sound files, using the following command:
// $ incbin.exe file.raw dtmf5_48kHz_16b_signed.h -n=dtmf5_48kHz_16b_signed -c=13 -d -h
#include "mp/dtmf5_48khz_16b_signed.h"
#define DTMF5_FN "dtmf5_48khz_16b_signed.wav"

/**
* Unittest for Wide band support in input and output device driver
*/
class MprFromFileTest : public MpGenericResourceTest
{
   CPPUNIT_TEST_SUITE(MprFromFileTest);
   CPPUNIT_TEST(testFileToneDetect);
   CPPUNIT_TEST(testBufferToneDetect);
   CPPUNIT_TEST_SUITE_END();


public:
   enum TestAudioSource { MpfftFile = 0, MpfftBuffer, MpfftMaxSources };

   // This is a CPPUNIT compliant method that actually does the calling of the 
   // test, giving various values for sample rates.
   void testFileToneDetect()
   {
      int rateIdx;
      for (rateIdx = 0; rateIdx < sNumRates; rateIdx++)
      {
         printf("Test playFile %d Hz\n", sSampleRates[rateIdx]);
         // For this test, we want to modify the sample rate and samples per frame
         // so we need to de-inititialize what has already been initialized for us
         // by cppunit, or by a previous loop.
         tearDown();

         // Set the sample rates 
         setSamplesPerSec(sSampleRates[rateIdx]);
         setSamplesPerFrame(sSampleRates[rateIdx]/100);
         setUp();
         testPlayToneDetectHelper(MpfftFile, sSampleRates[rateIdx], sSampleRates[rateIdx]/100);
      }
   }

   // This is a CPPUNIT compliant method that actually does the calling of the 
   // test, giving various values for sample rates.
   void testBufferToneDetect()
   {
      int rateIdx;
      for (rateIdx = 0; rateIdx < sNumRates; rateIdx++)
      {
         printf("Test playBuffer %d Hz\n", sSampleRates[rateIdx]);
         // For this test, we want to modify the sample rate and samples per frame
         // so we need to de-inititialize what has already been initialized for us
         // by cppunit, or by a previous loop.
         tearDown();

         // Set the sample rates 
         setSamplesPerSec(sSampleRates[rateIdx]);
         setSamplesPerFrame(sSampleRates[rateIdx]/100);
         setUp();
         testPlayToneDetectHelper(MpfftBuffer, sSampleRates[rateIdx], sSampleRates[rateIdx]/100);
      }
   }


   /**
   *  @brief Test MprFromFile file or buffer playing at a given samples per 
   *  frame and sample rate.
   *
   *  How flowgraph is set up:
   *     * A test input resource, connected to an MprFromFile resource, 
   *       connected to test output resource.
   *
   *  This test does the following:
   *  * If /p source is /p MpfftFile:
   *     * MprFromFile loads and plays a very short audio file with DTMF digit
   *       '5' recorded at 48kHz using playFile
   *     * processes a few frames of this data,
   *       for each frame of data, it passes this on to a goertzel dtmf detector
   *     * once a few frames are gathered, a result is received from the DTMF
   *       detector.  This then is checked to make sure it detected '5'.
   *  * If /p source is /p MpfftBuffer:
   *     * MprFromFile plays a very short audio clip with DTMF digit '5' recorded
   *       at 48kHz using playBuffer
   *     * The rest is the same as described in #1.
   *
   */
   void testPlayToneDetectHelper(TestAudioSource source, 
                                 unsigned sampleRate, unsigned samplesPerFrame)
   {
      OsStatus res = OS_SUCCESS;
      int framesToProcess = 3;
      UtlString ffResName = "MprFromFile";
      MprFromFile* pFromFile = new MprFromFile(ffResName);
      CPPUNIT_ASSERT(pFromFile != NULL);

      // Create the Goertzel DTMF detector with current sample rate and samples
      // per frame figured in.
      MpDtmfDetector dtmfDetector(sampleRate, framesToProcess*samplesPerFrame);

      setupFramework(pFromFile);

      if(source == MpfftFile)
      {
         // Specify to play the dtmf '5' file.
         CPPUNIT_ASSERT_EQUAL(
            OS_SUCCESS, MprFromFile::playFile(ffResName, *mpFlowGraph->getMsgQ(), 
                                              mpFlowGraph->getSamplesPerSec(), 
                                              DTMF5_FN, FALSE));
      }
      else
      {
         // Specify to play the dtmf '5' buffer.
         CPPUNIT_ASSERT_EQUAL(
            OS_SUCCESS, MprFromFile::playBuffer(ffResName, *mpFlowGraph->getMsgQ(), 
                                                (const char*)dtmf5_48khz_16b_signed, 
                                                dtmf5_48khz_16b_signed_in_bytes,
                                                48000, mpFlowGraph->getSamplesPerSec(), 
                                                0, FALSE, NULL));
      }

      // pMixer enabled, there are buffers on the input 0
      CPPUNIT_ASSERT(mpSourceResource->enable());
      CPPUNIT_ASSERT(pFromFile->enable());

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

         // Make sure that the number of samples in the from file output frame 
         // is equal to the samples per frame that we set during this run.
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
      // the DTMF detector should have detected a '5' tone.
      // If it didn't then something went wrong -- one thing that could
      // have gone wrong in that case, is that the generation was assuming
      // a different sample rate than the detector, thus indicating a bug in 
      // wideband support in FromFile.
      char detectedDTMF = dtmfDetector.getLastDetectedDTMF();
      CPPUNIT_ASSERT_EQUAL((int)'5', (int)detectedDTMF);

      // Stop flowgraph
      haltFramework();
   }

protected:

   static int sSampleRates[];
   static int sNumRates;

};

CPPUNIT_TEST_SUITE_REGISTRATION(MprFromFileTest);

int MprFromFileTest::sSampleRates[] = {8000, 16000, 32000, 48000};
int MprFromFileTest::sNumRates = sizeof(MprFromFileTest::sSampleRates)/sizeof(int);
