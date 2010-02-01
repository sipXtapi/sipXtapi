//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <mp/MpTestResource.h>
#include <mp/MprSpeakerSelector.h>
#include <mp/MpSpeakerSelectBase.h>

#include "mp/MpGenericResourceTest.h"

#ifdef RTL_ENABLED // [
#  include "rtl_macro.h"
#else  // RTL_ENABLED ][
#  define RTL_WRITE(x)
#  define RTL_BLOCK(x)
#  define RTL_START(x)
#  define RTL_STOP
#endif // RTL_ENABLED ]

#define NUM_ACTIVE_SPEAKERS     2
#define NUM_STREAMS             10
#define NUM_OUTPUTS             2

class MpTestSpeakerSelection : public MpSpeakerSelectBase
{
public:
   MpTestSpeakerSelection()
   : mNumParticipants(0)
   , mEnabled(NULL)
   {}
   OsStatus init(int maxParticipants, int maxActive)
      {mNumParticipants = maxParticipants; mEnabled = new UtlBoolean[maxParticipants]; return OS_SUCCESS;};
   ~MpTestSpeakerSelection() {delete[] mEnabled;};
   void reset() {};
   OsStatus enableParticipant(int num, UtlBoolean newState)
      {mEnabled[num] = newState; return OS_SUCCESS;};
   OsStatus processFrame(MpSpeechParams* speechParams[], int frameSize)
      {
         for (int i=0; i<mNumParticipants; i++)
         {
            if (speechParams[i] != NULL)
            {
               if (isActiveAudio(speechParams[i]->mSpeechType))
               {
                  speechParams[i]->mSpeakerRank = mNumParticipants-i;
               }
               else
               {
                  speechParams[i]->mSpeakerRank = UINT_MAX;
               }
            }
         }
         return OS_SUCCESS;
      };
   OsStatus setParam(const char* paramName, void* value) {return OS_SUCCESS;};
   OsStatus isParticipantEnabled(int num, UtlBoolean &enabled)
      {enabled = mEnabled[num]; return OS_SUCCESS;};

   int mNumParticipants;
   UtlBoolean *mEnabled;
};

///  Unit test for MprSpeakerSelector
class MprSpeakerSelectorTest : public MpGenericResourceTest
{
    CPPUNIT_TEST_SUB_SUITE(MprSpeakerSelectorTest, MpGenericResourceTest);
    CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST(testDisabled);
    CPPUNIT_TEST(testEnabledNoData);
    CPPUNIT_TEST(testEnabledWithData);
    CPPUNIT_TEST_SUITE_END();

public:

   void testCreators()
   {
       MprSpeakerSelector*     pSpeakerSelector    = NULL;
       MpTestSpeakerSelection* pSS = new MpTestSpeakerSelection();
       OsStatus                res;

       // when we have a flow graph that contains resources and links,
       // verify that destroying the flow graph also gets rid of the resources
       // and links.
       pSpeakerSelector = new MprSpeakerSelector("MprSpeakerSelector",
                                                 NUM_STREAMS,
                                                 NUM_OUTPUTS,
                                                 NUM_ACTIVE_SPEAKERS,
                                                 pSS);

       res = mpFlowGraph->addResource(*pSpeakerSelector);
       CPPUNIT_ASSERT(res == OS_SUCCESS);
   }

   void testDisabled()
   {
       MprSpeakerSelector*     pSpeakerSelector    = NULL;
       MpTestSpeakerSelection* pSS = new MpTestSpeakerSelection();
       OsStatus                res;
       int                     i;

       pSpeakerSelector = new MprSpeakerSelector("MprSpeakerSelector",
                                                 NUM_STREAMS,
                                                 NUM_OUTPUTS,
                                                 NUM_ACTIVE_SPEAKERS,
                                                 pSS);
       CPPUNIT_ASSERT(pSpeakerSelector != NULL);

       setupFramework(pSpeakerSelector);

       // pSpeakerSelector disabled, there are buffers on all inputs.
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pSpeakerSelector->disable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Data should be simply forwarded.
       for (i=0; i<pSpeakerSelector->maxOutputs(); i++)
          CPPUNIT_ASSERT(  mpSinkResource->mLastDoProcessArgs.inBufs[i]
                        == mpSourceResource->mLastDoProcessArgs.outBufs[i]);

       // Stop flowgraph
       haltFramework();
   }

