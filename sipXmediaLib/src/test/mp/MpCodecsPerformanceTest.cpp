//  
// Copyright (C) 2006-2010 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#include <os/OsIntTypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <mp/MpCodecFactory.h>
#include <os/OsTime.h>
#include <os/OsDateTime.h>

#include <sipxunittests.h>

/// Duration of one frame in milliseconds
#define FRAME_MS                 20
/// Maximum length of audio data we expect from decoder (in samples).
#define DECODED_FRAME_MAX_SIZE   1600
/// Maximum size of encoded frame (in bytes).
#define ENCODED_FRAME_MAX_SIZE   1480
/// Number of RTP packets to encode/decode.
#define NUM_PACKETS_TO_TEST      3
/// Maximum number of milliseconds in packet.
#define MAX_PACKET_TIME          20

// Setup codec paths..
static UtlString sCodecPaths[] = {
#ifdef WIN32
                                  "bin",
                                  "..\\bin",
#elif defined(__pingtel_on_posix__)
                                  "../../../../bin",
                                  "../../../bin",
                                  "../../bin",
#else
#                                 error "Unknown platform"
#endif
                                  "."
                                 };
static size_t sNumCodecPaths = sizeof(sCodecPaths)/sizeof(sCodecPaths[0]);


///  Unit test for testing performance of supported codecs.
class MpCodecsPerformanceTest : public SIPX_UNIT_BASE_CLASS
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
      const MppCodecInfoV1_1 **pCodecInfo;
      unsigned         codecInfoNum;

      // Get/create codec factory
      pCodecFactory = MpCodecFactory::getMpCodecFactory();
      CPPUNIT_ASSERT(pCodecFactory != NULL);

      // Load all available codecs
      size_t i;
      for(i = 0; i < sNumCodecPaths; i++)
      {
         printf("MpCodecsPerformanceTest loading codecs from: %s\n", sCodecPaths[i].data());
         pCodecFactory->loadAllDynCodecs(sCodecPaths[i],
                                         CODEC_PLUGINS_FILTER);
      }

      // Get list of loaded codecs
      pCodecFactory->getCodecInfoArray(codecInfoNum, pCodecInfo);
      CPPUNIT_ASSERT(codecInfoNum>0);

      unsigned j;
      for (j=0; j<codecInfoNum; j++)
      {
         const char **pCodecFmtps;
         unsigned     codecFmtpsNum;
         codecFmtpsNum = pCodecInfo[j]->fmtpsNum;
         pCodecFmtps = pCodecInfo[j]->fmtps;
         if (codecFmtpsNum == 0)
         {
            testOneCodecPreformance(pCodecFactory,
                                    pCodecInfo[j]->mimeSubtype,
                                    "",
                                    pCodecInfo[j]->sampleRate,
                                    pCodecInfo[j]->numChannels);
         } 
         else
         {
            for (unsigned fmtpNum=0; fmtpNum<codecFmtpsNum; fmtpNum++)
            {
               testOneCodecPreformance(pCodecFactory,
                                       pCodecInfo[j]->mimeSubtype,
                                       pCodecFmtps[fmtpNum],
                                       pCodecInfo[j]->sampleRate,
                                       pCodecInfo[j]->numChannels);
            }
         }
      }

      // Free codec factory
      MpCodecFactory::freeSingletonHandle();
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
      MpAudioSample *pOriginal;
      MpAudioSample  pDecoded[DECODED_FRAME_MAX_SIZE];
      int            encodeFrameNum = 0;
      int            maxPacketSamples = (MAX_PACKET_TIME*sampleRate)/1000;
      int            frameSize = (sampleRate*FRAME_MS)/1000;
      int            codecFrameSamples;

      // Allocate buffer for raw audio data
      pOriginal = new MpAudioSample[frameSize];
      srand(time(NULL));
      for (int k=0; k<frameSize; k++)
      {
         pOriginal[k] = rand()%32000 - 16000;
      }

      // Create and initialize decoder and encoder
      //printf("creating decoder: %s/%d/%d fmpt=\"%s\"\n",
      //       codecMime.data(), sampleRate, numChannels, codecFmtp.data());
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           pCodecFactory->createDecoder(codecMime, codecFmtp,
                                                        sampleRate, numChannels,
                                                        0, pDecoder));

      CPPUNIT_ASSERT(pDecoder);

      OsStatus initDecodeStatus = pDecoder->initDecode();
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, initDecodeStatus);
                           
      // Could not test speed of signaling codec
      if (pDecoder->getInfo() == NULL || pDecoder->getInfo()->isSignalingCodec())
      {
         pDecoder->freeDecode();
         delete pDecoder;
         return;
      }
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           pCodecFactory->createEncoder(codecMime, codecFmtp,
                                                        sampleRate, numChannels,
                                                        0, pEncoder));

      CPPUNIT_ASSERT(pEncoder);

      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           pEncoder->initEncode());

      // Get number of samples we'll pack to get one packet
      if (pEncoder->getInfo()->getCodecType() == CODEC_TYPE_FRAME_BASED)
      {
         codecFrameSamples = pEncoder->getInfo()->getNumSamplesPerFrame();
      }
      else if (pEncoder->getInfo()->getCodecType() == CODEC_TYPE_SAMPLE_BASED)
      {
         codecFrameSamples = frameSize;
      }
      else
      {
         assert(!"Unknown codec type!");
      }


      for (int i=0; i<NUM_PACKETS_TO_TEST; i++)
      {
         int            tmpSamplesConsumed;
         int            tmpEncodedSize;
         UtlBoolean     tmpIsPacketReady;
         UtlBoolean     tmpIsPacketSilent;
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
            UtlBoolean setMarkerBit;
            OsStatus result;
            OsDateTime::getCurTime(start);
            result = pEncoder->encode(pOriginal, frameSize, tmpSamplesConsumed,
                                      pRtpPayloadPtr,
                                      ENCODED_FRAME_MAX_SIZE-payloadSize,
                                      tmpEncodedSize, tmpIsPacketReady,
                                      tmpIsPacketSilent,
                                      setMarkerBit);
            OsDateTime::getCurTime(stop);
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, result);
            
            // Print timing in CSV format
            diff = stop - start;
            printf("encode %s/%d/%d %s;%d;%ld.%06ld;%ld.%06ld\n",
                   codecMime.data(), sampleRate, numChannels, codecFmtp.data(),
                   encodeFrameNum,
                   start.seconds(), start.usecs(),
                   diff.seconds(), diff.usecs());

            // Adjust encoding state
            payloadSize += tmpEncodedSize;
            pRtpPayloadPtr += tmpEncodedSize;
            samplesInPacket += tmpSamplesConsumed;
            CPPUNIT_ASSERT(payloadSize <= ENCODED_FRAME_MAX_SIZE);
            encodeFrameNum++;
         } while( (payloadSize == 0)
                ||( (tmpIsPacketReady != TRUE) &&
                    (samplesInPacket+codecFrameSamples <= maxPacketSamples)));
         pRtpPacket->setPayloadSize(payloadSize);

         // Decode frame, measure time and verify, that we decoded same number of samples.
         OsDateTime::getCurTime(start);
         tmpSamplesConsumed = pDecoder->decode(pRtpPacket, DECODED_FRAME_MAX_SIZE,
                                               pDecoded);
         OsDateTime::getCurTime(stop);
         CPPUNIT_ASSERT_EQUAL(samplesInPacket, tmpSamplesConsumed);
            
         // Print timing in TSV format
         diff = stop - start;
         printf("decode %s/%d/%d %s;%d;%ld.%06ld;%ld.%06ld\n",
                codecMime.data(), sampleRate, numChannels, codecFmtp.data(),
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

      // Free buffer used for raw audio data
      delete[] pOriginal;
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(MpCodecsPerformanceTest);
