//
// Copyright (C) 2006 Plantronics
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef EXCLUDE_SIPX_SDP_HELPER

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <net/NetBase64Codec.h>
#include <sdp/SdpCodec.h>
#include <sdp/SdpHelper.h>
#include <sdp/Sdp.h>
#include <sdp/SdpMediaLine.h>
#include <sdp/SdpCandidate.h>


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define MAXIMUM_MEDIA_TYPES 20

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

SdpMediaLine::SdpCryptoSuiteType 
SdpHelper::convertCryptoSuiteType(int sdpBodyType)
{
   switch(sdpBodyType)
   { 
   case AES_CM_128_HMAC_SHA1_80:
      return SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80;
   case AES_CM_128_HMAC_SHA1_32:
      return SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_32;
   case F8_128_HMAC_SHA1_80:
      return SdpMediaLine::CRYPTO_SUITE_TYPE_F8_128_HMAC_SHA1_80;
   default:
      return SdpMediaLine::CRYPTO_SUITE_TYPE_NONE;
   }
}

Sdp* SdpHelper::createSdpFromSdpBody(SdpBody& sdpBody)
{
   bool rtcpEnabled = true;
   Sdp* sdp = new Sdp();

   // !slg! Note:  the current implementation of SdpBody does not allow access to all of the data in the SDP body
   // in the future codec similar to that present in SdpHelperResip should be implemented

   // Iterate through the m= lines
   int i;
   for(i = 0; i < sdpBody.getMediaSetCount(); i++)
   {
      bool rtcpEnabledForMedia = rtcpEnabled;  // Default to Session setting
      SdpMediaLine* mediaLine = new SdpMediaLine();
  
      UtlString utlString;  // Temp String
      UtlString mediaType;
      int mediaPort=0;
      int mediaNumPorts=0;
      UtlString mediaTransportType;
      int numPayloadTypes=0;
      int payloadTypes[MAXIMUM_MEDIA_TYPES];
      sdpBody.getMediaData(i, &mediaType, &mediaPort, &mediaNumPorts, &mediaTransportType, MAXIMUM_MEDIA_TYPES, &numPayloadTypes, payloadTypes);

      mediaLine->setMediaType(SdpMediaLine::getMediaTypeFromString(mediaType.data()));
      mediaLine->setTransportProtocolType(SdpMediaLine::getTransportProtocolTypeFromString(mediaTransportType.data()));

      // Get PTime for media line to assign to codecs
      unsigned int ptime=20; // default  !slg! this default should be dependant on codec type - ie. G723 should be 30, etc.

      // Iterate Through Codecs
      {
         int typeIndex;

         for(typeIndex = 0; typeIndex < numPayloadTypes; typeIndex++)
         {
            UtlString mimeSubType;
            UtlString payloadFormat;
            int sampleRate=8000;
            int numChannels=1;
            if(!sdpBody.getPayloadRtpMap(payloadTypes[typeIndex], mimeSubType, sampleRate, numChannels))
            {
               // static codecs as defined in RFC 3551
               switch(payloadTypes[typeIndex])
               {
               case 0:
                  mimeSubType = "PCMU";
                  sampleRate = 8000;
                  break;
               case 3:
                  mimeSubType = "GSM";
                  sampleRate = 8000;
                  break;
               case 4:
                  mimeSubType = "G723";
                  sampleRate = 8000;
                  ptime = 30;
                  break;
               case 5:
                  mimeSubType = "DVI4";
                  sampleRate = 8000;
                  break;
               case 6:
                  mimeSubType = "DVI4";
                  sampleRate = 16000;
                  break;
               case 7:
                  mimeSubType = "LPC";
                  sampleRate = 8000;
                  break;
               case 8:
                  mimeSubType = "PCMA";
                  sampleRate = 8000;
                  break;
               case 9:
                  mimeSubType = "G722";
                  sampleRate = 8000;
                  break;
               case 10:
                  mimeSubType = "L16-2";
                  sampleRate = 44100;
                  break;
               case 11:
                  mimeSubType = "L16-1";
                  sampleRate = 44100;
                  break;
               case 12:
                  mimeSubType = "QCELP";
                  sampleRate = 8000;
                  break;
               case 13:
                  mimeSubType = "CN";
                  sampleRate = 8000;
                  break;
               case 14:
                  mimeSubType = "MPA";
                  sampleRate = 90000;
                  break;
               case 15:
                  mimeSubType = "G728";
                  sampleRate = 8000;
                  break;
               case 16:
                  mimeSubType = "DVI4";
                  sampleRate = 11025;
                  break;
               case 17:
                  mimeSubType = "DVI4";
                  sampleRate = 22050;
                  break;
               case 18:
                  mimeSubType = "G729";
                  sampleRate = 8000;
                  break;
               case 25:
                  mimeSubType = "CelB";
                  sampleRate = 90000;
                  break;
               case 26:
                  mimeSubType = "JPEG";
                  sampleRate = 90000;
                  break;
               case 28:
                  mimeSubType = "nv";
                  sampleRate = 90000;
                  break;
               case 31:
                  mimeSubType = "H261";
                  sampleRate = 90000;
                  break;
               case 32:
                  mimeSubType = "MPV";
                  sampleRate = 90000;
                  break;
               case 33:
                  mimeSubType = "MP2T";
                  sampleRate = 90000;
                  break;
               case 34:
                  mimeSubType = "H263";
                  sampleRate = 90000;
                  break;
               }
            }
            else
            {
               if(numChannels == -1) // Note:  SdpBody returns -1 if no numChannels is specified - default should be one
               {
                  numChannels = 1;
               }
            }

            int videoFmtp; 
            sdpBody.getPayloadFormat(payloadTypes[typeIndex], payloadFormat, videoFmtp);

            SdpCodec* codec = new SdpCodec(payloadTypes[typeIndex], 
                                           mediaType.data(), 
                                           mimeSubType.data(), 
                                           sampleRate, 
                                           ptime * 1000, 
                                           numChannels, 
                                           payloadFormat);
             mediaLine->addCodec(codec);
         }
      }

      // Add Connection
      UtlString mediaAddress;
      sdpBody.getMediaAddress(i, &mediaAddress);
      //sdpBody.getMediaNetworkType(i, &utlString);  !slg! not implemented in SdpBody
      //Sdp::SdpAddressType addrType = Sdp::getAddressTypeFromString(utlString.data());
      Sdp::SdpAddressType addrType = Sdp::ADDRESS_TYPE_IP4;
      if(mediaPort != 0)
      {
         int j;
         for(j = 0; j < mediaNumPorts; j++)
         {
            mediaLine->addConnection(Sdp::NET_TYPE_IN, addrType, mediaAddress.data(), mediaPort + (2 * j));
         }
      }

      // Add Rtcp Connection
      int rtcpPort = 0;
      sdpBody.getMediaRtcpPort(i, &rtcpPort);
      if(rtcpPort != 0)
      {
         int j;
         for(j = 0; j < mediaNumPorts; j++)
         {
            mediaLine->addRtcpConnection(Sdp::NET_TYPE_IN, addrType, mediaAddress.data(), rtcpPort + (2 * j));
         }
      }      

      // Get the SRTP Crypto Settings
      SdpSrtpParameters srtpParameters;
      int index=1;
      while(sdpBody.getSrtpCryptoField(i, index, srtpParameters))
      {
         SdpMediaLine::SdpCrypto* sdpCrypto = new SdpMediaLine::SdpCrypto;
         sdpCrypto->setTag(index);
         sdpCrypto->setSuite(convertCryptoSuiteType(srtpParameters.cipherType));
         UtlString encodedKey;  // Note:  Key returned from SdpBody is Base64 decoded, but Sdp container expects encoded key
         NetBase64Codec::encode(sizeof(srtpParameters.masterKey)-1, (const char*)srtpParameters.masterKey, encodedKey);
         sdpCrypto->addCryptoKeyParam(SdpMediaLine::CRYPTO_KEY_METHOD_INLINE, encodedKey.data()); 
         sdpCrypto->setEncryptedSrtp((srtpParameters.securityLevel & ENCRYPTION) != 0);
         sdpCrypto->setAuthenticatedSrtp((srtpParameters.securityLevel & AUTHENTICATION) != 0);
         mediaLine->addCryptoSettings(sdpCrypto); 
         index++;
      }

      // Get Ice Candidate(s) - !slg! note: ice candidate support in SdpBody is old and should be updated - at which time the following code
      // should also be updated
      UtlString id;
      double qvalue;
      UtlString userFrag;
      UtlString password;
      UtlString ip;
      int port;
      UtlString candidateIp;
      int candidatePort;
      if(sdpBody.getCandidateAttribute(i, id, qvalue, userFrag, password, ip, port, candidateIp, candidatePort))
      {
         mediaLine->setIceUserFrag(userFrag.data());
         mediaLine->setIcePassword(password.data());
         mediaLine->addCandidate(id.data(), 1, SdpCandidate::CANDIDATE_TRANSPORT_TYPE_UDP, (UInt64)qvalue, ip.data(), port, SdpCandidate::CANDIDATE_TYPE_NONE, candidateIp.data(), candidatePort); 
      }

      // Add the media line to the sdp
      sdp->addMediaLine(mediaLine);
   }

   return sdp;
}


/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

#endif