   void testEnabledNoData()
   {
       MprSpeakerSelector*     pSpeakerSelector    = NULL;
       MpTestSpeakerSelection* pSS = new MpTestSpeakerSelection();
       OsStatus                res;

       pSpeakerSelector = new MprSpeakerSelector("MprSpeakerSelector",
                                                 NUM_STREAMS,
                                                 NUM_OUTPUTS,
                                                 NUM_ACTIVE_SPEAKERS,
                                                 pSS);
       CPPUNIT_ASSERT(pSpeakerSelector != NULL);

       setupFramework(pSpeakerSelector);

       // pSpeakerSelector enabled, there are no buffers on the input 0
       CPPUNIT_ASSERT(mpSourceResource->disable());
       CPPUNIT_ASSERT(pSpeakerSelector->enable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // We did not generated any buffers on remote parts
       for (int i=0; i<pSpeakerSelector->maxOutputs(); i++)
       {
          CPPUNIT_ASSERT(!mpSinkResource->mLastDoProcessArgs.inBufs[i].isValid());
       }

       // Stop flowgraph
       haltFramework();
   }

   // Test helper - perform test with M given active speakers, other inputs
   // are NULL.
   void testWithNullInputs(int numInputs)
   {
       int stream;
       OsStatus res;

       // Tell source resource to generate buffers on inputs.
       mpSourceResource->setGenOutBufMask((1<<numInputs)-1);
       for (stream=0; stream<numInputs; stream++)
       {
          mpSourceResource->setSpeechType(stream, MP_SPEECH_ACTIVE);
       }

       // Process frame.
       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Check outputs.
       int numActiveOutputs=0;
       for (stream=0;
            stream<NUM_STREAMS && stream<numInputs && numActiveOutputs<NUM_OUTPUTS;
            stream++)
       {
          UtlBoolean isFound=FALSE;
          for (int outPort=0; outPort<NUM_OUTPUTS; outPort++)
          {
             if (  mpSinkResource->mLastDoProcessArgs.inBufs[outPort]
                == mpSourceResource->mLastDoProcessArgs.outBufs[numInputs-1-stream])
             {
                isFound = TRUE;
                break;
             }
          }
          CPPUNIT_ASSERT(isFound);
          numActiveOutputs++;
       }
       if (numInputs < NUM_ACTIVE_SPEAKERS)
       {
          CPPUNIT_ASSERT_EQUAL(numInputs, numActiveOutputs);
       }
       else
       {
          CPPUNIT_ASSERT_EQUAL(NUM_ACTIVE_SPEAKERS, numActiveOutputs);
       }
   }

   // Test helper - perform test with M given active speakers, other inputs
   // are silent.
   void testWithSilentInputs(int numInputs)
   {
       int stream;
       OsStatus res;

       // Tell source resource to generate buffers on inputs.
       mpSourceResource->setGenOutBufMask((1<<NUM_STREAMS)-1);
       for (stream=0; stream<numInputs; stream++)
       {
          mpSourceResource->setSpeechType(stream, MP_SPEECH_ACTIVE);
       }
       for (; stream<NUM_STREAMS; stream++)
       {
          mpSourceResource->setSpeechType(stream, MP_SPEECH_SILENT);
       }

       // Process frame.
       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Check outputs.
       int numActiveOutputs=0;
       for (stream=0;
            stream<NUM_STREAMS && stream<numInputs && numActiveOutputs<NUM_OUTPUTS;
            stream++)
       {
          UtlBoolean isFound=FALSE;
          for (int outPort=0; outPort<NUM_OUTPUTS; outPort++)
          {
             if (  mpSinkResource->mLastDoProcessArgs.inBufs[outPort]
                == mpSourceResource->mLastDoProcessArgs.outBufs[numInputs-1-stream])
             {
                isFound = TRUE;
                break;
             }
          }
          CPPUNIT_ASSERT(isFound);
          numActiveOutputs++;
       }
       if (numInputs < NUM_ACTIVE_SPEAKERS)
       {
          CPPUNIT_ASSERT_EQUAL(numInputs, numActiveOutputs);
       }
       else
       {
          CPPUNIT_ASSERT_EQUAL(NUM_ACTIVE_SPEAKERS, numActiveOutputs);
       }
   }

   void testEnabledWithData()
   {
       MprSpeakerSelector*     pSpeakerSelector    = NULL;
       MpTestSpeakerSelection* pSS = new MpTestSpeakerSelection();

       pSpeakerSelector = new MprSpeakerSelector("MprSpeakerSelector",
                                                 NUM_STREAMS,
                                                 NUM_OUTPUTS,
                                                 NUM_ACTIVE_SPEAKERS,
                                                 pSS);
       CPPUNIT_ASSERT(pSpeakerSelector != NULL);

       setupFramework(pSpeakerSelector);

       // pSpeakerSelector enabled, there are buffers on the input 0
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pSpeakerSelector->enable());

       // Run tests for all possible number of inputs.
       for (int i=1; i<=NUM_STREAMS; i++)
       {
          testWithNullInputs(i);
          testWithSilentInputs(i);
       }

       // Stop flowgraph
       haltFramework();
   }
};


CPPUNIT_TEST_SUITE_REGISTRATION(MprSpeakerSelectorTest);


