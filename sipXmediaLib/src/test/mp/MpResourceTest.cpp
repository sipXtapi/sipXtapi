//  
// Copyright (C) 2006-2010 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <os/OsIntTypes.h>

#include <sipxunittests.h>

#include <os/OsDefs.h>
#include <os/OsMsgQ.h>
#include <mp/MpResource.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MpTestResource.h>

/**
 * Unittest for MpResource and MpResourceTest
 */
class MpResourceTest : public SIPX_UNIT_BASE_CLASS
{
    CPPUNIT_TEST_SUITE(MpResourceTest);
    CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST(testGetFlowGraph);
    CPPUNIT_TEST(testEnableAndDisable);
    CPPUNIT_TEST(testDoProcessFrame);
    CPPUNIT_TEST(testSetVisitState);
    CPPUNIT_TEST(testMessageHandling);
    CPPUNIT_TEST(testInputOutputInfoAndCounts);
    CPPUNIT_TEST(testIsEnabled);
    CPPUNIT_TEST(testIsInputOutputConnectedDisconnected);
    CPPUNIT_TEST(testNewEnableAndDisable);
    CPPUNIT_TEST_SUITE_END();

public:
    void testCreators()
    {
        MpTestResource* pResource1 = NULL;
        MpTestResource* pResource2 = NULL;
        int             portIdx    = 0;

        // test the constructor
        pResource1 = new MpTestResource("Test", 0, 5, 1, 4);

        // verify that the initial state of the object is sensible
        CPPUNIT_ASSERT(pResource1->getName() == "Test");
        CPPUNIT_ASSERT(pResource1->getFlowGraph() == NULL);
        CPPUNIT_ASSERT_EQUAL(5, pResource1->maxInputs());
        CPPUNIT_ASSERT_EQUAL(4, pResource1->maxOutputs());
        CPPUNIT_ASSERT_EQUAL(0, pResource1->minInputs());
        CPPUNIT_ASSERT_EQUAL(1, pResource1->minOutputs());
        CPPUNIT_ASSERT_EQUAL(0, pResource1->numInputs());
        CPPUNIT_ASSERT_EQUAL(0, pResource1->numOutputs());
        CPPUNIT_ASSERT(pResource1->isEnabled() == FALSE);
        CPPUNIT_ASSERT(pResource1->isInputConnected(0) == FALSE);
        CPPUNIT_ASSERT(pResource1->isInputUnconnected(0) == TRUE);
        CPPUNIT_ASSERT(pResource1->isOutputConnected(0) == FALSE);
        CPPUNIT_ASSERT(pResource1->isOutputUnconnected(0) == TRUE);

        // check for sensible initial info for input and output ports
        pResource1->getInputInfo(0, (MpResource*&) pResource2, portIdx);
        CPPUNIT_ASSERT(pResource2 == NULL);

        pResource1->getOutputInfo(1, (MpResource*&) pResource2, portIdx);
        CPPUNIT_ASSERT(pResource2 == NULL);

        // test the destructor
        delete pResource1;
    }

