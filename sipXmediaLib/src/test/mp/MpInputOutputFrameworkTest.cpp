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

#define TEST_TIME_MS                  10000 ///< Time to runs test (in milliseconds)
#define AUDIO_BUFS_NUM                500   ///< Number of buffers in buffer pool we'll use.
#define BUFFERS_TO_BUFFER_ON_INPUT    3     ///< Number of buffers to buffer in input manager.
#define BUFFERS_TO_BUFFER_ON_OUTPUT   0     ///< Number of buffers to buffer in output manager.

#define TEST_SAMPLES_PER_FRAME        80    ///< in samples
#define TEST_SAMPLES_PER_SECOND       8000  ///< in samples/sec (Hz)

#define DEFAULT_BUFFER_ON_OUTPUT_MS   (BUFFERS_TO_BUFFER_ON_OUTPUT*TEST_SAMPLES_PER_FRAME*1000/TEST_SAMPLES_PER_SECOND)
                                            ///< Buffer size in output manager in milliseconds.
#define DEFAULT_INPUT_DEVICE          "SineGenerator1"
                                            ///< This device would be used if
                                            ///< test does require single input
                                            ///< device.
#define DEFAULT_OUTPUT_DEVICE         "BufferRecorder1"
                                            ///< This device would be used if
                                            ///< test does require single output
                                            ///< device. It would be also used
                                            ///< to select ticker source if test
                                            ///< engage all output devices.

#define TEST_INPUT_DRIVERS            10    ///< Number of sine generators
#define TEST_OUTPUT_DRIVERS           10    ///< Number of buffer recorders
#define OSS_INPUT_DRIVERS             2     ///< Number of OSS input drivers
#define OSS_OUTPUT_DRIVERS            2     ///< Number of OSS output drivers

#define USE_TEST_INPUT_DRIVER
#define USE_TEST_OUTPUT_DRIVER

// OS-specific device drivers
#ifdef __pingtel_on_posix__ // [
#  define USE_OSS_INPUT_DRIVER
#  define USE_OSS_OUTPUT_DRIVER
#endif // __pingtel_on_posix__ ]
#ifdef WIN32 // [
#  define USE_WNT_INPUT_DRIVER
#endif // WIN32 ]

#ifdef USE_TEST_INPUT_DRIVER // [
#  include <mp/MpSineWaveGeneratorDeviceDriver.h>
#endif // USE_TEST_INPUT_DRIVER ]
#ifdef USE_WNT_INPUT_DRIVER // [
#  include <mp/MpInputDeviceDriverWnt.h>
#endif // USE_WNT_INPUT_DRIVER ]
#ifdef __pingtel_on_posix__ // [
#  include <mp/MpidOSS.h>
#endif // __pingtel_on_posix__ ]

#ifdef USE_TEST_OUTPUT_DRIVER // [
#  include <mp/MpodBufferRecorder.h>
#endif // USE_TEST_OUTPUT_DRIVER ]
#ifdef __pingtel_on_posix__ // [
#  include <mp/MpodOSS.h>
#endif // __pingtel_on_posix__ ]


///  Unit test for MprSplitter
class MpInputOutputFrameworkTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(MpInputOutputFrameworkTest);
   CPPUNIT_TEST(testShortCircuit);
   CPPUNIT_TEST(testManyOutputDevices);
   CPPUNIT_TEST(testManyInputDevices);
   CPPUNIT_TEST(testManyInputDevicesToOneOutputDevice);
   CPPUNIT_TEST_SUITE_END();

