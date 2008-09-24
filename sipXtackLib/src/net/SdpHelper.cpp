//
// Copyright (C) 2007 Plantronics
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////
// Author: Scott Godin (sgodin AT SipSpectrum DOT com)

#ifndef EXCLUDE_SIPX_SDP_HELPER

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <net/NetBase64Codec.h>
#include <sdp/SdpCodec.h>
#include <net/SdpHelper.h>
#include <net/SdpBody.h>
#include <sdp/Sdp.h>
#include <sdp/SdpMediaLine.h>
#include <sdp/SdpCandidate.h>


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define MAXIMUM_MEDIA_TYPES 30 // should match values from SdpBody.cpp
#define MAXIMUM_VIDEO_SIZES 6
#define MAXIMUM_CANDIDATES 20

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

   // Get Bandwidth - !slg! Current SdpBody implementation does not support getting non CT bandwidths or to get medialine specific bandwidths
   int bandwidth;
   if(sdpBody.getBandwidthField(bandwidth))
   {
      sdp->addBandwidth(Sdp::BANDWIDTH_TYPE_CT, (unsigned int) bandwidth);
   }

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
            int numVideoSizes = MAXIMUM_VIDEO_SIZES;
            int videoSizes[MAXIMUM_VIDEO_SIZES];
            sdpBody.getPayloadFormat(payloadTypes[typeIndex], payloadFormat, videoFmtp, numVideoSizes, videoSizes);

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

      // Set direction, a=sendrecv, a=sendonly, a=recvonly, a=inactive
      // !slg! Note: SdpBody does not currenlty support reading attributes from a particular media line
      SdpMediaLine::SdpDirectionType direction = SdpMediaLine::DIRECTION_TYPE_SENDRECV; // default
      if(sdpBody.findValueInField("a", "sendrecv"))
      {
         direction = SdpMediaLine::DIRECTION_TYPE_SENDRECV;
      }
      else if(sdpBody.findValueInField("a", "sendonly"))
      {
         direction = SdpMediaLine::DIRECTION_TYPE_SENDONLY;
      }
      else if(sdpBody.findValueInField("a", "recvonly"))
      {
         direction = SdpMediaLine::DIRECTION_TYPE_RECVONLY;
      }
      else if(sdpBody.findValueInField("a", "inactive"))
      {
         direction = SdpMediaLine::DIRECTION_TYPE_INACTIVE;
      }
      mediaLine->setDirection(direction);

      int frameRate;
      if(sdpBody.getFramerateField(i, frameRate))
      {
         mediaLine->setFrameRate((unsigned int)frameRate);
      }

      // TCP Setup Attribute - !slg! SdpBody currently does not support getting this for a particular media line
      mediaLine->setTcpSetupAttribute(SdpMediaLine::getTcpSetupAttributeFromString(sdpBody.getRtpTcpRole().data()));

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
         sdpCrypto->setEncryptedSrtp((srtpParameters.securityLevel & SRTP_ENCRYPTION) != 0);
         sdpCrypto->setAuthenticatedSrtp((srtpParameters.securityLevel & SRTP_AUTHENTICATION) != 0);
         mediaLine->addCryptoSettings(sdpCrypto); 
         index++;
      }

      // Get Ice Candidate(s) - !slg! note: ice candidate support in SdpBody is old and should be updated - at which time the following code
      // should also be updated      
      int candidateIds[MAXIMUM_CANDIDATES];
      UtlString transportIds[MAXIMUM_CANDIDATES];
      UtlString transportTypes[MAXIMUM_CANDIDATES];
      double qvalues[MAXIMUM_CANDIDATES];
      UtlString candidateIps[MAXIMUM_CANDIDATES];
      int candidatePorts[MAXIMUM_CANDIDATES];
      int numCandidates;

      if(sdpBody.getCandidateAttributes(i, MAXIMUM_CANDIDATES, candidateIds, transportIds, transportTypes, qvalues, candidateIps, candidatePorts, numCandidates))
      {
         UtlString userFrag;  // !slg! Currently no way to retrieve these
         UtlString password;

         //mediaLine->setIceUserFrag(userFrag.data());
         //mediaLine->setIcePassword(password.data());
         int idx;
         for(idx = 0; idx < numCandidates; idx++)
         {
            mediaLine->addCandidate(transportIds[idx].data(), 
                                    candidateIds[idx], 
                                    SdpCandidate::getCandidateTransportTypeFromString(transportTypes[idx].data()), 
                                    (UInt64)qvalues[idx], 
                                    candidateIps[idx].data(), 
                                    candidatePorts[idx], 
                                    SdpCandidate::CANDIDATE_TYPE_NONE); 
         }
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

