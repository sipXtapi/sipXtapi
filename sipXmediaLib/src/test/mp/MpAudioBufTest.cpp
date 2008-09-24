//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <mp/MpAudioBuf.h>

#define AUDIOBUF_TEST_SAMPLES_PER_FRAME 80
#define BUFFER_NUM    5

/**
 * Unittest for MpAudioBuf
 */
class MpAudioBufTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(MpAudioBufTest);
    CPPUNIT_TEST(testSampleCompare);
    CPPUNIT_TEST_SUITE_END();


public:

    void setUp()
    {
        // Create pool for data buffers
        mpPool = 
            new MpBufPool((AUDIOBUF_TEST_SAMPLES_PER_FRAME * sizeof(MpAudioSample))
                             + MpArrayBuf::getHeaderSize(), 
                          BUFFER_NUM);
        CPPUNIT_ASSERT(mpPool != NULL);

        // Create pool for buffer headers
        mpHeadersPool = new MpBufPool(sizeof(MpAudioBuf), BUFFER_NUM);
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
    }

    void testSampleCompare()
    {
        MpAudioBufPtr frame1 = mpPool->getBuffer();
        MpAudioBufPtr frame2 = mpPool->getBuffer();
        CPPUNIT_ASSERT(frame1.isValid());
        CPPUNIT_ASSERT(frame2.isValid());
        MpAudioSample* samples1 = frame1->getSamplesWritePtr();
        MpAudioSample* samples2 = frame2->getSamplesWritePtr();
        CPPUNIT_ASSERT_EQUAL(frame1->getSamplesNumber(), (unsigned int)AUDIOBUF_TEST_SAMPLES_PER_FRAME);
        CPPUNIT_ASSERT_EQUAL(frame2->getSamplesNumber(), (unsigned int)AUDIOBUF_TEST_SAMPLES_PER_FRAME);
        frame1->setSamplesNumber(AUDIOBUF_TEST_SAMPLES_PER_FRAME);
        frame2->setSamplesNumber(AUDIOBUF_TEST_SAMPLES_PER_FRAME);

        short positiveNegative;
        short sampleIndex;
        for(sampleIndex = 0; sampleIndex < AUDIOBUF_TEST_SAMPLES_PER_FRAME; sampleIndex++)
        {
            if(sampleIndex % 2)
            {
                positiveNegative = -1;
            }
            else
            {
                positiveNegative = 1;
            }
            samples1[sampleIndex] = sampleIndex * positiveNegative;
            samples2[sampleIndex] = sampleIndex * positiveNegative;
        }

        // The frames are identical so tolerance should not make any difference

        // Zero tolerance
        CPPUNIT_ASSERT_EQUAL(frame1.compareSamples(frame2, 0), 0); 

        // Tolerance = 2
        CPPUNIT_ASSERT_EQUAL(frame1.compareSamples(frame2, 2), 0);

        // Tweek the samples so there is some difference
        samples1[0] +=3;
        samples1[7] +=3;
        samples1[20] +=3;
        samples1[42] +=3;

        // Difference greater than tolerance (differ)
        CPPUNIT_ASSERT_EQUAL(frame1.compareSamples(frame2, 2), 1);

        // Difference equal than tolerance (the same)
        CPPUNIT_ASSERT_EQUAL(frame1.compareSamples(frame2, 3), 0);

        // Difference less than tolerance (the same)
        CPPUNIT_ASSERT_EQUAL(frame1.compareSamples(frame2, 5), 0);
    }



protected:
    MpBufPool *mpPool;         ///< Pool for data buffers
    MpBufPool *mpHeadersPool;  ///< Pool for buffers headers
};

CPPUNIT_TEST_SUITE_REGISTRATION(MpAudioBufTest);
