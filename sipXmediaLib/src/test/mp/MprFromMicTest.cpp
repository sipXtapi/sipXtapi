//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#include <mp/MpMediaTask.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MpTestResource.h>
#include <mp/MpMisc.h>
#include <mp/MprFromMic.h>
#include <mp/MpBufferMsg.h>

#include "mp/MpGenericResourceTest.h"

///  Unit test for MprFromMic
class MprFromMicTest : public MpGenericResourceTest
{
    CPPUNIT_TEST_SUB_SUITE(MprFromMicTest, MpGenericResourceTest);
    CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST(testDisabled);
    CPPUNIT_TEST(testEnabledNoQueue);
    CPPUNIT_TEST(testEnabledEmptyQueue);
    CPPUNIT_TEST(testEnabledWithData);
    CPPUNIT_TEST(testWideband);
    CPPUNIT_TEST_SUITE_END();

/// Length of message queue used to communicate with MprToSpkr
#define MSG_Q_LEN            1

public:

   void testCreators()
   {
       MprFromMic*       pFromMic = NULL;
       OsStatus          res;

       // when we have a flow graph that contains resources and links,
       // verify that destroying the flow graph also gets rid of the resources
       // and links.
       pFromMic = new MprFromMic("MprFromMic",
                                 NULL);

       res = mpFlowGraph->addResource(*pFromMic);
       CPPUNIT_ASSERT(res == OS_SUCCESS);
   }

