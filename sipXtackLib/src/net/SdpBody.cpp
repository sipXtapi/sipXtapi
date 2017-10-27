//  
// Copyright (C) 2007-2017 SIPez LLC. All rights reserved.
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <stdio.h>
#include <stdlib.h>

// APPLICATION INCLUDES
#include <os/OsLock.h>
#include <os/OsSysLog.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlTokenizer.h>
#include <utl/UtlLongLongInt.h>
#include <sdp/SdpCodec.h>
#include <sdp/SdpMediaLine.h>
#include <net/SdpBody.h>
#include <net/SdpHelper.h>
#include <net/NameValuePair.h>
#include <utl/UtlNameValueTokenizer.h>
#include <sdp/SdpCodecList.h>
#include <utl/UtlTokenizer.h>
#include <net/NetBase64Codec.h>

#define MAXIMUM_LONG_INT_CHARS 20
#define MAXIMUM_MEDIA_TYPES 128
#define MAXIMUM_VIDEO_SIZES 6
//#define TEST_PRINT

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define SDP_NAME_VALUE_DELIMITOR '='
#define SDP_SUBFIELD_SEPARATOR ' '
#define SDP_SUBFIELD_SEPARATORS "\t "

#define SDP_RTP_MEDIA_TRANSPORT_TYPE "RTP/AVP"
#define SDP_RTP_TCP_MEDIA_TRANSPORT_TYPE "TCP/RTP/AVP"
#define SDP_SRTP_MEDIA_TRANSPORT_TYPE "RTP/SAVP"
#define SDP_MLAW_PAYLOAD 0
#define SDP_ALAW_PAYLOAD 8

#define SDP_NETWORK_TYPE "IN"
#define SDP_IP4_ADDRESS_TYPE "IP4"
#define NTP_TO_EPOCH_DELTA 2208988800UL
#define PRIORITY_OFFSET (10000000100ULL)

// STATIC VARIABLE INITIALIZATIONS
static int     sSessionCount = 5 ;  // Session version for SDP body
static OsMutex sSessionLock(OsMutex::Q_FIFO) ;


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SdpBody::SdpBody(const char* bodyBytes, int byteCount)
 : HttpBody(bodyBytes, byteCount)
{
   mClassType = SDP_BODY_CLASS;
   remove(0);
   append(SDP_CONTENT_TYPE);

   sdpFields = new UtlSList();

   if(bodyBytes)
   {
      if(byteCount < 0)
      {
         bodyLength = strlen(bodyBytes);
      }
      parseBody(bodyBytes, byteCount);
   }
   else
   {
      // this is the mandated order of the header fields
      addValue("v", "0" );
      addValue("o", "sipX 5 5 IN IP4 127.0.0.1");
      addValue("s");
      addValue("i");
      addValue("u");
      addValue("e");
      addValue("p");
      addValue("c");
      addValue("b");
   }
}

// Copy constructor
SdpBody::SdpBody(const SdpBody& rSdpBody) :
   HttpBody(rSdpBody)
{
   mClassType = SDP_BODY_CLASS;
   if(rSdpBody.sdpFields)
   {
      sdpFields = new UtlSList();

      NameValuePair* headerField;
      NameValuePair* copiedHeader = NULL;
      UtlSListIterator iterator((UtlSList&)(*(rSdpBody.sdpFields)));
      while((headerField = (NameValuePair*) iterator()))
      {
         copiedHeader = new NameValuePair(*headerField);
         sdpFields->append(copiedHeader);
      }
   }
   else
   {
      sdpFields = NULL;
   }
}

// Destructor
SdpBody::~SdpBody()
{
   if(sdpFields)
   {
      while(! sdpFields->isEmpty())
      {
         delete sdpFields->get();
      }
      delete sdpFields;
   }
}

/* ============================ MANIPULATORS ============================== */

void SdpBody::parseBody(const char* bodyBytes, int byteCount)
{
   if(byteCount < 0)
   {
      bodyLength = strlen(bodyBytes);
   }

   if(bodyBytes)
   {
      UtlString name;
      UtlString value;
      int nameFound;
      NameValuePair* nameValue;

      UtlNameValueTokenizer parser(bodyBytes, byteCount);
      do
      {
         name.remove(0);
         value.remove(0);
         nameFound = parser.getNextPair(SDP_NAME_VALUE_DELIMITOR,
                                        &name, &value);
         if(nameFound)
         {
            nameValue = new NameValuePair(name.data(), value.data());
            sdpFields->append(nameValue);

         }
      }
      while(nameFound);
   }
}


// Assignment operator
SdpBody&
SdpBody::operator=(const SdpBody& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   // Copy the base class stuff
   this->HttpBody::operator=((const HttpBody&)rhs);

   if(sdpFields)
   {
      sdpFields->destroyAll();
   }

   if(rhs.sdpFields)
   {
      if(sdpFields == NULL)
      {
         sdpFields = new UtlSList();
      }

      NameValuePair* headerField;
      NameValuePair* copiedHeader = NULL;
      UtlSListIterator iterator((UtlSList&)(*(rhs.sdpFields)));
      while((headerField = (NameValuePair*) iterator()))
      {
         copiedHeader = new NameValuePair(*headerField);
         sdpFields->append(copiedHeader);
      }
   }

   // Set the class type just to play it safe
   mClassType = SDP_BODY_CLASS;
   return *this;
}

/* ============================ ACCESSORS ================================= */
void SdpBody::setStandardHeaderFields(const char* sessionName,
                                      const char* emailAddress,
                                      const char* phoneNumber,
                                      const char* originatorAddress)
{
   OsLock lock (sSessionLock) ;

   setOriginator("sipX", 5, sSessionCount++,
                 (originatorAddress && *originatorAddress) ?
                 originatorAddress : "127.0.0.1");
   setSessionNameField(sessionName);
   if(emailAddress && strlen(emailAddress) > 0)
   {
      setEmailAddressField(emailAddress);
   }
   if(phoneNumber && strlen(phoneNumber) > 0)
   {
      setPhoneNumberField(phoneNumber);
   }
}

void SdpBody::setSessionNameField(const char* sessionName)
{
   setValue("s", sessionName);
}

void SdpBody::setEmailAddressField(const char* emailAddress)
{
   setValue("e", emailAddress);
}

void SdpBody::setPhoneNumberField(const char* phoneNumber)
{
   setValue("p", phoneNumber);
}

void SdpBody::setValue(const char* name, const char* value)
{
   NameValuePair nvToMatch(name);
   NameValuePair* nvFound = NULL;
   UtlSListIterator iterator(*sdpFields);

   nvFound = (NameValuePair*) iterator.findNext(&nvToMatch);
   if(nvFound)
   {
      // field exists - replace the value
      nvFound->setValue(value);
   }
   else
   {
      addValue(name, value);
   }
}

int SdpBody::getMediaSetCount() const
{
   UtlSListIterator iterator(*sdpFields);
   NameValuePair mediaName("m");
   int count = 0;
   while(iterator.findNext(&mediaName))
   {
      count++;
   }
   return(count);

}

UtlBoolean SdpBody::getMediaType(int mediaIndex, UtlString* mediaType) const
{
   return(getMediaSubfield(mediaIndex, 0, mediaType));
}

UtlBoolean SdpBody::getMediaPort(int mediaIndex, int* port) const
{
   UtlString portString;
   UtlBoolean portFound = getMediaSubfield(mediaIndex, 1, &portString);
   int portCountSeparator;

   if(!portString.isNull())
   {
      // Remove the port pair count if it exists
      portCountSeparator = portString.index("/");
      if(portCountSeparator >= 0)
      {
         portString.remove(portCountSeparator);
      }

      *port = atoi(portString.data());
      portFound = TRUE;
   }

   return(portFound);
}

UtlBoolean SdpBody::getMediaRtcpPort(int mediaIndex, int* port) const
{
    UtlBoolean bFound = FALSE ;
    int iRtpPort ;
    
    if (getMediaPort(mediaIndex, &iRtpPort))
    {
        bFound = TRUE ;
        *port = iRtpPort + 1;

        UtlSListIterator iterator(*sdpFields);
        NameValuePair* nv = positionFieldInstance(mediaIndex, &iterator, "m");
        if(nv)
        {
            while ((nv = findFieldNameBefore(&iterator, "a", "m")))
            {
                //printf("->%s:%s\n", nv->data(), nv->getValue()) ;

                UtlString typeAttribute ;
                UtlString portAttribute ;

                UtlNameValueTokenizer::getSubField(nv->getValue(), 0, 
                        ":", &typeAttribute) ;
                UtlNameValueTokenizer::getSubField(nv->getValue(), 1,
                        ":", &portAttribute) ;

                if (typeAttribute.compareTo("rtcp", UtlString::ignoreCase) == 0)
                {
                    *port = atoi(portAttribute.data()) ;
                    
                }
            }
        }
    }

    return bFound ;
}

UtlBoolean SdpBody::getControlTrackId(int mediaIndex, UtlString& trackId) const
{
    UtlBoolean trackIdFound = FALSE;
    UtlSListIterator iterator(*sdpFields);
    if(positionFieldInstance(mediaIndex, &iterator, "m"))
    {
        NameValuePair* sdpAttribute = NULL;
        while ((sdpAttribute = findFieldNameBefore(&iterator, "a", "m")))
        {
            UtlString value = sdpAttribute->getValue();
            UtlString valueLowered(value);
            valueLowered.toLower();
            UtlString token("control:trackid");
            int tokenIndex = valueLowered.index(token);
            if(tokenIndex >=0)
            {
                value.remove(0, tokenIndex + token.length());
                tokenIndex = value.index('=');
                if(tokenIndex >= 0)
                {
                    value.remove(0, tokenIndex + 1);

                    UtlNameValueTokenizer::getSubField(value, 0,
                                      " \t:=/", // separators
                                      &trackId);
                    if(!trackId.isNull())
                    {
                        trackIdFound = TRUE;
                    }
                }
            }
        }
    }

   return(trackIdFound); 
}

UtlBoolean SdpBody::getMediaStreamDirection(int mediaIndex, SessionDirection& direction) const
{
    UtlBoolean found = FALSE;
    direction = Unknown;
    UtlString mediaType;

    if(getMediaType(mediaIndex, &mediaType))
    {
        UtlSListIterator iterator(*sdpFields);
        NameValuePair* sdpAttribute = positionFieldInstance(mediaIndex, &iterator, "m");
        if(sdpAttribute)
        {
            while ((sdpAttribute = findFieldNameBefore(&iterator, "a", "m")))
            {
                UtlString directionToken = sdpAttribute->getValue();

                if (directionToken.compareTo("inactive", UtlString::ignoreCase) == 0)
                {
                    direction = Inactive;
                    found = TRUE;
                }
                else if (directionToken.compareTo("sendonly", UtlString::ignoreCase) == 0)
                {
                    direction = SendOnly;
                    found = TRUE;
                }
                else if (directionToken.compareTo("recvonly", UtlString::ignoreCase) == 0)
                {
                    direction = RecvOnly;
                    found = TRUE;
                }
                else if (directionToken.compareTo("sendrecv", UtlString::ignoreCase) == 0)
                {
                    direction = SendRecv;
                    found = TRUE;
                }
            }
        }
    }

    return(found);
}

UtlBoolean SdpBody::getMediaProtocol(int mediaIndex, UtlString* transportProtocol) const
{
   return(getMediaSubfield(mediaIndex, 2, transportProtocol));
}

UtlBoolean SdpBody::getMediaPayloadType(int mediaIndex, int maxTypes,
                                        int* numTypes, int payloadTypes[]) const
{
    UtlString payloadTypeString;
    int typeCount = 0;
    int index = 0 ;

    while (index < maxTypes &&
            getMediaSubfield(mediaIndex, 3 + index++, &payloadTypeString))
    {
        if(!payloadTypeString.isNull())
        {
            // Add the payload type and increment typeCount if not 
            // already in the list
            bool bFound = false ;
            int payload = atoi(payloadTypeString.data()) ;
            for (int i=0; i<typeCount; i++)
            {
                if (payloadTypes[i] == payload)
                {
                    bFound = true ;
                    break ;
                }
            }

            if (!bFound)
            {            
                payloadTypes[typeCount] = payload ;
                typeCount++;
            }
        }
    }

    *numTypes = typeCount;

    return(typeCount > 0);
}

UtlBoolean SdpBody::getMediaSubfield(int mediaIndex, int subfieldIndex, UtlString* subField) const
{
   UtlBoolean subfieldFound = FALSE;
   UtlSListIterator iterator(*sdpFields);
   NameValuePair* nv = positionFieldInstance(mediaIndex, &iterator, "m");
   const char* value;
   subField->remove(0);

   if(nv)
   {
      value =  nv->getValue();
      UtlNameValueTokenizer::getSubField(value, subfieldIndex,
                                      SDP_SUBFIELD_SEPARATORS, subField);
      if(!subField->isNull())
      {
         subfieldFound = TRUE;
      }
   }
   return(subfieldFound);
}

UtlBoolean SdpBody::getPayloadRtpMap(int mediaIndex,
                                     int payloadType,
                                     UtlString& mimeSubtype,
                                     int& sampleRate,
                                     int& numChannels) const
{
   // an "a" record look something like:
   // "a=rtpmap:<payloadType> <mimeSubtype/sampleRate>[/numChannels]"

   // Loop through all of the "a" records
   UtlBoolean foundRtpMap = FALSE;
   UtlSListIterator iterator(*sdpFields);
   NameValuePair* nv = NULL;
   int aFieldIndex = 0;
   const char* value;
   UtlString aFieldType;
   UtlString payloadString;
   UtlString sampleRateString;
   UtlString numChannelString;
   UtlString aFieldMatch("a");

   // Position to the correct media line
   NameValuePair* mediaLineFound = positionFieldInstance(mediaIndex, &iterator, "m");

   // Look at the a lines
   while(mediaLineFound &&
         (nv = findFieldNameBefore(&iterator, "a", "m")) != NULL)
   //while((nv = (NameValuePair*) iterator.findNext(&aFieldMatch)) != NULL)
   {
      value =  nv->getValue();

      // Verify this is an rtpmap "a" record
      UtlNameValueTokenizer::getSubField(value, 0,
                                      " \t:/", // separators
                                      &aFieldType);
      if(aFieldType.compareTo("rtpmap", UtlString::ignoreCase) == 0)
      {
         // If this is the rtpmap for the requested payload type
         UtlNameValueTokenizer::getSubField(value, 1,
                                         " \t:/", // separators
                                         &payloadString);
         if(atoi(payloadString.data()) == payloadType)
         {
            // The mime subtype is the 3nd subfield
            UtlNameValueTokenizer::getSubField(value, 2,
                                            " \t:/", // separators
                                            &mimeSubtype);

            // The sample rate is the 4rd subfield
            UtlNameValueTokenizer::getSubField(value, 3,
                                            " \t:/", // separators
                                            &sampleRateString);
            sampleRate = atoi(sampleRateString.data());
            if(sampleRate <= 0) sampleRate = -1;

            // The number of channels is the 5th subfield
            UtlNameValueTokenizer::getSubField(value, 4,
                                            " \t:/", // separators
                                            &numChannelString);
            numChannels = atoi(numChannelString.data());
            if(numChannels <= 0) numChannels = -1;

            // we are all done
            foundRtpMap = TRUE;
            break;
         }
      }

      aFieldIndex++;
   }
   return(foundRtpMap);
}

