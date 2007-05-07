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
#include "mp/MpFlowGraphBase.h"
#include "mp/MpMisc.h"
#include "mp/MpMediaTask.h"
#include "os/OsTask.h"
#ifdef RTL_ENABLED
#  include <rtl_macro.h>
#else
#  define RTL_BLOCK(x)
#  define RTL_EVENT(x,y)
#endif

#include <os/OsFS.h>

#define TEST_TIME_MS                  10000 ///< Time to runs test (in milliseconds)
#define AUDIO_BUFS_NUM                500   ///< Number of buffers in buffer pool we'll use.
#define BUFFERS_TO_BUFFER_ON_INPUT    3     ///< Number of buffers to buffer in input manager.
#define BUFFERS_TO_BUFFER_ON_OUTPUT   0     ///< Number of buffers to buffer in output manager.

#define TEST_SAMPLES_PER_FRAME        80    ///< in samples
#define TEST_SAMPLES_PER_SECOND       8000  ///< in samples/sec (Hz)

#define BUFFER_ON_OUTPUT_MS           (BUFFERS_TO_BUFFER_ON_OUTPUT*TEST_SAMPLES_PER_FRAME*1000/TEST_SAMPLES_PER_SECOND)
                                            ///< Buffer size in output manager in milliseconds.

//#define USE_TEST_INPUT_DRIVER
//#define USE_TEST_OUTPUT_DRIVER

#ifdef USE_TEST_INPUT_DRIVER // USE_TEST_DRIVER [
#include <mp/MpSineWaveGeneratorDeviceDriver.h>
#define INPUT_DRIVER MpSineWaveGeneratorDeviceDriver
#define INPUT_DRIVER_CONSTRUCTOR_PARAMS(manager) "SineGenerator", (manager), 32000, 3000, 0

#elif defined(WIN32) // USE_TEST_DRIVER ][ WIN32
#include <mp/MpInputDeviceDriverWnt.h>
#define INPUT_DRIVER MpInputDeviceDriverWnt
#define INPUT_DRIVER_CONSTRUCTOR_PARAMS(manager) "SoundMAX HD Audio", (manager)

#elif defined(__pingtel_on_posix__) // WIN32 ][ __pingtel_on_posix__
#include <mp/MpidOSS.h>
#define INPUT_DRIVER MpidOSS
#define INPUT_DRIVER_CONSTRUCTOR_PARAMS(manager) "/dev/dsp", (manager)

#else // __pingtel_on_possix__ ]
#error Unknown platform!
#endif

#ifdef USE_TEST_OUTPUT_DRIVER // USE_TEST_DRIVER [
#include <mp/MpodBufferRecorder.h>
#define OUTPUT_DRIVER MpodBufferRecorder
#define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS "default", TEST_TIME_MS

#elif defined(WIN32) // USE_TEST_DRIVER ][ WIN32
#error No output driver for Windows exist!

#elif defined(__pingtel_on_posix__) // WIN32 ][ __pingtel_on_posix__
#include <mp/MpodOSS.h>
#define OUTPUT_DRIVER MpodOSS
#define OUTPUT_DRIVER_CONSTRUCTOR_PARAMS "/dev/dsp"

#else // __pingtel_on_possix__ ]
#error Unknown platform!
#endif


///  Unit test for MprSplitter
class MpInputOutputFrameworkTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(MpInputOutputFrameworkTest);
   CPPUNIT_TEST(testShortCircuit);
   CPPUNIT_TEST_SUITE_END();

protected:
   MpFlowGraphBase*  mpFlowGraph; ///< Flowgraph for our fromInputDevice and
                              ///< toOutputDevice resources.