   void testDisabled()
   {
       MprFromMic*       pFromMic   = NULL;
       OsStatus          res;

       pFromMic = new MprFromMic("MprFromMic",
                                 NULL);
       CPPUNIT_ASSERT(pFromMic != NULL);

       setupFramework(pFromMic);

       // TESTCASE 1:
       // pFromMic disabled, there are no buffers on the input 0.
       CPPUNIT_ASSERT(mpSourceResource->disable());
       CPPUNIT_ASSERT(pFromMic->disable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // We did not generated any buffers
       CPPUNIT_ASSERT(  !mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                     && !mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid());

       // TESTCASE 2:
       // pFromMic disabled, there are buffers on the input 0.
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pFromMic->disable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Buffer should be passed through
       CPPUNIT_ASSERT(  mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                     && (  mpSourceResource->mLastDoProcessArgs.outBufs[0]
                        == mpSinkResource->mLastDoProcessArgs.inBufs[0]));

       // Stop flowgraph
       haltFramework();
   }

   void testEnabledNoQueue()
   {
       MprFromMic*       pFromMic   = NULL;
       OsStatus          res;

       pFromMic = new MprFromMic("MprFromMic",
                                 NULL);
       CPPUNIT_ASSERT(pFromMic != NULL);

       setupFramework(pFromMic);

       // pFromMic enabled, there are no buffers on the input 0.
       CPPUNIT_ASSERT(mpSourceResource->disable());
       CPPUNIT_ASSERT(pFromMic->enable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // We did not generated any buffers
       CPPUNIT_ASSERT(  !mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                     && !mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid());

       // pFromMic enabled, there are buffers on the input 0.
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pFromMic->enable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // We should discard incoming packet
       CPPUNIT_ASSERT(  mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                     && !mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid());

       // Stop flowgraph
       haltFramework();
   }

   void testEnabledEmptyQueue()
   {
       MprFromMic*       pFromMic   = NULL;
       OsMsgQ*           pMsgQ      = NULL;
       MpAudioBufPtr     pBuf;
       OsStatus          res;

       // Create message queue to send data to MprFromMic
       pMsgQ = new OsMsgQ(MSG_Q_LEN);
       CPPUNIT_ASSERT(pMsgQ != NULL);

       pFromMic = new MprFromMic("MprFromMic",
                                 pMsgQ);
       CPPUNIT_ASSERT(pFromMic != NULL);

       setupFramework(pFromMic);

       // TESTCASE 1:
       // pFromMic enabled, there are no buffers on the input 0, message queue
       // is empty.
       CPPUNIT_ASSERT(mpSourceResource->disable());
       CPPUNIT_ASSERT(pFromMic->enable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // No buffers processed
       CPPUNIT_ASSERT(  !mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                     && !mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid());

       // TESTCASE 2:
       // pFromMic enabled, there are buffers on the input 0, message queue
       // is empty.
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pFromMic->enable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Buffer is replaced with empty buffer
       CPPUNIT_ASSERT(mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                     && !mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid());

       // Stop flowgraph
       haltFramework();

       // Free message queue
       delete pMsgQ;
   }

   void testEnabledWithData()
   {
       MprFromMic*       pFromMic   = NULL;
       OsMsgQ*           pMsgQ      = NULL;
       MpBufferMsg*      pMsg       = NULL;
       MpAudioBufPtr     pBuf;
       OsStatus          res;

       // Create message queue to send data to MprFromMic
       pMsgQ = new OsMsgQ(MSG_Q_LEN);
       CPPUNIT_ASSERT(pMsgQ != NULL);

       pFromMic = new MprFromMic("MprFromMic",
                                 pMsgQ);
       CPPUNIT_ASSERT(pFromMic != NULL);

       setupFramework(pFromMic);

       // TESTCASE 1:
       // pFromMic enabled, there are no buffers on the input 0, message queue
       // have one message.
       CPPUNIT_ASSERT(mpSourceResource->disable());
       CPPUNIT_ASSERT(pFromMic->enable());

       // Create message for message queue
       pMsg = new MpBufferMsg(MpBufferMsg::AUD_RECORDED);
       CPPUNIT_ASSERT(pMsg != NULL);
       pMsg->setMsgSubType(MpBufferMsg::AUD_RECORDED);

       // Fill message with buffer. Set audio type to muted.
       pBuf = MpMisc.RawAudioPool->getBuffer();
       CPPUNIT_ASSERT(pBuf.isValid());
       memset(pBuf->getSamplesWritePtr(), 0,
              pBuf->getSamplesNumber()*sizeof(MpAudioSample));
       pBuf->setSpeechType(MpAudioBuf::MP_SPEECH_UNKNOWN);
       pMsg->setBuffer(pBuf);

       // Send message to queue. MprFromMic should receive it.
       res = pMsgQ->send(*pMsg, OsTime::OS_INFINITY);
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Buffer is read from queue
       CPPUNIT_ASSERT(mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid());
       CPPUNIT_ASSERT(pMsgQ->isEmpty());

       // Free message and buffer
       if (!pMsg->isMsgReusable())
          delete pMsg;
       pBuf.release();

       // TESTCASE 2:
       // pFromMic enabled, there are buffers on the input 0, message queue
       // have one message.
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pFromMic->enable());

       // Create message for message queue
       pMsg = new MpBufferMsg(MpBufferMsg::AUD_RECORDED);
       CPPUNIT_ASSERT(pMsg != NULL);
       pMsg->setMsgSubType(MpBufferMsg::AUD_RECORDED);

       // Fill message with buffer. Set audio type to muted.
       pBuf = MpMisc.RawAudioPool->getBuffer();
       CPPUNIT_ASSERT(pBuf.isValid());
       memset(pBuf->getSamplesWritePtr(), 0,
              pBuf->getSamplesNumber()*sizeof(MpAudioSample));
       pBuf->setSpeechType(MpAudioBuf::MP_SPEECH_UNKNOWN);
       pMsg->setBuffer(pBuf);

       // Send message to queue. MprFromMic should receive it.
       res = pMsgQ->send(*pMsg, OsTime::OS_INFINITY);
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Buffer is read from queue
       CPPUNIT_ASSERT(mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid());
       CPPUNIT_ASSERT(pMsgQ->isEmpty());

       // Free message and buffer
       if (!pMsg->isMsgReusable())
          delete pMsg;
       pBuf.release();

       // Stop flowgraph
       haltFramework();

       // Free message queue
       delete pMsgQ;
   }
   void testWideband()
   {
      MprFromMic* pFromMic = NULL;
      MpBufPtr pBuf;
      OsStatus res;

      // Create message queue to send data to MprFromMic
      OsMsgQ* pMsgQ = new OsMsgQ(MSG_Q_LEN);
      CPPUNIT_ASSERT(pMsgQ != NULL);

      size_t i;
      for(i = 0; i < sNumRates; i++)
         //for(i = 0; i < 1; i++)
      {
         printf("Test %d Hz\n", sSampleRates[i]);
         // For this test, we want to modify the sample rate and samples per frame
         // so we need to de-inititialize what has already been initialized for us
         // by cppunit, or by a previous loop.
         tearDown();

         // Set the sample rates 
         setSamplesPerSec(sSampleRates[i]);
         setSamplesPerFrame(sSampleRates[i]/100);
         setUp();


         pFromMic = new MprFromMic("MprFromMic", 
                                   pMsgQ);
         CPPUNIT_ASSERT(pFromMic != NULL);

         setupFramework(pFromMic);

         // pFromMic enabled, there are buffers on the input 0
         CPPUNIT_ASSERT(mpSourceResource->enable());
         CPPUNIT_ASSERT(pFromMic->enable());

         res = mpFlowGraph->processNextFrame();
         CPPUNIT_ASSERT(res == OS_SUCCESS);

         // Store output buffer for convenience
         pBuf = mpSinkResource->mLastDoProcessArgs.inBufs[0];

         CPPUNIT_ASSERT(pBuf.isValid());
         // New buffer should be generated.
         CPPUNIT_ASSERT(   (mpSourceResource->mLastDoProcessArgs.outBufs[0] != pBuf)
                        && (mpSourceResource->mLastDoProcessArgs.outBufs[1] != pBuf)
                       );

         // And it should contain the same number of samples going out as coming
         // in, and should match the expected number of samples per frame.
         MpAudioBufPtr pMixedOutAudioBuf = pBuf;
         MpAudioBufPtr pFromMicInAudioBuf0 = mpSourceResource->mLastDoProcessArgs.outBufs[0];
         MpAudioBufPtr pFromMicInAudioBuf1 = mpSourceResource->mLastDoProcessArgs.outBufs[1];

         // Check to see that the number of samples in the mixer output frame
         // is equal to the samples per frame that we set during this run.
         CPPUNIT_ASSERT_EQUAL(getSamplesPerFrame(), 
            pMixedOutAudioBuf->getSamplesNumber());
         // Then check to see that the number of samples in the output buffer
         // matches the number of samples in each of the inputs to the mixer
         CPPUNIT_ASSERT_EQUAL(pFromMicInAudioBuf0->getSamplesNumber(), 
            pMixedOutAudioBuf->getSamplesNumber());
         CPPUNIT_ASSERT_EQUAL(pFromMicInAudioBuf1->getSamplesNumber(), 
            pMixedOutAudioBuf->getSamplesNumber());


         // Free stored buffer
         pBuf.release();

         // Stop flowgraph
         haltFramework();

         // No need to delete mixer, as haltFramework deletes all resources
         // in the flowgraph.
      }
      
      // Clean up the mic message queue.
      delete(pMsgQ);
      pMsgQ = NULL;
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MprFromMicTest);