UtlBoolean SdpBody::getPayloadFormat(int mediaIndex, 
                                     int payloadType,
                                     UtlString& fmtp) const
{
   // See SdpCodec for utilities to parse the fmtp field (e.g. getFmtpParameter and getVideoSizes)

   // an "a" record look something like:
   // "a=fmtp:<payloadType> <fmtpdata>"

   // Loop through all of the "a" records
   UtlBoolean foundPayloadFmtp = FALSE;
   UtlBoolean foundField;
   UtlSListIterator iterator(*sdpFields);
   UtlString aFieldType;
   UtlString payloadString;
   UtlString modifierString;
   NameValuePair* nv = NULL;
   int aFieldIndex = 0;
   const char* value;
   UtlString temp;
   UtlString aFieldMatch("a");

   fmtp.remove(0);

   // Position to the correct media line
   NameValuePair* mediaLineFound = positionFieldInstance(mediaIndex, &iterator, "m");

   // Look at the a lines
   while(mediaLineFound &&
         (nv = findFieldNameBefore(&iterator, "a", "m")) != NULL)
   //while((nv = (NameValuePair*) iterator.findNext(&aFieldMatch)) != NULL)
   {
      value =  nv->getValue();

      // Verify this is an fmtp "a" record
      UtlNameValueTokenizer::getSubField(value, 0,
                                      " \t:/", // separators
                                      &aFieldType);
      if(aFieldType.compareTo("fmtp", UtlString::ignoreCase) == 0)
      {
         UtlNameValueTokenizer::getSubField(value, 1,
                                         " \t:/", // separators
                                         &payloadString);
         if(atoi(payloadString.data()) == payloadType)
         {
            const char *fmtpSubField;
            int subFieldLen;
            // If this is the fmtp for the requested payload type
            foundField = UtlNameValueTokenizer::getSubField(value, -1, 2, 
                                                         " \t:",  // separators
                                                         fmtpSubField,
                                                         subFieldLen,
                                                         0);
            if(foundField) 
            {
               fmtp = fmtpSubField;
            }

            foundPayloadFmtp = TRUE;
         }
      }

      aFieldIndex++;
   }
   return(foundPayloadFmtp);
}

UtlBoolean SdpBody::getSrtpCryptoField(int mediaIndex,
                                       int index,
                                       SdpSrtpParameters& params) const
{
    UtlBoolean foundCrypto = FALSE;
    UtlBoolean foundField;
    UtlString aFieldType;
    UtlSListIterator iterator(*sdpFields);
    NameValuePair* nv = positionFieldInstance(mediaIndex, &iterator, "m");
    const char* value;
    UtlString indexString;
    UtlString cryptoSuite;
    UtlString temp;
    int size;
    char srtpKey[SRTP_KEY_LENGTH+1];

    size = sdpFields->entries();
    while ((nv = findFieldNameBefore(&iterator, "a", "m")))
    {
        value =  nv->getValue();

        // Verify this is an crypto "a" record
        UtlNameValueTokenizer::getSubField(value, 0,
                                        " \t:/", // separators
                                        &aFieldType);
        if(aFieldType.compareTo("crypto", UtlString::ignoreCase) == 0)
        {
            UtlNameValueTokenizer::getSubField(value, 1,
                                            " \t:/", // separators
                                            &indexString);
            if(atoi(indexString.data()) == index)
            {
                foundCrypto = TRUE;

                // Encryption & authentication on by default
                params.securityLevel = SRTP_ENCRYPTION | SRTP_AUTHENTICATION;

                UtlNameValueTokenizer::getSubField(value, 2,
                                                " \t:/", // separators
                                                &cryptoSuite);
                // Check the crypto suite 
                if (cryptoSuite.compareTo("AES_CM_128_HMAC_SHA1_80", UtlString::ignoreCase) == 0)
                {
                    params.cipherType = AES_CM_128_HMAC_SHA1_80;
                }
                else if (cryptoSuite.compareTo("AES_CM_128_HMAC_SHA1_32", UtlString::ignoreCase) == 0)
                {
                    params.cipherType = AES_CM_128_HMAC_SHA1_32;
                }
                else if (cryptoSuite.compareTo("F8_128_HMAC_SHA1_80", UtlString::ignoreCase) == 0)
                {
                    params.cipherType = F8_128_HMAC_SHA1_80;
                }
                else
                {
                    //Couldn't find crypto suite, no secritiy
                    params.securityLevel = 0;
                }

                // Get key
                foundField = UtlNameValueTokenizer::getSubField(value, 4,
                                                             " \t/:|", // separators
                                                             &temp);
                NetBase64Codec::decode(temp.length(), temp.data(), size, srtpKey);
                if (size <= SRTP_KEY_LENGTH)
                {
                    srtpKey[size] = 0;
                }
                else
                {
                    srtpKey[SRTP_KEY_LENGTH] = 0;
                }
                strcpy((char*)params.masterKey, srtpKey);

                // Modify security level with session parameters
                for (int index=5; foundField; ++index)
                {
                    foundField = UtlNameValueTokenizer::getSubField(value, index,
                                                                 " \t/:|", // separators
                                                                 &temp);
                    if (foundField)
                    {
                        if (temp.compareTo("UNENCRYPTED_SRTP", UtlString::ignoreCase) == 0)
                        {
                            params.securityLevel &= ~SRTP_ENCRYPTION;
                        }
                        if (temp.compareTo("UNAUTHENTICATED_SRTP", UtlString::ignoreCase) == 0)
                        {
                            params.securityLevel &= ~SRTP_AUTHENTICATION;
                        }
                    }
                }
                break;
            }
        }
    }
    return foundCrypto;
}

UtlBoolean SdpBody::getFramerateField(int mediaIndex,
                                      int& videoFramerate) const
{
    UtlBoolean foundFramerate = FALSE;
    UtlString aFieldType;
    UtlSListIterator iterator(*sdpFields);
    NameValuePair* nv = positionFieldInstance(mediaIndex, &iterator, "m");
    const char* value;
    UtlString aFieldMatch("a");
    UtlString rateString;
    UtlString temp;
    videoFramerate = 0;

    while((nv = (NameValuePair*) iterator.findNext(&aFieldMatch)) != NULL)
    {
        value =  nv->getValue();

        // Verify this is an crypto "a" record
        UtlNameValueTokenizer::getSubField(value, 0,
                                        " \t:/", // separators
                                        &aFieldType);
        if(aFieldType.compareTo("framerate", UtlString::ignoreCase) == 0)
        {
            UtlNameValueTokenizer::getSubField(value, 1,
                                            " \t:/", // separators
                                            &rateString);
            videoFramerate = atoi(rateString.data());
            foundFramerate = TRUE;
        }
    }
    return foundFramerate;
}

UtlBoolean SdpBody::getBandwidthField(int& bandwidth) const
{
    UtlBoolean bFound = FALSE;

   // Loop through all of the "b" records
   UtlSListIterator iterator(*sdpFields);
   NameValuePair* nv = NULL;
   const char* value;
   UtlString aFieldMatch("b");
   UtlString aFieldModifier;
   UtlString temp;

   // Indicate no "b" field was sent with 0
   bandwidth  = 0;

   while((nv = (NameValuePair*) iterator.findNext(&aFieldMatch)) != NULL)
   {
      value =  nv->getValue();

      // Verify this is an crypto "a" record
      UtlNameValueTokenizer::getSubField(value, 0,
                                      " \t:/", // separators
                                        &aFieldModifier);
      if(aFieldModifier.compareTo("CT", UtlString::ignoreCase) == 0)
      {
            UtlNameValueTokenizer::getSubField(value, 1,
                                            " \t:/", // separators
                                            &temp);

            bandwidth = atoi(temp.data());
            bFound = TRUE;
      }
   }
   return bFound;
}

UtlBoolean SdpBody::getValue(int fieldIndex, UtlString* name, UtlString* value) const
{
   NameValuePair* nv = NULL;
   name->remove(0);
   value->remove(0);
   if(fieldIndex >=0)
   {
      nv = (NameValuePair*) sdpFields->at(fieldIndex);
      if(nv)
      {
         *name = *nv;
         *value = nv->getValue();
      }
   }
   return(nv != NULL);
}

UtlBoolean SdpBody::getMediaData(int mediaIndex, UtlString* mediaType,
                                 int* mediaPort, int* mediaPortPairs,
                                 UtlString* mediaTransportType,
                                 int maxPayloadTypes, int* numPayloadTypes,
                                 int payloadTypes[]) const
{
   UtlBoolean fieldFound = FALSE;
   UtlSListIterator iterator(*sdpFields);
   NameValuePair* nv = positionFieldInstance(mediaIndex, &iterator, "m");
   const char* value;
   UtlString portString;
   UtlString portPairString;
   int portCountSeparator;
   int typeCount = 0;
   UtlString payloadTypeString;

   if(nv)
   {
      fieldFound = TRUE;
      value =  nv->getValue();

      // media Type
      UtlNameValueTokenizer::getSubField(value, 0,
                                      SDP_SUBFIELD_SEPARATORS, mediaType);

      // media port and media port pair count
      UtlNameValueTokenizer::getSubField(value, 1,
                                      SDP_SUBFIELD_SEPARATORS, &portString);
      if(!portString.isNull())
      {
         // Copy for obtaining the pair count
         portPairString.append(portString);

         // Remove the port pair count if it exists
         portCountSeparator = portString.index("/");
         if(portCountSeparator >= 0)
         {
            portString.remove(portCountSeparator);

            // Get the other part of the field
            portPairString.remove(0, portCountSeparator + 1);
         }
         else
         {
            portPairString.remove(0);
         }

         *mediaPort = atoi(portString.data());
         if(portPairString.isNull())
         {
            *mediaPortPairs = 1;
         }
         else
         {
            *mediaPortPairs = atoi(portPairString.data());
         }
      }
      else
      {
         *mediaPort = 0;
         *mediaPortPairs = 0;
      }

      // media transport type
      UtlNameValueTokenizer::getSubField(value, 2,
                                      SDP_SUBFIELD_SEPARATORS, mediaTransportType);

      // media payload/codec types
      UtlNameValueTokenizer::getSubField(value,  3 + typeCount,
                                      SDP_SUBFIELD_SEPARATORS, &payloadTypeString);
      while(typeCount < maxPayloadTypes &&
            !payloadTypeString.isNull())
      {
         payloadTypes[typeCount] = atoi(payloadTypeString.data());
         typeCount++;
         UtlNameValueTokenizer::getSubField(value,  3 + typeCount,
                                         SDP_SUBFIELD_SEPARATORS, &payloadTypeString);
      }
      *numPayloadTypes = typeCount;
   }

   return(fieldFound);
}

int SdpBody::findMediaType(const char* mediaType, int startMediaIndex) const
{
   NameValuePair* mediaField;
   UtlSListIterator iterator(*sdpFields);
   UtlBoolean mediaTypeFound = FALSE;
   int index = startMediaIndex;
   NameValuePair mediaName("m");
   mediaField = positionFieldInstance(startMediaIndex, &iterator, "m");
   const char* value;

   while(mediaField && ! mediaTypeFound)
   {
      value = mediaField->getValue();
      if(strstr(value, mediaType) == value)
      {
         mediaTypeFound = TRUE;
         break;
      }

      mediaField = (NameValuePair*) iterator.findNext(&mediaName);
      index++;
   }

   if(! mediaTypeFound)
   {
      index = -1;
   }
   return(index);
}

UtlBoolean SdpBody::getMediaAddress(int mediaIndex, UtlString* address) const
{
   UtlSListIterator iterator(*sdpFields);
   NameValuePair* nv;
   address->remove(0);
   const char* value = NULL;
   int ttlIndex;

   // Try to find a specific address for the given media set
   nv = positionFieldInstance(mediaIndex, &iterator, "m");
   if(nv)
   {
      nv = findFieldNameBefore(&iterator, "c", "m");
      if(nv)
      {
         value = nv->getValue();
         if(value)
         {
            UtlNameValueTokenizer::getSubField(value, 2,
                                            SDP_SUBFIELD_SEPARATORS, address);
         }
      }

      // Did not find a specific address try to find the default
      if(address->isNull())
      {
         iterator.reset();
         nv = findFieldNameBefore(&iterator, "c", "m");

         // Default address exists in the header
         if(nv)
         {
            value = nv->getValue();
            if(value)
            {
               UtlNameValueTokenizer::getSubField(value, 2,
                                               SDP_SUBFIELD_SEPARATORS, address);
            }
         }
      }

      if(!address->isNull())
      {
         // Check if there is a time to live attribute and remove it
         ttlIndex = address->index("/");
         if(ttlIndex >= 0)
         {
            address->remove(ttlIndex);
         }
      }
   }

   return(!address->isNull());
}

UtlBoolean SdpBody::getPtime(int mediaIndex, int& pTime) const
{
    UtlBoolean foundPtime = FALSE;
    UtlSListIterator iterator(*sdpFields);
    NameValuePair* nv;
    pTime = 0;
    const char* value = NULL;

    // Try to find a specific address for the given media set
    nv = positionFieldInstance(mediaIndex, &iterator, "m");
    if(nv)
    {
        UtlString aParameterName;
        UtlString pTimeValueString;
        while((nv = findFieldNameBefore(&iterator, "a", "m")))
        {
            value = nv->getValue();
            if(value)
            {
                // Get the a line parameter name
                UtlNameValueTokenizer::getSubField(value, 0,
                                      " \t:/", // separators
                                      &aParameterName);
                // See if this is a ptime parameter
                if(aParameterName.compareTo("ptime", UtlString::ignoreCase) == 0)
                {
                     // get the ptime value
                    UtlNameValueTokenizer::getSubField(value, 1,
                                                    " \t:/", // separators, 
                                                    &pTimeValueString);
                    if(!pTimeValueString.isNull())
                    {
                        pTime = atoi(pTimeValueString);
                        // should only be one ptime per media set (m line)
                        // Ignore all but the first one.
                        if(pTime > 0)
                        {
                            foundPtime = TRUE;
                            break;
                        }
                    }
                }
            }
        }
    }
    return(foundPtime);
}

#if 0 //{
void SdpBody::getBestAudioCodecs(int numRtpCodecs, SdpCodec rtpCodecs[],
                                 UtlString* rtpAddress, int* rtpPort,
                                 int* sendCodecIndex,
                                 int* receiveCodecIndex) const
{

   int mediaIndex = 0;
   UtlBoolean sendCodecFound = FALSE;
   UtlBoolean receiveCodecFound = FALSE;
   int numTypes;
   int payloadTypes[MAXIMUM_MEDIA_TYPES];
   int typeIndex;
   int codecIndex;

   rtpAddress->remove(0);
   *rtpPort = 0;
   *sendCodecIndex = -1;
   *receiveCodecIndex = -1;

   while(mediaIndex >= 0 &&
         (!sendCodecFound || !receiveCodecFound))
   {
      mediaIndex = findMediaType(SDP_AUDIO_MEDIA_TYPE, mediaIndex);

      if(mediaIndex >= 0)
      {
         getMediaPort(mediaIndex, rtpPort);

         if(*rtpPort >= 0)
         {
            getMediaPayloadType(mediaIndex, MAXIMUM_MEDIA_TYPES,
                                &numTypes, payloadTypes);
            for(typeIndex = 0; typeIndex < numTypes; typeIndex++)
            {
               // Until the real SdpCodec is needed we assume all of
               // the rtpCodecs are send AND receive.
               // We are also going to cheat and assume that all of
               // the media records are send AND receive
               for(codecIndex = 0; codecIndex < numRtpCodecs; codecIndex++)
               {
                  if(payloadTypes[typeIndex] ==
                     (rtpCodecs[codecIndex]).getCodecType())
                  {
                     sendCodecFound = TRUE;
                     receiveCodecFound = TRUE;
                     *sendCodecIndex = codecIndex;
                     *receiveCodecIndex = codecIndex;
                     getMediaAddress(mediaIndex, rtpAddress);
                     getMediaPort(mediaIndex, rtpPort);
                     break;
                  }
               }
               if(sendCodecFound && receiveCodecFound)
               {
                  break;
               }
            }
         }
         mediaIndex++;
      }
   }
}
#endif //}

