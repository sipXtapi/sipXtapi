//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <ptapi/PtMediaCapabilities.h>
#include <ptapi/PtAudioCodec.h>

/**
 * Unittest for PtMediaCapabilities
 */
class PtMediaCapabilitiesTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(PtMediaCapabilitiesTest);
    CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST(testManipulators);
    CPPUNIT_TEST(testAccessors);
    CPPUNIT_TEST_SUITE_END();


public:
    void testCreators()
    {
        PtMediaCapabilities*         pTempCapabilities;
        PtMediaCapabilities*         pTempCapabilities_1;
        PtAudioCodec*                        pTempAudioCodec;

        pTempAudioCodec = new PtAudioCodec[9];
        pTempCapabilities = new PtMediaCapabilities(pTempAudioCodec, 9);
        delete pTempCapabilities;
        delete[] pTempAudioCodec;
                                                                                
        pTempCapabilities = new PtMediaCapabilities();
        delete pTempCapabilities;
                                                                                
        pTempAudioCodec = new PtAudioCodec[9];
        pTempCapabilities = new PtMediaCapabilities(pTempAudioCodec, 9);
        pTempCapabilities_1 = new PtMediaCapabilities(*pTempCapabilities);
        delete pTempCapabilities_1;
        delete pTempCapabilities;
        delete[] pTempAudioCodec;
    }

    void testManipulators()
    {
        PtAudioCodec*                pTempAudioCodec;
        PtMediaCapabilities* pTempCapabilities;
        PtMediaCapabilities* pTempCapabilities_1;
                                                                                
        pTempAudioCodec = new PtAudioCodec[9];
        pTempCapabilities = new PtMediaCapabilities(pTempAudioCodec, 9);
        pTempCapabilities_1 = new PtMediaCapabilities();
        *pTempCapabilities_1 = *pTempCapabilities;
        delete pTempCapabilities;
        delete pTempCapabilities_1;
        delete[] pTempAudioCodec;
    }

    void testAccessors()
    {
        PtAudioCodec*                pTempAudioCodec;
        PtAudioCodec*                pTempAudioCodec_1;
        PtMediaCapabilities* pTempCapabilities;
                                                                                
        pTempAudioCodec = new PtAudioCodec[9];
        pTempCapabilities = new PtMediaCapabilities(pTempAudioCodec, 9);
        pTempCapabilities->getNumAudioCodecs();
        delete[] pTempAudioCodec;
        delete pTempCapabilities;
                                                                                
        // test getAudioCodec
        pTempAudioCodec_1 = new PtAudioCodec(PtAudioCodec::MU_LAW_8B_8K);
        pTempAudioCodec = new PtAudioCodec[9];
        pTempCapabilities = new PtMediaCapabilities(pTempAudioCodec, 9);
        pTempCapabilities->getAudioCodec(8, *pTempAudioCodec_1);
        delete pTempAudioCodec_1;
        delete[] pTempAudioCodec;
        delete pTempCapabilities;
                                                                                
        pTempAudioCodec_1 = new PtAudioCodec(PtAudioCodec::MU_LAW_8B_8K);
        pTempAudioCodec = new PtAudioCodec[9];
        pTempCapabilities = new PtMediaCapabilities(pTempAudioCodec, 9);
        pTempCapabilities->addAudioCodec(*pTempAudioCodec_1);
        pTempCapabilities->addAudioCodec(*pTempAudioCodec_1);
        delete pTempAudioCodec_1;
        delete[] pTempAudioCodec;
        delete pTempCapabilities;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PtMediaCapabilitiesTest);