    void testEnableAndDisable()
    {
        MpFlowGraphBase* pFlowGraph = NULL;
        MpTestResource*  pResource  = NULL;

        pFlowGraph = new MpFlowGraphBase(30, 30);
        pResource = new MpTestResource("Test", 0, 5, 1, 4);

        // verify that the resource starts out disabled
        CPPUNIT_ASSERT(pResource->isEnabled() == FALSE);

        // add the resource to the flow graph
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pFlowGraph->addResource(*pResource));
        // start flowgraph
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pFlowGraph->start());

        // verify that the isEnabled flag is getting passed properly via
        // doProcessFrame()
        CPPUNIT_ASSERT_EQUAL(0, pResource->numFramesProcessed());
        pFlowGraph->processNextFrame();
        CPPUNIT_ASSERT_EQUAL(1, pResource->numFramesProcessed());
        CPPUNIT_ASSERT_EQUAL(FALSE, pResource->mLastDoProcessArgs.isEnabled);

        // now call the enable() method for the resource
        pResource->enable();
        pFlowGraph->processNextFrame();
        CPPUNIT_ASSERT(pResource->isEnabled());
        CPPUNIT_ASSERT_EQUAL(2, pResource->numFramesProcessed());
        CPPUNIT_ASSERT_EQUAL(TRUE, pResource->mLastDoProcessArgs.isEnabled);

        // now call the disable() method for the resource
        pResource->disable();
        pFlowGraph->processNextFrame();
        CPPUNIT_ASSERT(pResource->isEnabled() == FALSE);
        CPPUNIT_ASSERT_EQUAL(3, pResource->numFramesProcessed());
        CPPUNIT_ASSERT_EQUAL(FALSE, pResource->mLastDoProcessArgs.isEnabled);

        // stop flowgraph
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pFlowGraph->stop());
        pFlowGraph->processNextFrame();
        // delete flowgraph and destroy all resources.
        delete pFlowGraph;
    }

    void testDoProcessFrame()
    {
        MpFlowGraphBase* pFlowGraph = NULL;
        MpTestResource*  pResource  = NULL;

        pFlowGraph = new MpFlowGraphBase(30, 30);
        pResource = new MpTestResource("Test", 0, 5, 1, 4);

        // add the resource to the flow graph
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pFlowGraph->addResource(*pResource));
        // start flowgraph
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pFlowGraph->start());

        // call processFrame() and then verify that doProcessFrame() is getting
        // invoked with sensible arguments
        pFlowGraph->processNextFrame();
        CPPUNIT_ASSERT_EQUAL(1, pResource->numFramesProcessed());
        CPPUNIT_ASSERT(pResource->mLastDoProcessArgs.inBufs != NULL);
        CPPUNIT_ASSERT(pResource->mLastDoProcessArgs.outBufs != NULL);
        CPPUNIT_ASSERT_EQUAL(pResource->mLastDoProcessArgs.inBufsSize, 
                             pResource->maxInputs());

        CPPUNIT_ASSERT_EQUAL(pResource->mLastDoProcessArgs.outBufsSize,
                             pResource->maxOutputs());

        CPPUNIT_ASSERT(pResource->mLastDoProcessArgs.isEnabled == FALSE);

        CPPUNIT_ASSERT_EQUAL(30, 
                             pResource->mLastDoProcessArgs.samplesPerFrame);

        CPPUNIT_ASSERT_EQUAL(30, 
                             pResource->mLastDoProcessArgs.samplesPerSecond);

        // stop flowgraph
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pFlowGraph->stop());
        pFlowGraph->processNextFrame();
        // delete flowgraph
        delete pFlowGraph;
    }

    void testSetVisitState()
    {
        MpTestResource* pResource = NULL;

        pResource = new MpTestResource("Test", 0, 5, 1, 4);
        
        pResource->setVisitState(MpResource::NOT_VISITED);
        CPPUNIT_ASSERT(MpResource::NOT_VISITED == pResource->getVisitState());
        
        pResource->setVisitState(MpResource::IN_PROGRESS);
        CPPUNIT_ASSERT(pResource->getVisitState() == MpResource::IN_PROGRESS);

        pResource->setVisitState(MpResource::FINISHED);
        CPPUNIT_ASSERT(pResource->getVisitState() == MpResource::FINISHED);
        
        delete pResource;
    }

    void testMessageHandling()
    {
        MpTestResource* pResource = NULL;

        pResource = new MpTestResource("Test", 0, 5, 1, 4);
        pResource->sendTestMessage((void*) 1, (void*) 2, 3, 4);
        CPPUNIT_ASSERT(pResource->mLastMsg.getMsg() ==
                       MpFlowGraphMsg::RESOURCE_SPECIFIC_START);
        CPPUNIT_ASSERT(pResource->mLastMsg.getMsgDest() == pResource);
        CPPUNIT_ASSERT(pResource->mLastMsg.getPtr1() == (void*) 1);
        CPPUNIT_ASSERT(pResource->mLastMsg.getPtr2() == (void*) 2);
        CPPUNIT_ASSERT_EQUAL((intptr_t)3, pResource->mLastMsg.getInt1());
        CPPUNIT_ASSERT_EQUAL((intptr_t)4, pResource->mLastMsg.getInt2());

        delete pResource;
    }

    void testGetFlowGraph()
    {
        MpFlowGraphBase* pFlowGraph  = NULL;
        MpTestResource*  pResource1  = NULL;
        OsStatus         res;

        pFlowGraph = new MpFlowGraphBase(30, 30);
        pResource1 = new MpTestResource("Test", 0, 5, 1, 4);

        // verify that initially, the resource is not associated with a flow graph
        CPPUNIT_ASSERT(pResource1->getFlowGraph() == NULL);

        // add the resource to the flow graph
        res = pFlowGraph->addResource(*pResource1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        CPPUNIT_ASSERT(pResource1->getFlowGraph() == pFlowGraph);

        // remove the resource from the flow graph
        res = pFlowGraph->removeResource(*pResource1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        CPPUNIT_ASSERT(pResource1->getFlowGraph() == NULL);

        delete pResource1;
        delete pFlowGraph;
    }

    void testInputOutputInfoAndCounts()
    {
        MpFlowGraphBase* pFlowGraph  = NULL;
        MpTestResource*  pResource1  = NULL;
        MpTestResource*  pResource2  = NULL;
        MpTestResource*  pDownstream = NULL;
        MpTestResource*  pUpstream   = NULL;
        int              downstreamPortIdx;
        int              upstreamPortIdx;
        OsStatus         res;

        pFlowGraph = new MpFlowGraphBase(30, 30);
        pResource1 = new MpTestResource("Test1", 0, 5, 1, 4);
        pResource2 = new MpTestResource("Test2", 0, 5, 1, 4);

        // Initially, there should be no input or output links
        CPPUNIT_ASSERT_EQUAL(0, pResource1->numInputs());
        CPPUNIT_ASSERT_EQUAL(0, pResource1->numOutputs());
        CPPUNIT_ASSERT_EQUAL(0, pResource2->numInputs());
        CPPUNIT_ASSERT_EQUAL(0, pResource2->numOutputs());
        res = pFlowGraph->addResource(*pResource1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pFlowGraph->addResource(*pResource2);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        // add links from
        //   pResource1, output port 0  ->  pResource2, input port 1
        //   pResource1, output port 3  ->  pResource2, input port 2
        res = pFlowGraph->addLink(*pResource1, 0, *pResource2, 1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pFlowGraph->addLink(*pResource1, 3, *pResource2, 2);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        // verify that the number of input and output links are correct
        CPPUNIT_ASSERT_EQUAL(0, pResource1->numInputs());
        CPPUNIT_ASSERT_EQUAL(2, pResource1->numOutputs());
        CPPUNIT_ASSERT_EQUAL(2, pResource2->numInputs());
        CPPUNIT_ASSERT_EQUAL(0, pResource2->numOutputs());

        // verify the input information for the downstream resource
        pResource2->getInputInfo(0, (MpResource*&) pUpstream, upstreamPortIdx);
        CPPUNIT_ASSERT(pUpstream == NULL);    // nothing should be connected on input 0

        pResource2->getInputInfo(1, (MpResource*&) pUpstream, upstreamPortIdx);
        CPPUNIT_ASSERT(pUpstream == pResource1);
        CPPUNIT_ASSERT_EQUAL(0, upstreamPortIdx);

        pResource2->getInputInfo(2, (MpResource*&) pUpstream, upstreamPortIdx);
        CPPUNIT_ASSERT(pUpstream == pResource1);
        CPPUNIT_ASSERT_EQUAL(3, upstreamPortIdx);

        // verify the output information for the upstream resource
        pResource1->getOutputInfo(1, (MpResource*&) pDownstream, downstreamPortIdx);
        CPPUNIT_ASSERT(pDownstream == NULL);  // nothing should be connected on output 1

        pResource1->getOutputInfo(0, (MpResource*&) pDownstream, downstreamPortIdx);
        CPPUNIT_ASSERT(pDownstream == pResource2);
        CPPUNIT_ASSERT_EQUAL(1, downstreamPortIdx);

        pResource1->getOutputInfo(3, (MpResource*&) pDownstream, downstreamPortIdx);
        CPPUNIT_ASSERT(pDownstream == pResource2);
        CPPUNIT_ASSERT_EQUAL(2, downstreamPortIdx);

        // remove the two links
        res = pFlowGraph->removeLink(*pResource1, 0);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
   
        res = pFlowGraph->removeLink(*pResource1, 3);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        // verify the input information for the downstream resource
        pResource2->getInputInfo(1, (MpResource*&) pUpstream, upstreamPortIdx);
        CPPUNIT_ASSERT(pUpstream == NULL);

        pResource2->getInputInfo(2, (MpResource*&) pUpstream, upstreamPortIdx);
        CPPUNIT_ASSERT(pUpstream == NULL);

        // verify the output information for the upstream resource
        pResource1->getOutputInfo(0, (MpResource*&) pDownstream, downstreamPortIdx);
        CPPUNIT_ASSERT(pDownstream == NULL);

        pResource1->getOutputInfo(3, (MpResource*&) pDownstream, downstreamPortIdx);
        CPPUNIT_ASSERT(pDownstream == NULL);

        // verify the number of input and output links are correct
        CPPUNIT_ASSERT_EQUAL(0, pResource1->numInputs());
        CPPUNIT_ASSERT_EQUAL(0, pResource1->numOutputs());
        CPPUNIT_ASSERT_EQUAL(0, pResource2->numInputs());
        CPPUNIT_ASSERT_EQUAL(0, pResource2->numOutputs());

        // remove the resources from the flow graph
        res = pFlowGraph->removeResource(*pResource1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pFlowGraph->removeResource(*pResource2);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        
        delete pResource1;
        delete pResource2;
        delete pFlowGraph;
    }

    void testIsEnabled()
    {
        MpTestResource*  pResource1  = NULL;

        pResource1 = new MpTestResource("Test", 0, 5, 1, 4);
        CPPUNIT_ASSERT(!pResource1->isEnabled());  // should be disabled initially
        
        pResource1->enable();
        CPPUNIT_ASSERT(pResource1->isEnabled());

        pResource1->enable();               // second enable() should be a no-op
        CPPUNIT_ASSERT(pResource1->isEnabled());

        pResource1->disable();
        CPPUNIT_ASSERT(! pResource1->isEnabled());
        
        pResource1->disable();              // second disable() should be a no-op
        CPPUNIT_ASSERT(! pResource1->isEnabled());
        
        delete pResource1;
    }


    void testIsInputOutputConnectedDisconnected()
    {
        MpFlowGraphBase* pFlowGraph  = NULL;
        MpTestResource*  pResource1  = NULL;
        MpTestResource*  pResource2  = NULL;
        OsStatus         res;

        pFlowGraph = new MpFlowGraphBase(30, 30);
        pResource1 = new MpTestResource("Test1", 0, 2, 0, 2);
        pResource2 = new MpTestResource("Test2", 0, 2, 0, 2);
        
        res = pFlowGraph->addResource(*pResource1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        
        res = pFlowGraph->addResource(*pResource2);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        // add link from
        //   pResource1, output port 1  ->  pResource2, input port 1
        res = pFlowGraph->addLink(*pResource1, 1, *pResource2, 1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        // verify the link state for the inputs and outputs
        CPPUNIT_ASSERT(!pResource1->isInputConnected(-1)); // check invalid ports
        CPPUNIT_ASSERT(! pResource1->isInputConnected(2));
        CPPUNIT_ASSERT(! pResource1->isInputUnconnected(-1));
        CPPUNIT_ASSERT(! pResource1->isInputUnconnected(2));

        CPPUNIT_ASSERT(! pResource1->isInputConnected(0));  // check valid parts that
        CPPUNIT_ASSERT(! pResource1->isInputConnected(1));  // are not connected
        CPPUNIT_ASSERT(! pResource1->isOutputConnected(0));
        CPPUNIT_ASSERT(  pResource1->isInputUnconnected(0));
        CPPUNIT_ASSERT(  pResource1->isInputUnconnected(1));
        CPPUNIT_ASSERT(  pResource1->isOutputUnconnected(0));
        CPPUNIT_ASSERT(! pResource2->isInputConnected(0));
        CPPUNIT_ASSERT(! pResource2->isOutputConnected(0));
        CPPUNIT_ASSERT(! pResource2->isOutputConnected(1));
        CPPUNIT_ASSERT(  pResource2->isInputUnconnected(0));
        CPPUNIT_ASSERT(  pResource2->isOutputUnconnected(0));
        CPPUNIT_ASSERT(  pResource2->isOutputUnconnected(1));

        CPPUNIT_ASSERT(  pResource1->isOutputConnected(1));  // check connected ports
        CPPUNIT_ASSERT(! pResource1->isOutputUnconnected(1));
        CPPUNIT_ASSERT(  pResource2->isInputConnected(1));
        CPPUNIT_ASSERT(! pResource2->isInputUnconnected(1));

        // remove the link and resources from the flow graph
        res = pFlowGraph->removeLink(*pResource1, 1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pFlowGraph->removeResource(*pResource1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        
        res = pFlowGraph->removeResource(*pResource2);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        delete pResource1;
        delete pResource2;
        delete pFlowGraph;
    }

    void testNewEnableAndDisable()
    {
       MpFlowGraphBase* pFlowGraph = NULL;
       MpTestResource*  pResource  = NULL;

       pFlowGraph = new MpFlowGraphBase(30, 30);
       pResource = new MpTestResource("test1", 0, 5, 1, 4);

       // add the resource to the flow graph
       CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pFlowGraph->addResource(*pResource));
       // start flowgraph
       CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pFlowGraph->start());

       // verify that the resource starts out disabled
       CPPUNIT_ASSERT(pResource->isEnabled() == FALSE);

       // verify that the isEnabled flag is getting passed properly via
       // doProcessFrame()
       CPPUNIT_ASSERT_EQUAL(0, pResource->numFramesProcessed());
       pFlowGraph->processNextFrame();
       CPPUNIT_ASSERT_EQUAL(1, pResource->numFramesProcessed());
       CPPUNIT_ASSERT(pResource->mLastDoProcessArgs.isEnabled == FALSE);

       // Create a queue..
       OsMsgQ msgQ;
       // Have the resource create an enable message and stick it on
       // the queue provided.
       MpResource::enable("test1", msgQ);
       // Then have it process the queue immediately, doing the enable.
       pResource->handleMessages(msgQ);

       // Check that it's enabled now.
       CPPUNIT_ASSERT(pResource->isEnabled());

       pFlowGraph->processNextFrame();
       CPPUNIT_ASSERT_EQUAL(2, pResource->numFramesProcessed());
       CPPUNIT_ASSERT(pResource->mLastDoProcessArgs.isEnabled);

       // Have the resource create a disable message and stick it on
       // the queue provided.
       MpResource::disable("test1", msgQ);
       // Then have it process the queue immediately, doing the disable.
       pResource->handleMessages(msgQ);
       CPPUNIT_ASSERT(pResource->isEnabled() == FALSE);

       pFlowGraph->processNextFrame();
       CPPUNIT_ASSERT_EQUAL(3, pResource->numFramesProcessed());
       CPPUNIT_ASSERT(pResource->mLastDoProcessArgs.isEnabled == FALSE);

       // stop flowgraph
       CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pFlowGraph->stop());
       pFlowGraph->processNextFrame();
       // delete flowgraph
       delete pFlowGraph;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MpResourceTest);