// TODO: This interface is total broken and needs to be eliminated
// It munges the codecs and payload types accross all of the m lines
void SdpBody::getBestAudioCodecs(SdpCodecList& localRtpCodecs,
                                 int& numCodecsInCommon,
                                 SdpCodec**& commonCodecsForEncoder,
                                 //SdpCodec**& commonCodecsForDecoder,
                                 UtlString& rtpAddress, 
                                 int& rtpPort,
                                 int& rtcpPort,
                                 int& videoRtpPort,
                                 int& videoRtcpPort,
                                 SdpSrtpParameters& localSrtpParams,
                                 SdpSrtpParameters& matchingSrtpParams,
                                 int localBandwidth,
                                 int& matchingBandwidth,
                                 int localVideoFramerate,
                                 int& matchingVideoFramerate) const
{
   int mediaAudioIndex = 0;
   int mediaVideoIndex = 0;
   int numAudioTypes;
   int numVideoTypes;
   int bandwidth;
   int framerate;
   int audioPayloadTypes[MAXIMUM_MEDIA_TYPES];
   int videoPayloadTypes[MAXIMUM_MEDIA_TYPES];
   numCodecsInCommon = 0;
   int maxCodecs = sipx_max(getMediaSetCount() * MAXIMUM_MEDIA_TYPES, localRtpCodecs.getCodecCount());
   commonCodecsForEncoder = new SdpCodec*[maxCodecs];
   SdpCodec** commonCodecsForDecoder = new SdpCodec*[maxCodecs];
   for(int codecIndex = 0; codecIndex < maxCodecs; codecIndex++)
   {
       commonCodecsForDecoder[codecIndex] = NULL;
       commonCodecsForEncoder[codecIndex] = NULL;

   }
   SdpSrtpParameters remoteSrtpParams;

   memset((void*)&remoteSrtpParams, 0, sizeof(SdpSrtpParameters));
   memset((void*)&matchingSrtpParams, 0, sizeof(SdpSrtpParameters));
   rtpAddress.remove(0);
   rtpPort = 0;
   videoRtpPort = 0 ;

   while(mediaAudioIndex >= 0 || mediaVideoIndex >=0)
   {
      mediaAudioIndex = findMediaType(SDP_AUDIO_MEDIA_TYPE, mediaAudioIndex);
      mediaVideoIndex = findMediaType(SDP_VIDEO_MEDIA_TYPE, mediaVideoIndex);

      getBandwidthField(bandwidth);
      getBandwidthInCommon(localBandwidth, bandwidth, matchingBandwidth);

      getFramerateField(mediaVideoIndex, framerate);
      getVideoFramerateInCommon(localVideoFramerate, framerate, matchingVideoFramerate);

      if(mediaAudioIndex >= 0)
      {
         // This is kind of a bad assumption if there is more
         // than one media field, each might have a different
         // port and address
         getMediaPort(mediaAudioIndex, &rtpPort);
         getMediaRtcpPort(mediaAudioIndex, &rtcpPort);
         getSrtpCryptoField(mediaAudioIndex, 1, remoteSrtpParams);
         getMediaPort(mediaVideoIndex, &videoRtpPort);
         getMediaRtcpPort(mediaVideoIndex, &videoRtcpPort);
         getMediaAddress(mediaAudioIndex, &rtpAddress);

         if(rtpPort >= 0)
         {
            getMediaPayloadType(mediaAudioIndex, MAXIMUM_MEDIA_TYPES,
                                &numAudioTypes, audioPayloadTypes);

            getMediaPayloadType(mediaVideoIndex, MAXIMUM_MEDIA_TYPES,
                                &numVideoTypes, videoPayloadTypes);

            // TODO:
            // This method is broken as it munges information from multiple media lines
            // This should be removed.
            getCodecsInCommon(mediaAudioIndex,
                              mediaVideoIndex,
                              numAudioTypes,
                              numVideoTypes,
                              audioPayloadTypes,
                              videoPayloadTypes,
                              videoRtpPort,
                              localRtpCodecs,
                              numCodecsInCommon,
                              commonCodecsForEncoder,
                              commonCodecsForDecoder);

            getEncryptionInCommon(localSrtpParams, remoteSrtpParams, matchingSrtpParams);

            if (numCodecsInCommon >0)
               break;
         }
         mediaAudioIndex++;
         mediaVideoIndex++;
      }
   }

   if(commonCodecsForDecoder)
   {
      SdpCodecList::freeArray(maxCodecs, commonCodecsForDecoder);
   }
}


void SdpBody::getEncryptionInCommon(SdpSrtpParameters& audioParams,
                                    SdpSrtpParameters& remoteParams,
                                    SdpSrtpParameters& commonAudioParams) const
{
    memset(&commonAudioParams, 0, sizeof(SdpSrtpParameters));

    // Test for same cipher type and remote encryption being on
    if (audioParams.cipherType == remoteParams.cipherType && remoteParams.securityLevel)
    {
        // If we agree on a cipherType then the caller controls
        // if encryption is on - even if we switched it off locally.
        memcpy(&commonAudioParams, &remoteParams, sizeof(SdpSrtpParameters));
        // Set encryption on sending and receiving - if we can figure out
        // how to negotiate this we can be more selective
        commonAudioParams.securityLevel |= SRTP_SEND | SRTP_RECEIVE;
    }
}


void SdpBody::getBandwidthInCommon(int localBandwidth,
                                   int remoteBandwidth,
                                   int& commonBandwidth) const
{
    if (localBandwidth != 0 && remoteBandwidth != 0)
    {
        commonBandwidth = (remoteBandwidth <= localBandwidth) ? 
                                remoteBandwidth : localBandwidth;
    }
    else if (remoteBandwidth != 0)
    { 
        commonBandwidth = remoteBandwidth;
    }
    else
    {
        commonBandwidth = localBandwidth;
    }
}

void SdpBody::getVideoFramerateInCommon(int localVideoFramerate,
                                        int remoteVideoFramerate,
                                        int& commonVideoFramerate) const
{
    if (remoteVideoFramerate != 0 && remoteVideoFramerate < localVideoFramerate)
    {
        commonVideoFramerate = remoteVideoFramerate;
    }
    else
    {
        commonVideoFramerate = localVideoFramerate;
    }
}

UtlBoolean SdpBody::getMediaLine(int remoteMediaLineIndex, SdpMediaLine& mediaLine, const SdpCodecList* codecFactory) const
{

    // Get the port, media type and codec payload IDs for this media line
    // Get the codecs definitions for the payload IDs for this media line
    return(SdpHelper::getMediaLine(*this, remoteMediaLineIndex, mediaLine, codecFactory));
}

void SdpBody::addMediaLine(const SdpMediaLine& mediaLine, SdpBody& sdpBody)
{
    SdpMediaLine::SdpMediaType mimeTypeEnum = mediaLine.getMediaType();
    UtlString mimeType = SdpMediaLine::getStringForMediaType(mimeTypeEnum);
    int mediaPort = 0;
    // TODO: check for optional additional mediaPortPairs
    int mediaPortPairs = 1;
    UtlString mediaTransportType = SdpMediaLine::getStringForTransportProtocolType(mediaLine.getTransportProtocolType());

    const SdpMediaLine::SdpConnection* connection = 
        (SdpMediaLine::SdpConnection*) mediaLine.getConnections().first();
    if(connection)
    {
        mediaPort = connection->getPort();
    }

    const SdpCodecList* codecList = mediaLine.getCodecs();
    int numCodecs;
    SdpCodec** codecArray = NULL;
    codecList->getCodecs(numCodecs, codecArray);
    int* payloadTypes = new int[numCodecs];
    for(int codecIndex = 0; codecIndex < numCodecs; codecIndex++)
    {
        payloadTypes[codecIndex] = codecArray[codecIndex]->getCodecPayloadFormat();
    }

    addMediaData(mimeType,
                 mediaPort, 
                 mediaPortPairs,
                 mediaTransportType,
                 numCodecs,
                 payloadTypes);

    // Address
    if(connection)
    {
        UtlString address = connection->getAddress();
        if(!address.isNull())
        {
            addConnectionAddress(connection->getNetTypeToken(), connection->getAddressTypeToken(), address);
        }
    }

    // TODO: Crypto
    //addSrtpCryptoField(commonAudioSrtpParams);

    // RTCP port
    const SdpMediaLine::SdpConnection* rtcpConnection =
        (SdpMediaLine::SdpConnection*) mediaLine.getRtcpConnections().first();
    if(rtcpConnection)
    {
        int rtcpPort = rtcpConnection->getPort();
        // Non-standard RTCP port
        if(rtcpPort > 0 &&
           (
            ((rtcpPort % 2) == 0) || // not odd number
            (rtcpPort != mediaPort + 1) // not port after RTP
           )
          )
        {
            // So we need to add an a record with the RTCP port number
            UtlString rtcpRecord("rtcp:");
            rtcpRecord.appendFormat("%d", rtcpPort);
            addValue("a", rtcpRecord);
        }
    }

    // TODO:: TCP support

    // TODO: Candidate address/ports

    if(numCodecs)
    {
        addCodecParameters(numCodecs, codecArray, mimeType);
    }
//dddd

    SdpCodecList::freeArray(numCodecs, codecArray);
    delete[] payloadTypes;
}

int SdpBody::getCodecsInCommon(const SdpMediaLine& localMediaLine, 
                               int remoteMediaLineIndex,
                               SdpMediaLine& remoteMediaLine,
                               SdpCodecList& localDecodeCodecs) const
{
    // This SDP is assumed to be the remote offer

    int numCodecsInCommon = 0;
    // Get the codecs from the offer
    // Cheat the const on localMediaLine
    const SdpCodecList* localCodecList = ((SdpMediaLine&)localMediaLine).getCodecs();

    if(getMediaLine(remoteMediaLineIndex, remoteMediaLine, localCodecList))
    {
        if(remoteMediaLine.getMediaType() == localMediaLine.getMediaType())
        {
            // Remove codecs that are not supported by this side
            SdpCodecList* remoteCodecList = (SdpCodecList*)remoteMediaLine.getCodecs();
            remoteCodecList->limitCodecs(*localCodecList);
            numCodecsInCommon = remoteCodecList->getCodecCount();

            // Need to copy payload IDs from remote codec list into local codec list
            // so that we do the friendly/interop thing and use the same payload ID as the
            // remote side where possible.
            localDecodeCodecs = *localCodecList;
            localDecodeCodecs.copyPayloadTypes(*remoteCodecList);
        }
        else
        {
            OsSysLog::add(FAC_NET, PRI_ERR,
                "SdpBody::getCodecsInCommon local m line type: %d remote m line type: %d differ for index: %d",
                localMediaLine.getMediaType(), remoteMediaLine.getMediaType(), remoteMediaLineIndex);
        }
    }
    else
    {
        localDecodeCodecs.clearCodecs();
        OsSysLog::add(FAC_NET, PRI_ERR,
            "SdpBody::getCodecsInCommon media line index: %d does not exist", remoteMediaLineIndex);
    }

    return(numCodecsInCommon);
}

