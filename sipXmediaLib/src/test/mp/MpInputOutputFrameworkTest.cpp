//  
// Copyright (C) 2006-2017 SIPez LLC.  All rights reserved.
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#include <sipxunittests.h>

#include "MpTestResource.h"
#include "mp/MpInputDeviceManager.h"
#include "mp/MprFromInputDevice.h"
#include "mp/MpOutputDeviceManager.h"
#include "mp/MprToOutputDevice.h"
#include "mp/MprToneGen.h"
#include "mp/MprSplitter.h"
#include "mp/MprNull.h"
#include "mp/MpFlowGraphBase.h"
#include "mp/MpMisc.h"
#include "mp/MpMediaTask.h"
#include "os/OsTask.h"

// Setup codec paths..
#include <../test/mp/MpTestCodecPaths.h>

#ifdef RTL_ENABLED
#  include <rtl_macro.h>
#else
#  define RTL_START(x)
#  define RTL_BLOCK(x)
#  define RTL_EVENT(x,y)
#  define RTL_WRITE(x)
#  define RTL_STOP
#endif

#include <os/OsFS.h>

#define TEST_TIME_MS                  3000  ///< Time to runs test (in milliseconds)
#define AUDIO_BUFS_NUM                500   ///< Number of buffers in buffer pool we'll use.
#define BUFFERS_TO_BUFFER_ON_INPUT    3     ///< Number of buffers to buffer in input manager.
#define BUFFERS_TO_BUFFER_ON_OUTPUT   2     ///< Number of buffers to buffer in output manager.

#ifdef ANDROID // [
#  define TEST_SAMPLES_PER_FRAME        160    ///< in samples
#  define TEST_SAMPLES_PER_SECOND       16000  ///< in samples/sec (Hz)
#else // ANDROID ][
#  define TEST_SAMPLES_PER_FRAME        480    ///< in samples
#  define TEST_SAMPLES_PER_SECOND       48000  ///< in samples/sec (Hz)
#endif // !ANDROID ]

#define DEFAULT_BUFFER_ON_OUTPUT_MS   (BUFFERS_TO_BUFFER_ON_OUTPUT*TEST_SAMPLES_PER_FRAME*1000/TEST_SAMPLES_PER_SECOND)
                                            ///< Buffer size in output manager in milliseconds.

// The sipxportunit harness (Android, Windows) has no CppUnit exceptions:
// // compile the try blocks as plain blocks and drop the catch handlers.
#if defined(ANDROID) || defined(WIN32) // [
#  define try
#  define SIPX_NO_CPPUNIT_EXCEPTIONS
#endif // ]

//#define USE_TEST_INPUT_DRIVER
//#define USE_TEST_OUTPUT_DRIVER

// OS-specific device input drivers
#ifndef USE_TEST_INPUT_DRIVER
#  ifdef ANDROID // [
#     define USE_ANDROID_INPUT_DRIVER
#  elif defined(__linux__) // [
#     define USE_OSS_INPUT_DRIVER
#  elif defined(__APPLE__) // [
#     define USE_COREAUDIO_INPUT_DRIVER
#  elif defined(WIN32) // [
#     define USE_WNT_INPUT_DRIVER
#  endif // WIN32 ]
#endif // ifndef USE_TEST_INPUT_DRIVER ]

// OS-specific device output drivers
#ifndef USE_TEST_OUTPUT_DRIVER
#  ifdef ANDROID // [
#     define USE_ANDROID_OUTPUT_DRIVER
#  elif defined(__linux__) // [
#     define USE_OSS_OUTPUT_DRIVER
#  elif defined(__APPLE__) // [
#     define USE_COREAUDIO_OUTPUT_DRIVER
#  elif defined(WIN32) // [
#     define USE_WNT_OUTPUT_DRIVER
#  endif // WIN32 ]
#endif // ifndef USE_TEST_OUTPUT_DRIVER ]



#ifdef USE_TEST_INPUT_DRIVER // [
#  include <mp/MpSineWaveGeneratorDeviceDriver.h>
#elif defined(USE_ANDROID_INPUT_DRIVER) // [
#  include <mp/MpidAndroid.h>
#elif defined(USE_OSS_INPUT_DRIVER) // [
#  include <mp/MpidOss.h>
#elif defined(USE_COREAUDIO_INPUT_DRIVER) // [
#  include <mp/MpidCoreAudio.h>
#elif defined(USE_WNT_INPUT_DRIVER) // [
#  include <mp/MpidWinMM.h>
#endif // USE_*_INPUT_DRIVER ]

#ifdef USE_TEST_OUTPUT_DRIVER // [
#  include <mp/MpodBufferRecorder.h>
#elif defined(USE_ANDROID_OUTPUT_DRIVER) // [
#  include <mp/MpodAndroid.h>
#elif defined(USE_OSS_OUTPUT_DRIVER) // [
#  include <mp/MpodOss.h>
#elif defined(USE_COREAUDIO_OUTPUT_DRIVER) // [
#  include <mp/MpodCoreAudio.h>
#elif defined(USE_WNT_OUTPUT_DRIVER) // [
#  include <mp/MpodWinMM.h>
#endif // USE_*_OUTPUT_DRIVER ]

/// Number of input drivers to test
static size_t nInputDrivers = 
#ifdef USE_TEST_INPUT_DRIVER
   10;
#elif defined(USE_ANDROID_INPUT_DRIVER) // [
   1;
#elif defined(USE_OSS_INPUT_DRIVER) // [
   2;
#elif defined(USE_COREAUDIO_INPUT_DRIVER) // [
   1;
#elif defined(USE_WNT_INPUT_DRIVER)
   1;
#else
   0;
#endif
static UtlString* sInputDriverNames = NULL;

/// Number of output drivers to test
static size_t nOutputDrivers = 
#ifdef USE_TEST_OUTPUT_DRIVER
   10;
#elif defined(USE_ANDROID_OUTPUT_DRIVER) // [
   1;
#elif defined(USE_OSS_OUTPUT_DRIVER) // [
   2;
#elif defined(USE_COREAUDIO_OUTPUT_DRIVER) // [
   1;
#elif defined(USE_WNT_OUTPUT_DRIVER)
   1;
#else
   0;
#endif
static UtlString* outputDriverNames = NULL;


///  Unit test for MprSplitter
class MpInputOutputFrameworkTest : public SIPX_UNIT_BASE_CLASS
{
   CPPUNIT_TEST_SUITE(MpInputOutputFrameworkTest);
   CPPUNIT_TEST(testShortCircuit);
   // testOutput is a special purpose test. It is aimed to measure audio
   // output jitter and clock-skew. To go with this test you need to connect
   // output of your sound card to line input and record. You may then
   // be able to determine jitter, clock skew, etc by measuring distance
   // between frame starts.
//   CPPUNIT_TEST(testOutput);
   CPPUNIT_TEST(testManyOutputDevices);
   CPPUNIT_TEST(testManyInputDevices);
   CPPUNIT_TEST(testManyInputDevicesToOneOutputDevice);
   // Bench tests: real device departure driven by scripts/bench_trigger.py.
   // Skip loudly when the bench is not configured on this machine.
   CPPUNIT_TEST(testBenchDisableDuringDeparture);
   CPPUNIT_TEST(testBenchUninitializeAfterDeparture);
   CPPUNIT_TEST(testBenchRecoveryAfterReturn);
   CPPUNIT_TEST_SUITE_END();

public:

