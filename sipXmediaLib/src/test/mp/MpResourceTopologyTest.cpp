//  
// Copyright (C) 2006-2010 SIPfoundry Inc.  All rights reserved.
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#include <os/OsIntTypes.h>

#include <sipxunittests.h>

#include <mp/MpResourceTopology.h>
#include <mp/MpResourceFactory.h>
#include <mp/MprNullConstructor.h>

///  Unit test for MprSplitter
class MpResourceTopologyTest : public SIPX_UNIT_BASE_CLASS
{
    CPPUNIT_TEST_SUITE(MpResourceTopologyTest);
    CPPUNIT_TEST(testValidators);
    CPPUNIT_TEST_SUITE_END();

public:


    void testValidators()
    {
        MpResourceTopology topology;
        MpResourceFactory resourceFactory;
        int firstInvalidResourceIndex;

        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, topology.validateResourceTypes(resourceFactory, firstInvalidResourceIndex));
        CPPUNIT_ASSERT_EQUAL(-1, firstInvalidResourceIndex);

        UtlString firstUnconnectedResourceName;
        UtlString firstDanglingResourceName;
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, topology.validateConnections(firstUnconnectedResourceName,
                                   firstDanglingResourceName,
                                   FALSE));  // full topology, disallow external resource references
        CPPUNIT_ASSERT(firstUnconnectedResourceName.isNull());
        CPPUNIT_ASSERT(firstDanglingResourceName.isNull());

        topology.addResource(DEFAULT_NULL_RESOURCE_TYPE, "Null1");
        topology.addConnection("Null1", 1, "Null1", 1);

        // The factory does not have a constructor for the null resource,
        // so the resource should be considered invalid
        CPPUNIT_ASSERT_EQUAL(OS_NOT_FOUND, topology.validateResourceTypes(resourceFactory, firstInvalidResourceIndex));
        CPPUNIT_ASSERT_EQUAL(0, firstInvalidResourceIndex);

        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, topology.validateConnections(firstUnconnectedResourceName,
                                   firstDanglingResourceName,
                                   FALSE));  // full topology, disallow external resource references
        CPPUNIT_ASSERT(firstUnconnectedResourceName.isNull());
        CPPUNIT_ASSERT(firstDanglingResourceName.isNull());

        // Null resource
        resourceFactory.addConstructor(*(new MprNullConstructor()));

        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, topology.validateResourceTypes(resourceFactory, firstInvalidResourceIndex));
        CPPUNIT_ASSERT_EQUAL(-1, firstInvalidResourceIndex);

        // Add an unconnected resource
        topology.addResource(DEFAULT_NULL_RESOURCE_TYPE, "Null2");

        CPPUNIT_ASSERT_EQUAL(OS_INVALID, topology.validateConnections(firstUnconnectedResourceName,
                                   firstDanglingResourceName,
                                   FALSE));  // full topology, disallow external resource references
        CPPUNIT_ASSERT(firstUnconnectedResourceName.compareTo("Null2") == 0);
        CPPUNIT_ASSERT(firstDanglingResourceName.isNull());


        // Add a connection to no where for the unconnected resource 
        topology.addConnection("Null2", 1, "Null3", 1);

        CPPUNIT_ASSERT_EQUAL(OS_NOT_FOUND, topology.validateConnections(firstUnconnectedResourceName,
                                   firstDanglingResourceName,
                                   FALSE));  // full topology, disallow external resource references
        CPPUNIT_ASSERT(firstUnconnectedResourceName.isNull());
        CPPUNIT_ASSERT(firstDanglingResourceName.compareTo("Null2") == 0);

        // Allow external references to resources not defined in this topology
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, topology.validateConnections(firstUnconnectedResourceName,
                                   firstDanglingResourceName,
                                   TRUE));  // incremental topology, allow external resource references
        CPPUNIT_ASSERT(firstUnconnectedResourceName.isNull());
        CPPUNIT_ASSERT(firstDanglingResourceName.isNull());

        // Connect it all together and make it a full stand along topology
        topology.addConnection("Null1", 2, "Null2", 1);
        topology.addResource(DEFAULT_NULL_RESOURCE_TYPE, "Null3");
        topology.addConnection("Null2", 2, "Null3", 1);

        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, topology.validateConnections(firstUnconnectedResourceName,
                                   firstDanglingResourceName,
                                   FALSE));  // full topology, disallow external resource references
        CPPUNIT_ASSERT(firstUnconnectedResourceName.isNull());
        CPPUNIT_ASSERT(firstDanglingResourceName.isNull());

    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MpResourceTopologyTest);