// TODO: This interface is total broken and needs to be eliminated
// It munges the codecs and payload types accross all of the m lines
void SdpBody::getCodecsInCommon(int audioMediaSetIndex,
                                int videoMediaSetIndex,
                                int audioPayloadIdCount,
                                int videoPayloadIdCount,
                                int audioPayloadTypes[],
                                int videoPayloadTypes[],
                                int videoRtpPort,
                                SdpCodecList& localRtpCodecs,
                                int& numCodecsInCommon,
                                SdpCodec* commonCodecsForEncoder[],
                                SdpCodec* commonCodecsForDecoder[]) const
{
   //printf("getCodecsInCommon audioMediaSetIndex: %d videoMediaSetIndex: %d audioPayloadIdCount: %d videoPayloadIdCount: %d\n",
   //         audioMediaSetIndex, videoMediaSetIndex, audioPayloadIdCount, videoPayloadIdCount);
   UtlString mimeSubtype;
   UtlString fmtp;
   int sampleRate;
   int numChannels;
   const SdpCodec* matchingCodec = NULL;
   int typeIndex;
   UtlBoolean commonCodec;
   int videoSizes[MAXIMUM_VIDEO_SIZES];
   int numVideoSizes;

   numCodecsInCommon = 0;
   memset((void*)videoSizes, 0, sizeof(videoSizes));

   // TODO: properly support media sets
   // This is a bit of a mess.  We have completely blurred over the
   // concept of separate m lines other than the mime type separation
   // of audio and video.  There may be different properties for the
   // same codecs which may appear in more than one media section
   // (m line)

   // So for example the ptime property is specific to a media
   // set (m line).  At this point we no longer know which codec
   // comes from which media set.  So for now we search through the
   // audio media sets until we find a ptime and assume the ptime
   // applies to all audio codecs.  ptime mostly only make sense for
   // audio
   int defaultPtime = 0;
   UtlString mediaType;
   for(int mediaSetIndex = 0; getMediaType(mediaSetIndex, &mediaType); mediaSetIndex++)
   {
       if(mediaType.compareTo(MIME_TYPE_AUDIO, UtlString::ignoreCase) == 0)
       {
           if(getPtime(mediaSetIndex, defaultPtime) &&
               defaultPtime > 0)
           {
               break;
           }
       }
   }

   for(typeIndex = 0; typeIndex < audioPayloadIdCount; typeIndex++)
   {
      // Until the real SdpCodec is needed we assume all of
      // the rtpCodecs are send AND receive.
      // We are also going to cheat and assume that all of
      // the media records are send AND receive

      // Get the rtpmap for the payload type
      if(getPayloadRtpMap(audioMediaSetIndex, audioPayloadTypes[typeIndex],
                          mimeSubtype, sampleRate, numChannels))
      {
         int codecMode;
         getPayloadFormat(audioMediaSetIndex, audioPayloadTypes[typeIndex], fmtp);
         SdpCodec::getFmtpParameter(fmtp, "mode", codecMode);
  
         // Workaround RFC bug with G.722 samplerate.
         // Read RFC 3551 Section 4.5.2 "G722" for details.
         if (mimeSubtype.compareTo("g722", UtlString::ignoreCase) == 0)
         {
            sampleRate = 16000;
         }
         
         // Find a match for the mime type
         matchingCodec = localRtpCodecs.getCodec(MIME_TYPE_AUDIO,
                                                 mimeSubtype.data(),
                                                 sampleRate,
                                                 numChannels,
                                                 fmtp);
         if (matchingCodec != NULL)
         {
            int frameSize = 0;
            commonCodec = TRUE;

            if (matchingCodec->getCodecType() == SdpCodec::SDP_CODEC_ILBC)
            {
                frameSize = codecMode;
                //OsSysLog::add(FAC_NET, PRI_DEBUG,
                //              "SdpBody::getCodecsInCommon ILBC codec framesize: %d",
                //              frameSize);
                if(frameSize == -1)
                {
                    // Unspecified mode assume 30
                    frameSize = 30;
                }
                else if (frameSize == 20 || frameSize == 30 )
                {
                }
                else if (frameSize == 0)
                {
                    frameSize = 20;
                }
                else
                {
                    // Nothing in common here
                    commonCodec = FALSE;
                }
            }
            else if(defaultPtime > 0)
            {
                frameSize = defaultPtime;
            }

            // Create a copy of the SDP codec and set
            // the payload type for it
            if (commonCodec)
            {
               commonCodecsForEncoder[numCodecsInCommon] = new SdpCodec(*matchingCodec);
               commonCodecsForDecoder[numCodecsInCommon] = new SdpCodec(*matchingCodec);

               // Let both decoder and encoder use the same payload type - this
               // is a friendly thing to do and minimize problems with other
               // clients.
               commonCodecsForEncoder[numCodecsInCommon]->setCodecPayloadFormat(audioPayloadTypes[typeIndex]);
               commonCodecsForDecoder[numCodecsInCommon]->setCodecPayloadFormat(audioPayloadTypes[typeIndex]);

               if (frameSize)
               {
                  commonCodecsForEncoder[numCodecsInCommon]->setPacketSize(frameSize*1000);
                  commonCodecsForDecoder[numCodecsInCommon]->setPacketSize(frameSize*1000);
               }

               if(!fmtp.isNull())
               {
                  commonCodecsForEncoder[numCodecsInCommon]->setSdpFmtpField(fmtp);
                  commonCodecsForDecoder[numCodecsInCommon]->setSdpFmtpField(fmtp);
               }
               numCodecsInCommon++;
            }
         

         }
      }

      // If no payload type set and this is a static payload
      // type assume the payload type is the same as our internal
      // codec id
      else if(audioPayloadTypes[typeIndex] <=
              SdpCodec::SDP_CODEC_MAXIMUM_STATIC_CODEC)
      {
         if((matchingCodec = localRtpCodecs.getCodecByType(audioPayloadTypes[typeIndex])))
         {
            // Create a copy of the SDP codec
            commonCodecsForEncoder[numCodecsInCommon] = new SdpCodec(*matchingCodec);
            commonCodecsForDecoder[numCodecsInCommon] = new SdpCodec(*matchingCodec);

            // Set the preferred frame size if ptime was set
            if (defaultPtime > 0)
            {
               commonCodecsForEncoder[numCodecsInCommon]->setPacketSize(defaultPtime*1000);
               commonCodecsForDecoder[numCodecsInCommon]->setPacketSize(defaultPtime*1000);
            }

            numCodecsInCommon++;
         }
      }
   }
#ifdef TEST_PRINT
   OsSysLog::add(FAC_NET, PRI_DEBUG, "SdpBody::getCodecsInCommon videoRtpPort=%d", videoRtpPort);
#endif

   if (videoRtpPort != 0)
   {
        for(typeIndex = 0; typeIndex < videoPayloadIdCount; typeIndex++)
        {
            SdpCodec** sdpCodecArray;
            int numCodecs;
            int codecIndex;
            int videoSize;
            // Until the real SdpCodec is needed we assume all of
            // the rtpCodecs are send AND receive.
            // We are also going to cheat and assume that all of
            // the media records are send AND receive

            mimeSubtype = "";
            numVideoSizes = 0;
            // Get the rtpmap for the payload type
            UtlBoolean bHasRtpMap = getPayloadRtpMap(videoMediaSetIndex, videoPayloadTypes[typeIndex],
                                                     mimeSubtype, sampleRate, numChannels);
                            // Get the video sizes in an array
            UtlBoolean bHasFmtp= getPayloadFormat(videoMediaSetIndex, videoPayloadTypes[typeIndex], fmtp);
            SdpCodec::getVideoSizes(fmtp, MAXIMUM_VIDEO_SIZES, numVideoSizes, videoSizes);

            UtlBoolean isH264 = (mimeSubtype.compareTo(MIME_SUBTYPE_H264, UtlString::ignoreCase) == 0);
            UtlString h264ProfileLevelId;
            UtlString h264LevelIdc;
            UtlString h264ProfileIdc;

            if(bHasFmtp && isH264)
            {
                SdpCodec::getFmtpParameter(fmtp, "profile-level-id", h264ProfileLevelId);
                // the last 2 of 6 hex chars of profile-level-id are level_idc
                h264LevelIdc = &(h264ProfileLevelId.data()[4]);
                h264LevelIdc.toLower();
                // we ignore the middle 2 of 6 hex chars for now (profile-iop)
                // first 2 of 6 hex chars of profile-level-id are profile_idc
                h264ProfileIdc = h264ProfileLevelId;
                h264ProfileIdc.remove(2);
            }
            else
            {
                h264LevelIdc = "";
                h264ProfileIdc = "";
            }

            if (bHasRtpMap || bHasFmtp)
            {
                // Fixing iChat case where there is no rtpmap field for H263
                if (mimeSubtype.length() == 0 && videoPayloadTypes[typeIndex] == 34)
                {
                    mimeSubtype = "h263";
                }

                // Get all codecs with the same mime subtype. Same codecs with different
                // video resolutions are added separately but have the same mime subtype.
                // The codec negotiation depends on the fact that codecs with the same
                // mime subtype are added sequentially.
                localRtpCodecs.getCodecs(numCodecs, sdpCodecArray, SDP_VIDEO_MEDIA_TYPE, mimeSubtype);
#ifdef TEST_PRINT
                OsSysLog::add(FAC_NET, PRI_DEBUG, "SdpBody::getCodecsInCommon got %d local video codec(s)", numCodecs);
#endif

                // Loop through the other side's video sizes first so we match the size
                // preferences of the other side
                if (numVideoSizes == 0)
                {
                    // If no size parameter default to QCIF
                    numVideoSizes = 1;
                    videoSizes[0] = SDP_VIDEO_FORMAT_QCIF;
                }

                UtlString matchingCodecH264ProfileLevelId;
                UtlString matchingCodecH264ProfileIdc;
                UtlString priorH264ProfileIdc;

                for (videoSize = 0; videoSize < numVideoSizes; videoSize++)
                {
                    UtlBoolean matchAlreadyFound = FALSE;

                    for (codecIndex = 0; codecIndex < numCodecs; ++codecIndex)
                    {
                        matchingCodec = sdpCodecArray[codecIndex];

                        if(!h264ProfileIdc.isNull() && matchingCodec)
                        {
                            matchingCodec->getFmtpParameter("profile-level-id", matchingCodecH264ProfileLevelId);
                            matchingCodecH264ProfileIdc = &(matchingCodecH264ProfileLevelId.data()[4]);
                            matchingCodecH264ProfileIdc.toLower();
                        }
                        else
                        {
                            matchingCodecH264ProfileIdc = "";
                        }

                        // In addition to everything else do a bit-wise comparison of video formats. For
                        // every codec with the same sub mime type that supports one of the video formats
                        // add a seperate codec in the codecsInCommonArray.  For H.264 size is not important.
                        if((matchingCodec != NULL) && 
                           // Render sizes for non-H264 video codecs
                           ((matchingCodec->getVideoFormat() == videoSizes[videoSize]) ||  

                            // H264 specific fmpt parameter matching
                            (isH264 &&
                             matchingCodec->isFmtpParameterSame(fmtp, "packetization-mode", "0") &&
                             !h264LevelIdc.isNull() &&
                             matchingCodecH264ProfileLevelId.index(h264ProfileIdc) == 0
                            )

                           ) &&
                           // Sample rate and num channels are pretty meaningless for video
                           (matchingCodec->getSampleRate() == sampleRate ||
                            sampleRate == -1) &&
                            (matchingCodec->getNumChannels() == numChannels ||
                            numChannels == -1
                            ))
                        {
                            if(matchAlreadyFound)
                            {
                                // For now we are assuming that we want the closest match on the level-idc that is equal or
                                // greater than that of the remote side's.  We may find that we want to be less than, but
                                // for now we are going with greater than.  So there are 3 cases when the new codec is 
                                // better than the old one given that we have 3 codecs: the remote one, the prior matched one
                                // and the new matched one:
                                //     new >= remote > prior
                                //     remote >= new > prior
                                //     prior > new >= remote

                                // If this match is better than the prior, take this one instead
                                if((matchingCodecH264ProfileIdc.compareTo(h264ProfileIdc, UtlString::ignoreCase) >= 0 &&
                                      h264ProfileIdc.compareTo(priorH264ProfileIdc, UtlString::ignoreCase) > 0) ||
                                   (h264ProfileIdc.compareTo(matchingCodecH264ProfileIdc, UtlString::ignoreCase) >= 0 &&
                                      matchingCodecH264ProfileIdc.compareTo(priorH264ProfileIdc, UtlString::ignoreCase) > 0) ||
                                   (priorH264ProfileIdc.compareTo(matchingCodecH264ProfileIdc, UtlString::ignoreCase) > 0 &&
                                      matchingCodecH264ProfileIdc.compareTo(h264ProfileIdc, UtlString::ignoreCase) >= 0))
                                {
                                    OsSysLog::add(FAC_NET, PRI_DEBUG,
                                        "SdpBody::getCodecsInCommon match better than prior profile-idc, prior: %s better: %s remote: %s",
                                        priorH264ProfileIdc.data(), matchingCodecH264ProfileIdc.data(), h264ProfileIdc.data());

                                    // Copy over prior added codec as this one is a better match
                                    *(commonCodecsForEncoder[numCodecsInCommon - 1]) = *matchingCodec;
                                    *(commonCodecsForDecoder[numCodecsInCommon - 1]) = *matchingCodec;

                                    commonCodecsForEncoder[numCodecsInCommon - 1]->setCodecPayloadFormat(videoPayloadTypes[typeIndex]);
                                    // decoder will use our SDP payload IDs, not those we received

                                    if(bHasFmtp)
                                    {
                                        commonCodecsForEncoder[numCodecsInCommon - 1]->setSdpFmtpField(fmtp);
                                    }

                                    priorH264ProfileIdc = matchingCodecH264ProfileIdc;
                                }
                                else
                                {
                                    OsSysLog::add(FAC_NET, PRI_DEBUG,
                                        "SdpBody::getCodecsInCommon NOT better than prior profile-idc, prior: %s better: %s remote: %s",
                                        priorH264ProfileIdc.data(), matchingCodecH264ProfileIdc.data(), h264ProfileIdc.data());
                                }
                            }

                            else
                            {
                                // Create a copy of the SDP codec and set
                                // the payload type for it
                                commonCodecsForEncoder[numCodecsInCommon] = new SdpCodec(*matchingCodec);
                                commonCodecsForDecoder[numCodecsInCommon] = new SdpCodec(*matchingCodec);

                                commonCodecsForEncoder[numCodecsInCommon]->setCodecPayloadFormat(videoPayloadTypes[typeIndex]);
                                // decoder will use our SDP payload IDs, not those we received

                                if(bHasFmtp)
                                {
                                    commonCodecsForEncoder[numCodecsInCommon]->setSdpFmtpField(fmtp);
                                }

                                priorH264ProfileIdc = matchingCodecH264ProfileIdc;
                                numCodecsInCommon++;
                                matchAlreadyFound = TRUE;
                            }

                        }
                        else
                        {
                            if(matchingCodec == NULL)
                            {
                                OsSysLog::add(FAC_NET, PRI_DEBUG,
                                    "SdpBody::getCodecsInCommon matchingCodec NULL");
                            }
                            else
                            {
                                UtlString matchFmtp;
                                matchingCodec->getSdpFmtpField(matchFmtp);
                                OsSysLog::add(FAC_NET, PRI_DEBUG, 
                                    "SdpBody::getCodecsInCommon codec does not match mime subtype: %s match rate: %d remote rate: %d match channels: %d remote channels: %d match profile-level-id: %s remote level-idc: %s \nmatch fmtp: %s \nremote fmtp: %s",
                                    mimeSubtype.data(), matchingCodec->getSampleRate(), sampleRate, matchingCodec->getNumChannels(), numChannels,
                                    matchingCodecH264ProfileLevelId.data(), h264ProfileIdc.data(), matchFmtp.data(), fmtp.data());
                            }
                        }
                    }
                }
                // Delete the codec array we got to loop through codecs with the same mime subtype
                for (codecIndex = 0; codecIndex < numCodecs; ++codecIndex)
                {
                    if (sdpCodecArray[codecIndex])
                    {
                        delete sdpCodecArray[codecIndex];
                        sdpCodecArray[codecIndex] = NULL;
                    }
                }
                delete[] sdpCodecArray;
            }

            // If no payload type set and this is a static payload
            // type assume the payload type is the same as our internal
            // codec id
            else if(videoPayloadTypes[typeIndex] <=
                    SdpCodec::SDP_CODEC_MAXIMUM_STATIC_CODEC)
            {
                if((matchingCodec = localRtpCodecs.getCodecByType(videoPayloadTypes[typeIndex])))
                {
                    // Create a copy of the SDP codec and set
                    // the payload type for it
                    commonCodecsForEncoder[numCodecsInCommon] = new SdpCodec(*matchingCodec);
                    commonCodecsForDecoder[numCodecsInCommon] = new SdpCodec(*matchingCodec);

                    numCodecsInCommon++;
                }
            }
        }
   }
}