   // This function will be called before every test to setup framework.
   void setUp()
   {
      //enableConsoleOutput(1);

      // Setup media task
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpStartUp(TEST_SAMPLES_PER_SECOND,
                                     TEST_SAMPLES_PER_FRAME, 6*10, 0,
                                     sNumCodecPaths, sCodecPaths));

      // Create flowgraph
      mpFlowGraph = new MpFlowGraphBase(TEST_SAMPLES_PER_FRAME,
                                        TEST_SAMPLES_PER_SECOND);
      CPPUNIT_ASSERT(mpFlowGraph != NULL);

      // Create MediaTask
      mpMediaTask = MpMediaTask::createMediaTask(10);
      CPPUNIT_ASSERT(mpMediaTask != NULL);

      // Get ticker.
      mpTicker = mpMediaTask->getTickerNotification();

      // Create input and output device managers
      mpInputDeviceManager = new MpInputDeviceManager(TEST_SAMPLES_PER_FRAME, 
                                                      TEST_SAMPLES_PER_SECOND,
                                                      BUFFERS_TO_BUFFER_ON_INPUT,
                                                      *MpMisc.RawAudioPool);
      CPPUNIT_ASSERT(mpInputDeviceManager != NULL);
      mpOutputDeviceManager = new MpOutputDeviceManager(TEST_SAMPLES_PER_FRAME,
                                                        TEST_SAMPLES_PER_SECOND,
                                                        DEFAULT_BUFFER_ON_OUTPUT_MS);
      CPPUNIT_ASSERT(mpOutputDeviceManager != NULL);

      // No drivers in managers
      mInputDeviceNumber = 0;
      mOutputDeviceNumber = 0;

      // NOTE: Next functions would add devices only if they are available in
      //       current environment.
      createTestInputDrivers();
      createWntInputDrivers();
      createAndroidInputDrivers();
      createOSSInputDrivers();
      createTestOutputDrivers();
      createWntOutputDrivers();
      createAndroidOutputDrivers();
      createOSSOutputDrivers();
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

      // Free all input device drivers
      for (; mInputDeviceNumber>0; mInputDeviceNumber--)
      {
         MpInputDeviceDriver *pDriver = mpInputDeviceManager->removeDevice(mInputDeviceNumber);
         CPPUNIT_ASSERT(pDriver != NULL);
         if (pDriver->lastDisableEscaped())
         {
            // A stuck thread may still reference it: retire, never delete.
            printf("tearDown: input driver '%s' escaped teardown; leaking it\n",
                   pDriver->getDeviceName().data());
         }
         else
         {
            delete pDriver;
         }
      }

      // Free all output device drivers
      for (; mOutputDeviceNumber>0; mOutputDeviceNumber--)
      {
         MpOutputDeviceDriver *pDriver = mpOutputDeviceManager->removeDevice(mOutputDeviceNumber);
         CPPUNIT_ASSERT(pDriver != NULL);
         delete pDriver;
      }

      // Free device managers
      delete mpOutputDeviceManager;
      mpOutputDeviceManager = NULL;
      delete mpInputDeviceManager;
      mpInputDeviceManager = NULL;

      // Free flowgraph
      delete mpFlowGraph;
      mpFlowGraph = NULL;

      // Free the input and output driver names, if any.
      if(sInputDriverNames)
      {
         delete[] sInputDriverNames;
         sInputDriverNames = NULL;
      }
      if(outputDriverNames)
      {
         delete[] outputDriverNames;
         outputDriverNames = NULL;
      }

      // Clear all media processing data and delete MpMediaTask instance.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpShutdown());
   }

   void testShortCircuit()
   {
      RTL_START(10000000);

      // We should have at least one input driver and one output driver
      CPPUNIT_ASSERT(nInputDrivers >= 1);
      CPPUNIT_ASSERT(nOutputDrivers >= 1);

      MpInputDeviceHandle sourceDeviceId;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpInputDeviceManager->getDeviceId(sInputDriverNames[0],
                                                             sourceDeviceId));

      MpOutputDeviceHandle  sinkDeviceId;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpOutputDeviceManager->getDeviceId(outputDriverNames[0],
                                                              sinkDeviceId));

      // Create source (input) and sink (output) resources.
      MprFromInputDevice sourceResource("MprFromInputDevice",
                                        mpInputDeviceManager,
                                        sourceDeviceId);
      MprToOutputDevice sinkResource("MprToOutputDevice",
                                     mpOutputDeviceManager,
                                     sinkDeviceId);

      try {
         // Add source and sink resources to flowgraph and link them together.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addResource(sourceResource));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addResource(sinkResource));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addLink(sourceResource, 0, sinkResource, 0));

         // Enable devices
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              mpInputDeviceManager->enableDevice(sourceDeviceId));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              mpOutputDeviceManager->enableDevice(sinkDeviceId));

         // Set flowgraph ticker
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              mpOutputDeviceManager->setFlowgraphTickerSource(sinkDeviceId,
                                                                              mpTicker));

         try {
            // Enable resources
            CPPUNIT_ASSERT(sourceResource.enable());
            CPPUNIT_ASSERT(sinkResource.enable());

            // Manage flowgraph with media task.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpMediaTask->manageFlowGraph(*mpFlowGraph));

            // Start flowgraph
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpMediaTask->startFlowGraph(*mpFlowGraph));

            // Run test!
            OsTask::delay(TEST_TIME_MS);

            // Clear flowgraph ticker
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpOutputDeviceManager->setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE,
                                                                                 NULL));
         }
#ifndef SIPX_NO_CPPUNIT_EXCEPTIONS // [
         catch (CppUnit::Exception& e)
         {
            // Clear flowgraph ticker if assert failed.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpOutputDeviceManager->setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE,
                                                                                 NULL));

            // Rethrow exception.
            throw(e);
         }
#endif // !ANDROID ]

         // Unmanage flowgraph with media task.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpMediaTask->unmanageFlowGraph(*mpFlowGraph));
         MpMediaTask::signalFrameStart();
         // MediaTask need some time to receive unmanage message.
         OsTask::delay(20);

         // Disable devices
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              mpInputDeviceManager->disableDevice(sourceDeviceId));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              mpOutputDeviceManager->disableDevice(sinkDeviceId));

         // Remove resources from flowgraph. We should remove them explicitly
         // here, because they are stored on the stack and will be destroyed.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(sinkResource));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(sourceResource));
         mpFlowGraph->processNextFrame();
      }