public:

   void setUp()
   {
      // Setup media task
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpStartUp(TEST_SAMPLES_PER_SECOND, TEST_SAMPLES_PER_FRAME, 6*10, 0));

      mpFlowGraph = new MpFlowGraphBase( TEST_SAMPLES_PER_FRAME
                                       , TEST_SAMPLES_PER_SECOND);
      CPPUNIT_ASSERT(mpFlowGraph != NULL);

      // Call getMediaTask() which causes the task to get instantiated
      CPPUNIT_ASSERT(MpMediaTask::getMediaTask(10) != NULL);
   }

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

      // Free flowgraph resources
      delete mpFlowGraph;
      mpFlowGraph = NULL;

      // Clear all Media Tasks data
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpShutdown());
   }

   void testShortCircuit()
   {
      enableConsoleOutput(1);

#ifdef RTL_ENABLED
      RTL_START(10000000);
#endif

      // Get media processing task.
      MpMediaTask *pMediaTask = MpMediaTask::getMediaTask(10);
      CPPUNIT_ASSERT(pMediaTask != NULL);

      // Create input and output device managers
      MpInputDeviceManager inputDeviceManager(TEST_SAMPLES_PER_FRAME, 
                                              TEST_SAMPLES_PER_SECOND,
                                              BUFFERS_TO_BUFFER_ON_INPUT,
                                              *MpMisc.RawAudioPool);
      MpOutputDeviceManager outputDeviceManager(TEST_SAMPLES_PER_FRAME,
                                                TEST_SAMPLES_PER_SECOND,
                                                BUFFER_ON_OUTPUT_MS);


      // Create source (input) device and add it to manager.
      INPUT_DRIVER sourceDevice(INPUT_DRIVER_CONSTRUCTOR_PARAMS(inputDeviceManager));
      MpInputDeviceHandle  sourceDeviceId = inputDeviceManager.addDevice(sourceDevice);
      CPPUNIT_ASSERT(sourceDeviceId > 0);
      CPPUNIT_ASSERT(!inputDeviceManager.isDeviceEnabled(sourceDeviceId));

      // Create sink (output) device and add it to manager.
      OUTPUT_DRIVER sinkDevice(OUTPUT_DRIVER_CONSTRUCTOR_PARAMS);
      MpOutputDeviceHandle  sinkDeviceId = outputDeviceManager.addDevice(&sinkDevice);
      CPPUNIT_ASSERT(sinkDeviceId > 0);
      CPPUNIT_ASSERT(!outputDeviceManager.isDeviceEnabled(sinkDeviceId));

      // Create source (input) and sink (output) resources.
      MprFromInputDevice pSource("MprFromInputDevice",
                                 TEST_SAMPLES_PER_FRAME,
                                 TEST_SAMPLES_PER_SECOND,
                                 &inputDeviceManager,
                                 sourceDeviceId);
      MprToOutputDevice pSink("MprToOutputDevice",
                              TEST_SAMPLES_PER_FRAME,
                              TEST_SAMPLES_PER_SECOND,
                              &outputDeviceManager,
                              sinkDeviceId);

      // Add source and sink resources to flowgraph and link them together.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addResource(pSource));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addResource(pSink));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addLink(pSource, 0, pSink, 0));

      try {

         // Set flowgraph ticker
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              outputDeviceManager.setFlowgraphTickerSource(sinkDeviceId));

         // Enable devices
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              inputDeviceManager.enableDevice(sourceDeviceId));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              outputDeviceManager.enableDevice(sinkDeviceId));

         // Enable resources
         CPPUNIT_ASSERT(pSource.enable());
         CPPUNIT_ASSERT(pSink.enable());

         // Manage flowgraph with media task.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMediaTask->manageFlowGraph(*mpFlowGraph));

         // Start flowgraph
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMediaTask->startFlowGraph(*mpFlowGraph));

         // Run test!
         OsTask::delay(TEST_TIME_MS);
/*       for (int i=0; i<TEST_TIME_MS*(TEST_SAMPLES_PER_SECOND/TEST_SAMPLES_PER_FRAME)/1000; i++)
         {
            RTL_BLOCK("test loop body");
            osPrintf("==> i=%d\n",i);
            OsTask::delay(TEST_SAMPLES_PER_FRAME*1000/TEST_SAMPLES_PER_SECOND-2);
            RTL_EVENT("test loop body", 2);
            MpMediaTask::signalFrameStart();
         }
*/

         // Clear flowgraph ticker
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              outputDeviceManager.setFlowgraphTickerSource(MP_INVALID_OUTPUT_DEVICE_HANDLE));

//         OsTask::delay(50);

         // Disable resources
         CPPUNIT_ASSERT(pSource.disable());
         CPPUNIT_ASSERT(pSink.disable());
         MpMediaTask::signalFrameStart();

         // Unmanage flowgraph with media task.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pMediaTask->unmanageFlowGraph(*mpFlowGraph));
         MpMediaTask::signalFrameStart();

#ifdef USE_TEST_OUTPUT_DRIVER // [
         OsFile::openAndWrite("capture.raw",
                              (const char*)sinkDevice.getBufferData(),
                              sinkDevice.getBufferLength()*sizeof(MpAudioSample));
#endif // USE_TEST_OUTPUT_DRIVER ]

         // Disable devices
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              inputDeviceManager.disableDevice(sourceDeviceId));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                              outputDeviceManager.disableDevice(sinkDeviceId));

         // Remove resources from flowgraph. We sohuld remove them explicitly
         // here, because they are stored on the stack and will be destroyed.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(pSink));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(pSource));
         mpFlowGraph->processNextFrame();
      }
      catch (CppUnit::Exception& e)
      {
         // Remove resources from flowgraph. We sohuld remove them explicitly
         // here, because they are stored on the stack and will be destroyed.
         // If we will not catch this assert we'll have this resources destroyed
         // while still referenced in flowgraph, causing crash.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(pSink));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(pSource));
         mpFlowGraph->processNextFrame();

         // Rethrow exception.
         throw(e);
      }

      // Remove devices from managers.
      CPPUNIT_ASSERT_EQUAL((MpInputDeviceDriver*)&sourceDevice,
                           inputDeviceManager.removeDevice(sourceDeviceId));
      CPPUNIT_ASSERT_EQUAL((MpOutputDeviceDriver*)&sinkDevice,
                           outputDeviceManager.removeDevice(sinkDeviceId));

#ifdef RTL_ENABLED
      RTL_WRITE("testShortCircuit.rtl");
#endif

   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MpInputOutputFrameworkTest);