void SdpBody::addCodecsOffer(int iNumAddresses,
                             const UtlString mediaAddresses[],
                             int rtpAudioPorts[],
                             int rtcpAudioPorts[],
                             int rtpVideoPorts[],
                             int rtcpVideoPorts[],
                             RTP_TRANSPORT transportTypes[],
                             int numRtpCodecs,
                             SdpCodec* rtpCodecs[],
                             SdpSrtpParameters& srtpParams,
                             int totalBandwidth,
                             int videoFramerate,
                             RTP_TRANSPORT transportOffering)
{
    int codecArray[MAXIMUM_MEDIA_TYPES];
    int formatArray[MAXIMUM_MEDIA_TYPES];
    UtlString videoFormat;
    int codecIndex;
    int destIndex;
    int firstMimeSubTypeIndex = 0;
    int preExistingMedia = getMediaSetCount();
    UtlString mimeType;
    UtlString seenMimeType;
    UtlString mimeSubType;
    UtlString prevMimeSubType = "none";
    int numAudioCodecs=0;
    int numVideoCodecs=0;
    const char* szTransportType = NULL;

    assert(iNumAddresses > 0) ;

    memset(formatArray, 0, sizeof(int)*MAXIMUM_MEDIA_TYPES);

    // If there are not media fields we only need one global one
    // for the SDP body
    if(!preExistingMedia)
    {
        setConnectionAddress(mediaAddresses[0]);
        char timeString[100];
        sprintf(timeString, "%d %d", 0, //OsDateTime::getSecsSinceEpoch(),
            0);
        addValue("t", timeString);
    }

    // Stuff the SDP audio codes in an integer array
    for(codecIndex = 0, destIndex = 0;
        codecIndex < MAXIMUM_MEDIA_TYPES && codecIndex < numRtpCodecs;
        codecIndex++)
    {
        rtpCodecs[codecIndex]->getMediaType(mimeType);
        if (mimeType.compareTo(SDP_AUDIO_MEDIA_TYPE, UtlString::ignoreCase) == 0 || mimeType.compareTo(SDP_VIDEO_MEDIA_TYPE, UtlString::ignoreCase) != 0)
        {
            seenMimeType = mimeType;
            ++numAudioCodecs;
            codecArray[destIndex++] =
                (rtpCodecs[codecIndex])->getCodecPayloadFormat();
        }
    }

    if (rtpAudioPorts[0] > 0)
    {
        // If any security is enabled we set RTP/SAVP and add a crypto field
        if (srtpParams.securityLevel)
        {
            // Add the media record
            addMediaData(SDP_AUDIO_MEDIA_TYPE, rtpAudioPorts[0], 1,
                SDP_SRTP_MEDIA_TRANSPORT_TYPE, numAudioCodecs,
                codecArray);

            // The following are for RTSP support as the players seem to want to have
            // these fields in the SDP
            addValue("a", "control:trackID=1");

            // If this is not the only media record we do need a local
            // address record for this media record
            if(preExistingMedia)
            {
                addConnectionAddress(mediaAddresses[0]);
            }
            addSrtpCryptoField(srtpParams);
        }
        else
        {
            if ((transportTypes[0] & RTP_TRANSPORT_UDP) == RTP_TRANSPORT_UDP)
            {
                szTransportType = SDP_RTP_MEDIA_TRANSPORT_TYPE;
            }                        
            else
            {
                szTransportType = SDP_RTP_TCP_MEDIA_TRANSPORT_TYPE;
            }
            // Add the media record
            addMediaData(SDP_AUDIO_MEDIA_TYPE, rtpAudioPorts[0], 1,
                szTransportType, numAudioCodecs,
                codecArray);

            // The following are for RTSP support as the players seem to want to have
            // these fields in the SDP
            addValue("a", "control:trackID=1");

            // If this is not the only media record we do need a local
            // address record for this media record
            if(preExistingMedia)
            {
                addConnectionAddress(mediaAddresses[0]);
            }                        
        }
        // It is assumed that rtcp is the odd port immediately after the rtp port.
        // If that is not true, we must add a parameter to specify the rtcp port.
        if (  (rtcpAudioPorts[0] > 0) && ((rtcpAudioPorts[0] != rtpAudioPorts[0] + 1) 
            || (rtcpAudioPorts[0] % 2) == 0))
        {
            char cRtcpBuf[32] ;
            sprintf(cRtcpBuf, "rtcp:%d", rtcpAudioPorts[0]) ;
            addValue("a", cRtcpBuf) ;
        }

        // Add candidate addresses if available
        if (iNumAddresses > 1)
        {
            char szTransportString[16];
            // http://tools.ietf.org/html/draft-ietf-mmusic-ice-17
            // currently only UDP is defined
            strcpy(szTransportString, "UDP");

            for (int i=0; i<iNumAddresses; i++)
            {
                if ((transportTypes[i] & RTP_TRANSPORT_UDP) == RTP_TRANSPORT_UDP)
                {
                    szTransportType = SDP_RTP_MEDIA_TRANSPORT_TYPE;
                }                        
                else
                {
                    szTransportType = SDP_RTP_TCP_MEDIA_TRANSPORT_TYPE;
                }            
                uint64_t priority = (iNumAddresses-i) + PRIORITY_OFFSET;

                assert(mediaAddresses[i].length() > 0) ;
                if (rtpAudioPorts[0] && rtpAudioPorts[i] && mediaAddresses[i])
                {
                    addCandidateAttribute(i, "t", szTransportString, priority, mediaAddresses[i], rtpAudioPorts[i]) ;
                }

                if (rtcpAudioPorts[0] && rtcpAudioPorts[i] && mediaAddresses[i])
                {
                    addCandidateAttribute(i, "t", szTransportString, priority, mediaAddresses[i], rtcpAudioPorts[i]) ;
                }
            }
        }

        // add attribute records defining the extended types
        addCodecParameters(numRtpCodecs, rtpCodecs, seenMimeType); //, videoFramerate);

    }

    // Stuff the SDP video codecs codes in an integer array
    for(codecIndex = 0, destIndex = -1;
        codecIndex < MAXIMUM_MEDIA_TYPES && codecIndex < numRtpCodecs;
        codecIndex++)
    {
        rtpCodecs[codecIndex]->getMediaType(mimeType);

        rtpCodecs[codecIndex]->getEncodingName(mimeSubType);
//#ifdef TEST_PRINT
        OsSysLog::add(FAC_NET, PRI_DEBUG, "SdpBody::addCodecsOffer (video loop) codec %s mime type: %s payload ID: %d\n", 
            mimeSubType.data(), mimeType.data(), rtpCodecs[codecIndex]->getCodecPayloadFormat());
//#endif

        if (mimeType.compareTo(SDP_VIDEO_MEDIA_TYPE, UtlString::ignoreCase) == 0)
        {
            // printf("found video codec\n");
            rtpCodecs[codecIndex]->getEncodingName(mimeSubType);

            //  Cannot combine H264 fmtp strings as they will conflict
            // Not even sure this makes sense for other video codecs either
            if ((mimeSubType.compareTo(prevMimeSubType, UtlString::ignoreCase) == 0) && (mimeSubType.compareTo(MIME_SUBTYPE_H264, UtlString::ignoreCase) != 0))
            {
                // printf("duplicate mime subtype\n");
                // If we still have the same mime type only change format. We're depending on the
                // fact that codecs with the same mime subtype are added sequentially to the 
                // codec factory. Otherwise this won't work.
                formatArray[destIndex] |= (rtpCodecs[codecIndex])->getVideoFormat();
                (rtpCodecs[firstMimeSubTypeIndex])->setVideoFmtp(formatArray[destIndex]);
                (rtpCodecs[firstMimeSubTypeIndex])->setVideoFmtpString((rtpCodecs[codecIndex])->getVideoFormat());
            }
            else
            {
                // TODO: Pick highest capability H264 codec and eliminate duplicates

                // printf("new format\n");
                // New mime subtype - add new codec to codec list. Mark this index and put all
                // video format information into this codec because it will be looked at later.
                firstMimeSubTypeIndex = codecIndex;
                ++destIndex;
                prevMimeSubType = mimeSubType;
                ++numVideoCodecs;
                formatArray[destIndex] = (rtpCodecs[codecIndex])->getVideoFormat();
                codecArray[destIndex] =
                    (rtpCodecs[codecIndex])->getCodecPayloadFormat();
                (rtpCodecs[firstMimeSubTypeIndex])->setVideoFmtp(formatArray[destIndex]);
                (rtpCodecs[firstMimeSubTypeIndex])->clearVideoFmtpString();
                (rtpCodecs[firstMimeSubTypeIndex])->setVideoFmtpString((rtpCodecs[codecIndex])->getVideoFormat());
            }
        }
    }

    // printf("rtpVideoPorts[0]=%d\n", rtpVideoPorts[0]);
    if (rtpVideoPorts[0] > 0)
    {
        // If any security is enabled we set RTP/SAVP and add a crypto field - not for video at this time
        if (0)
        {
            // Add the media record
            addMediaData(SDP_VIDEO_MEDIA_TYPE, rtpVideoPorts[0], 1,
                SDP_SRTP_MEDIA_TRANSPORT_TYPE, numVideoCodecs,
                codecArray);

            // The following are for RTSP support as the players seem to want to have
            // these fields in the SDP
            addValue("a", "control:trackID=2");

            addSrtpCryptoField(srtpParams);
            // If this is not the only media record we do need a local
            // address record for this media record
            if(preExistingMedia)
            {
                addConnectionAddress(mediaAddresses[1]);
            }         
        }
        else
        {
            if ((transportTypes[0] & RTP_TRANSPORT_UDP) == RTP_TRANSPORT_UDP)
            {
                szTransportType = SDP_RTP_MEDIA_TRANSPORT_TYPE;
            }                        
            else
            {
                szTransportType = SDP_RTP_TCP_MEDIA_TRANSPORT_TYPE;
            }
            // Add the media record
            addMediaData(SDP_VIDEO_MEDIA_TYPE, rtpVideoPorts[0], 1,
                szTransportType, numVideoCodecs,
                codecArray);

            // The following are for RTSP support as the players seem to want to have
            // these fields in the SDP
            addValue("a", "control:trackID=2");

            // If this is not the only media record we do need a local
            // address record for this media record
            if(preExistingMedia)
            {
                addConnectionAddress(mediaAddresses[0]);
            }
        }
        // It is assumed that rtcp is the odd port immediately after the rtp port.
        // If that is not true, we must add a parameter to specify the rtcp port.
        if ((rtcpVideoPorts[0] > 0) && ((rtcpVideoPorts[0] != rtpVideoPorts[0] + 1) 
            || (rtcpVideoPorts[0] % 2) == 0))
        {
            char cRtcpBuf[32] ;
            sprintf(cRtcpBuf, "rtcp:%d", rtcpVideoPorts[0]) ;

            addValue("a", cRtcpBuf) ;
        }

        // Add candidate addresses if available
        if (iNumAddresses > 1)
        {
            char szTransportString[16];
            // http://tools.ietf.org/html/draft-ietf-mmusic-ice-17
            // currently only UDP is defined
            strcpy(szTransportString, "UDP");

            for (int i=0; i<iNumAddresses; i++)
            {
                uint64_t priority = (iNumAddresses-i) + PRIORITY_OFFSET;

                if ((transportTypes[0] & RTP_TRANSPORT_UDP) == RTP_TRANSPORT_UDP)
                {
                    szTransportType = SDP_RTP_MEDIA_TRANSPORT_TYPE;
                }                        
                else
                {
                    szTransportType = SDP_RTP_TCP_MEDIA_TRANSPORT_TYPE;
                }
                assert(mediaAddresses[i].length() > 0) ;
                if (rtpVideoPorts[0] && rtpVideoPorts[i] && mediaAddresses[i])
                {
                    addCandidateAttribute(i, "t", szTransportString, priority, mediaAddresses[i], rtpVideoPorts[i]) ;
                }

                if (rtcpVideoPorts[0] && rtcpVideoPorts[i] && mediaAddresses[i])
                {
                    addCandidateAttribute(i, "t", szTransportString, priority, mediaAddresses[i], rtcpVideoPorts[i]) ;
                }
            }
        }

        // add attribute records defining the extended types
        addCodecParameters(numRtpCodecs, rtpCodecs, SDP_VIDEO_MEDIA_TYPE); //, videoFramerate);

        if (totalBandwidth != 0)
        {
            char ct[16];
            sprintf(ct, "CT:%d", totalBandwidth);
            setValue("b", ct);
        }
    }
    else if(destIndex  >= 0)
    {
        OsSysLog::add(FAC_NET, PRI_WARNING, "SdpBody %d video codecs enabled, but video port: %d < 0", destIndex + 1, rtpVideoPorts[0]);
    }
}



void SdpBody::addCodecParameters(int numRtpCodecs,
                                 SdpCodec* rtpCodecs[],
                                 const char *szMimeType)
{
   const SdpCodec* codec = NULL;
   UtlString mimeSubtype;
   int payloadType;
   int sampleRate;
   int numChannels;
   int videoFmtp;
   UtlString formatParameters;
   UtlString mimeType;
   UtlString formatTemp;
   UtlString formatString;
   int pTime = 0;
   int codecPtime = 0;

   for(int codecIndex = 0;
       codecIndex < MAXIMUM_MEDIA_TYPES && codecIndex < numRtpCodecs;
       codecIndex++)
   {
      codec = rtpCodecs[codecIndex];
      rtpCodecs[codecIndex]->getMediaType(mimeType);
      if(codec && mimeType.compareTo(szMimeType, UtlString::ignoreCase) == 0)
      {
         codec->getEncodingName(mimeSubtype);
         sampleRate = codec->getSampleRate();
         numChannels = codec->getNumChannels();
         codec->getSdpFmtpField(formatParameters);
         payloadType = codec->getCodecPayloadFormat();
#ifdef TEST_PRINT
         OsSysLog::add(FAC_NET, PRI_DEBUG, "SdpBody::addCodecParameters adding codec: %s payload ID: %d",
            mimeSubtype.data(), payloadType);
#endif


         // Workarmund RFC bug with G.722 samplerate.
         // Read RFC 3551 Section 4.5.2 "G722" for details.
         if (codec->getCodecType() == SdpCodec::SDP_CODEC_G722)
         {
            sampleRate = 8000;
         }

         // Not sure what the right heuristic is for determining the
         // correct ptime.  ptime is a media (m line) parameters.  As such
         // it is a global property to all codecs for a single media
         // (m line) section.  For now lets try using the largest one as
         // SDP does not support different ptime for each codec.
         codecPtime = (codec->getPacketLength()) / 1000; // converted to milliseconds
         if(codecPtime > pTime)
         {
               pTime = codecPtime;
         }

         // Build an rtpmap
         addRtpmap(payloadType, mimeSubtype.data(), sampleRate, numChannels);

         // Non H264 video codecs need to construct format string.
         // H.264 codecs just use the configured fmtp string for the codec.
         if ((videoFmtp=codec->getVideoFmtp()) != 0 && mimeSubtype.compareTo(MIME_SUBTYPE_H264, UtlString::ignoreCase))
         {
            if (codec->getCodecPayloadFormat() != SdpCodec::SDP_CODEC_H263)
            {
               codec->getVideoFmtpString(formatString);
               formatTemp = "size:" + formatString;

               formatParameters = formatTemp(0, formatTemp.length()-1);
            }
            else
            {
               switch (codec->getCodecType())
               {
               case SdpCodec::SDP_CODEC_H263_CIF:
                  formatParameters = "imagesize 1";
                  break;
               case SdpCodec::SDP_CODEC_H263_QCIF:
                  formatParameters = "imagesize 0";
                  break;
               default:
                  break;
               }
            }
         }

         // Add the format specific parameters if present
         if(!formatParameters.isNull())
         {
               addFormatParameters(payloadType, formatParameters.data());
               /* Not quite sure if we want to send a global framerate limit
               if (codec->getCodecPayloadFormat() == SdpCodec::SDP_CODEC_H263 &&
                  videoFramerate != 0)
               {
                  sprintf(valueBuf, "framerate:%d", videoFramerate);
                  addValue("a", valueBuf);    
               }
               */
         }
      }
#ifdef TEST_PRINT
      else
      {
         if(codec)
         {
            codec->getEncodingName(mimeSubtype);
         }
         OsSysLog::add(FAC_NET, PRI_DEBUG, "SdpBody::addCodecParameters skipping codec MIME type: %s/%s payload: %d expecting: %s",
            mimeType.data(), mimeSubtype.data(), codec ? codec->getCodecPayloadFormat() : -1, szMimeType);
      }
#endif

   }

   // ptime only really make sense for audio
   if(pTime > 0 &&
      strcmp(szMimeType, SDP_AUDIO_MEDIA_TYPE) == 0 )
   {
       addPtime(pTime);
   }
}

void SdpBody::buildMediaLine(SdpMediaLine::SdpMediaType mediaType, 
                            int totalBandwidth, 
                            int iNumAddresses,
                            UtlString hostAddresses[],
                            int rtpPorts[],
                            int rtcpPorts[],
                            int numRtpCodecs,
                            SdpCodec* rtpCodecs[],
                            SdpMediaLine& mediaLine)
{
    mediaLine.setMediaType(mediaType);

    // TODO: Conditional upon TCP or SRTP parameters
    mediaLine.setTransportProtocolType(SdpMediaLine::PROTOCOL_TYPE_RTP_AVP);

    if(totalBandwidth > 0)
    {
        mediaLine.addBandwidth(Sdp::BANDWIDTH_TYPE_CT, totalBandwidth);
    }

    // Loop through the codecs and add them to media line
    for(int codecIndex = 0; codecIndex < numRtpCodecs; codecIndex++)
    {
        UtlString codecMediaType;
        rtpCodecs[codecIndex]->getMediaType(codecMediaType);
        if(codecMediaType.compareTo(SdpMediaLine::getStringForMediaType(mediaType), UtlString::ignoreCase) == 0)
        {
            mediaLine.addCodec(new SdpCodec(*rtpCodecs[codecIndex]));
        }
    }

    // Add the connection c=
    mediaLine.addConnection(Sdp::NET_TYPE_IN, 
        OsSocket::isIp4Address(hostAddresses[0]) ? Sdp::ADDRESS_TYPE_IP4 : Sdp::ADDRESS_TYPE_IP6,
        hostAddresses[0], rtpPorts[0]);
    mediaLine.addRtcpConnection(Sdp::NET_TYPE_IN,
        OsSocket::isIp4Address(hostAddresses[0]) ? Sdp::ADDRESS_TYPE_IP4 : Sdp::ADDRESS_TYPE_IP6,
        hostAddresses[0], rtcpPorts[0]);

    mediaLine.setDirection(rtpPorts[0] ? SdpMediaLine::DIRECTION_TYPE_SENDRECV : SdpMediaLine::DIRECTION_TYPE_INACTIVE);

    // Loop through the candidate port/addresses and add them to the m line
    if(iNumAddresses > 1)
    {
        for(int candidateIndex = 0; candidateIndex < iNumAddresses; candidateIndex++)
        {
            uint64_t priority = (iNumAddresses - candidateIndex) + PRIORITY_OFFSET;

            SdpCandidate* candidate = new SdpCandidate("t", candidateIndex, 
                SdpCandidate::CANDIDATE_TRANSPORT_TYPE_UDP, priority, 
                hostAddresses[candidateIndex], rtpPorts[candidateIndex]);

            mediaLine.addCandidate(candidate);
        }
    }

    // ptime not set for video.  I think this is correct.

}

