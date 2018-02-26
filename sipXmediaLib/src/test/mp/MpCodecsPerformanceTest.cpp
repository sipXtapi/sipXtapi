//  
// Copyright (C) 2006-2018 SIPez LLC.  All rights reserved.
//  
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#include <os/OsIntTypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <mp/MpCodecFactory.h>
#include <mp/NetInTask.h>
#include <os/OsTime.h>
#include <os/OsDateTime.h>

#include <sipxunittests.h>

// Setup codec paths..
#include <../test/mp/MpTestCodecPaths.h>

/// Duration of one frame in milliseconds
#define FRAME_MS                 10
/// Maximum length of audio data we expect from decoder (in samples).
#define DECODED_FRAME_MAX_SIZE   3200
/// Maximum size of encoded frame (in bytes).
#define ENCODED_FRAME_MAX_SIZE   RTP_MTU
/// Number of RTP packets to encode/decode.
#define NUM_PACKETS_TO_TEST      3
/// Maximum number of milliseconds in packet.
#define MAX_PACKET_TIME          20

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
      mpPool = new MpBufPool(ENCODED_FRAME_MAX_SIZE + MpArrayBuf::getHeaderSize(), 1, "MpCodecsPerformanceTest");
      CPPUNIT_ASSERT(mpPool != NULL);

      // Create pool for buffer headers
      mpHeadersPool = new MpBufPool(sizeof(MpRtpBuf), 1, "MpCodecsPerformanceTestHeaders");
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

      printf("mediaFrame size: %d mSec\n", FRAME_MS);

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
         if(strcmp(pCodecInfo[j]->mimeSubtype, MIME_SUBTYPE_H264) == 0)
         {
             printf("Skipping codec: %s\n", pCodecInfo[j]->mimeSubtype);
         }
         else if (codecFmtpsNum == 0)
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
      int            maxPacketSamples = (MAX_PACKET_TIME*sampleRate)/1000 /
                        (MAX_PACKET_TIME/FRAME_MS) * (MAX_PACKET_TIME/FRAME_MS); // Fix roundoff issues
      int            frameSize = (sampleRate*FRAME_MS)/1000;
      int            codecFrameSamples;
      UtlString contextMessage;
      contextMessage.appendFormat("While testing codec: %s fmtp: %s sample rate: %d", codecMime.data(), codecFmtp.data(), sampleRate);

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

      OsStatus initDecodeStatus = pDecoder->initDecode(codecFmtp);
      CPPUNIT_ASSERT_EQUAL_MESSAGE(contextMessage.data(), OS_SUCCESS, initDecodeStatus);
                           
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

      printf("codec: %s sample rate: %d frame size: %d\n",
         codecMime.data(), sampleRate, codecFrameSamples);

      CPPUNIT_ASSERT_MESSAGE(contextMessage.data(), codecFrameSamples > 0);

      unsigned char* rtpDataStart = NULL;
      int algorithmicDelaySamplesCount = 0;

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

         UtlString loopMessage;
         loopMessage.appendFormat("%s packet: %d", codecMime.data(), i);

         if(rtpDataStart == NULL)
         {
             rtpDataStart = pRtpPayloadPtr;
         }

         // Encode frames until we get tmpSendNow set or reach packet size limit.
         do 
         {
            if(pRtpPayloadPtr != rtpDataStart)
            {
               printf("WARNING: rtpDataStart: %p pRtpPayloadPtr: %p\n",
                   rtpDataStart, pRtpPayloadPtr);
            }

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
            CPPUNIT_ASSERT_MESSAGE(contextMessage.data(), payloadSize <= ENCODED_FRAME_MAX_SIZE);
            if(payloadSize > ENCODED_FRAME_MAX_SIZE)
            {
                printf("payloadSize: %d ENCODED_FRAME_MAX_SIZE: %d\n",
                    payloadSize, ENCODED_FRAME_MAX_SIZE);
            }
            encodeFrameNum++;
            printf("payloadSize: %d tmpIsPacketReady: %s samplesInPacket: %d codecFrameSamples: %d maxPacketSamples: %d tmpEncodedSize: %d\n",
                payloadSize, tmpIsPacketReady ? "true" : "false", samplesInPacket, codecFrameSamples, maxPacketSamples, tmpEncodedSize);
         } while( (payloadSize == 0)
                ||( (tmpIsPacketReady != TRUE) &&
                    (samplesInPacket+codecFrameSamples <= maxPacketSamples)));

         printf("packet samples: %d\n", samplesInPacket);

         pRtpPacket->setPayloadSize(payloadSize);

         // Decode frame, measure time and verify, that we decoded same number of samples.
         OsDateTime::getCurTime(start);
         tmpSamplesConsumed = pDecoder->decode(pRtpPacket, DECODED_FRAME_MAX_SIZE,
                                               pDecoded);
         OsDateTime::getCurTime(stop);
         if(algorithmicDelaySamplesCount == 0 &&
            tmpSamplesConsumed > 0)
         {
             algorithmicDelaySamplesCount = samplesInPacket;
         
             CPPUNIT_ASSERT_MESSAGE(loopMessage.data(),
                                    algorithmicDelaySamplesCount >= 
                                    (int)pEncoder->getInfo()->getAlgorithmicDelay());
             if(algorithmicDelaySamplesCount < 
                (int)pEncoder->getInfo()->getAlgorithmicDelay())
             {
                printf("algorithmic delay observed: %d samples codec info: %d samples\n",
                       algorithmicDelaySamplesCount,
                       pEncoder->getInfo()->getAlgorithmicDelay());
             }
             // If we run into the MTU limit, we may not reach the algorithmic delay num samples
             if(pRtpPacket->getPayloadSize() < RTP_MTU ||
                algorithmicDelaySamplesCount >=
                                    codecFrameSamples)
             {
                 CPPUNIT_ASSERT_MESSAGE(loopMessage.data(),
                                        algorithmicDelaySamplesCount >= 
                                        codecFrameSamples);
                 if(algorithmicDelaySamplesCount <
                    codecFrameSamples)
                 {
                    printf("algorithmic delay observed: %d codecFrameSamples: %d\n",
                           algorithmicDelaySamplesCount,
                           codecFrameSamples);
                 }
             }
         }
         CPPUNIT_ASSERT_EQUAL_MESSAGE(loopMessage.data(),
                                      samplesInPacket, tmpSamplesConsumed);
         if(pEncoder->getInfo()->getCodecType() == CODEC_TYPE_SAMPLE_BASED)
         {
             if(pRtpPacket->getPayloadSize() < RTP_MTU ||
                tmpSamplesConsumed == maxPacketSamples)
             {
                 CPPUNIT_ASSERT_EQUAL_MESSAGE(loopMessage.data(),
                                              tmpSamplesConsumed,
                                              maxPacketSamples);
             }
             // TODO: need test for case when we fill the RTP packet before we get to maxPacketSamples
             else
             {
                 printf("WARNING: maximum RTP MTU (%d) reached before maxPacketSamples (%d), only %d encoded\n",
                        RTP_MTU,
                        maxPacketSamples,
                        tmpSamplesConsumed);
             }
         }
         else
         {
             CPPUNIT_ASSERT_EQUAL_MESSAGE(loopMessage.data(),
                                          tmpSamplesConsumed,
                                          codecFrameSamples);
         }
         printf("RTP payload first byte: %x payload size: %d\n",
             *(pRtpPacket->getDataPtr()), pRtpPacket->getPayloadSize());
 
         // Print timing in TSV format
         diff = stop - start;
         printf("decode %s/%d/%d %s;%d;%ld.%06ld;%ld.%06ld\n",
                codecMime.data(), sampleRate, numChannels, codecFmtp.data(),
                i,
                start.seconds(), start.usecs(),
                diff.seconds(), diff.usecs());
      }

      printf("algorithmic-delay %s/%d/%d %s;%d;%f\n",
             codecMime.data(), sampleRate, numChannels, codecFmtp.data(),
             algorithmicDelaySamplesCount, 
             ((float)algorithmicDelaySamplesCount) / ((float)sampleRate) * 1000.0);
      printf("packet-samples %s/%d/%d %s;%d;%f\n",
             codecMime.data(), sampleRate, numChannels, codecFmtp.data(),
             codecFrameSamples,
             ((float)codecFrameSamples) / ((float)sampleRate) * 1000.0);
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
