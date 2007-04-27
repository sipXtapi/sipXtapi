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

#include "mp/MpGenericResourceTest.h"
#include "mp/MpInputDeviceManager.h"
#include "mp/MprFromInputDevice.h"
#include "mp/MpOutputDeviceManager.h"
#include "mp/MprToOutputDevice.h"
#ifdef RTL_ENABLED
#  include <rtl_macro.h>
#endif

#include <os/OsFS.h>

#define TEST_TIME_MS                  10000  ///< Time to runs test (in milliseconds)
#define AUDIO_BUFS_NUM                500   ///< Number of buffers in buffer pool we'll use.
#define BUFFERS_TO_BUFFER_ON_INPUT    3     ///< Number of buffers to buffer in input manager.
#define BUFFERS_TO_BUFFER_ON_OUTPUT   3     ///< Number of buffers to buffer in output manager.

#define TEST_SAMPLES_PER_FRAME        80    ///< in samples
#define TEST_SAMPLES_PER_SECOND       8000  ///< in samples/sec (Hz)

//#define USE_TEST_INPUT_DRIVER
//#define USE_TEST_OUTPUT_DRIVER

#ifdef USE_TEST_INPUT_DRIVER // USE_TEST_DRIVER [
#error Not supported platform!

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
class MpInputOutputFrameworkTest : public MpGenericResourceTest
{
   CPPUNIT_TEST_SUB_SUITE(MpInputOutputFrameworkTest, MpGenericResourceTest);
   CPPUNIT_TEST(testShortCircuit);
   CPPUNIT_TEST_SUITE_END();

protected:
   MpBufPool *mpPool;         ///< Pool for data buffers
   MpBufPool *mpHeadersPool;  ///< Pool for buffers headers

public:

   void setUp()
   {
      MpGenericResourceTest::setUp();

      // Create pool for data buffers
      mpPool = new MpBufPool(TEST_SAMPLES_PER_FRAME * sizeof(MpAudioSample)
                             + MpArrayBuf::getHeaderSize(), AUDIO_BUFS_NUM);
      CPPUNIT_ASSERT(mpPool != NULL);

      // Create pool for buffer headers
      mpHeadersPool = new MpBufPool(sizeof(MpAudioBuf), AUDIO_BUFS_NUM);
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

      MpGenericResourceTest::setUp();
   }

   void testShortCircuit()
   {
#ifdef RTL_ENABLED
      RTL_START(1000000);
#endif

      // Create input and output device managers
      MpInputDeviceManager inputDeviceManager(TEST_SAMPLES_PER_FRAME, 
                                              TEST_SAMPLES_PER_SECOND,
                                              BUFFERS_TO_BUFFER_ON_INPUT,
                                              *mpPool);
      MpOutputDeviceManager outputDeviceManager(TEST_SAMPLES_PER_FRAME,
                                                TEST_SAMPLES_PER_SECOND,
                                                BUFFERS_TO_BUFFER_ON_OUTPUT);


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
                                 TEST_SAMPLES_PER_SEC,
                                 &inputDeviceManager,
                                 sourceDeviceId);
      MprToOutputDevice pSink("MprToOutputDevice",
                              TEST_SAMPLES_PER_FRAME,
                              TEST_SAMPLES_PER_SEC,
                              &outputDeviceManager,
                              sinkDeviceId);

      // Add source and sink resources to flowgraph and link them together.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addResource(pSource));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addResource(pSink));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addLink(pSource, 0, pSink, 0));

      // Enable devices
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           inputDeviceManager.enableDevice(sourceDeviceId));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           outputDeviceManager.enableDevice(sinkDeviceId));
   
      // Enable resources
      CPPUNIT_ASSERT(pSource.enable());
      CPPUNIT_ASSERT(pSink.enable());

      mpFlowGraph->start();

      // Run test!
      for (int i=0; i<TEST_TIME_MS*(TEST_SAMPLES_PER_SECOND/TEST_SAMPLES_PER_FRAME)/1000; i++)
      {
         RTL_BLOCK("test loop body");
         printf("==> i=%d\n",i);
         OsTask::delay(TEST_SAMPLES_PER_FRAME*1000/TEST_SAMPLES_PER_SECOND-2);
	 RTL_EVENT("test loop body", 2);
         mpFlowGraph->processNextFrame();
      }

      // Disable resources
      CPPUNIT_ASSERT(pSource.disable());
      CPPUNIT_ASSERT(pSink.disable());
      mpFlowGraph->processNextFrame();

      mpFlowGraph->stop();

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

      OsTask::delay(10);

#ifdef RTL_ENABLED
      RTL_WRITE("testShortCircuit.rtl");
#endif

   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MpInputOutputFrameworkTest);