void SdpBody::addCodecsAnswer(int iNumAddresses,
                             UtlString hostAddresses[],
                             int rtpAudioPorts[],
                             int rtcpAudioPorts[],
                             int rtpVideoPorts[],
                             int rtcpVideoPorts[],
                             RTP_TRANSPORT transportTypes[],
                             int numRtpCodecs, 
                             SdpCodec* rtpCodecs[], 
                             SdpSrtpParameters& srtpParams,
                             int totalBandwidth,
                             int videoFramerate,
                             const SdpBody* sdpRequest)
{
#if 1 //{
    const SdpMediaLine* mediaLines[2];
    int numMediaLines = 0;
    mediaLines[0] = NULL;
    mediaLines[1] = NULL;

    // ************* Audio M line *************
    int pTime = 0;
    SdpMediaLine audioMediaLine;
    audioMediaLine.setMediaType(SdpMediaLine::MEDIA_TYPE_AUDIO);

    //TODO: should this be conditional upon SdpSrtpParameters?
    // The old code assumed the tranport type from the offer.  Not sure what we should do here
    //(audioTransportType.compareTo(SDP_RTP_MEDIA_TRANSPORT_TYPE, UtlString::ignoreCase) == 0) ||
    //(audioTransportType.compareTo(SDP_RTP_TCP_MEDIA_TRANSPORT_TYPE, UtlString::ignoreCase) == 0) ||
    //(audioTransportType.compareTo(SDP_SRTP_MEDIA_TRANSPORT_TYPE, UtlString::ignoreCase) == 0))
    audioMediaLine.setTransportProtocolType(SdpMediaLine::PROTOCOL_TYPE_RTP_AVP);

    if(totalBandwidth > 0)
    {
        audioMediaLine.addBandwidth(Sdp::BANDWIDTH_TYPE_CT, totalBandwidth);
    }

    // Loop through the codecs and add them to media line
    for(int codecIndex = 0; codecIndex < numRtpCodecs; codecIndex++)
    {
        UtlString codecMediaType;
        rtpCodecs[codecIndex]->getMediaType(codecMediaType);
        if(codecMediaType.compareTo(SDP_AUDIO_MEDIA_TYPE, UtlString::ignoreCase) == 0)
        {
            audioMediaLine.addCodec(new SdpCodec(*rtpCodecs[codecIndex]));
            // convert micro seconds to milliseconds
            int codecPtime = (rtpCodecs[codecIndex]->getPacketLength()) / 1000;
            if(codecPtime > pTime)
            {
                pTime = codecPtime;
            }
        }
    }

    // Add the connection c=
    audioMediaLine.addConnection(Sdp::NET_TYPE_IN, 
        OsSocket::isIp4Address(hostAddresses[0]) ? Sdp::ADDRESS_TYPE_IP4 : Sdp::ADDRESS_TYPE_IP6,
        hostAddresses[0], rtpAudioPorts[0]);
    audioMediaLine.addRtcpConnection(Sdp::NET_TYPE_IN,
        OsSocket::isIp4Address(hostAddresses[0]) ? Sdp::ADDRESS_TYPE_IP4 : Sdp::ADDRESS_TYPE_IP6,
        hostAddresses[0], rtcpAudioPorts[0]);

    audioMediaLine.setDirection(rtpAudioPorts[0] ? SdpMediaLine::DIRECTION_TYPE_SENDRECV : SdpMediaLine::DIRECTION_TYPE_INACTIVE);

    // Loop through the candidate port/addresses and add them to the m line
    if(iNumAddresses > 1)
    {
        for(int candidateIndex = 0; candidateIndex < iNumAddresses; candidateIndex++)
        {
            uint64_t priority = (iNumAddresses - candidateIndex) + PRIORITY_OFFSET;

            SdpCandidate* candidate = new SdpCandidate("t", candidateIndex, 
                SdpCandidate::CANDIDATE_TRANSPORT_TYPE_UDP, priority, 
                hostAddresses[candidateIndex], rtpAudioPorts[candidateIndex]);
            audioMediaLine.addCandidate(candidate);
        }
    }

    // Not sure what the right heuristic is for determining the
    // correct ptime.  ptime is a media (m line) parameters.  As such
    // it is a global property to all codecs for a single media
    // (m line) section.  For now lets try using the largest one as
    // SDP does not support different ptime for each codec.
    if(pTime > 0)
    {
        audioMediaLine.setPacketTime(pTime);
    }


   // ************* Video M line *************

    SdpMediaLine videoMediaLine;

    buildMediaLine(SdpMediaLine::MEDIA_TYPE_VIDEO,
                   totalBandwidth,
                   iNumAddresses,
                   hostAddresses,
                   rtpVideoPorts,
                   rtcpVideoPorts,
                   numRtpCodecs,
                   rtpCodecs,
                   videoMediaLine);

    // TODO: videoFramerate  Does not look like this was ever set

    // Build the array of m lines for the streams supported on this side
    if(rtpAudioPorts[0])
    {
        mediaLines[numMediaLines] = &audioMediaLine;
        numMediaLines++;
    }
    if(rtpVideoPorts[0])
    {
        mediaLines[numMediaLines] = &videoMediaLine;
        numMediaLines++;
    }

    addMediaLinesAnswer(numMediaLines, mediaLines, *sdpRequest);

#endif // } 
#if 0 //{
   int preExistingMedia = getMediaSetCount();
   int mediaIndex = 0;
   UtlBoolean fieldFound = TRUE;
   UtlBoolean commonVideo = FALSE;
   UtlString mediaType;
   int mediaPort, audioPort, videoPort;
   int mediaPortPairs, audioPortPairs, videoPortPairs;
   UtlString mediaTransportType, audioTransportType, videoTransportType;
   int numPayloadTypes, numAudioPayloadTypes, numVideoPayloadTypes;
   int payloadTypes[MAXIMUM_MEDIA_TYPES];
   int audioPayloadTypes[MAXIMUM_MEDIA_TYPES];
   int videoPayloadTypes[MAXIMUM_MEDIA_TYPES];
   int supportedPayloadTypes[MAXIMUM_MEDIA_TYPES];
   int formatArray[MAXIMUM_MEDIA_TYPES];
   int remoteTotalBandwidth;
   int matchingBandwidth;
   SdpCodec* codecsInCommon[MAXIMUM_MEDIA_TYPES];
   SdpCodec* codecsDummy[MAXIMUM_MEDIA_TYPES]; // just a dummy codec array
   int supportedPayloadCount;
   int destIndex;
   int firstMimeSubTypeIndex = 0;
   SdpSrtpParameters receivedSrtpParams;
   SdpSrtpParameters receivedAudioSrtpParams;
   SdpSrtpParameters receivedVideoSrtpParams;
   UtlString prevMimeSubType = "none";
   UtlString mimeSubType;

   memset(formatArray, 0, sizeof(int)*MAXIMUM_MEDIA_TYPES); 
   memset(&receivedSrtpParams,0, sizeof(SdpSrtpParameters));
   // if there are no media fields already, add a global
   // address field
   if(!preExistingMedia)
   {
      setConnectionAddress(hostAddresses[0]);
      char timeString[100];
      sprintf(timeString, "%d %d", 0, //OsDateTime::getSecsSinceEpoch(),
              0);
      addValue("t", timeString);
   }

   numPayloadTypes = 0 ;
   memset(&payloadTypes, 0, sizeof(int) * MAXIMUM_MEDIA_TYPES) ;
      
   audioPort = 0 ;
   audioPortPairs = 0 ;
   numAudioPayloadTypes = 0 ;
   memset(&audioPayloadTypes, 0, sizeof(int) * MAXIMUM_MEDIA_TYPES) ;
   memset(&receivedAudioSrtpParams,0 , sizeof(SdpSrtpParameters));

   videoPort = 0 ;
   videoPortPairs = 0 ;
   numVideoPayloadTypes = 0 ;
   memset(&videoPayloadTypes, 0, sizeof(int) * MAXIMUM_MEDIA_TYPES) ;
   memset(&receivedVideoSrtpParams,0 , sizeof(SdpSrtpParameters));

   // Loop through the fields in the sdpRequest
   while(fieldFound)
   {
      fieldFound = sdpRequest->getMediaData(mediaIndex, &mediaType,
                                            &mediaPort, &mediaPortPairs,
                                            &mediaTransportType,
                                            MAXIMUM_MEDIA_TYPES, &numPayloadTypes,
                                            payloadTypes);

      sdpRequest->getBandwidthField(remoteTotalBandwidth);

      if(fieldFound)
      {

#ifdef TEST_PRINT 
         OsSysLog::add(FAC_NET, PRI_DEBUG, "SdpBody::addCodecsAnswer media type: %s", mediaType.data());
#endif

         // Check for unsupported stuff
         // i.e. non audio media, non RTP transported media, etc
          if(  (  mediaType.compareTo(SDP_AUDIO_MEDIA_TYPE, UtlString::ignoreCase) != 0
               && mediaType.compareTo(SDP_VIDEO_MEDIA_TYPE, UtlString::ignoreCase) != 0
               )
            || mediaPort <= 0
            || mediaPortPairs <= 0
            || (  mediaTransportType.compareTo(SDP_RTP_MEDIA_TRANSPORT_TYPE, UtlString::ignoreCase) != 0
               && mediaTransportType.compareTo(SDP_RTP_TCP_MEDIA_TRANSPORT_TYPE, UtlString::ignoreCase) != 0
               && mediaTransportType.compareTo(SDP_SRTP_MEDIA_TRANSPORT_TYPE, UtlString::ignoreCase) != 0
               )
            )
         {
            mediaPort = 0;
            addMediaData(mediaType.data(),
                         mediaPort, mediaPortPairs,
                         mediaTransportType.data(),
                         numPayloadTypes,
                         payloadTypes);
         }

         // Copy media fields and replace the port with:
         // rtpPort if one or more of the codecs are supported
         //       removing the unsupported codecs
         // zero if none of the codecs are supported
         else
         {
            sdpRequest->getSrtpCryptoField(mediaIndex, 1, receivedSrtpParams);
            if (mediaType.compareTo(SDP_AUDIO_MEDIA_TYPE, UtlString::ignoreCase) == 0)
            {
                audioPort = mediaPort;
                audioPortPairs = mediaPortPairs;
                audioTransportType = mediaTransportType;
                numAudioPayloadTypes = numPayloadTypes;
                memcpy(&audioPayloadTypes, &payloadTypes, sizeof(int)*MAXIMUM_MEDIA_TYPES);
                memcpy(&receivedAudioSrtpParams, &receivedSrtpParams, sizeof(SdpSrtpParameters));
            }
            else
            {  
                videoPort = mediaPort;
                videoPortPairs = mediaPortPairs;
                videoTransportType = mediaTransportType;
                numVideoPayloadTypes = numPayloadTypes;
                memcpy(&videoPayloadTypes, &payloadTypes, sizeof(int)*MAXIMUM_MEDIA_TYPES);
                memcpy(&receivedVideoSrtpParams, &receivedSrtpParams, sizeof(SdpSrtpParameters));
            }
         }
      }
      mediaIndex++;
   }

   SdpCodecList codecFactory(numRtpCodecs, rtpCodecs);

   supportedPayloadCount = 0;
   sdpRequest->getCodecsInCommon(numAudioPayloadTypes,
                                 numVideoPayloadTypes, 
                                 audioPayloadTypes,
                                 videoPayloadTypes,
                                 videoPort,
                                 codecFactory,
                                 supportedPayloadCount,
                                 codecsDummy,
                                 codecsInCommon);
#ifdef TEST_PRINT
   OsSysLog::add(FAC_NET, PRI_DEBUG, "SdpBody::addCodecsAnswer audio codecs enabled: %d video codecs enabled: %d", numAudioPayloadTypes, numVideoPayloadTypes);
#endif

   SdpSrtpParameters commonAudioSrtpParams;
   SdpSrtpParameters commonVideoSrtpParams;
   memset(&commonAudioSrtpParams,0 , sizeof(SdpSrtpParameters));
   memset(&commonVideoSrtpParams,0 , sizeof(SdpSrtpParameters));
   getEncryptionInCommon(srtpParams, receivedAudioSrtpParams,
                         commonAudioSrtpParams);
   getBandwidthInCommon(totalBandwidth, remoteTotalBandwidth, matchingBandwidth); 

    // Add the modified list of supported codecs
    if (supportedPayloadCount && 
        srtpParams.securityLevel==commonAudioSrtpParams.securityLevel)
    {
        // Do this for audio first
        destIndex = 0;
        int payloadIndex;
        for(payloadIndex = 0;
            payloadIndex < supportedPayloadCount;
            payloadIndex++)
        {
            codecsInCommon[payloadIndex]->getMediaType(mediaType);
            if (mediaType.compareTo(SDP_AUDIO_MEDIA_TYPE, UtlString::ignoreCase) == 0)
            {
                supportedPayloadTypes[destIndex++] =
                        codecsInCommon[payloadIndex]->getCodecPayloadFormat();
            }
        }
        addMediaData(SDP_AUDIO_MEDIA_TYPE, rtpAudioPorts[0], 1,
                     audioTransportType.data(), destIndex,
                     supportedPayloadTypes);

        // The following are for RTSP support as the players seem to want to have
        // these fields in the SDP
        addValue("a", "control:trackID=1");

        if (commonAudioSrtpParams.securityLevel)
        {
            addSrtpCryptoField(commonAudioSrtpParams);
        }

//dddd
        if (  audioTransportType.compareTo(SDP_RTP_MEDIA_TRANSPORT_TYPE, UtlString::ignoreCase) == 0
           || audioTransportType.compareTo(SDP_RTP_TCP_MEDIA_TRANSPORT_TYPE, UtlString::ignoreCase) == 0
           || audioTransportType.compareTo(SDP_SRTP_MEDIA_TRANSPORT_TYPE, UtlString::ignoreCase) == 0
           )
        {
            // It is assumed that rtcp is the odd port immediately after 
            // the rtp port.  If that is not true, we must add a parameter 
            // to specify the rtcp port.
            if (  rtcpAudioPorts[0] > 0
               && (  rtcpAudioPorts[0] != rtpAudioPorts[0] + 1
                  || (rtcpAudioPorts[0] % 2) == 0
                  )
               )
            {
                char cRtcpBuf[32] ;
                sprintf(cRtcpBuf, "rtcp:%d", rtcpAudioPorts[0]) ;
                addValue("a", cRtcpBuf) ;
            }
            
            if (mediaTransportType.compareTo(SDP_RTP_TCP_MEDIA_TRANSPORT_TYPE, UtlString::ignoreCase) == 0)
            {
                // if transport == TCP, add the a=setup:actpass
                addValue("a", "setup:actpass");
            }

            // Add candidate addresses if available
            if (iNumAddresses > 1)
            {
                for (int i=0; i<iNumAddresses; i++)
                {
                    uint64_t priority = (iNumAddresses-i) + PRIORITY_OFFSET;

                    assert(hostAddresses[i].length() > 0) ;
                    if (rtpAudioPorts[0] && rtpAudioPorts[i] && hostAddresses[i])
                    {
                        addCandidateAttribute(i, "t", "UDP", priority, hostAddresses[i], rtpAudioPorts[i]) ;
                    }

                    if (rtcpAudioPorts[0] && rtcpAudioPorts[i] && hostAddresses[i])
                    {
                        addCandidateAttribute(i, "t", "UDP", priority, hostAddresses[i], rtcpAudioPorts[i]) ;
                    }
                }
            }
        }

        addCodecParameters(supportedPayloadCount,
                            codecsInCommon, SDP_AUDIO_MEDIA_TYPE); //, videoFramerate);

        // Then do this for video
        destIndex = -1;
        for(payloadIndex = 0;
            payloadIndex < supportedPayloadCount;
            payloadIndex++)
        {
            codecsInCommon[payloadIndex]->getMediaType(mediaType);
//#ifdef TEST_PRINT
            OsSysLog::add(FAC_NET, PRI_DEBUG, 
                "SdpBody::addCodecsAnswer codecsInCommon[%d] looking for video, found: getMediaType=%s payload ID: %d internal codec ID: %d", 
                payloadIndex, mediaType.data(), codecsInCommon[payloadIndex]->getCodecPayloadFormat(), codecsInCommon[payloadIndex]->getCodecType());
//#endif

            if (mediaType.compareTo(SDP_VIDEO_MEDIA_TYPE, UtlString::ignoreCase) == 0)
            {
                // We've found at least one common video codec
                commonVideo = TRUE;
                codecsInCommon[payloadIndex]->getEncodingName(mimeSubType);

                // If we still have the same mime type only change format. We're depending on the
                // fact that codecs with the same mime subtype are added sequentially to the 
                // codec factory. Otherwise this won't work.
                // TODO: add better comparison of H264 codecs to check for duplicates
                if (prevMimeSubType.compareTo(MIME_SUBTYPE_H264, UtlString::ignoreCase) != 0 && 
                    prevMimeSubType.compareTo(mimeSubType, UtlString::ignoreCase) == 0)
                {
                    formatArray[destIndex] |= (codecsInCommon[payloadIndex])->getVideoFormat();
                    (codecsInCommon[firstMimeSubTypeIndex])->setVideoFmtp(formatArray[destIndex]);
                    (codecsInCommon[firstMimeSubTypeIndex])->setVideoFmtpString((codecsInCommon[payloadIndex])->getVideoFormat());
                }
                else
                {
                    ++destIndex;
                    prevMimeSubType = mimeSubType;
                    firstMimeSubTypeIndex = payloadIndex;
                    formatArray[destIndex] = (codecsInCommon[payloadIndex])->getVideoFormat();
                    supportedPayloadTypes[destIndex] =
                        codecsInCommon[firstMimeSubTypeIndex]->getCodecPayloadFormat();
                    (codecsInCommon[firstMimeSubTypeIndex])->setVideoFmtp(formatArray[destIndex]);
                    (codecsInCommon[firstMimeSubTypeIndex])->clearVideoFmtpString();
                    (codecsInCommon[firstMimeSubTypeIndex])->setVideoFmtpString((codecsInCommon[payloadIndex])->getVideoFormat());
                }

            }
        }
        // Only add m-line if we actually have common video codecs
        if (commonVideo)
        {
            addMediaData(SDP_VIDEO_MEDIA_TYPE,
                        rtpVideoPorts[0], 1,
                        videoTransportType.data(),
                        destIndex+1,
                        supportedPayloadTypes);

            // The following are for RTSP support as the players seem to want to have
            // these fields in the SDP
            addValue("a", "control:trackID=2");

            // We do not support video encryption at this time 
            //if (commonVideoSrtpParams.securityLevel)
            //{
            //    addSrtpCryptoField(commonAudioSrtpParams);
            //}

            if (    (audioTransportType.compareTo(SDP_RTP_MEDIA_TRANSPORT_TYPE, UtlString::ignoreCase) == 0) ||
                    (audioTransportType.compareTo(SDP_RTP_TCP_MEDIA_TRANSPORT_TYPE, UtlString::ignoreCase) == 0) ||
                    (audioTransportType.compareTo(SDP_SRTP_MEDIA_TRANSPORT_TYPE, UtlString::ignoreCase) == 0))
            {
                // It is assumed that rtcp is the odd port immediately after 
                // the rtp port.  If that is not true, we must add a parameter 
                // to specify the rtcp port.
                if ((rtcpVideoPorts[0] > 0) && ((rtcpVideoPorts[0] != rtpVideoPorts[0] + 1) 
                        || (rtcpVideoPorts[0] % 2) == 0))
                {
                    char cRtcpBuf[32] ;
                    sprintf(cRtcpBuf, "rtcp:%d", rtcpVideoPorts[0]) ;
                    addValue("a", cRtcpBuf) ;
                }
            }

            // Add candidate addresses if available
            if (iNumAddresses > 1)
            {
                for (int i=0; i<iNumAddresses; i++)
                {
                    uint64_t priority = (iNumAddresses-i) + PRIORITY_OFFSET;

                    assert(hostAddresses[i].length() > 0) ;
                    if (rtpVideoPorts[0] && rtpVideoPorts[i] && hostAddresses[i])
                    {
                        addCandidateAttribute(i, "t", "UDP", priority, hostAddresses[i], rtpVideoPorts[i]) ;
                    }

                    if (rtcpVideoPorts[0] && rtcpVideoPorts[i] && hostAddresses[i])
                    {
                        addCandidateAttribute(i, "t", "UDP", priority, hostAddresses[i], rtcpVideoPorts[i]) ;
                    }
                }
            }

            addCodecParameters(supportedPayloadCount,
                                codecsInCommon, SDP_VIDEO_MEDIA_TYPE); //, videoFramerate);
        }
    }

    // Zero out the port to indicate none are supported
    else
    {
        mediaPort = 0;
        addMediaData(SDP_AUDIO_MEDIA_TYPE,
                    mediaPort, audioPortPairs,
                    audioTransportType.data(),
                    numAudioPayloadTypes,
                    audioPayloadTypes);
        addMediaData(SDP_VIDEO_MEDIA_TYPE,
                    mediaPort, videoPortPairs,
                    videoTransportType.data(),
                    numVideoPayloadTypes,
                    videoPayloadTypes);
    }

    // Free up the codec copies
    for(int codecIndex = 0; codecIndex < supportedPayloadCount; codecIndex++)
    {
        delete codecsInCommon[codecIndex];
        codecsInCommon[codecIndex] = NULL;

        delete codecsDummy[codecIndex];
        codecsDummy[codecIndex] = NULL;
    }


    if(preExistingMedia)
    {
        addConnectionAddress(hostAddresses[0]);
    }
    // Indicate a low bandwidth client if bw <= 64 kpbs
    if (matchingBandwidth != 0 && matchingBandwidth <= 64)
    {
        char ct[16];
        sprintf(ct, "CT:%d", matchingBandwidth);
        setValue("b", ct);
    }

    // Copy all atribute fields verbatum
    // someday
#endif //}
}