#ifndef SIPX_NO_CPPUNIT_EXCEPTIONS // [
      catch (CppUnit::Exception& e)
      {
         // Remove resources from flowgraph. We should remove them explicitly
         // here, because they are stored on the stack and will be destroyed.
         // If we will not catch this assert we'll have this resources destroyed
         // while still referenced in flowgraph, causing crash.
         if (sinkResource.getFlowGraph() != NULL)
         {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(sinkResource));
         }
         if (sourceResource.getFlowGraph() != NULL)
         {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(sourceResource));
         }
         mpFlowGraph->processNextFrame();

         // Rethrow exception.
         throw(e);
      }
#endif // !ANDROID ]

      RTL_WRITE("testShortCircuit.rtl");
      RTL_STOP
   }

   void testOutput()
   {
      RTL_START(10000000);

      // We should have at least one input driver and one output driver
      CPPUNIT_ASSERT(nOutputDrivers >= 1);

      MpOutputDeviceHandle  sinkDeviceId;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpOutputDeviceManager->getDeviceId(outputDriverNames[0],
                                                              sinkDeviceId));

      // Create source (input) and sink (output) resources.
      MpTestResource sourceResource("TestSource", 0, 0, 1, 1);
      sourceResource.setGenOutBufMask(1);
      sourceResource.setOutSignalType(MpTestResource::MP_SINE_SAW);
      sourceResource.setSignalAmplitude(0, 16000);
      MprToOutputDevice sinkResource("MprToOutputDevice",
                                     mpOutputDeviceManager,
                                     sinkDeviceId);

      try {

         // Add source and sink resources to flowgraph and link them together.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addResource(sourceResource));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addResource(sinkResource));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addLink(sourceResource, 0, sinkResource, 0));

         // Enable devices
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              mpOutputDeviceManager->enableDevice(sinkDeviceId));

         // Set flowgraph ticker
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              mpOutputDeviceManager->setFlowgraphTickerSource(sinkDeviceId,
                                                                              mpTicker));

         try {
            // Enable resources
            CPPUNIT_ASSERT(sourceResource.enable());
            CPPUNIT_ASSERT(sinkResource.enable());

            // Manage flowgraph with media task.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpMediaTask->manageFlowGraph(*mpFlowGraph));

            // Start flowgraph
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpMediaTask->startFlowGraph(*mpFlowGraph));

            // Run test!
            OsTask::delay(TEST_TIME_MS);

            // Clear flowgraph ticker
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpOutputDeviceManager->setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE,
                                                                                 NULL));
         }
#ifndef SIPX_NO_CPPUNIT_EXCEPTIONS // [
         catch (CppUnit::Exception& e)
         {
            // Clear flowgraph ticker if assert failed.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpOutputDeviceManager->setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE,
                                                                                 NULL));

            // Rethrow exception.
            throw(e);
         }
#endif // !ANDROID ]

         // Unmanage flowgraph with media task.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpMediaTask->unmanageFlowGraph(*mpFlowGraph));
         MpMediaTask::signalFrameStart();
         // MediaTask need some time to receive unmanage message.
         OsTask::delay(20);

         // Disable devices
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              mpOutputDeviceManager->disableDevice(sinkDeviceId));

         // Remove resources from flowgraph. We should remove them explicitly
         // here, because they are stored on the stack and will be destroyed.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(sinkResource));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(sourceResource));
         mpFlowGraph->processNextFrame();
      }
#ifndef SIPX_NO_CPPUNIT_EXCEPTIONS // [
      catch (CppUnit::Exception& e)
      {
         // Remove resources from flowgraph. We should remove them explicitly
         // here, because they are stored on the stack and will be destroyed.
         // If we will not catch this assert we'll have this resources destroyed
         // while still referenced in flowgraph, causing crash.
         if (sinkResource.getFlowGraph() != NULL)
         {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(sinkResource));
         }
         if (sourceResource.getFlowGraph() != NULL)
         {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(sourceResource));
         }
         mpFlowGraph->processNextFrame();

         // Rethrow exception.
         throw(e);
      }
#endif // !ANDROID ]

      RTL_WRITE("testOutput.rtl");
      RTL_STOP
   }

   void testManyOutputDevices()
   {
      unsigned sinkDevice;

      RTL_START(10000000);

      // We should have at least one output driver
      CPPUNIT_ASSERT(nOutputDrivers >= 1);

      MpOutputDeviceHandle  tickerDeviceId;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpOutputDeviceManager->getDeviceId(outputDriverNames[0],
                                                              tickerDeviceId));

      // Create generator resource.
      MprToneGen toneGen("ToneGenerator",
                         NULL);

      MprSplitter splitter("Splitter",
                           mOutputDeviceNumber);

      // Create resources for all available output devices.
      MprToOutputDevice **pSinkResources = new MprToOutputDevice*[mOutputDeviceNumber];
      for (sinkDevice=0; sinkDevice<mOutputDeviceNumber; sinkDevice++)
      {
         char devName[1024];
         snprintf(devName, 1024, "MprToOutputDevice%d", sinkDevice);

         pSinkResources[sinkDevice] = new MprToOutputDevice(devName,
                                                            mpOutputDeviceManager,
                                                            sinkDevice+1);
      }

      try {

         // Add resources to flowgraph, link them together and enable devices.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addResource(toneGen));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addResource(splitter));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addLink(toneGen, 0, splitter, 0));
         for (sinkDevice=0; sinkDevice<mOutputDeviceNumber; sinkDevice++)
         {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addResource(*pSinkResources[sinkDevice]));
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addLink(splitter, sinkDevice,
                                                                  *pSinkResources[sinkDevice], 0));

            // Enable devices
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpOutputDeviceManager->enableDevice(sinkDevice+1));
         }

         // Enable all resources in flowgraph.
         mpFlowGraph->enable();

         // Set flowgraph ticker
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              mpOutputDeviceManager->setFlowgraphTickerSource(tickerDeviceId,
                                                                              mpTicker));

         try {
            // Manage flowgraph with media task.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpMediaTask->manageFlowGraph(*mpFlowGraph));

            // Start flowgraph
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpMediaTask->startFlowGraph(*mpFlowGraph));

            // Run test!
            toneGen.startTone(0);
            OsTask::delay(TEST_TIME_MS);
            toneGen.stopTone();

            // Clear flowgraph ticker
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpOutputDeviceManager->setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE,
                                                                                 NULL));

         }
#ifndef SIPX_NO_CPPUNIT_EXCEPTIONS // [
         catch (CppUnit::Exception& e)
         {
            // Clear flowgraph ticker if assert failed.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpOutputDeviceManager->setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE,
                                                                                 NULL));

            // Rethrow exception.
            throw(e);
         }
