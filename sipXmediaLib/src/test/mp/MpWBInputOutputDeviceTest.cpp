//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <mp/MpAudioBuf.h>
#include <mp/MpSineWaveGeneratorDeviceDriver.h>
#include <mp/MpInputDeviceManager.h>

#include <mp/MpFlowGraphBase.h>
#include <mp/MpOutputDeviceManager.h>
#include <mp/MprFromInputDevice.h>
#include <mp/MprFromFile.h>
#include <mp/MprMixer.h>
#include <mp/MprToOutputDevice.h>
#include <mp/MpodBufferRecorder.h>
#include <mp/MpMisc.h>
#include <mp/MpMediaTask.h>
#include <os/OsEvent.h>
#include <utl/UtlSList.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlInt.h>
#include <os/OsMsgDispatcher.h>
#include <mp/MpResNotificationMsg.h>

#define NUM_BUFFERS                   500
#define TEST_SAMPLE_DATA_MAGNITUDE    32000
#define TEST_SAMPLE_DATA_PERIOD       (1000000/60) //in microseconds 60 Hz
#define TEST_MAX_SAMPLE_RATE          48000
#define TEST_MAX_SAMPLES_PER_FRAME    TEST_MAX_SAMPLE_RATE/100
#define NUM_INPUT_DEV_BUFFERED_FRAMES 5  

#define TICKER_TEST_WRITE_RUNS_NUMBER 3
#define FREQ_TO_PERIODUS(X) 1000000/X

#undef USE_TEST_INPUT_DRIVER
#undef USE_TEST_OUTPUT_DRIVER

#ifdef USE_TEST_INPUT_DRIVER // USE_TEST_INPUT_DRIVER [
#  include <mp/MpSineWaveGeneratorDeviceDriver.h>
#  define INPUT_DRIVER MpSineWaveGeneratorDeviceDriver
#  define INPUT_DRIVER_CONSTRUCTOR_PARAMS(mgr) "default", mgr, TEST_SAMPLE_DATA_MAGNITUDE, TEST_SAMPLE_DATA_PERIOD, 0
#elif defined(WIN32) // USE_TEST_INPUT_DRIVER ][ WIN32
#  include <mp/MpidWinMM.h>
#  define INPUT_DRIVER MpidWinMM
#  define INPUT_DRIVER_CONSTRUCTOR_PARAMS(mgr) MpidWinMM::getDefaultDeviceName(), mgr
#elif defined(__pingtel_on_posix__) // WIN32 ][ __pingtel_on_posix__
#  include <mp/MpidOSS.h>
#  define INPUT_DRIVER MpidOSS
#  define INPUT_DRIVER_CONSTRUCTOR_PARAMS(mgr) "/dev/dsp", mgr
#else // __pingtel_on_posix__ ]
#  error Unknown platform!
#endif

#ifdef USE_TEST_OUTPUT_DRIVER // USE_TEST_OUTPUT_DRIVER [
#  include <mp/MpodBufferRecorder.h>
#  define OUTPUT_DRIVER MpodBufferRecorder
#  define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS "default", TEST_SAMPLE_DATA_SIZE*1000/TEST_SAMPLES_PER_SECOND
#elif defined(WIN32) // USE_TEST_OUTPUT_DRIVER ][ WIN32
#  include <mp/MpodWinMM.h>
#  define OUTPUT_DRIVER MpodWinMM
#  define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS MpodWinMM::getDefaultDeviceName()
#elif defined(__pingtel_on_posix__) // WIN32 ][ __pingtel_on_posix__
#  include <mp/MpodOSS.h>
#  define OUTPUT_DRIVER MpodOSS
#  define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS "/dev/dsp"
#else // __pingtel_on_posix__ ]
#  error Unknown platform!
#endif

/**
 * Unittest for Wide band support in input and output device driver
 */
class WBInputOutputDeviceTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(WBInputOutputDeviceTest);
   CPPUNIT_TEST(testWideband);
   CPPUNIT_TEST_SUITE_END();