void SdpBody::addMediaLinesAnswer(int numMediaLines, const SdpMediaLine* mediaLines[], const SdpBody& sdpOffer)
{
    if(numMediaLines > 0 && mediaLines[0])
    {
        int preExistingMedia = getMediaSetCount();
        SdpMediaLine::SdpConnection* connection = (SdpMediaLine::SdpConnection*) mediaLines[0]->getConnections().index(0);
        UtlString gloablHostAddresses = connection ? connection->getAddress() : "";

        // if there are no media fields already, add a global
        // address field
        if(!preExistingMedia)
        {
            if(! gloablHostAddresses.isNull())
            {
                setConnectionAddress(gloablHostAddresses);
            }
            char timeString[100];
            sprintf(timeString, "%d %d", 0, //OsDateTime::getSecsSinceEpoch(),
              0);
            addValue("t", timeString);
        }

        UtlBoolean* streamsUsed = new UtlBoolean[numMediaLines];
        int localStreamIndex;
        for(localStreamIndex = 0; localStreamIndex < numMediaLines; localStreamIndex++) 
        {
            streamsUsed[localStreamIndex] = FALSE;
        }

        UtlString remoteMediaType;
        int remoteMediaPort;
        int remoteMediaPortPairs;
        UtlString remoteMediaTransportType;
        int numRemotePayloadTypes;
        int remotePayloadTypes[MAXIMUM_MEDIA_TYPES];
        int remoteMediaIndex = 0;

        // Loop through the codecs on the remote side
        while(sdpOffer.getMediaData(remoteMediaIndex, 
                                    &remoteMediaType,
                                    &remoteMediaPort, 
                                    &remoteMediaPortPairs,
                                    &remoteMediaTransportType,
                                    MAXIMUM_MEDIA_TYPES, 
                                    &numRemotePayloadTypes,
                                    remotePayloadTypes))
        {
            UtlBoolean wroteMediaLine = FALSE;

            // Look through unused local streams to find a match
            for(localStreamIndex = 0; localStreamIndex < numMediaLines; localStreamIndex++)
            {
                if(! streamsUsed[localStreamIndex] &&
                   mediaLines[localStreamIndex]->getMediaType() == SdpMediaLine::getMediaTypeFromString(remoteMediaType))
                {
                    SdpMediaLine remoteMediaLine;
                    SdpCodecList localDecodeCodecs;
                    // Mime types match, check if any codecs match
                    sdpOffer.getCodecsInCommon(*mediaLines[localStreamIndex],
                                               remoteMediaIndex,
                                               remoteMediaLine,
                                               localDecodeCodecs);
                    if(remoteMediaLine.getCodecs()->getCodecCount())
                    {
                        // Mark this local stream as used so we do not have multiple
                        // remote streams sending to the same socket/stream.
                        streamsUsed[localStreamIndex] = TRUE;
                        wroteMediaLine = TRUE;

                        // Create a temporary mediaLine for the local side which uses the codec payload ID of the
                        // the rmoete side where possible.
                        SdpMediaLine tempLocalMediaLine = *mediaLines[localStreamIndex];
                        tempLocalMediaLine.setCodecs(localDecodeCodecs);

                        // Write out the SDP for this media line
                        addMediaLine(tempLocalMediaLine, *this);

                        // A unique track ID is needed for each media line when used with RTSP
                        UtlString trackString("control:trackID=");
                        trackString.appendFormat("%d", localStreamIndex + 1);
                        addValue("a", trackString);

                        // Break out of the for loop we found a match
                        break;
                    }
                }
            }

            // There was no match to this remote media line, write out a inactive m line
            if(!wroteMediaLine)
            {
                // c line is manditory and must go before any a lines
                addConnectionAddress("0.0.0.0");

                remoteMediaPort = 0;
                addMediaData(remoteMediaType,
                             remoteMediaPort, 
                             remoteMediaPortPairs,
                             remoteMediaTransportType,
                             numRemotePayloadTypes,
                             remotePayloadTypes);

                // Mark media line as inactive
                addValue("a", "inactive");
            }

            remoteMediaIndex++;
        }

        delete[] streamsUsed;
    }
}

void SdpBody::addRtpmap(int payloadType,
                        const char* mimeSubtype,
                        int sampleRate,
                        int numChannels)
{
   UtlString fieldValue("rtpmap:");
   char buffer[256];
   sprintf(buffer, "%d %s/%d", payloadType, mimeSubtype,
           sampleRate);

   fieldValue.append(buffer);

   if(numChannels > 0)
   {
      sprintf(buffer, "/%d", numChannels);
      fieldValue.append(buffer);
   }

   // Add the "a" field
   addValue("a", fieldValue.data());
}


void SdpBody::addSrtpCryptoField(SdpSrtpParameters& params)
{
    UtlString fieldValue("crypto:1 ");

    switch (params.cipherType)
    {
    case AES_CM_128_HMAC_SHA1_80:
        fieldValue.append("AES_CM_128_HMAC_SHA1_80 ");
        break;
    case AES_CM_128_HMAC_SHA1_32:
        fieldValue.append("AES_CM_128_HMAC_SHA1_32 ");
        break;
    case F8_128_HMAC_SHA1_80:
        fieldValue.append("F8_128_HMAC_SHA1_80 ");
        break;
    default: break;
    }
    fieldValue.append("inline:");

    // Base64-encode key string
    UtlString base64Key;
    NetBase64Codec::encode(SRTP_KEY_LENGTH, (char*)params.masterKey, base64Key);

    // Remove padding
    while (base64Key(base64Key.length()-1) == '=')
    {
        base64Key = base64Key(0, base64Key.length()-1);
    }
    fieldValue.append(base64Key);

    if (!(params.securityLevel & SRTP_ENCRYPTION))
    {
        fieldValue.append(" UNENCRYPTED_SRTP");
    }
    if (!(params.securityLevel & SRTP_AUTHENTICATION))
    {
        fieldValue.append(" UNAUTHENTICATED_SRTP");
    }
 
   // Add the "a" field for the crypto attribute
   addValue("a", fieldValue.data());
}


void SdpBody::addFormatParameters(int payloadType,
                                  const char* formatParameters)
{
   // Build "a" field:
   // "a=fmtp <payloadFormat> <formatParameters>"
   UtlString fieldValue("fmtp:");
   char buffer[100];
   sprintf(buffer, "%d ", payloadType);
   fieldValue.append(buffer);
   fieldValue.append(formatParameters);


   // Add the "a" field
   addValue("a", fieldValue.data());
}

void SdpBody::addPtime(int pTime)
{
    // Build "a" field for ptime"
    // a=ptime: <milliseconds>
    UtlString fieldValue("ptime:");
    char buffer[100];
    sprintf(buffer, "%d", pTime);
    fieldValue.append(buffer);

    // Add the "a" field
    addValue("a", fieldValue);
}

void SdpBody::addCandidateAttribute(int         candidateId, 
                                    const char* transportId, 
                                    const char* transportType,
                                    uint64_t    qValue, 
                                    const char* candidateIp, 
                                    int         candidatePort) 
{
    UtlString attributeData ;
    char buffer[64] ;    
    
    attributeData.append("candidate:") ;

    sprintf(buffer, "%d", candidateId) ;
    attributeData.append(buffer) ;
    attributeData.append(" ") ;

    attributeData.append(transportId) ;
    attributeData.append(" ") ;

    attributeData.append(transportType) ;
    attributeData.append(" ") ;
    
    sprintf(buffer, "%" FORMAT_INTLL "u", qValue) ;
    attributeData.append(buffer) ;
    attributeData.append(" ") ;

    attributeData.append(candidateIp) ;
    attributeData.append(" ") ;

    sprintf(buffer, "%d", candidatePort) ;
    attributeData.append(buffer) ;

    addValue("a", attributeData) ;
}


UtlBoolean SdpBody::getCandidateAttribute(int mediaIndex,
                                          int candidateIndex,
                                          int& rCandidateId,
                                          UtlString& rTransportId,
                                          UtlString& rTransportType,
                                          uint64_t& rQvalue, 
                                          UtlString& rCandidateIp, 
                                          int& rCandidatePort) const
{    
    UtlBoolean found = FALSE;
    UtlSListIterator iterator(*sdpFields);
    NameValuePair* nv = NULL;
    int aFieldIndex = 0;
    const char* value;
    UtlString aFieldMatch("a");
    UtlString aFieldType ;
    
    nv = positionFieldInstance(mediaIndex, &iterator, "m");
    if(nv)
    {
        while ((nv = findFieldNameBefore(&iterator, aFieldMatch, "m")))
        {
            value =  nv->getValue();
            
            // Verify this is an candidate "a" record
            UtlTokenizer tokenizer(value) ;
            if (tokenizer.next(aFieldType, ":"))
            {
                aFieldType.toLower() ;
                aFieldType.strip(UtlString::both, ' ') ;
                if(aFieldType.compareTo("candidate", UtlString::ignoreCase) == 0)
                {
                    if (aFieldIndex == candidateIndex)
                    {
                        UtlString tmpCandidateId ;
                        UtlString tmpQvalue ;                        
                        UtlString tmpCandidatePort ;

                        if (    tokenizer.next(tmpCandidateId, " \t")  &&
                                tokenizer.next(rTransportId, " \t") &&
                                tokenizer.next(rTransportType, " \t") &&
                                tokenizer.next(tmpQvalue, " \t") &&
                                tokenizer.next(rCandidateIp, " \t") &&
                                tokenizer.next(tmpCandidatePort, " \t"))
                        {
                            // Strip leading : from id -- is this a UtlTokenizer Bug?? 
                            tmpCandidateId.strip(UtlString::leading, ':') ;

                            rCandidateId = atoi(tmpCandidateId) ;
                            rQvalue = UtlLongLongInt::stringToLongLong(tmpQvalue) ;
                            rCandidatePort = atoi(tmpCandidatePort) ;
                       
                            found = TRUE;
                            break;
                        }
                    }
                    aFieldIndex++ ;
                }
            }      
        }
    }

    return(found) ;
}