#endif // !ANDROID ]

         // Unmanage flowgraph with media task.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpMediaTask->unmanageFlowGraph(*mpFlowGraph));
         MpMediaTask::signalFrameStart();
         // MediaTask need some time to receive unmanage message.
         OsTask::delay(20);

         // Disable devices
         for (sinkDevice=0; sinkDevice<mOutputDeviceNumber; sinkDevice++)
         {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpOutputDeviceManager->disableDevice(sinkDevice+1));
         }

         // Remove resources from flowgraph.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(toneGen));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(splitter));
         for (sinkDevice=0; sinkDevice<mOutputDeviceNumber; sinkDevice++)
         {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(*pSinkResources[sinkDevice]));
         }
         mpFlowGraph->processNextFrame();
      }
#ifndef SIPX_NO_CPPUNIT_EXCEPTIONS // [
      catch (CppUnit::Exception& e)
      {
         // Remove resources from flowgraph. We should remove them explicitly
         // here, because they are stored on the stack and will be destroyed.
         // If we will not catch this assert we'll have this resources destroyed
         // while still referenced in flowgraph, causing crash.
         if (toneGen.getFlowGraph() != NULL)
         {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(toneGen));
         }
         if (splitter.getFlowGraph() != NULL)
         {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(splitter));
         }
         for (unsigned i=0; i<mOutputDeviceNumber; i++)
         {
            if (pSinkResources[i]->getFlowGraph() != NULL)
            {
               CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(*pSinkResources[i]));
            }
         }
         mpFlowGraph->processNextFrame();

         // Rethrow exception.
         throw(e);
      }
#endif // !ANDROID ]

      // Free output resources.
      for (sinkDevice=0; sinkDevice<mOutputDeviceNumber; sinkDevice++)
      {
         delete pSinkResources[sinkDevice];
      }
      delete[] pSinkResources;

      RTL_WRITE("testManyOutputDevices.rtl");
      RTL_STOP
   }

   void testManyInputDevices()
   {
      unsigned sourceDevice;

      RTL_START(10000000);

      // We should have at least one output driver
      CPPUNIT_ASSERT(nOutputDrivers >= 1);

      MpOutputDeviceHandle  tickerDeviceId;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpOutputDeviceManager->getDeviceId(outputDriverNames[0],
                                                              tickerDeviceId));

      // Create sink resource.
      MprNull blackHole("MprNull",
                        mInputDeviceNumber);

      // Create resources for all available input devices.
      MprFromInputDevice **pSourceResources = new MprFromInputDevice*[mInputDeviceNumber];
      for (sourceDevice=0; sourceDevice<mInputDeviceNumber; sourceDevice++)
      {
         char devName[1024];
         snprintf(devName, 1024, "MprFromInputDevice%d", sourceDevice);

         pSourceResources[sourceDevice] = new MprFromInputDevice(devName,
                                                                 mpInputDeviceManager,
                                                                 sourceDevice+1);
      }

      try {

         // Add resources to flowgraph, link them together and enable devices.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addResource(blackHole));
         for (sourceDevice=0; sourceDevice<mInputDeviceNumber; sourceDevice++)
         {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addResource(*pSourceResources[sourceDevice]));
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addLink(*pSourceResources[sourceDevice], 0,
                                                                  blackHole, sourceDevice));

            // Enable devices
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpInputDeviceManager->enableDevice(sourceDevice+1));
         }

         // Enable all resources in flowgraph.
         mpFlowGraph->enable();

         // Enable one of output devices to get ticks from it and set flowgraph ticker
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              mpOutputDeviceManager->enableDevice(tickerDeviceId));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              mpOutputDeviceManager->setFlowgraphTickerSource(tickerDeviceId,
                                                                              mpTicker));

         try {
            // Manage flowgraph with media task.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpMediaTask->manageFlowGraph(*mpFlowGraph));

            // Start flowgraph
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpMediaTask->startFlowGraph(*mpFlowGraph));

            // Run test!
            OsTask::delay(TEST_TIME_MS);

            // Clear flowgraph ticker and disable device provided it.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpOutputDeviceManager->setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE,
                                                                                 NULL));
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpOutputDeviceManager->disableDevice(tickerDeviceId));
         }
#ifndef SIPX_NO_CPPUNIT_EXCEPTIONS // [
         catch (CppUnit::Exception& e)
         {
            // Clear flowgraph ticker if assert failed.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpOutputDeviceManager->setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE,
                                                                                 NULL));
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpOutputDeviceManager->disableDevice(tickerDeviceId));

            // Rethrow exception.
            throw(e);
         }
#endif // !ANDROID ]

         // Unmanage flowgraph with media task.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpMediaTask->unmanageFlowGraph(*mpFlowGraph));
         MpMediaTask::signalFrameStart();
         // MediaTask need some time to receive unmanage message.
         OsTask::delay(20);

         // Disable devices
         for (sourceDevice=0; sourceDevice<mInputDeviceNumber; sourceDevice++)
         {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpInputDeviceManager->disableDevice(sourceDevice+1));
         }

         // Remove resources from flowgraph.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(blackHole));
         for (sourceDevice=0; sourceDevice<mInputDeviceNumber; sourceDevice++)
         {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(*pSourceResources[sourceDevice]));
         }
         mpFlowGraph->processNextFrame();
      }
#ifndef SIPX_NO_CPPUNIT_EXCEPTIONS // [
      catch (CppUnit::Exception& e)
      {
         // Remove resources from flowgraph. We should remove them explicitly
         // here, because they are stored on the stack and will be destroyed.
         // If we will not catch this assert we'll have this resources destroyed
         // while still referenced in flowgraph, causing crash.
         if (blackHole.getFlowGraph() != NULL)
         {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(blackHole));
         }
         for (unsigned i=0; i<mInputDeviceNumber; i++)
         {
            if (pSourceResources[i]->getFlowGraph() != NULL)
            {
               CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(*pSourceResources[i]));
            }
         }
         mpFlowGraph->processNextFrame();

         // Rethrow exception.
         throw(e);
      }
