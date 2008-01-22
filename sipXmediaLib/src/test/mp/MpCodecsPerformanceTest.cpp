//  
// Copyright (C) 2006-2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#include <mp/MpCodecFactory.h>
#include <os/OsTime.h>
#include <os/OsDateTime.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

/// Sample rate in samples per second.
#define SAMPLE_RATE             8000
/// Size of audio frame in samples.
#define FRAME_SIZE              80
/// Maximum length of audio data we expect from decoder in samples.
#define DECODED_FRAME_MAX_SIZE  (FRAME_SIZE*6)
/// Maximum size of encoded frame in bytes.
#define ENCODED_FRAME_MAX_SIZE  (FRAME_SIZE*sizeof(MpAudioSample))
/// Number of RTP packets to encode/decode.
#define NUM_PACKETS_TO_TEST      3

///  Unit test for testing performance of supported codecs.
class MpCodecsPerformanceTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(MpCodecsPerformanceTest);
   CPPUNIT_TEST(testCodecsPreformance);
   CPPUNIT_TEST_SUITE_END();

public:

   void setUp()
   {
      // Create pool for data buffers
      mpPool = new MpBufPool(ENCODED_FRAME_MAX_SIZE + MpArrayBuf::getHeaderSize(), 1);
      CPPUNIT_ASSERT(mpPool != NULL);

      // Create pool for buffer headers
      mpHeadersPool = new MpBufPool(sizeof(MpRtpBuf), 1);
      CPPUNIT_ASSERT(mpHeadersPool != NULL);

      // Set mpHeadersPool as default pool for audio and data pools.
      MpRtpBuf::smpDefaultPool = mpHeadersPool;
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

   void testCodecsPreformance()
   {
      MpCodecFactory  *pCodecFactory;
      const UtlString *pCodecMimeTypes;
      unsigned         codecMimeTypesNum;

      // Get/create codec factory
      pCodecFactory = MpCodecFactory::getMpCodecFactory();
      CPPUNIT_ASSERT(pCodecFactory != NULL);

      // Load all available codecs
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           pCodecFactory->loadAllDynCodecs(".", CODEC_PLUGINS_FILTER));

      pCodecFactory->getMimeTypes(codecMimeTypesNum, (const UtlString*&)pCodecMimeTypes);
      CPPUNIT_ASSERT(codecMimeTypesNum>0);

      for (unsigned i=0; i<codecMimeTypesNum; i++)
      {
         const char **pCodecFmtps;
         unsigned     codecFmtpsNum;
         pCodecFactory->getCodecFmtps(pCodecMimeTypes[i], codecFmtpsNum, pCodecFmtps);
         if (codecFmtpsNum == 0)
         {
            testOneCodecPreformance(pCodecFactory, pCodecMimeTypes[i], "");
         } 
         else
         {
            for (int fmtpNum=0; fmtpNum<codecFmtpsNum; fmtpNum++)
            {
               testOneCodecPreformance(pCodecFactory,
                                       pCodecMimeTypes[i],
                                       pCodecFmtps[fmtpNum]);
            }
         }
      }
   }

protected:
   MpBufPool *mpPool;         ///< Pool for data buffers
   MpBufPool *mpHeadersPool;  ///< Pool for buffers headers

   void testOneCodecPreformance(MpCodecFactory *pCodecFactory,
                                const UtlString &codecMime,
                                const UtlString &codecFmtp,
                                int sampleRate,
                                int numChannels)
   {
      MpDecoderBase *pDecoder;
      MpEncoderBase *pEncoder;
      MpAudioSample  pOriginal[FRAME_SIZE];
      MpAudioSample  pDecoded[DECODED_FRAME_MAX_SIZE];
      int            encodeFrameNum = 0;

      // Create and initialize decoder and encoder
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           pCodecFactory->createDecoder(codecMime, codecFmtp,
                                                        sampleRate, numChannels,
                                                        0, pDecoder));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           pDecoder->initDecode());
      // Could not test speed of signaling codec
      if (pDecoder->getInfo()->isSignalingCodec())
      {
         pDecoder->freeDecode();
         delete pDecoder;
         return;
      }
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           pCodecFactory->createEncoder(codecMime, codecFmtp, 0, pEncoder));
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           pEncoder->initEncode());

      for (int i=0; i<NUM_PACKETS_TO_TEST; i++)
      {
         int            maxBytesPerPacket = (pEncoder->getInfo()->getMaxPacketBits() + 7) / 8;
         int            tmpSamplesConsumed;
         int            tmpEncodedSize;
         UtlBoolean     tmpSendNow;
         MpAudioBuf::SpeechType tmpSpeechType;
         MpRtpBufPtr    pRtpPacket = mpPool->getBuffer();
         unsigned char *pRtpPayloadPtr = (unsigned char *)pRtpPacket->getDataWritePtr();
         int            payloadSize = 0;
         int            samplesInPacket = 0;
         OsTime         start;
         OsTime         stop;
         OsTime         diff;

         // Encode frames until we get tmpSendNow set or reach packet size limit.
         do 
         {
            // Encode one frame and measure time it took.
            OsStatus result;
            OsDateTime::getCurTime(start);
            result = pEncoder->encode(pOriginal, FRAME_SIZE, tmpSamplesConsumed,
                                      pRtpPayloadPtr,
                                      ENCODED_FRAME_MAX_SIZE-payloadSize,
                                      tmpEncodedSize, tmpSendNow,
                                      tmpSpeechType);
            OsDateTime::getCurTime(stop);
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, result);
            
            // Print timing in TSV format
            diff = stop - start;
            printf("encode %s %s;%d;%ld.%06ld;%ld.%06ld\n",
                   codecMime.data(), codecFmtp.data(),
                   encodeFrameNum,
                   start.seconds(), start.usecs(),
                   diff.seconds(), diff.usecs());

            // Adjust encoding state
            payloadSize += tmpEncodedSize;
            pRtpPayloadPtr += tmpEncodedSize;
            samplesInPacket += tmpSamplesConsumed;
            CPPUNIT_ASSERT(payloadSize <= ENCODED_FRAME_MAX_SIZE);
            encodeFrameNum++;
         } while(! ((tmpSendNow == TRUE) || (maxBytesPerPacket == payloadSize)));
         pRtpPacket->setPayloadSize(payloadSize);

         // Decode frame, measure time and verify, that we decoded same number of samples.
         OsDateTime::getCurTime(start);
         tmpSamplesConsumed = pDecoder->decode(pRtpPacket, DECODED_FRAME_MAX_SIZE,
                                             pDecoded);
         OsDateTime::getCurTime(stop);
         CPPUNIT_ASSERT_EQUAL(samplesInPacket, tmpSamplesConsumed);
            
         // Print timing in TSV format
         diff = stop - start;
         printf("decode %s %s;%d;%ld.%06ld;%ld.%06ld\n",
                codecMime.data(), codecFmtp.data(),
                i,
                start.seconds(), start.usecs(),
                diff.seconds(), diff.usecs());
      }

      // Free encoder and decoder
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           pDecoder->freeDecode());
      delete pDecoder;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           pEncoder->freeEncode());
      delete pEncoder;
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(MpCodecsPerformanceTest);