UtlBoolean SdpBody::getCandidateAttributes(const char* szMimeType,
                                           int         nMaxAddresses,
                                           int         candidateIds[],
                                           UtlString   transportIds[],
                                           UtlString   transportTypes[],
                                           uint64_t    qvalues[], 
                                           UtlString   candidateIps[], 
                                           int         candidatePorts[],
                                           int&        nActualAddresses) const 
{
    int mediaIndex = findMediaType(szMimeType, 0) ;
    return getCandidateAttributes(mediaIndex, nMaxAddresses, candidateIds,
                                  transportIds, transportTypes, qvalues,
                                  candidateIps, candidatePorts, nActualAddresses);
}

UtlBoolean SdpBody::getCandidateAttributes(int         mediaIndex,
                                           int         nMaxAddresses,
                                           int         candidateIds[],
                                           UtlString   transportIds[],
                                           UtlString   transportTypes[],
                                           uint64_t    qvalues[], 
                                           UtlString   candidateIps[], 
                                           int         candidatePorts[],
                                           int&        nActualAddresses) const 
{
    nActualAddresses = 0 ;

    while (nActualAddresses < nMaxAddresses)
    {
        if (getCandidateAttribute(mediaIndex, 
                nActualAddresses, candidateIds[nActualAddresses], 
                transportIds[nActualAddresses], transportTypes[nActualAddresses], 
                qvalues[nActualAddresses], candidateIps[nActualAddresses], 
                candidatePorts[nActualAddresses]))
        {
            nActualAddresses++ ;
        }
        else
        {
            break ;
        }
    }

    return (nActualAddresses > 0) ;
}

void SdpBody::addMediaData(const char* mediaType,
                           int portNumber, int portPairCount,
                           const char* mediaTransportType,
                           int numPayloadTypes,
                           int payloadTypes[])
{
#ifdef TEST_PRINT
   UtlString payloadString;
   for(int payloadIndex = 0; payloadIndex < numPayloadTypes; payloadIndex++)
   {
       if(payloadIndex > 0)
       {
           payloadString.append(", ");
       }
       payloadString.appendFormat("%d", payloadTypes);
   }

   OsSysLog::add(FAC_NET, PRI_DEBUG, "SdpBody::addMediaData mediaType: %s numPayloadTypes: %d {%s}", 
      mediaType, numPayloadTypes, payloadString.data());
#endif

   UtlString value;
   char integerString[MAXIMUM_LONG_INT_CHARS];
   int codecIndex;

   // Media type (i.e. audio, application or video)
   value.append(mediaType);
   value.append(SDP_SUBFIELD_SEPARATOR);

   // Port and optional number of port pairs
   sprintf(integerString, "%d", portNumber);
   value.append(integerString);
   if(portPairCount > 1)
   {
      sprintf(integerString, "%d", portPairCount);
      value.append("/");
      value.append(integerString);
   }
   value.append(SDP_SUBFIELD_SEPARATOR);

   // Media transport type
   value.append(mediaTransportType);
   //value.append(SDP_SUBFIELD_SEPARATOR);

   for(codecIndex = 0; codecIndex < numPayloadTypes; codecIndex++)
   {
      // Media payload type (i.e. alaw, mlaw, etc.)
      sprintf(integerString, "%c%d", SDP_SUBFIELD_SEPARATOR,
              payloadTypes[codecIndex]);
      value.append(integerString);
   }

   // printf("adding m line: \"%s\"\n", value.data());
   addValue("m", value.data());

}


void SdpBody::addConnectionAddress(const char* ipAddress)
{

   const char* networkType = SDP_NETWORK_TYPE;
   const char* addressType = SDP_IP4_ADDRESS_TYPE;
   addConnectionAddress(networkType, addressType, ipAddress);
}

void SdpBody::addConnectionAddress(const char* networkType, const char* addressType, const char* ipAddress)
{
   UtlString value;

   value.append(networkType);
   value.append(SDP_SUBFIELD_SEPARATOR);
   value.append(addressType);
   value.append(SDP_SUBFIELD_SEPARATOR);
   value.append(ipAddress);

   addValue("c", value.data());
}

void SdpBody::setConnectionAddress(const char* ipAddress)
{

   const char* networkType = SDP_NETWORK_TYPE;
   const char* addressType = SDP_IP4_ADDRESS_TYPE;
   setConnectionAddress(networkType, addressType, ipAddress);
}

void SdpBody::setConnectionAddress(const char* networkType, const char* addressType, const char* ipAddress)
{
   UtlString value;

   value.append(networkType);
   value.append(SDP_SUBFIELD_SEPARATOR);
   value.append(addressType);
   value.append(SDP_SUBFIELD_SEPARATOR);
   value.append(ipAddress);

   setValue("c", value.data());
}

void SdpBody::addValue(const char* name, const char* value, int fieldIndex)
{
   NameValuePair* nv = new NameValuePair(name, value);
   if(-1 == fieldIndex)
   {
      sdpFields->append(nv);
   }
   else
   {
      sdpFields->insertAt(fieldIndex, nv);
   }
}

void SdpBody::addEpochTime(unsigned long epochStartTime, unsigned long epochEndTime)
{
   epochStartTime += NTP_TO_EPOCH_DELTA;
   if(epochEndTime != 0) // zero means unbounded in sdp, so don't convert it
   {
      epochEndTime += NTP_TO_EPOCH_DELTA;
   }
   addNtpTime(epochStartTime, epochEndTime);
}

void SdpBody::addNtpTime(unsigned long ntpStartTime, unsigned long ntpEndTime)
{
   char integerString[MAXIMUM_LONG_INT_CHARS];
   UtlString value;

   sprintf(integerString, "%lu", ntpStartTime);
   value.append(integerString);
   value.append(SDP_SUBFIELD_SEPARATOR);
   sprintf(integerString, "%lu", ntpEndTime);
   value.append(integerString);

   size_t timeLocation = findFirstOf("zkam");
   addValue("t", value.data(), timeLocation);
}

void SdpBody::setOriginator(const char* userId, int sessionId, int sessionVersion, const char* address)
{
   char integerString[MAXIMUM_LONG_INT_CHARS];
   UtlString value;

   const char* networkType = SDP_NETWORK_TYPE;
   const char* addressType = SDP_IP4_ADDRESS_TYPE;

   //o=<username> <session id> <version> <network type> <address type> <address>
   // o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4

   value.append(userId);
   value.append(SDP_SUBFIELD_SEPARATOR);
   sprintf(integerString, "%d", sessionId);
   value.append(integerString);
   value.append(SDP_SUBFIELD_SEPARATOR);
   sprintf(integerString, "%d", sessionVersion);
   value.append(integerString);
   value.append(SDP_SUBFIELD_SEPARATOR);
   value.append(networkType);
   value.append(SDP_SUBFIELD_SEPARATOR);
   value.append(addressType);
   value.append(SDP_SUBFIELD_SEPARATOR);
   value.append(address);

   setValue("o", value.data());
}

int SdpBody::getLength() const
{
   UtlSListIterator iterator(*sdpFields);
   NameValuePair* nv = NULL;
   const char* value;
   int length = 0;
   
   while((nv = static_cast<NameValuePair*>(iterator())))
   {
      value = nv->getValue();
      if(value)
      {
         length += (  nv->length()
                    + 3 // SDP_NAME_VALUE_DELIMITOR + CARRIAGE_RETURN_NEWLINE
                    + strlen(value)
                    );
      }
      else if (!isOptionalField(nv->data()))
      {
         // not optional, so append it even if empty
         length += (  nv->length()
                    + 3 // SDP_NAME_VALUE_DELIMITOR + CARRIAGE_RETURN_NEWLINE
                    );
      }
   }
   return(length);
}

void SdpBody::getBytes(const char** bytes, int* length) const
{
   // This version of getBytes exists so that a caller who is
   // calling this method through an HttpBody will get the right
   // thing - we fill in the mBody string and then return that.
   UtlString tempBody;
   getBytes( &tempBody, length );
   ((SdpBody*)this)->mBody = tempBody.data();
   *bytes = mBody.data();
}

void SdpBody::getBytes(UtlString* bytes, int* length) const
{
   UtlSListIterator iterator(*sdpFields);
   NameValuePair* nv = NULL;
   const char* value;
   bytes->remove(0);
   while((nv = static_cast<NameValuePair*>(iterator())))
   {
      value = nv->getValue();
      if(value)
      {
         bytes->append(nv->data());
         bytes->append(SDP_NAME_VALUE_DELIMITOR);
         bytes->append(value);
         bytes->append(CARRIAGE_RETURN_NEWLINE);
      }
      else if (!isOptionalField(nv->data()))
      {
         // not optional, so append it even if empty
         bytes->append(nv->data());
         bytes->append(SDP_NAME_VALUE_DELIMITOR);
         bytes->append(CARRIAGE_RETURN_NEWLINE);
      }
   }

   *length = bytes->length();
}

int SdpBody::getFieldCount() const
{
   return(sdpFields->entries());
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


bool SdpBody::isOptionalField(const char* name) const
{
   const UtlString OptionalStandardFields("iuepcbzkar");
   
   return OptionalStandardFields.index(name) != UtlString::UTLSTRING_NOT_FOUND;
}

size_t SdpBody::findFirstOf( const char* headers )
{
   size_t first = UTL_NOT_FOUND;
   size_t found;
   size_t lookingFor;
   size_t headersToTry;
   
   for ( lookingFor = 0, headersToTry = strlen(headers);
         lookingFor < headersToTry;
         lookingFor++
        )
   {
      char thisHeader[2];
      thisHeader[0] = headers[lookingFor];
      thisHeader[1] = '\0';
      NameValuePair header(thisHeader,NULL);
      
      found = sdpFields->index(&header);
      if ( UTL_NOT_FOUND != found )
      {
         first = (UTL_NOT_FOUND == first) ? found : first > found ? found : first;
      }
   }

   return first;
}

NameValuePair* SdpBody::positionFieldInstance(int fieldInstanceIndex,
                                              UtlSListIterator* iter,
                                              const char* fieldName)
{
   UtlContainable* nv = NULL;
   if ( fieldInstanceIndex >= 0 )
   {
      NameValuePair fieldNV(fieldName);
      iter->reset();

      int index = 0;
      nv = iter->findNext(&fieldNV);
      while(nv && index < fieldInstanceIndex)
      {
         nv = iter->findNext(&fieldNV);
         index++;
      }
   }

   return((NameValuePair*) nv);
}

NameValuePair* SdpBody::findFieldNameBefore(UtlSListIterator* iter,
                                            const char* targetFieldName,
                                            const char* beforeFieldName)
{
   // Find a default address if one exist
   NameValuePair* nv = (NameValuePair*) (*iter)();
   while(nv)
   {
      // Target field not found before beforeFieldName
      if(strcmp(nv->data(), beforeFieldName) == 0)
      {
         nv = NULL;
         break;
      }
      // Target field found
      else if(strcmp(nv->data(), targetFieldName) == 0)
      {
         break;
      }
      nv = (NameValuePair*) (*iter)();
   }
   return(nv);
}

UtlBoolean SdpBody::findValueInField(const char* pField, const char* pvalue) const
{
   UtlSListIterator iterator(*sdpFields);
   NameValuePair* nv = positionFieldInstance(0, &iterator, "m");
   UtlString aFieldMatch(pField);

   while((nv = (NameValuePair*) iterator.findNext(&aFieldMatch)) != NULL)
   {
       if ( strcmp(nv->getValue(), pvalue) == 0 )
          return TRUE;
   }

   return FALSE;
}

const bool SdpBody::isTransportAvailable(const OsSocket::IpProtocolSocketType protocol,
                                         const SIPX_MEDIA_TYPE mediaType) const
{
    bool bIsAvailable = false;
    int mediaIndex = 0;
    UtlString sdpMediaType;
    int mediaPort;
    int mediaPortPairs;
    UtlString mediaTransportType;
    int maxPayloadTypes = 32;
    int numPayloadTypes;
    int payloadTypes[32];
    UtlBoolean bFound = true;
    while ((protocol == OsSocket::UDP || protocol == OsSocket::TCP) &&
           bFound)
    {
        bFound = getMediaData(mediaIndex,
                              &sdpMediaType,
                              &mediaPort,
                              &mediaPortPairs,
                              &mediaTransportType,
                              maxPayloadTypes,
                              &numPayloadTypes,
                              payloadTypes);
        if (bFound)
        {                              
            bool bMediaTypeMatch = false;
            bool bTransportTypeMatch = false;
            if (protocol == OsSocket::UDP &&
                mediaTransportType.compareTo(SDP_RTP_MEDIA_TRANSPORT_TYPE, UtlString::ignoreCase) == 0)
            {
                bTransportTypeMatch = true;
            }
            if (protocol == OsSocket::TCP &&
                mediaTransportType.compareTo(SDP_RTP_TCP_MEDIA_TRANSPORT_TYPE, UtlString::ignoreCase) == 0)
            {
                bTransportTypeMatch = true;
            }
            if (mediaType == MEDIA_TYPE_AUDIO &&
                sdpMediaType.compareTo(SDP_AUDIO_MEDIA_TYPE, UtlString::ignoreCase) == 0)
            {
                bMediaTypeMatch = true;
            }                
            if (mediaType == MEDIA_TYPE_VIDEO &&
                sdpMediaType.compareTo(SDP_VIDEO_MEDIA_TYPE, UtlString::ignoreCase) == 0)
            {
                bMediaTypeMatch = true;
            }                
            if (bMediaTypeMatch && bTransportTypeMatch)
            {
                bIsAvailable = true;
                break;
            }
        }                        
        mediaIndex++;                                 
    }                                 
    return bIsAvailable;
}
                         
// set all media attributes to either a=setup:actpass, a=setup:active, or a=setup:passive                   
void SdpBody::setRtpTcpRole(RtpTcpRoles role)
{
    UtlString sRole;
    
    if ((role & RTP_TCP_ROLE_ACTPASS) == RTP_TCP_ROLE_ACTPASS)
    {
            sRole = "actpass";
    }
    else if ((role & RTP_TCP_ROLE_ACTIVE) == RTP_TCP_ROLE_ACTIVE)
    {
            sRole = "active";
    }
    else
    {
            sRole = "passive";
    }
    UtlSListIterator iterator((UtlSList&)(*(sdpFields)));
    NameValuePair* headerField;
    UtlString value;
    while((headerField = (NameValuePair*) iterator()))
    {
       value = headerField->getValue();
       if (value.contains("setup:"))
       {
          value = "setup:" + sRole;
          headerField->setValue(value);
       }
    }
}

UtlString SdpBody::getRtpTcpRole() const
{
    UtlSListIterator iterator((UtlSList&)(*(sdpFields)));
    UtlString sRole;
    UtlString value;
    NameValuePair* headerField;
    
    while((headerField = (NameValuePair*) iterator()))
    {
       value = headerField->getValue();
       if (value.contains("setup:"))
       {
           sRole = value.data() + 6;
           break;
       }
    }
    return sRole;
}
                                   
/* ============================ FUNCTIONS ================================= */