#endif // !ANDROID ]

      // Free output resources.
      for (sourceDevice=0; sourceDevice<mInputDeviceNumber; sourceDevice++)
      {
         delete pSourceResources[sourceDevice];
      }
      delete[] pSourceResources;

      RTL_WRITE("testManyInputDevices.rtl");
      RTL_STOP
   }

   void testManyInputDevicesToOneOutputDevice()
   {
      unsigned sourceDevice;

      RTL_START(10000000);

      // We should have at least one output driver
      CPPUNIT_ASSERT(nOutputDrivers >= 1);

      MpOutputDeviceHandle  sinkDeviceId;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpOutputDeviceManager->getDeviceId(outputDriverNames[0],
                                                              sinkDeviceId));

      // Create resources for all available input devices and resources for
      // our sole sink device.
      MprFromInputDevice **pSourceResources = new MprFromInputDevice*[mInputDeviceNumber];
      MprToOutputDevice  **pSinkResources = new MprToOutputDevice*[mInputDeviceNumber];
      for (sourceDevice=0; sourceDevice<mInputDeviceNumber; sourceDevice++)
      {
         char devName[1024];

         snprintf(devName, 1024, "MprFromInputDevice%d", sourceDevice);
         pSourceResources[sourceDevice] = new MprFromInputDevice(devName,
                                                                 mpInputDeviceManager,
                                                                 sourceDevice+1);

         snprintf(devName, 1024, "MprToOutputDevice%d", sourceDevice);
         pSinkResources[sourceDevice] = new MprToOutputDevice(devName,
                                                              mpOutputDeviceManager,
                                                              sinkDeviceId);
      }

      try {

         // Add resources to flowgraph, link them together and enable devices.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              mpOutputDeviceManager->enableDevice(sinkDeviceId, 30));
         for (sourceDevice=0; sourceDevice<mInputDeviceNumber; sourceDevice++)
         {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addResource(*pSourceResources[sourceDevice]));
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addResource(*pSinkResources[sourceDevice]));
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addLink(*pSourceResources[sourceDevice], 0,
                                                                  *pSinkResources[sourceDevice], 0));

            // Enable input devices
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpInputDeviceManager->enableDevice(sourceDevice+1));
         }

         // Enable all resources in flowgraph.
         mpFlowGraph->enable();

         // Set flowgraph ticker
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              mpOutputDeviceManager->setFlowgraphTickerSource(sinkDeviceId,
                                                                              mpTicker));

         try {
            // Manage flowgraph with media task.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpMediaTask->manageFlowGraph(*mpFlowGraph));

            // Start flowgraph
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpMediaTask->startFlowGraph(*mpFlowGraph));

            // Run test!
            OsTask::delay(TEST_TIME_MS);

            // Clear flowgraph ticker and disable device provided it.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpOutputDeviceManager->setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE,
                                                                                 NULL));
         }
#ifndef SIPX_NO_CPPUNIT_EXCEPTIONS // [
         catch (CppUnit::Exception& e)
         {
            // Clear flowgraph ticker if assert failed.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpOutputDeviceManager->setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE,
                                                                                 NULL));

            // Rethrow exception.
            throw(e);
         }
#endif // !ANDROID ]

         // Unmanage flowgraph with media task.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpMediaTask->unmanageFlowGraph(*mpFlowGraph));
         MpMediaTask::signalFrameStart();
         // MediaTask need some time to receive unmanage message.
         OsTask::delay(20);

         // Disable devices
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              mpOutputDeviceManager->disableDevice(sinkDeviceId));
         for (sourceDevice=0; sourceDevice<mInputDeviceNumber; sourceDevice++)
         {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpInputDeviceManager->disableDevice(sourceDevice+1));
         }

         // Remove resources from flowgraph.
         for (sourceDevice=0; sourceDevice<mInputDeviceNumber; sourceDevice++)
         {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(*pSourceResources[sourceDevice]));
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(*pSinkResources[sourceDevice]));
         }
         mpFlowGraph->processNextFrame();
      }
#ifndef SIPX_NO_CPPUNIT_EXCEPTIONS // [
      catch (CppUnit::Exception& e)
      {
         // Remove resources from flowgraph. We should remove them explicitly
         // here, because they are stored on the stack and will be destroyed.
         // If we will not catch this assert we'll have this resources destroyed
         // while still referenced in flowgraph, causing crash.
         for (unsigned i=0; i<mInputDeviceNumber; i++)
         {
            if (pSourceResources[i]->getFlowGraph() != NULL)
            {
               CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(*pSourceResources[i]));
            }
            if (pSinkResources[i]->getFlowGraph() != NULL)
            {
               CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(*pSinkResources[i]));
            }
         }
         mpFlowGraph->processNextFrame();

         // Rethrow exception.
         throw(e);
      }
#endif // !ANDROID ]

      // Free output resources.
      for (sourceDevice=0; sourceDevice<mInputDeviceNumber; sourceDevice++)
      {
         delete pSourceResources[sourceDevice];
      }
      delete[] pSourceResources;

      RTL_WRITE("testManyInputDevicesToOneOutputDevice.rtl");
      RTL_STOP
   }