public:

   void setUp()
   {
      // Create pool for data buffers
      mpPool = new MpBufPool(TEST_MAX_SAMPLES_PER_FRAME * sizeof(MpAudioSample)
                             + MpArrayBuf::getHeaderSize(), NUM_BUFFERS);
      CPPUNIT_ASSERT(mpPool != NULL);

      // Create pool for buffer headers
      mpHeadersPool = new MpBufPool(sizeof(MpAudioBuf), NUM_BUFFERS);
      CPPUNIT_ASSERT(mpHeadersPool != NULL);

      // Set mpHeadersPool as default pool for audio and data pools.
      MpAudioBuf::smpDefaultPool = mpHeadersPool;
      MpDataBuf::smpDefaultPool = mpHeadersPool;
   }

   void tearDown()
   {
      if (mpPool != NULL)
      {
         delete mpPool;
      }
      if (mpHeadersPool != NULL)
      {
         delete mpHeadersPool;
      }
   }


   void printCountdownDelay(size_t nSecs)
   {
      assert(nSecs < 10);  // this only works for values < 10.
      printf("  seconds.");
      int i;
      for(i = nSecs; i > 0; i--)
      {
         printf("\b\b\b\b\b\b\b\b\b\b%d seconds.", i);
         OsTask::delay(1000);
      }
      printf("\b\b\b\b\b\b\b\b\b\b\b\b\bNOW.         \n");

   }

   // Caller owns pFlowgraph.
   void setupWBMediaTask(unsigned sampleRate, unsigned samplesPerFrame,
                         MpFlowGraphBase*& pFlowgraph, MpMediaTask*& pMediaTask)
   {
      // Both pointers passed in should be NULL, as we allocate it here.
      assert(pFlowgraph == NULL);
      assert(pMediaTask == NULL);

      unsigned mpTaskNBuffers = 6*10; // # taken from MpFlowGraphTest

      // Setup codec paths..
      UtlString codecPaths[] = {
#ifdef WIN32
         "bin",
         "..\\bin",
#elif __pingtel_on_posix__
         "../../../../bin",
         "../../../bin",
#else
#        error "Unknown platform"
#endif
         "."
      };
      int numCodecPaths = sizeof(codecPaths)/sizeof(codecPaths[0]);

      // Setup media task
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
         mpStartUp(sampleRate, samplesPerFrame, mpTaskNBuffers, NULL,
         numCodecPaths, codecPaths));

      pFlowgraph = new MpFlowGraphBase(samplesPerFrame, sampleRate);

      // Call getMediaTask() which causes the task to get instantiated
      pMediaTask = MpMediaTask::getMediaTask(10);
   }

   void tearDownWBMediaTask(MpInputDeviceManager& inMgr, UtlSList& inDevHandles, 
                            MpOutputDeviceManager& outMgr, UtlSList& outDevHandles,
                            MpFlowGraphBase* pFlowgraph, MpMediaTask*& pMediaTask)
   {
      // Clear the flowgraph ticker so it stops ticking..
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
         outMgr.setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE));

      // Disable all of the devices..
      UtlInt* pCurHnd;
      UtlSListIterator outHndIter(outDevHandles);
      while((pCurHnd = (UtlInt*)outHndIter()) != NULL)
      {
         // Disable the output devices.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, outMgr.disableDevice(pCurHnd->getValue()));
      }
      UtlSListIterator inHndIter(inDevHandles);
      while((pCurHnd = (UtlInt*)inHndIter()) != NULL)
      {
         // Disable the input devices.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, inMgr.disableDevice(pCurHnd->getValue()));
      }

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMediaTask->unmanageFlowGraph(*pFlowgraph));
      MpMediaTask::signalFrameStart(); // ? what does this do?
      OsTask::delay(20);

      // Stop the flow graph, process next frame so the stop actually happens.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pFlowgraph->stop());
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pFlowgraph->processNextFrame());

      // Remove all drivers from managers..
      outHndIter.reset(); // reset the iterator positions to the beginning of the lists.
      inHndIter.reset();
      while((pCurHnd = (UtlInt*)inHndIter()) != NULL)
      {
         // Remove the input devices.
         inMgr.removeDevice(pCurHnd->getValue());
      }
      while((pCurHnd = (UtlInt*)outHndIter()) != NULL)
      {
         // Remove the output devices.
         outMgr.removeDevice(pCurHnd->getValue());
      }

      // Clear all media processing data and delete MpMediaTask instance.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpShutdown());

      // Set the media task to null, since (I think) mpShutdown deletes it.
      pMediaTask = NULL;
   }

   // This is a CPPUNIT compliant method that actually does the calling of the wideband
   // test, giving various values for sample rates.
   void testWideband()
   {
      printf("For this test, please plug in a male minijack-to-male minijack "
             "cable from your default sound card's speaker output to it's mic "
             "input.\n");
      printf("The test will begin in ");
      printCountdownDelay(5); // 5 seconds.
      int i;
      for(i = 0; i < sNumRates; i++)
      {
         printf("Testing rate %d\n", sSampleRates[i]);
         testIOFreqRange(sSampleRates[i], sSampleRates[i]/100);

         // If there's going to be another run, pause a bit.
         if(i < sNumRates-1)
         {
            printf("Please plug the minijack-to-minijack cable back into your "
                   "mic and speaker jacks now.\n"
                   "We will proceed with testing the next sample rate in ");
            printCountdownDelay(3);
         }
      }
   }

   /**
   *  Test wideband support in flowgraph, from input device, through resources,
   *  to output device.
   *
   *  This test does the following:
   *  Sets up one flowgraph with two legs.  
   *     * One leg of the flowgraph, the 'record' leg, which just listens to the 
   *       microphone, recording to a buffer output device driver.
   *     * The second leg, the 'play' leg uses a sine wave generator device
   *       driver to generate tones, and plays them out to the speaker.
   *  
   *  The tester is expected to connect the speaker output to the microphone 
   *  input via a male minijack-to-male minijack cable, thereby having the 
   *  'record' leg record the speaker output generated by the 'play' leg.
   *
   *  Once the tones are recorded, the test moves onto a new phase.
   *  The tester then needs to disconnect the minijack-to-minijack connection from
   *  the speaker output, instead playing to an actual speaker that the tester
   *  can hear from.
   *  Next, the recorded tones recorded in the last phase by the buffer recorder 
   *  device driver are played back by the 'play' leg.
   *  And that concludes the test.
   *  
   *  Additional todo: Write the recorded audio to a file for further testing.
   */
   void testIOFreqRange(unsigned sampleRate, unsigned samplesPerFrame)
   {
      MpFrameTime outMgrDefaultMixerBufLen = 30;
      unsigned sineMagnitude = 8000; // FIXME! Put a proper value here.
      int frequencies[] = {1000, 2000, 3800, 7600, 15000, 20000, 24000, 28000};
      int numFreqs = sizeof(frequencies)/sizeof(int);
      int toneDurationMS = 1000; // duration of playing a tone.
      MpFrameTime bufRecDevBufferLen = toneDurationMS*numFreqs;

      MpMediaTask* pMediaTask = NULL;
      MpFlowGraphBase* pFlowgraph = NULL;

      // Things we need for the general flowgraph
      MpInputDeviceManager* pInMgr = NULL;
      MpOutputDeviceManager* pOutMgr = NULL;

      // Specific devices needed for the microphone 'record' leg of the flowgraph.
      INPUT_DRIVER* pMicInDrv = NULL;
      MpInputDeviceHandle micDevHnd = 0;
      MprFromInputDevice* pRecRFromInDev = NULL;
      MprToOutputDevice* pRecRToOutputDev = NULL;
      MpodBufferRecorder* pRecBufOutDrv = NULL;
      MpOutputDeviceHandle recBufDevHnd = 0;

      // Things we need for the tone and speaker 'play' leg of the flowgraph
      MpSineWaveGeneratorDeviceDriver* pPlaySineInDrv = NULL;
      MpInputDeviceHandle playSineDevHnd = 0;
      MprFromInputDevice* pPlayRFromInDev = NULL;
      MprFromFile* pPlayRFromFile = NULL;
      MprMixer* pPlayRMixer = NULL;
      MprToOutputDevice* pPlayRToOutputDev = NULL;
      OUTPUT_DRIVER* pSpkrOutDrv = NULL;
      MpOutputDeviceHandle spkrDevHnd = 0;

      // Setup the initial bits of the flowgraph, allocating the flowgraph and 
      // media task (thereby filling in the passed in pointers, which should
      // always be null when passed in).
      setupWBMediaTask(sampleRate, samplesPerFrame, pFlowgraph, pMediaTask);

      // Turn on notifications
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pFlowgraph->setNotificationsEnabled(true));

      // Create the input and output managers -- those are common to both the 
      // 'record' and 'play' legs of the flowgraph.
      pInMgr = new MpInputDeviceManager(samplesPerFrame, sampleRate, 
                                        NUM_INPUT_DEV_BUFFERED_FRAMES, *mpPool);
      pOutMgr = new MpOutputDeviceManager(samplesPerFrame, sampleRate, 
                                          outMgrDefaultMixerBufLen);


      // Create the 'record' leg of the flowgraph.
      // Create the input and output devices, and add the devices for the 'record' leg.
      pMicInDrv = new INPUT_DRIVER(INPUT_DRIVER_CONSTRUCTOR_PARAMS(*pInMgr));
      micDevHnd = pInMgr->addDevice(*pMicInDrv);
      CPPUNIT_ASSERT(micDevHnd > 0);
      pRecBufOutDrv = new MpodBufferRecorder("MpodBufferRecorder-%d", bufRecDevBufferLen);
      recBufDevHnd = pOutMgr->addDevice(pRecBufOutDrv);
      CPPUNIT_ASSERT(recBufDevHnd > 0);

      // Create the resources for the 'record' leg.
      pRecRFromInDev = 
         new MprFromInputDevice("FromInput-RecordMic", 
                                samplesPerFrame, sampleRate, pInMgr, micDevHnd);
      pRecRToOutputDev =
         new MprToOutputDevice("ToOutput-RecordBuffer", 
                               samplesPerFrame, sampleRate, pOutMgr, recBufDevHnd);


      // Create the 'play' leg of the flowgraph.
      // Create the input and output devices, and add the devices for the 'play' leg.
      pPlaySineInDrv = 
         new MpSineWaveGeneratorDeviceDriver("SineGeneratorDriver", *pInMgr, 
                                             sineMagnitude, 
                                             FREQ_TO_PERIODUS(frequencies[0]), 0);
      playSineDevHnd = pInMgr->addDevice(*pPlaySineInDrv);
      CPPUNIT_ASSERT(playSineDevHnd > 0);
      pSpkrOutDrv = new OUTPUT_DRIVER(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      spkrDevHnd = pOutMgr->addDevice(pSpkrOutDrv);
      CPPUNIT_ASSERT(spkrDevHnd > 0);

      // Create the resources for the 'play' leg.
      pPlayRFromInDev = 
         new MprFromInputDevice("FromInput-PlaySine", 
                                samplesPerFrame, sampleRate, pInMgr, playSineDevHnd);
      pPlayRFromFile = new MprFromFile("FromFile-1", samplesPerFrame, sampleRate);
      pPlayRMixer = new MprMixer("Play-Mixer-%d", 2, samplesPerFrame, sampleRate);
      pPlayRToOutputDev =
         new MprToOutputDevice("ToOutput-PlaySpkr", 
                               samplesPerFrame, sampleRate, pOutMgr, spkrDevHnd);



      // Ok, now that everything is created, the flowgraph needs to be wired up
      // with those resources.
      // Add and connect up the 'record' leg of the flowgraph.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
                           pFlowgraph->addResource(*pRecRFromInDev));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
                           pFlowgraph->addResource(*pRecRToOutputDev));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           pFlowgraph->addLink(*pRecRFromInDev, 0, 
                                               *pRecRToOutputDev, 0));

      // Add and connect up the 'play' leg of the flowgraph.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
                           pFlowgraph->addResource(*pPlayRFromInDev));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
                           pFlowgraph->addResource(*pPlayRFromFile));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
                           pFlowgraph->addResource(*pPlayRMixer));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
                           pFlowgraph->addResource(*pPlayRToOutputDev));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           pFlowgraph->addLink(*pPlayRFromInDev, 0, 
                                               *pPlayRMixer, 0));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           pFlowgraph->addLink(*pPlayRFromFile, 0, 
                                               *pPlayRMixer, 1));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           pFlowgraph->addLink(*pPlayRMixer, 0, 
                                               *pPlayRToOutputDev, 0));

      // Make sure that the mixer weights are set, and set equally.
      pPlayRMixer->setWeight(1, 0);
      pPlayRMixer->setWeight(1, 1);

      // Enable all of the devices..
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pInMgr->enableDevice(micDevHnd));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pOutMgr->enableDevice(recBufDevHnd));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pInMgr->enableDevice(playSineDevHnd));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pOutMgr->enableDevice(spkrDevHnd));

      // Set the flowgraph ticker source..
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
                           pOutMgr->setFlowgraphTickerSource(spkrDevHnd));

      // Now we enable the flowgraph..  Which should enable resources.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pFlowgraph->enable());

      // Manage the flow graph so it flows, and finally start it flowing, 
      // and bring it into focus
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMediaTask->manageFlowGraph(*pFlowgraph));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMediaTask->startFlowGraph(*pFlowgraph));

      // Let the tone play for a few seconds..
      printf("Playing frequency: %d, ", frequencies[0]);
      int i;
      for (i = 1; i < numFreqs; i++)
      {
         OsTask::delay(toneDurationMS);
         printf("%d, ", frequencies[i]);
         pPlaySineInDrv->setNewTone(sineMagnitude, FREQ_TO_PERIODUS(frequencies[i]), 0);
      }
      // Let the last tone play for it's duration.
      OsTask::delay(toneDurationMS);
      printf("\b\b  \nDone playing tones\n");

      // We should ensure that the entire capacity of the buffer provided in 
      // the buffer recorder has been filled.
      CPPUNIT_ASSERT_EQUAL(pRecBufOutDrv->getBufferLength(), pRecBufOutDrv->getBufferEnd());

      // Disable the sine wave generator device so that we don't keep hearing
      // it's generated tone, and instead hear the recorded buffer played back.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, 
                           MpResource::disable("FromInput-PlaySine", *pFlowgraph->getMsgQ()));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pInMgr->disableDevice(playSineDevHnd));
      CPPUNIT_ASSERT_EQUAL((MpInputDeviceDriver*)pPlaySineInDrv, pInMgr->removeDevice(playSineDevHnd));



      //------------------------------------------------------------------------
      // Phase 2 of the test:  Play back the result we just recorded via the mic.

      printf("Please unplug the minijack-to-minijack cable from your speaker input now.\n"
             "We will proceed with the playback portion of the test in ");
      printCountdownDelay(3);

      const char* pRecBuf = (const char*)pRecBufOutDrv->getBufferData();
      unsigned pRecBufLen = pRecBufOutDrv->getBufferEnd() * sizeof(MpAudioSample);

      OsMsgDispatcher notfDisp;
      pFlowgraph->setNotificationDispatcher(&notfDisp);

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           MprFromFile::playBuffer("FromFile-1", *pFlowgraph->getMsgQ(), 
                                                   pRecBuf, pRecBufLen, 
                                                   0, FALSE, NULL));

      int maxPlayLoops = (toneDurationMS * numFreqs)/100 + 10;
      for(i = 0; i < maxPlayLoops && notfDisp.numMsgs() < 2; i++)
      {
         OsTask::delay(100);
      }
      
      OsMsg* pMsg;
      CPPUNIT_ASSERT_EQUAL(2, notfDisp.numMsgs());
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, notfDisp.receive((OsMsg*)pMsg, OsTime(10)));
      CPPUNIT_ASSERT_EQUAL(MpResNotificationMsg::MPRNM_FROMFILE_STARTED, 
                           (MpResNotificationMsg::RNMsgType)((MpResNotificationMsg*)pMsg)->getMsg());
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, notfDisp.receive((OsMsg*)pMsg, OsTime(10)));
      CPPUNIT_ASSERT_EQUAL(MpResNotificationMsg::MPRNM_FROMFILE_FINISHED, 
                           (MpResNotificationMsg::RNMsgType)((MpResNotificationMsg*)pMsg)->getMsg());

      // Shutdown the test.

      // Cobble together two lists of the device handles that are added to 
      // the device managers, so they can properly be cleaned up from their managers.
      UtlSList inDevHandles;
      inDevHandles.append(new UtlInt(micDevHnd));
      // No need to disable and remove pPlaySineInDrv, as we did it earlier
      UtlSList outDevHandles;
      outDevHandles.append(new UtlInt(recBufDevHnd));
      outDevHandles.append(new UtlInt(spkrDevHnd));

      tearDownWBMediaTask(*pInMgr, inDevHandles, *pOutMgr, outDevHandles,
                          pFlowgraph, pMediaTask);

      inDevHandles.destroyAll();
      outDevHandles.destroyAll();

      // Delete all our data.
      // No need to delete resources in the flowgraph explicitly, as the 
      // flowgraph deletes any it has.
      delete pFlowgraph;
      delete pInMgr;
      delete pOutMgr;
      delete pRecBufOutDrv;
      delete pMicInDrv;
      delete pSpkrOutDrv;
      delete pPlaySineInDrv;
   }

protected:
   MpBufPool *mpPool;         ///< Pool for data buffers
   MpBufPool *mpHeadersPool;  ///< Pool for buffers headers

   static int sSampleRates[];
   static int sNumRates;

};

CPPUNIT_TEST_SUITE_REGISTRATION(WBInputOutputDeviceTest);

int WBInputOutputDeviceTest::sSampleRates[] = {8000, 16000, 32000, 48000};
int WBInputOutputDeviceTest::sNumRates = sizeof(WBInputOutputDeviceTest::sSampleRates)/sizeof(int);