public:

   // This function will be called before every test to setup framework.
   void setUp()
   {
      enableConsoleOutput(1);

      // Setup media task
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpStartUp(TEST_SAMPLES_PER_SECOND,
                                     TEST_SAMPLES_PER_FRAME, 6*10, 0));

      // Create flowgraph
      mpFlowGraph = new MpFlowGraphBase(TEST_SAMPLES_PER_FRAME,
                                        TEST_SAMPLES_PER_SECOND);
      CPPUNIT_ASSERT(mpFlowGraph != NULL);

      // Call getMediaTask() which causes the task to get instantiated
      mpMediaTask = MpMediaTask::getMediaTask(10);
      CPPUNIT_ASSERT(mpMediaTask != NULL);

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
      createOSSInputDrivers();
      createTestOutputDrivers();
      createOSSOutputDrivers();
   }

   // This function will be called after every test to clean up framework.
   void tearDown()
   {
      // This should normally be done by haltFramework, but if we aborted due
      // to an assertion the flowgraph will need to be shutdown here
      if (mpFlowGraph && mpFlowGraph->isStarted())
      {
         osPrintf("WARNING: flowgraph found still running, shutting down\n");

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
         delete pDriver;
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

      // Clear all media processing data and delete MpMediaTask instance.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpShutdown());
   }

   void testShortCircuit()
   {
      RTL_START(10000000);

      MpInputDeviceHandle sourceDeviceId;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpInputDeviceManager->getDeviceId(DEFAULT_INPUT_DEVICE,
                                                             sourceDeviceId));

      MpOutputDeviceHandle  sinkDeviceId;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpOutputDeviceManager->getDeviceId(DEFAULT_OUTPUT_DEVICE,
                                                              sinkDeviceId));

      // Create source (input) and sink (output) resources.
      MprFromInputDevice sourceResource("MprFromInputDevice",
                                        TEST_SAMPLES_PER_FRAME,
                                        TEST_SAMPLES_PER_SECOND,
                                        mpInputDeviceManager,
                                        sourceDeviceId);
      MprToOutputDevice sinkResource("MprToOutputDevice",
                                     TEST_SAMPLES_PER_FRAME,
                                     TEST_SAMPLES_PER_SECOND,
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
                              mpOutputDeviceManager->setFlowgraphTickerSource(sinkDeviceId));

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
                                 mpOutputDeviceManager->setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE));
         }
         catch (CppUnit::Exception& e)
         {
            // Clear flowgraph ticker if assert failed.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpOutputDeviceManager->setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE));

            // Rethrow exception.
            throw(e);
         }

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

      RTL_WRITE("testShortCircuit.rtl");
      RTL_STOP
   }

   void testManyOutputDevices()
   {
      unsigned sinkDevice;

      RTL_START(10000000);

      MpOutputDeviceHandle  tickerDeviceId;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpOutputDeviceManager->getDeviceId(DEFAULT_OUTPUT_DEVICE,
                                                              tickerDeviceId));

      // Create generator resource.
      MprToneGen toneGen("ToneGenerator",
                         TEST_SAMPLES_PER_FRAME,
                         TEST_SAMPLES_PER_SECOND,
                         NULL);

      MprSplitter splitter("Splitter",
                           mOutputDeviceNumber,
                           TEST_SAMPLES_PER_FRAME,
                           TEST_SAMPLES_PER_SECOND);

      // Create resources for all available output devices.
      MprToOutputDevice **pSinkResources = new MprToOutputDevice*[mOutputDeviceNumber];
      for (sinkDevice=0; sinkDevice<mOutputDeviceNumber; sinkDevice++)
      {
         char devName[1024];
         snprintf(devName, 1024, "MprToOutputDevice%d", sinkDevice);

         pSinkResources[sinkDevice] = new MprToOutputDevice(devName,
                                                            TEST_SAMPLES_PER_FRAME,
                                                            TEST_SAMPLES_PER_SECOND,
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
                              mpOutputDeviceManager->setFlowgraphTickerSource(tickerDeviceId));

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
                                 mpOutputDeviceManager->setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE));

         }
         catch (CppUnit::Exception& e)
         {
            // Clear flowgraph ticker if assert failed.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpOutputDeviceManager->setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE));

            // Rethrow exception.
            throw(e);
         }

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

      MpOutputDeviceHandle  tickerDeviceId;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpOutputDeviceManager->getDeviceId(DEFAULT_OUTPUT_DEVICE,
                                                              tickerDeviceId));

      // Create sink resource.
      MprNull blackHole("MprNull",
                        mInputDeviceNumber,
                        TEST_SAMPLES_PER_FRAME,
                        TEST_SAMPLES_PER_SECOND);

      // Create resources for all available input devices.
      MprFromInputDevice **pSourceResources = new MprFromInputDevice*[mInputDeviceNumber];
      for (sourceDevice=0; sourceDevice<mInputDeviceNumber; sourceDevice++)
      {
         char devName[1024];
         snprintf(devName, 1024, "MprFromInputDevice%d", sourceDevice);

         pSourceResources[sourceDevice] = new MprFromInputDevice(devName,
                                                                 TEST_SAMPLES_PER_FRAME,
                                                                 TEST_SAMPLES_PER_SECOND,
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
                              mpOutputDeviceManager->setFlowgraphTickerSource(tickerDeviceId));

         try {
            // Manage flowgraph with media task.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpMediaTask->manageFlowGraph(*mpFlowGraph));

            // Start flowgraph
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpMediaTask->startFlowGraph(*mpFlowGraph));

            // Run test!
            OsTask::delay(TEST_TIME_MS);

            // Clear flowgraph ticker and disable device provided it.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpOutputDeviceManager->setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE));
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpOutputDeviceManager->disableDevice(tickerDeviceId));
         }
         catch (CppUnit::Exception& e)
         {
            // Clear flowgraph ticker if assert failed.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpOutputDeviceManager->setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE));
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpOutputDeviceManager->disableDevice(tickerDeviceId));

            // Rethrow exception.
            throw(e);
         }

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

      MpOutputDeviceHandle  sinkDeviceId;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpOutputDeviceManager->getDeviceId(DEFAULT_OUTPUT_DEVICE,
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
                                                                 TEST_SAMPLES_PER_FRAME,
                                                                 TEST_SAMPLES_PER_SECOND,
                                                                 mpInputDeviceManager,
                                                                 sourceDevice+1);

         snprintf(devName, 1024, "MprToOutputDevice%d", sourceDevice);
         pSinkResources[sourceDevice] = new MprToOutputDevice(devName,
                                                              TEST_SAMPLES_PER_FRAME,
                                                              TEST_SAMPLES_PER_SECOND,
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
                              mpOutputDeviceManager->setFlowgraphTickerSource(sinkDeviceId));

         try {
            // Manage flowgraph with media task.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpMediaTask->manageFlowGraph(*mpFlowGraph));

            // Start flowgraph
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpMediaTask->startFlowGraph(*mpFlowGraph));

            // Run test!
            OsTask::delay(TEST_TIME_MS);

            // Clear flowgraph ticker and disable device provided it.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpOutputDeviceManager->setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE));
         }
         catch (CppUnit::Exception& e)
         {
            // Clear flowgraph ticker if assert failed.
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                 mpOutputDeviceManager->setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE));

            // Rethrow exception.
            throw(e);
         }

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
      for (int i=0; i < TEST_INPUT_DRIVERS; i++)
      {
         char devName[1024];
         snprintf(devName, 1024, "SineGenerator%d", i);

         // Create driver
         MpSineWaveGeneratorDeviceDriver *pDriver
            = new MpSineWaveGeneratorDeviceDriver(devName,
                                                  *mpInputDeviceManager,
                                                  3000, 3000, 0);
         CPPUNIT_ASSERT(pDriver != NULL);

         // Add driver to manager
         manageInputDevice(pDriver);
      }