protected:
   MpFlowGraphBase  *mpFlowGraph; ///< Flowgraph for our fromInputDevice and
                                  ///< toOutputDevice resources.
   MpMediaTask *mpMediaTask;      ///< Pointer to media task instance.
   OsNotification *mpTicker;      ///< Media task ticker.
   MpInputDeviceManager  *mpInputDeviceManager;  ///< Manager for input devices.
   MpOutputDeviceManager *mpOutputDeviceManager; ///< Manager for output devices.
   unsigned     mInputDeviceNumber;
   unsigned     mOutputDeviceNumber;


   /// Add passed device to input device manager.
   void manageInputDevice(MpInputDeviceDriver *pDriver)
   {
      // Add driver to manager
      MpInputDeviceHandle deviceId = mpInputDeviceManager->addDevice(*pDriver);
      CPPUNIT_ASSERT(deviceId > 0);
      CPPUNIT_ASSERT(!mpInputDeviceManager->isDeviceEnabled(deviceId));

      // Driver is successfully added
      mInputDeviceNumber++;
   }

     /// Add passed device to output device manager.
   void manageOutputDevice(MpOutputDeviceDriver *pDriver)
   {
      // Add driver to manager
      MpOutputDeviceHandle deviceId = mpOutputDeviceManager->addDevice(pDriver);
      CPPUNIT_ASSERT(deviceId > 0);
      CPPUNIT_ASSERT(!mpOutputDeviceManager->isDeviceEnabled(deviceId));

      // Driver is successfully added
      mOutputDeviceNumber++;
   }

   void createTestInputDrivers()
   {
#ifdef USE_TEST_INPUT_DRIVER // [
      assert(sInputDriverNames == NULL);
      sInputDriverNames = new UtlString[nInputDrivers];

      size_t i;
      for (i=0; i < nInputDrivers; i++)
      {
         char devName[1024];
         snprintf(devName, 1024, "SineGenerator%d", i);
         sInputDriverNames[i] = devName;

         // Create driver
         MpSineWaveGeneratorDeviceDriver *pDriver
            = new MpSineWaveGeneratorDeviceDriver(sInputDriverNames[i],
                                                  *mpInputDeviceManager,
                                                  3000, 3000, 0);
         CPPUNIT_ASSERT(pDriver != NULL);

         // Add driver to manager
         manageInputDevice(pDriver);
      }
#endif // USE_TEST_INPUT_DRIVER ]
   }

   // ------------------------------------------------------------ bench helpers
   //
   // These drive scripts/bench_trigger.py, which makes the bench capture
   // device go away and come back (VMware USB detach on the VM, btaudio_ctl
   // on the laptop). Config lives outside the repo; when it is missing the
   // script exits 2 and the tests skip with a loud BENCH SKIPPED line.

   /// Run one trigger action. Returns the script's exit code; the single
   /// output line goes to outLine.
   int benchTrigger(const char* action, UtlString& outLine)
   {
      outLine = "";
#ifdef WIN32
      // The runner starts us in x64/Release; a manual run is from the
      // repo root. Find the script from either.
      const char* script = "scripts/bench_trigger.py";
      if (_access(script, 0) != 0)
      {
         script = "../../scripts/bench_trigger.py";
      }
      char cmd[256];
      _snprintf(cmd, sizeof(cmd) - 1,
                "sh -c \"python3 %s %s\"", script, action);
      cmd[sizeof(cmd) - 1] = 0;
      FILE* p = _popen(cmd, "rt");
      if (!p)
      {
         outLine = "FAILED: could not run bench_trigger.py";
         return 1;
      }
      char line[512];
      while (fgets(line, sizeof(line), p))
      {
         outLine.append(line);
      }
      outLine.strip(UtlString::both, '\n');
      outLine.strip(UtlString::both, '\r');
      int rc = _pclose(p);
      printf("bench_trigger %s -> %d: %s\n", action, rc, outLine.data());
      fflush(stdout);
      return rc;
#else
      outLine = "BENCH NOT CONFIGURED: bench tests are Windows-only";
      return 2;
#endif
   }

   /// Skip loudly unless the bench is configured and the device is ready.
   /// Fills match (guest device-name substring), runs and the offset list.
   void benchGate(UtlString& match, int& runs, UtlString& offsets)
   {
      UtlString line;
      int rc = benchTrigger("info", line);
      if (rc == 2)
      {
         printf("BENCH SKIPPED: %s\n", line.isNull() ? "bench_trigger.py not found or not runnable" : line.data());
         fflush(stdout);
         SIPX_TEST_SKIP("BENCH SKIPPED: bench not configured (see line above)");
      }
      CPPUNIT_ASSERT_MESSAGE(line.data(), rc == 0);

      // INFO trigger=... match=<m> runs=<n> offsets_ms=<a,b,c>
      match = "";
      runs = 3;
      offsets = "0,250,5000";
      const char* s = strstr(line.data(), "match=");
      if (s)
      {
         s += 6;
         const char* e = strchr(s, ' ');
         match.append(s, e ? (size_t)(e - s) : strlen(s));
      }
      s = strstr(line.data(), "runs=");
      if (s) runs = atoi(s + 5);
      s = strstr(line.data(), "offsets_ms=");
      if (s) offsets = s + 11;
      CPPUNIT_ASSERT_MESSAGE("bench info gave no device match", !match.isNull());

      rc = benchTrigger("status", line);
      if (rc == 3)
      {
         benchTrigger("connect", line);
         rc = benchTrigger("status", line);
      }
      if (rc != 0)
      {
         printf("BENCH SKIPPED: device not ready: %s\n", line.data());
         fflush(stdout);
         SIPX_TEST_SKIP("BENCH SKIPPED: bench device not ready (see line above)");
      }
   }

   /// Nth offset from the comma list, cycling.
   int benchOffsetMs(const UtlString& offsets, int iteration)
   {
      int values[16];
      int n = 0;
      const char* s = offsets.data();
      while (*s && n < 16)
      {
         values[n++] = atoi(s);
         const char* c = strchr(s, ',');
         if (!c) break;
         s = c + 1;
      }
      return n ? values[iteration % n] : 0;
   }

#ifdef WIN32
   /// Full WinMM capture name whose text contains match, or empty.
   UtlBoolean benchFindCaptureName(const UtlString& match, UtlString& fullName)
   {
      UINT n = waveInGetNumDevs();
      for (UINT i = 0; i < n; i++)
      {
         WAVEINCAPSA caps;
         memset(&caps, 0, sizeof(caps));
         if (waveInGetDevCapsA(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR
             && strstr(caps.szPname, match.data()))
         {
            fullName = caps.szPname;
            return TRUE;
         }
      }
      fullName = "";
      return FALSE;
   }

   /// Pull the most recent frames back from the manager and count the
   /// ones that carry audio (any non-zero sample). Proves capture is live
   /// from the consumer's side, not the driver's.
   int benchNonSilentFrames(MpInputDeviceHandle deviceId, int frames)
   {
      MpFrameTime frameTime = mpInputDeviceManager->getCurrentFrameTime(deviceId)
                              - (TEST_SAMPLES_PER_FRAME * 1000 / TEST_SAMPLES_PER_SECOND) * frames;
      int nonSilent = 0;
      for (int f = 0; f < frames; f++)
      {
         MpBufPtr buffer;
         unsigned before = 0, after = 0;
         if (mpInputDeviceManager->getFrame(deviceId, frameTime, buffer, before, after)
             == OS_SUCCESS && buffer.isValid())
         {
            MpAudioBufPtr audio = buffer;
            const MpAudioSample* s = audio->getSamplesPtr();
            unsigned n = audio->getSamplesNumber();
            for (unsigned i = 0; i < n; i++)
            {
               if (s[i] != 0) { nonSilent++; break; }
            }
         }
         frameTime += TEST_SAMPLES_PER_FRAME * 1000 / TEST_SAMPLES_PER_SECOND;
      }
      return nonSilent;
   }

   /// Create a fresh MpidWinMM for the bench device by name (the factory's
   /// path), add it, and return its handle. The driver is NOT counted in
   /// mInputDeviceNumber; the caller owns removal.
   MpInputDeviceHandle benchAddDevice(const UtlString& fullName, MpidWinMM*& pDriver)
   {
      pDriver = new MpidWinMM(fullName, *mpInputDeviceManager);
      CPPUNIT_ASSERT(pDriver != NULL);
      CPPUNIT_ASSERT_MESSAGE("bench device is not valid for WinMM", pDriver->isDeviceValid());
      MpInputDeviceHandle id = mpInputDeviceManager->addDevice(*pDriver);
      CPPUNIT_ASSERT(id > 0);
      return id;
   }

   /// Run a live graph bench device -> default output for ms, and assert
   /// audio-bearing frames reached the manager.
   void benchStreamAndVerify(MpInputDeviceHandle inId, MpOutputDeviceHandle outId,
                             MprFromInputDevice& source, MprToOutputDevice& sink,
                             int ms, const char* phase)
   {
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addResource(source));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addResource(sink));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addLink(source, 0, sink, 0));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpInputDeviceManager->enableDevice(inId));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpOutputDeviceManager->enableDevice(outId));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpOutputDeviceManager->setFlowgraphTickerSource(outId, mpTicker));
      CPPUNIT_ASSERT(source.enable());
      CPPUNIT_ASSERT(sink.enable());
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpMediaTask->manageFlowGraph(*mpFlowGraph));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpMediaTask->startFlowGraph(*mpFlowGraph));
      OsTask::delay(ms);
      int nonSilent = benchNonSilentFrames(inId, 8);
      printf("bench %s: %d of 8 recent frames carry audio\n", phase, nonSilent);
      fflush(stdout);
      CPPUNIT_ASSERT_MESSAGE("no audio reached the manager from the bench device",
                             nonSilent > 0);
   }

   /// Tear the graph down (not the input device: that is the operation
   /// under test and the caller does it).
   void benchStopGraph(MpOutputDeviceHandle outId,
                       MprFromInputDevice& source, MprToOutputDevice& sink)
   {
      mpOutputDeviceManager->setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE, NULL);
      mpMediaTask->unmanageFlowGraph(*mpFlowGraph);
      MpMediaTask::signalFrameStart();
      OsTask::delay(20);
      mpOutputDeviceManager->disableDevice(outId);
      mpFlowGraph->removeResource(sink);
      mpFlowGraph->removeResource(source);
      mpFlowGraph->processNextFrame();
   }
