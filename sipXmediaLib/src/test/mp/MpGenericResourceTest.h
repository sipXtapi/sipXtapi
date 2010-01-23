//  
// Copyright (C) 2006-2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifndef _GENERIC_RESOURCE_TEST_H_ // [
#define _GENERIC_RESOURCE_TEST_H_

#include <mp/MpMediaTask.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MpTestResource.h>
#include <mp/MpMisc.h>

#include <sipxunittests.h>

/// Number of frames in one frame
#define TEST_DEFAULT_SAMPLES_PER_FRAME 80
/// Number of frames in one second
#define TEST_DEFAULT_SAMPLES_PER_SEC 8000

/**
*  Generic framework for unit test of media resources.
* 
*  For each testing resource create subclass of this class.
*/
class MpGenericResourceTest : public SIPX_UNIT_BASE_CLASS
{
#if !defined(NO_CPPUNIT)
   CPPUNIT_TEST_SUITE(MpGenericResourceTest);
   CPPUNIT_TEST_SUITE_END();
#endif
public:

   MpGenericResourceTest();

   // Initialize test framework
   void setUp();

   // Clean up after test is done.
   void tearDown();

   /**
   *  Get the samples per frame.
   *  
   *  Use this in derived classes as opposed to directly accessing the member, 
   *  as test setup and shutdown may wish to wait until a new test is started 
   *  before a new value set using setSamplesPerFrame is used.
   */
   unsigned getSamplesPerFrame() const;

   /**
   *  Get the sample rate.
   *  
   *  Use this in derived classes as opposed to directly accessing the member, 
   *  as test setup and shutdown may wish to wait until a new test is started 
   *  before a new value set using setSamplesPerSec is used.
   */
   unsigned getSamplesPerSec() const;

   /**
   *  Set the samples per frame.
   *  
   *  @Note Make sure to put this call after you're finished testing all 
   *  resources in your test at the frame rate previously provided -- i.e. all 
   *  buffers at the old frame rate are flushed through the flowgraph.  The 
   *  flowgraph should do this for you, so this may be a non-issue.
   */
   void setSamplesPerFrame(const unsigned samplesPerFrame);

   /**
   *  Set the samples per sec.
   *  
   *  @Note Make sure to put this call after you're finished testing all 
   *  resources in your test at the frame rate previously provided -- i.e. all 
   *  buffers at the old frame rate are flushed through the flowgraph.  The 
   *  flowgraph should do this for you, so this may be a non-issue.
   */
   void setSamplesPerSec(const unsigned samplesPerSec);

protected:

   MpFlowGraphBase*  mpFlowGraph;
   MpTestResource*   mpSourceResource; ///< Outputs of this resource will be
                                       ///<  connected to inputs of tested
                                       ///<  resource.
   MpTestResource*   mpSinkResource;   ///< Inputs of this resource will be
                                       ///<  connected to outputs of tested
                                       ///<  resource.

   static unsigned sSampleRates[]; ///< Sample rates that can be used to test differing rates.
   static unsigned sNumRates;      ///< The number of static sample rates that are held in sSampleRates.

   /**
   *  @brief Setup generic resource testing framework.
   *
   *  <ol>
   *  <li> Create source and sink supply resources.
   *  <li> Add all resources to the flowgraph
   *  <li> Let source resource generate buffers and sink resource conceal them.
   *  <li> Link all resources: sourceResource -> testResource -> sinkResource.
   *  <li> Start flowgraph.
   *  <li> Enable sink resource.
   *  </ol>
   */
   void setupFramework(MpResource *pTestResource);

   /**
   *  @brief Halt generic resource testing framework.
   *
   *  <ol>
   *  <li> Stop flowgraph.
   *  <li> delete all resources contained in flowgraph.
   *  </ol>
   */
   void haltFramework();

   private:
      // Private data
      unsigned mSamplesPerSec;
      unsigned mSamplesPerFrame;
};

//CPPUNIT_TEST_SUITE_REGISTRATION(MpGenericResourceTest);

#endif // _GENERIC_RESOURCE_TEST_H_ ]