#endif // USE_TEST_INPUT_DRIVER ]
   }

   void createWntInputDrivers()
   {
#ifdef USE_WNT_INPUT_DRIVER // [
      // Create driver
      MpInputDeviceDriverWnt *pDriver
         = new MpInputDeviceDriverWnt("SoundMAX HD Audio", *mpInputDeviceManager);
      CPPUNIT_ASSERT(pDriver != NULL);

      // Add driver to manager
      manageInputDevice(pDriver);
#endif // USE_WNT_INPUT_DRIVER ]
   }

   void createOSSInputDrivers()
   {
#ifdef USE_OSS_INPUT_DRIVER // [
      for (int i=0; i < OSS_INPUT_DRIVERS; i++)
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

         // Create driver
         MpidOSS *pDriver = new MpidOSS(devName, *mpInputDeviceManager);
         CPPUNIT_ASSERT(pDriver != NULL);

         // Add driver to manager
         manageInputDevice(pDriver);
      }
#endif // USE_OSS_INPUT_DRIVER ]
   }

   void createTestOutputDrivers()
   {
#ifdef USE_TEST_OUTPUT_DRIVER // [
      for (int i=0; i < TEST_OUTPUT_DRIVERS; i++)
      {
         char devName[1024];
         snprintf(devName, 1024, "BufferRecorder%d", i);

         // Create driver
         MpodBufferRecorder *pDriver = new MpodBufferRecorder(devName,
                                                              TEST_TIME_MS);
         CPPUNIT_ASSERT(pDriver != NULL);

         // Add driver to manager
         manageOutputDevice(pDriver);
      }
#endif // USE_TEST_OUTPUT_DRIVER ]
   }

   void createOSSOutputDrivers()
   {
#ifdef USE_OSS_OUTPUT_DRIVER // [
      for (int i=0; i < OSS_OUTPUT_DRIVERS; i++)
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

         // Create driver
         MpodOSS *pDriver = new MpodOSS(devName);
         CPPUNIT_ASSERT(pDriver != NULL);

         // Add driver to manager
         manageOutputDevice(pDriver);
      }
#endif // USE_OSS_OUTPUT_DRIVER ]
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(MpInputOutputFrameworkTest);