#endif // WIN32

   // Path 1: the device departs mid-stream, then the app disables it.
   // Expected on a real departure: clean, bounded teardown, no escape.
   void testBenchDisableDuringDeparture()
   {
#ifdef WIN32
      UtlString match, offsets, line, fullName;
      int runs;
      benchGate(match, runs, offsets);
      MpOutputDeviceHandle outId;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpOutputDeviceManager->getDeviceId(outputDriverNames[0], outId));

      for (int it = 0; it < runs; it++)
      {
         int offset = benchOffsetMs(offsets, it);
         CPPUNIT_ASSERT_MESSAGE("bench device not enumerated",
                                benchFindCaptureName(match, fullName));
         MpidWinMM* pDriver = NULL;
         MpInputDeviceHandle inId = benchAddDevice(fullName, pDriver);
         MprFromInputDevice source("BenchFromInput", mpInputDeviceManager, inId);
         MprToOutputDevice sink("BenchToOutput", mpOutputDeviceManager, outId);
         benchStreamAndVerify(inId, outId, source, sink, 1500, "before departure");

         CPPUNIT_ASSERT_EQUAL_MESSAGE(line.data(), 0, benchTrigger("disconnect", line));
         OsTask::delay(offset);

         DWORD t0 = GetTickCount();
         OsStatus st = mpInputDeviceManager->disableDevice(inId);
         DWORD elapsed = GetTickCount() - t0;
         printf("bench iteration %d offset %d ms: disableDevice -> %d in %lu ms, escaped=%d\n",
                it, offset, (int)st, (unsigned long)elapsed, (int)pDriver->lastDisableEscaped());
         fflush(stdout);
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, st);
         CPPUNIT_ASSERT_MESSAGE("disableDevice not bounded during departure", elapsed < 2000);
         CPPUNIT_ASSERT_MESSAGE("real departure should tear down cleanly, not escape",
                                !pDriver->lastDisableEscaped());

         benchStopGraph(outId, source, sink);
         mpInputDeviceManager->removeDevice(inId);
         if (!pDriver->lastDisableEscaped()) delete pDriver;

         CPPUNIT_ASSERT_EQUAL_MESSAGE(line.data(), 0, benchTrigger("connect", line));
      }
#else
      SIPX_TEST_SKIP("BENCH SKIPPED: bench tests are Windows-only");
#endif
   }

   // Path 2: the customer's crash path. Device departs with input still
   // enabled and the graph running; the app then tears everything down
   // the way sipxUnInitialize does, ending in removeAllDevices.
   void testBenchUninitializeAfterDeparture()
   {
#ifdef WIN32
      UtlString match, offsets, line, fullName;
      int runs;
      benchGate(match, runs, offsets);
      MpOutputDeviceHandle outId;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpOutputDeviceManager->getDeviceId(outputDriverNames[0], outId));

      for (int it = 0; it < runs; it++)
      {
         int offset = benchOffsetMs(offsets, it);
         CPPUNIT_ASSERT_MESSAGE("bench device not enumerated",
                                benchFindCaptureName(match, fullName));
         MpidWinMM* pDriver = NULL;
         MpInputDeviceHandle inId = benchAddDevice(fullName, pDriver);
         MprFromInputDevice source("BenchFromInput", mpInputDeviceManager, inId);
         MprToOutputDevice sink("BenchToOutput", mpOutputDeviceManager, outId);
         benchStreamAndVerify(inId, outId, source, sink, 1500, "before departure");

         CPPUNIT_ASSERT_EQUAL_MESSAGE(line.data(), 0, benchTrigger("disconnect", line));
         OsTask::delay(offset);

         // No disable. Stop the graph, then removeAllDevices with the input
         // still enabled: the uninitialize sequence.
         benchStopGraph(outId, source, sink);
         DWORD t0 = GetTickCount();
         int removed = mpInputDeviceManager->removeAllDevices();
         DWORD elapsed = GetTickCount() - t0;
         printf("bench iteration %d offset %d ms: removeAllDevices removed %d in %lu ms\n",
                it, offset, removed, (unsigned long)elapsed);
         fflush(stdout);
         CPPUNIT_ASSERT_MESSAGE("removeAllDevices not bounded after departure", elapsed < 3000);
         // removeAllDevices deleted (or retired) every driver, including
         // setUp's; tearDown must not touch them again.
         mInputDeviceNumber = 0;

         CPPUNIT_ASSERT_EQUAL_MESSAGE(line.data(), 0, benchTrigger("connect", line));

         // Re-create setUp's default driver so the next iteration (and
         // tearDown) see the state they expect.
         if (it + 1 < runs)
         {
            MpidWinMM* pDefault = new MpidWinMM(sInputDriverNames[0], *mpInputDeviceManager);
            CPPUNIT_ASSERT(pDefault != NULL);
            manageInputDevice(pDefault);
         }
      }
#else
      SIPX_TEST_SKIP("BENCH SKIPPED: bench tests are Windows-only");
#endif
   }

   // Path 3: recovery. After a departure cycle, the device returns and a
   // fresh driver (the factory's path) must deliver audio.
   void testBenchRecoveryAfterReturn()
   {
#ifdef WIN32
      UtlString match, offsets, line, fullName;
      int runs;
      benchGate(match, runs, offsets);
      MpOutputDeviceHandle outId;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpOutputDeviceManager->getDeviceId(outputDriverNames[0], outId));

      for (int it = 0; it < runs; it++)
      {
         // Departure with a live driver, clean disable, device returns.
         CPPUNIT_ASSERT_MESSAGE("bench device not enumerated",
                                benchFindCaptureName(match, fullName));
         MpidWinMM* pOld = NULL;
         MpInputDeviceHandle oldId = benchAddDevice(fullName, pOld);
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpInputDeviceManager->enableDevice(oldId));
         OsTask::delay(500);
         CPPUNIT_ASSERT_EQUAL_MESSAGE(line.data(), 0, benchTrigger("disconnect", line));
         OsTask::delay(250);
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpInputDeviceManager->disableDevice(oldId));
         mpInputDeviceManager->removeDevice(oldId);
         if (!pOld->lastDisableEscaped()) delete pOld;
         CPPUNIT_ASSERT_EQUAL_MESSAGE(line.data(), 0, benchTrigger("connect", line));

         // Fresh driver on the returned device: must stream real audio.
         CPPUNIT_ASSERT_MESSAGE("bench device did not re-enumerate",
                                benchFindCaptureName(match, fullName));
         MpidWinMM* pNew = NULL;
         MpInputDeviceHandle newId = benchAddDevice(fullName, pNew);
         MprFromInputDevice source("BenchFromInput", mpInputDeviceManager, newId);
         MprToOutputDevice sink("BenchToOutput", mpOutputDeviceManager, outId);
         DWORD t0 = GetTickCount();
         benchStreamAndVerify(newId, outId, source, sink, 2000, "after return");
         printf("bench iteration %d: recovery verified %lu ms after re-enable\n",
                it, (unsigned long)(GetTickCount() - t0));
         fflush(stdout);

         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpInputDeviceManager->disableDevice(newId));
         CPPUNIT_ASSERT(!pNew->lastDisableEscaped());
         benchStopGraph(outId, source, sink);
         mpInputDeviceManager->removeDevice(newId);
         delete pNew;
      }
#else
      SIPX_TEST_SKIP("BENCH SKIPPED: bench tests are Windows-only");
#endif
   }

   void createWntInputDrivers()
   {
#ifdef USE_WNT_INPUT_DRIVER // [
      nInputDrivers = 1;  // Make sure we want only one driver.
      assert(sInputDriverNames == NULL);
      sInputDriverNames = new UtlString[nInputDrivers];
      sInputDriverNames[0] = MpidWinMM::getDefaultDeviceName();

      // Create driver
      MpidWinMM *pDriver
         = new MpidWinMM(sInputDriverNames[0], *mpInputDeviceManager);
      CPPUNIT_ASSERT(pDriver != NULL);

      // Add driver to manager
      manageInputDevice(pDriver);
#endif // USE_WNT_INPUT_DRIVER ]
   }

   void createAndroidInputDrivers()
   {
#ifdef USE_ANDROID_INPUT_DRIVER // [
      assert(sInputDriverNames == NULL);
      sInputDriverNames = new UtlString[nInputDrivers];

      size_t i;
      for (i=0; i < nInputDrivers; i++)
      {
         sInputDriverNames[i] = "default";

         // Create driver
         MpidAndroid *pDriver = new MpidAndroid(MpidAndroid::AUDIO_SOURCE_DEFAULT,
                                                *mpInputDeviceManager);
         CPPUNIT_ASSERT(pDriver != NULL);

         // Add driver to manager
         manageInputDevice(pDriver);
      }
#endif // USE_ANDROID_INPUT_DRIVER ]
   }

   void createOSSInputDrivers()
   {
#ifdef USE_OSS_INPUT_DRIVER // [
      assert(sInputDriverNames == NULL);
      sInputDriverNames = new UtlString[nInputDrivers];

      size_t i;
      for (i=0; i < nInputDrivers; i++)
      {
         char devName[1024];
         if (i == 0)
         {
            snprintf(devName, 1024, "/dev/dsp");
         }
         else
         {
            snprintf(devName, 1024, "/dev/dsp%d", i);
         }
         sInputDriverNames[i] = devName;

         // Create driver
         MpidOss *pDriver = new MpidOss(sInputDriverNames[i], *mpInputDeviceManager);
         CPPUNIT_ASSERT(pDriver != NULL);

         // Add driver to manager
         manageInputDevice(pDriver);
      }
#endif // USE_OSS_INPUT_DRIVER ]
   }

   void createTestOutputDrivers()
   {
#ifdef USE_TEST_OUTPUT_DRIVER // [
      assert(outputDriverNames == NULL);
      outputDriverNames = new UtlString[nOutputDrivers];

      size_t i;
      for (i=0; i < nOutputDrivers; i++)
      {
         char devName[1024];
         snprintf(devName, 1024, "BufferRecorder%d", i);
         outputDriverNames[i] = devName;

         // Create driver
         MpodBufferRecorder *pDriver = new MpodBufferRecorder(outputDriverNames[i],
                                                              TEST_TIME_MS);
         CPPUNIT_ASSERT(pDriver != NULL);

         // Add driver to manager
         manageOutputDevice(pDriver);
      }
#endif // USE_TEST_OUTPUT_DRIVER ]
   }

   void createWntOutputDrivers()
   {
#ifdef USE_WNT_INPUT_DRIVER // [
      nOutputDrivers = 1;  // Make sure we want only one driver.
      assert(outputDriverNames == NULL);
      outputDriverNames = new UtlString[nOutputDrivers];
      outputDriverNames[0] = MpodWinMM::getDefaultDeviceName();

      // Create driver
      MpodWinMM *pDriver = new MpodWinMM(outputDriverNames[0], mpOutputDeviceManager);
      CPPUNIT_ASSERT(pDriver != NULL);

      // Add driver to manager
      manageOutputDevice(pDriver);
#endif // USE_WNT_INPUT_DRIVER ]
   }

   void createAndroidOutputDrivers()
   {
#ifdef USE_ANDROID_OUTPUT_DRIVER // [
      assert(outputDriverNames == NULL);
      outputDriverNames = new UtlString[nOutputDrivers];

      size_t i;
      for (i=0; i < nOutputDrivers; i++)
      {
         // Create driver
         MpodAndroid *pDriver = new MpodAndroid(MpAndroidAudioBindingInterface::DEFAULT);
         CPPUNIT_ASSERT(pDriver != NULL);

         // Add driver to manager
         manageOutputDevice(pDriver);
      }
#endif // USE_ANDROID_OUTPUT_DRIVER ]
   }

   void createOSSOutputDrivers()
   {
#ifdef USE_OSS_OUTPUT_DRIVER // [
      assert(outputDriverNames == NULL);
      outputDriverNames = new UtlString[nOutputDrivers];

      size_t i;
      for (i=0; i < nOutputDrivers; i++)
      {
         char devName[1024];
         if (i == 0)
         {
            snprintf(devName, 1024, "/dev/dsp");
         }
         else
         {
            snprintf(devName, 1024, "/dev/dsp%d", i);
         }
         outputDriverNames[i] = devName;

         // Create driver
         MpodOss *pDriver = new MpodOss(outputDriverNames[i]);
         CPPUNIT_ASSERT(pDriver != NULL);

         // Add driver to manager
         manageOutputDevice(pDriver);
      }
#endif // USE_OSS_OUTPUT_DRIVER ]
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(MpInputOutputFrameworkTest);
