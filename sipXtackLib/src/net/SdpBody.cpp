//
// Copyright (C) 2004-2006 SIPfoundry Inc.
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

#include <utl/UtlSListIterator.h>
#include <utl/UtlTokenizer.h>
#include <net/SdpCodec.h>
#include <net/SdpBody.h>
#include <net/NameValuePair.h>
#include <net/NameValueTokenizer.h>
#include <net/SdpCodecFactory.h>
#include <utl/UtlTokenizer.h>
#include <net/NetBase64Codec.h>

#define MAXIMUM_LONG_INT_CHARS 20
#define MAXIMUM_MEDIA_TYPES 30
#define MAXIMUM_VIDEO_SIZES 6

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define SDP_NAME_VALUE_DELIMITOR '='
#define SDP_SUBFIELD_SEPARATOR ' '
#define SDP_SUBFIELD_SEPARATORS "\t "

#define SDP_RTP_MEDIA_TRANSPORT_TYPE "RTP/AVP"
#define SDP_SRTP_MEDIA_TRANSPORT_TYPE "RTP/SAVP"
#define SDP_MLAW_PAYLOAD 0
#define SDP_ALAW_PAYLOAD 8

#define SDP_NETWORK_TYPE "IN"
#define SDP_IP4_ADDRESS_TYPE "IP4"
#define NTP_TO_EPOCH_DELTA 2208988800UL

// STATIC VARIABLE INITIALIZATIONS
static int     sSessionCount = 5 ;  // Session version for SDP body
static OsMutex sSessionLock(OsMutex::Q_FIFO) ;


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SdpBody::SdpBody(const char* bodyBytes, int byteCount)
 : HttpBody(bodyBytes, byteCount)
{
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

      NameValueTokenizer parser(bodyBytes, byteCount);
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
            while (nv = findFieldNameBefore(&iterator, "a", "m"))
            {
                //printf("->%s:%s\n", nv->data(), nv->getValue()) ;

                UtlString typeAttribute ;
                UtlString portAttribute ;

                NameValueTokenizer::getSubField(nv->getValue(), 0, 
                        ":", &typeAttribute) ;
                NameValueTokenizer::getSubField(nv->getValue(), 1,
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

UtlBoolean SdpBody::getMediaProtocol(int mediaIndex, UtlString* transportProtocol) const
{
   return(getMediaSubfield(mediaIndex, 2, transportProtocol));
}

UtlBoolean SdpBody::getMediaPayloadType(int mediaIndex, int maxTypes,
                                        int* numTypes, int payloadTypes[]) const
{
   UtlString payloadTypeString;
   int typeCount = 0;

   while(typeCount < maxTypes &&
         getMediaSubfield(mediaIndex, 3 + typeCount,
                          &payloadTypeString))
   {

      if(!payloadTypeString.isNull())
      {
         payloadTypes[typeCount] = atoi(payloadTypeString.data());
         typeCount++;
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
      NameValueTokenizer::getSubField(value, subfieldIndex,
                                      SDP_SUBFIELD_SEPARATORS, subField);
      if(!subField->isNull())
      {
         subfieldFound = TRUE;
      }
   }
   return(subfieldFound);
}

UtlBoolean SdpBody::getPayloadRtpMap(int payloadType,
                                     UtlString& mimeSubtype,
                                     int& sampleRate,
                                     int& numChannels) const
{
   // an "a" record look something like:
   // "a=rtpmap: <payloadType> <mimeSubtype/sampleRate[/numChannels]"

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

   while((nv = (NameValuePair*) iterator.findNext(&aFieldMatch)) != NULL)
   {
      value =  nv->getValue();

      // Verify this is an rtpmap "a" record
      NameValueTokenizer::getSubField(value, 0,
                                      " \t:/", // seperators
                                      &aFieldType);
      if(aFieldType.compareTo("rtpmap") == 0)
      {
         // If this is the rtpmap for the requested payload type
         NameValueTokenizer::getSubField(value, 1,
                                         " \t:/", // seperators
                                         &payloadString);
         if(atoi(payloadString.data()) == payloadType)
         {
            // The mime subtype is the 3nd subfield
            NameValueTokenizer::getSubField(value, 2,
                                            " \t:/", // seperators
                                            &mimeSubtype);

            // The sample rate is the 4rd subfield
            NameValueTokenizer::getSubField(value, 3,
                                            " \t:/", // seperators
                                            &sampleRateString);
            sampleRate = atoi(sampleRateString.data());
            if(sampleRate <= 0) sampleRate = -1;

            // The number of channels is the 5th subfield
            NameValueTokenizer::getSubField(value, 4,
                                            " \t:/", // seperators
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

UtlBoolean SdpBody::getPayloadFormat(int payloadType,
                                     UtlString& fmtp,
                                     int& valueFmtp,
                                     int& numVideoSizes,
                                     int videoSizes[]) const
{
   // an "a" record look something like:
   // "a=rtpmap: <payloadType> <mimeSubtype/sampleRate[/numChannels]"

   // Loop through all of the "a" records
   UtlBoolean foundPayloadFmtp = FALSE;
   UtlBoolean foundField;
   UtlSListIterator iterator(*sdpFields);
   UtlString aFieldType;
   UtlString payloadString;
   UtlString modifierString;
   NameValuePair* nv = NULL;
   int index = 0;
   int aFieldIndex = 0;
   int videoIndex = 0;
   const char* value;
   UtlString temp;
   UtlString aFieldMatch("a");

   numVideoSizes = 0;
   valueFmtp = 0;

   while((nv = (NameValuePair*) iterator.findNext(&aFieldMatch)) != NULL)
   {
      value =  nv->getValue();

      // Verify this is an rtpmap "a" record
      NameValueTokenizer::getSubField(value, 0,
                                      " \t:/", // seperators
                                      &aFieldType);
      if(aFieldType.compareTo("fmtp") == 0)
      {
         NameValueTokenizer::getSubField(value, 1,
                                         " \t:/", // seperators
                                         &payloadString);
         if(atoi(payloadString.data()) == payloadType)
         {
            foundPayloadFmtp = TRUE;
            // Get modifier
            NameValueTokenizer::getSubField(value, 2,
                                                    " \t:=", // seperators
                                                    &modifierString);
            // If this is the rtpmap for the requested payload type
            foundField = NameValueTokenizer::getSubField(value, 3,
                                                    " \t:=", // seperators
                                                    &fmtp);


            valueFmtp = 0;
            index = 3;
            if (modifierString.compareTo("mode") == 0)
            {
                valueFmtp  = atoi(fmtp.data());
            }
            else if (modifierString.compareTo("imagesize") == 0)
            {
                // Checking size information for imagesize modifier
                valueFmtp = atoi(fmtp.data());
                switch (valueFmtp)
                {
                case 0: 
                    videoSizes[videoIndex++] = SDP_VIDEO_FORMAT_QCIF;
                    break;
                case 1:
                    videoSizes[videoIndex++] = SDP_VIDEO_FORMAT_CIF;
                    break;
                }
                valueFmtp = 0;
                numVideoSizes = videoIndex;
            }
            else if (modifierString.compareTo("size") == 0)
            {
                // Checking size information for other modifier
                while (foundField && index < 8 && videoIndex < MAXIMUM_VIDEO_SIZES)
                {
                    foundField = NameValueTokenizer::getSubField(value, index++,
                                                                " \t/:", // seperators
                                                                &temp);
                    if (temp.compareTo("CIF") == 0)
                    {
                        videoSizes[videoIndex++] = SDP_VIDEO_FORMAT_CIF;
                    }
                    else if (temp.compareTo("QVGA") == 0)
                    {
                        videoSizes[videoIndex++] = SDP_VIDEO_FORMAT_QVGA;
                    }
                    else if (temp.compareTo("QCIF") == 0)
                    {
                        videoSizes[videoIndex++] = SDP_VIDEO_FORMAT_QCIF;
                    }
                    else if (temp.compareTo("SQCIF") == 0)
                    {
                        videoSizes[videoIndex++] = SDP_VIDEO_FORMAT_SQCIF;
                    }
                }
                numVideoSizes = videoIndex;
             }
             if (videoIndex == 0)
             {
                 // Default to QCIF if no size is present
                 videoSizes[videoIndex++] = SDP_VIDEO_FORMAT_QCIF;
                 numVideoSizes = 1;
             }
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
    UtlString aFieldMatch("a");
    UtlString indexString;
    UtlString cryptoSuite;
    UtlString temp;
    int size;
    char srtpKey[SRTP_KEY_LENGTH+1];

    size = sdpFields->entries();
    while((nv = (NameValuePair*) iterator.findNext(&aFieldMatch)) != NULL)
    {
        value =  nv->getValue();

        // Verify this is an crypto "a" record
        NameValueTokenizer::getSubField(value, 0,
                                        " \t:/", // seperators
                                        &aFieldType);
        if(aFieldType.compareTo("crypto") == 0)
        {
            NameValueTokenizer::getSubField(value, 1,
                                            " \t:/", // seperators
                                            &indexString);
            if(atoi(indexString.data()) == index)
            {
                foundCrypto = TRUE;

                // Encryption & authentication on by default
                params.securityLevel = SRTP_ENCRYPTION | SRTP_AUTHENTICATION;

                NameValueTokenizer::getSubField(value, 2,
                                                " \t:/", // seperators
                                                &cryptoSuite);
                // Check the crypto suite 
                if (cryptoSuite.compareTo("AES_CM_128_HMAC_SHA1_80") == 0)
                {
                    params.cipherType = AES_CM_128_HMAC_SHA1_80;
                }
                else if (cryptoSuite.compareTo("AES_CM_128_HAMC_SHA1_32") == 0)
                {
                    params.cipherType = AES_CM_128_HAMC_SHA1_32;
                }
                else if (cryptoSuite.compareTo("F8_128_HMAC_SHA1_80") == 0)
                {
                    params.cipherType = F8_128_HMAC_SHA1_80;
                }
                else
                {
                    //Couldn't find crypto suite, no secritiy
                    params.securityLevel = 0;
                }

                // Get key
                foundField = NameValueTokenizer::getSubField(value, 4,
                                                             " \t/:|", // seperators
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
                    foundField = NameValueTokenizer::getSubField(value, index,
                                                                 " \t/:|", // seperators
                                                                 &temp);
                    if (foundField)
                    {
                        if (temp.compareTo("UNENCRYPTED_SRTP") == 0)
                        {
                            params.securityLevel &= ~SRTP_ENCRYPTION;
                        }
                        if (temp.compareTo("UNAUTHENTICATED_SRTP") == 0)
                        {
                            params.securityLevel &= ~SRTP_AUTHENTICATION;
                        }
                    }
                }

            }
            break;
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
    int size;
    UtlString temp;
    videoFramerate = 0;

    size = sdpFields->entries();
    while((nv = (NameValuePair*) iterator.findNext(&aFieldMatch)) != NULL)
    {
        value =  nv->getValue();

        // Verify this is an crypto "a" record
        NameValueTokenizer::getSubField(value, 0,
                                        " \t:/", // seperators
                                        &aFieldType);
        if(aFieldType.compareTo("framerate") == 0)
        {
            NameValueTokenizer::getSubField(value, 1,
                                            " \t:/", // seperators
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
   int iTemp = 0;

   // Indicate no "b" field was sent with 0
   bandwidth  = 0;

   while((nv = (NameValuePair*) iterator.findNext(&aFieldMatch)) != NULL)
   {
      value =  nv->getValue();

      // Verify this is an crypto "a" record
      NameValueTokenizer::getSubField(value, 0,
                                      " \t:/", // seperators
                                        &aFieldModifier);
      if(aFieldModifier.compareTo("CT") == 0)
      {
            NameValueTokenizer::getSubField(value, 1,
                                            " \t:/", // seperators
                                            &temp);

            bandwidth = atoi(temp.data());
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
      NameValueTokenizer::getSubField(value, 0,
                                      SDP_SUBFIELD_SEPARATORS, mediaType);

      // media port and media port pair count
      NameValueTokenizer::getSubField(value, 1,
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
      NameValueTokenizer::getSubField(value, 2,
                                      SDP_SUBFIELD_SEPARATORS, mediaTransportType);

      // media payload/codec types
      NameValueTokenizer::getSubField(value,  3 + typeCount,
                                      SDP_SUBFIELD_SEPARATORS, &payloadTypeString);
      while(typeCount < maxPayloadTypes &&
            !payloadTypeString.isNull())
      {
         payloadTypes[typeCount] = atoi(payloadTypeString.data());
         typeCount++;
         NameValueTokenizer::getSubField(value,  3 + typeCount,
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
            NameValueTokenizer::getSubField(value, 2,
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
               NameValueTokenizer::getSubField(value, 2,
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


void SdpBody::getBestAudioCodecs(SdpCodecFactory& localRtpCodecs,
                                 int& numCodecsInCommon,
                                 SdpCodec**& codecsInCommonArray,
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
   codecsInCommonArray = new SdpCodec*[localRtpCodecs.getCodecCount()];
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

            getCodecsInCommon(numAudioTypes,
                              numVideoTypes,
                              audioPayloadTypes,
                              videoPayloadTypes,
                              videoRtpPort,
                              localRtpCodecs,
                              numCodecsInCommon,
                              codecsInCommonArray);

            getEncryptionInCommon(localSrtpParams, remoteSrtpParams, matchingSrtpParams);

            if (numCodecsInCommon >0)
               break;
         }
         mediaAudioIndex++;
         mediaVideoIndex++;
      }
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

void SdpBody::getCodecsInCommon(int audioPayloadIdCount,
                                int videoPayloadIdCount,
                                int audioPayloadTypes[],
                                int videoPayloadTypes[],
                                int videoRtpPort,
                                SdpCodecFactory& localRtpCodecs,
                                int& numCodecsInCommon,
                                SdpCodec* codecsInCommonArray[]) const
{
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

   for(typeIndex = 0; typeIndex < audioPayloadIdCount; typeIndex++)
   {
      // Until the real SdpCodec is needed we assume all of
      // the rtpCodecs are send AND receive.
      // We are also going to cheat and assume that all of
      // the media records are send AND receive

      // Get the rtpmap for the payload type
      if(getPayloadRtpMap(audioPayloadTypes[typeIndex],
                          mimeSubtype, sampleRate, numChannels))
      {
         
         // Find a match for the mime type
         matchingCodec = localRtpCodecs.getCodec(MIME_TYPE_AUDIO, mimeSubtype.data());
         if((matchingCodec != NULL) &&
            (matchingCodec->getSampleRate() == sampleRate ||
             sampleRate == -1) &&
            (matchingCodec->getNumChannels() == numChannels ||
             numChannels == -1))
         {
            commonCodec = TRUE;
            int frameSize = 0;

            if (matchingCodec->getCodecType() == SdpCodec::SDP_CODEC_GIPS_ILBC)
            {
                getPayloadFormat(audioPayloadTypes[typeIndex], fmtp, frameSize, 
                                 numVideoSizes, videoSizes);
                if (frameSize == 20 || frameSize == 30 || frameSize == 0)
                {
                   if (frameSize == 0)
                   {
                       frameSize = 20;
                   }

                }
                else
                {
                    // Nothing in common here
                    commonCodec = FALSE;
                }
            }
            // Create a copy of the SDP codec and set
            // the payload type for it
            if (commonCodec)
            {
               codecsInCommonArray[numCodecsInCommon] =
                  new SdpCodec(*matchingCodec);
               codecsInCommonArray[numCodecsInCommon]->setCodecPayloadFormat(audioPayloadTypes[typeIndex]);
               if (frameSize)
               {
                  codecsInCommonArray[numCodecsInCommon]->setPacketSize(frameSize*1000);
               }

            }
         
            numCodecsInCommon++;

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
            // Create a copy of the SDP codec and set
            // the payload type for it
            codecsInCommonArray[numCodecsInCommon] =
               new SdpCodec(*matchingCodec);
            codecsInCommonArray[numCodecsInCommon]->setCodecPayloadFormat(audioPayloadTypes[typeIndex]);

            numCodecsInCommon++;
         }
      }
   }
   if (videoRtpPort != 0)
   {
        for(typeIndex = 0; typeIndex < videoPayloadIdCount; typeIndex++)
        {
            int videoFmtp = 0;
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
            UtlBoolean bHasRtpMap = getPayloadRtpMap(videoPayloadTypes[typeIndex],
                                                     mimeSubtype, sampleRate, numChannels);
                            // Get the video sizes in an array
            UtlBoolean bHasFmtp= getPayloadFormat(videoPayloadTypes[typeIndex], fmtp, videoFmtp, 
                                                  numVideoSizes, videoSizes);
            if (bHasRtpMap || bHasFmtp)
            {
                // Fixing iChat case where there is no rtpmap field for H263
                if (mimeSubtype.length() == 0 && videoPayloadTypes[typeIndex] == 34)
                {
                    mimeSubtype = "h263";
                }

                // Get all codecs with the same mime subtype. Same codecs with different
                // video resolutions are added seperately but have the same mime subtype.
                // The codec negotiation depends on the fact that codecs with the same
                // mime subtype are added sequentially.
                localRtpCodecs.getCodecs(numCodecs, sdpCodecArray, SDP_VIDEO_MEDIA_TYPE, mimeSubtype);

                // Loop through the other side's video sizes first so we match the size
                // preferences of the other side
                if (numVideoSizes == 0)
                {
                    // If no size parameter default to QCIF
                    numVideoSizes = 1;
                    videoSizes[0] = SDP_VIDEO_FORMAT_QCIF;
                }
                for (videoSize = 0; videoSize < numVideoSizes; videoSize++)
                {
                    for (codecIndex = 0; codecIndex < numCodecs; ++codecIndex)
                    {
                        matchingCodec = sdpCodecArray[codecIndex];
                        
                        // In addition to everything else do a bit-wise comparison of video formats. For
                        // every codec with the same sub mime type that supports one of the video formats
                        // add a seperate codec in the codecsInCommonArray.
                        if((matchingCodec != NULL) && (matchingCodec->getVideoFormat() == videoSizes[videoSize]) &&
                            (matchingCodec->getSampleRate() == sampleRate ||
                            sampleRate == -1) &&
                            (matchingCodec->getNumChannels() == numChannels ||
                            numChannels == -1
                            ))
                        {
                            // Create a copy of the SDP codec and set
                            // the payload type for it
                            codecsInCommonArray[numCodecsInCommon] =
                            new SdpCodec(*matchingCodec);
                            codecsInCommonArray[numCodecsInCommon]->setCodecPayloadFormat(videoPayloadTypes[typeIndex]);
                            numCodecsInCommon++;

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
                    codecsInCommonArray[numCodecsInCommon] =
                    new SdpCodec(*matchingCodec);
                    codecsInCommonArray[numCodecsInCommon]->setCodecPayloadFormat(videoPayloadTypes[typeIndex]);

                    numCodecsInCommon++;
                }
            }
        }
   }
}

void SdpBody::addAudioCodecs(int iNumAddresses,
                             UtlString hostAddresses[],
                             int rtpAudioPorts[],
                             int rtcpAudioPorts[],
                             int rtpVideoPorts[],
                             int rtcpVideoPorts[],
                             int numRtpCodecs,
                             SdpCodec* rtpCodecs[],
                             SdpSrtpParameters& srtpParams,
                             int totalBandwidth,
                             int videoFramerate)
{
   int codecArray[MAXIMUM_MEDIA_TYPES];
   int formatArray[MAXIMUM_MEDIA_TYPES];
   UtlString videoFormat;
   int codecIndex;
   int destIndex;
   int firstMimeSubTypeIndex;
   int preExistingMedia = getMediaSetCount();
   UtlString mimeType;
   UtlString seenMimeType;
   UtlString mimeSubType;
   UtlString prevMimeSubType = "none";
   int numAudioCodecs=0;
   int numVideoCodecs=0;

   assert(iNumAddresses > 0) ;

   memset(formatArray, 0, sizeof(int)*MAXIMUM_MEDIA_TYPES);

   // If there are not media fields we only need one global one
   // for the SDP body
   if(!preExistingMedia)
   {
      addAddressData(hostAddresses[0]);
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
      if (mimeType.compareTo(SDP_AUDIO_MEDIA_TYPE) == 0 || mimeType.compareTo(SDP_VIDEO_MEDIA_TYPE) != 0)
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
           addSrtpCryptoField(srtpParams);
      }
      else
      {
          // Add the media record
           addMediaData(SDP_AUDIO_MEDIA_TYPE, rtpAudioPorts[0], 1,
                        SDP_RTP_MEDIA_TRANSPORT_TYPE, numAudioCodecs,
                        codecArray);
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
          for (int i=0; i<iNumAddresses; i++)
          {
              double priority = (double) (iNumAddresses-i) / (double) iNumAddresses ;

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

      // add attribute records defining the extended types
      addCodecParameters(numRtpCodecs, rtpCodecs, seenMimeType.data(), videoFramerate);

      // If this is not the only media record we need a local
      // address record for this media record
      if(preExistingMedia)
      {
         addAddressData(hostAddresses[0]);
      }
   }

   // Stuff the SDP video codecs codes in an integer array
   for(codecIndex = 0, destIndex = -1;
       codecIndex < MAXIMUM_MEDIA_TYPES && codecIndex < numRtpCodecs;
       codecIndex++)
   {
      rtpCodecs[codecIndex]->getMediaType(mimeType);

      if (mimeType.compareTo(SDP_VIDEO_MEDIA_TYPE) == 0)
      {
         rtpCodecs[codecIndex]->getEncodingName(mimeSubType);

         if (mimeSubType.compareTo(prevMimeSubType) == 0)
         {
            // If we still have the same mime type only change format. We're depending on the
            // fact that codecs with the same mime subtype are added sequentially to the 
            // codec factory. Otherwise this won't work.
            formatArray[destIndex] |= (rtpCodecs[codecIndex])->getVideoFormat();
            (rtpCodecs[firstMimeSubTypeIndex])->setVideoFmtp(formatArray[destIndex]);
            (rtpCodecs[firstMimeSubTypeIndex])->setVideoFmtpString((rtpCodecs[codecIndex])->getVideoFormat());
         }
         else
         {
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

   if (rtpVideoPorts[0] > 0)
   {
      // If any security is enabled we set RTP/SAVP and add a crypto field - not for video at this time
      if (0)
      {
         // Add the media record
         addMediaData(SDP_VIDEO_MEDIA_TYPE, rtpVideoPorts[0], 1,
                      SDP_SRTP_MEDIA_TRANSPORT_TYPE, numVideoCodecs,
                      codecArray);
         addSrtpCryptoField(srtpParams);
      }
      else
      {
         // Add the media record
         addMediaData(SDP_VIDEO_MEDIA_TYPE, rtpVideoPorts[0], 1,
                      SDP_RTP_MEDIA_TRANSPORT_TYPE, numVideoCodecs,
                      codecArray);
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
          for (int i=0; i<iNumAddresses; i++)
          {
              double priority = (double) (iNumAddresses-i) / (double) iNumAddresses ;

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

      // add attribute records defining the extended types
      addCodecParameters(numRtpCodecs, rtpCodecs, SDP_VIDEO_MEDIA_TYPE, videoFramerate);

      // If this is not the only media record we need a local
      // address record for this media record
      if(preExistingMedia)
      {
         addAddressData(hostAddresses[0]);
            // Indicate a low bandwidth client
      }
      if (totalBandwidth != 0)
      {
          char ct[16];
          sprintf(ct, "CT:%d", totalBandwidth);
          setValue("b", ct);
      }
   }
}



void SdpBody::addCodecParameters(int numRtpCodecs,
                                 SdpCodec* rtpCodecs[],
                                 const char *szMimeType, 
                                 int videoFramerate)
{
   const SdpCodec* codec = NULL;
   UtlString mimeSubtype;
   int payloadType;
   int sampleRate;
   int numChannels;
   int videoFmtp;
   char valueBuf[64];
   UtlString formatParameters;
   UtlString mimeType;
   UtlString prevMimeSubType = "none";
   UtlString formatTemp;
   UtlString formatString;

   for(int codecIndex = 0;
       codecIndex < MAXIMUM_MEDIA_TYPES && codecIndex < numRtpCodecs;
       codecIndex++)
   {
      codec = rtpCodecs[codecIndex];
      rtpCodecs[codecIndex]->getMediaType(mimeType);
      if(codec && mimeType.compareTo(szMimeType) == 0)
      {
         codec->getEncodingName(mimeSubtype);
         // Only add to map if we have a new mime sub type
         if (mimeSubtype.compareTo(prevMimeSubType) != 0)
         {
            prevMimeSubType = mimeSubtype;
            sampleRate = codec->getSampleRate();
            numChannels = codec->getNumChannels();
            codec->getSdpFmtpField(formatParameters);
            payloadType = codec->getCodecPayloadFormat();

            // Build an rtpmap
            addRtpmap(payloadType, mimeSubtype.data(),
                       sampleRate, numChannels);

            if ((videoFmtp=codec->getVideoFmtp()) != 0)
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
                    }
                }
            }
            if (codec->getCodecType() == SdpCodec::SDP_CODEC_GIPS_ILBC)
            {
                sprintf(valueBuf, "mode=%d", codec->getPacketLength()/1000);
                formatParameters = valueBuf;
            }
            // Add the format specific parameters if present
            if(!formatParameters.isNull())
            {
                addFormatParameters(payloadType,
                                    formatParameters.data());
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
      }
   }
}


void SdpBody::addAudioCodecs(int iNumAddresses,
                             UtlString hostAddresses[],
                             int rtpAudioPorts[],
                             int rtcpAudioPorts[],
                             int rtpVideoPorts[],
                             int rtcpVideoPorts[],
                             int numRtpCodecs, 
                             SdpCodec* rtpCodecs[], 
                             SdpSrtpParameters& srtpParams,
                             int totalBandwidth,
                             int videoFramerate,
                             const SdpBody* sdpRequest)
{
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
   int supportedPayloadCount;
   int destIndex;
   int firstMimeSubTypeIndex;
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
      addAddressData(hostAddresses[0]);
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
         // Check for unsupported stuff
         // i.e. non audio media, non RTP transported media, etc
          if((mediaType.compareTo(SDP_AUDIO_MEDIA_TYPE, UtlString::ignoreCase) != 0 &&
                mediaType.compareTo(SDP_VIDEO_MEDIA_TYPE, UtlString::ignoreCase) != 0) ||
                mediaPort <= 0 || mediaPortPairs <= 0 ||
                (mediaTransportType.compareTo(SDP_RTP_MEDIA_TRANSPORT_TYPE, UtlString::ignoreCase) != 0 &&
                mediaTransportType.compareTo(SDP_SRTP_MEDIA_TRANSPORT_TYPE, UtlString::ignoreCase) != 0))
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
            if (mediaType.compareTo(SDP_AUDIO_MEDIA_TYPE) == 0)
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

   SdpCodecFactory codecFactory(numRtpCodecs,
                                    rtpCodecs);

   supportedPayloadCount = 0;
   sdpRequest->getCodecsInCommon(numAudioPayloadTypes, numVideoPayloadTypes, 
                                 audioPayloadTypes, videoPayloadTypes, videoPort,
                                 codecFactory, supportedPayloadCount,
                                 codecsInCommon);

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
            if (mediaType.compareTo(SDP_AUDIO_MEDIA_TYPE) == 0)
            {
                supportedPayloadTypes[destIndex++] =
                        codecsInCommon[payloadIndex]->getCodecPayloadFormat();
            }
        }
        addMediaData(SDP_AUDIO_MEDIA_TYPE, rtpAudioPorts[0], 1,
                    audioTransportType.data(), destIndex,
                    supportedPayloadTypes);
        if (commonAudioSrtpParams.securityLevel)
        {
            addSrtpCryptoField(commonAudioSrtpParams);
        }

        if ((audioTransportType.compareTo(SDP_RTP_MEDIA_TRANSPORT_TYPE, UtlString::ignoreCase) == 0) ||
                (audioTransportType.compareTo(SDP_SRTP_MEDIA_TRANSPORT_TYPE, UtlString::ignoreCase) == 0))
        {
            // It is assumed that rtcp is the odd port immediately after 
            // the rtp port.  If that is not true, we must add a parameter 
            // to specify the rtcp port.
            if ((rtcpAudioPorts[0] > 0) && ((rtcpAudioPorts[0] != rtpAudioPorts[0] + 1) 
                    || (rtcpAudioPorts[0] % 2) == 0))
            {
                char cRtcpBuf[32] ;
                sprintf(cRtcpBuf, "rtcp:%d", rtcpAudioPorts[0]) ;
                addValue("a", cRtcpBuf) ;
            }

            // Add candidate addresses if available
            if (iNumAddresses > 1)
            {
                for (int i=0; i<iNumAddresses; i++)
                {
                    double priority = (double) (iNumAddresses-i) / (double) iNumAddresses ;

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
                            codecsInCommon, SDP_AUDIO_MEDIA_TYPE, videoFramerate);

        // Then do this for video
        destIndex = -1;
        for(payloadIndex = 0;
            payloadIndex < supportedPayloadCount;
            payloadIndex++)
        {
            codecsInCommon[payloadIndex]->getMediaType(mediaType);
            if (mediaType.compareTo(SDP_VIDEO_MEDIA_TYPE) == 0)
            {
                // We've found at least one common video codec
                commonVideo = TRUE;
                codecsInCommon[payloadIndex]->getEncodingName(mimeSubType);

                // If we still have the same mime type only change format. We're depending on the
                // fact that codecs with the same mime subtype are added sequentially to the 
                // codec factory. Otherwise this won't work.
                if (prevMimeSubType.compareTo(mimeSubType) == 0)
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
            // We do not support video encryption at this time 
            //if (commonVideoSrtpParams.securityLevel)
            //{
            //    addSrtpCryptoField(commonAudioSrtpParams);
            //}

            if ((audioTransportType.compareTo(SDP_RTP_MEDIA_TRANSPORT_TYPE, UtlString::ignoreCase) == 0) ||
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
                    double priority = (double) (iNumAddresses-i) / (double) iNumAddresses ;

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
                                codecsInCommon, SDP_VIDEO_MEDIA_TYPE, videoFramerate);
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
    if (codecsInCommon != NULL) {
        for(int codecIndex = 0; codecIndex < supportedPayloadCount; codecIndex++)
        {
            delete codecsInCommon[codecIndex];
            codecsInCommon[codecIndex] = NULL;
        }
    }


    if(preExistingMedia)
    {
        addAddressData(hostAddresses[0]);
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
    case AES_CM_128_HAMC_SHA1_32:
        fieldValue.append("AES_CM_128_HAMC_SHA1_32 ");
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

void SdpBody::addCandidateAttribute(int         candidateId, 
                                    const char* transportId, 
                                    const char* transportType,
                                    double      qValue, 
                                    const char* candidateIp, 
                                    int         candidatePort) 
{
    UtlString attributeData ;
    char buffer[64] ;    
    
    attributeData.append("cand:") ;

    sprintf(buffer, "%d", candidateId) ;
    attributeData.append(buffer) ;
    attributeData.append(" ") ;

    attributeData.append(transportId) ;
    attributeData.append(" ") ;

    attributeData.append(transportType) ;
    attributeData.append(" ") ;
    
    sprintf(buffer, "%.1f", qValue) ;
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
                                          double& rQvalue, 
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
        while (nv = findFieldNameBefore(&iterator, aFieldMatch, "m"))
        {
            value =  nv->getValue();
            
            // Verify this is an candidate "a" record
            UtlTokenizer tokenizer(value) ;
            if (tokenizer.next(aFieldType, ":"))
            {
                aFieldType.toLower() ;
                aFieldType.strip(UtlString::both, ' ') ;
                if(aFieldType.compareTo("cand") == 0)
                {
                    if (aFieldIndex == candidateIndex)
                    {
                        UtlString tmpCandidateId ;
                        UtlString tmpQvalue ;                        
                        UtlString tmpCandidatePort ;

                        // candidate:id qValue userFrag password Ip port candidateIp candidatePort         
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
                            rQvalue = atof(tmpQvalue) ;
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
                                           double      qvalues[], 
                                           UtlString   candidateIps[], 
                                           int         candidatePorts[],
                                           int&        nActualAddresses) const 
{
    int mediaIndex = findMediaType(szMimeType, 0) ;
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

   addValue("m", value.data());

}

void SdpBody::addAddressData(const char* ipAddress)
{
   const char* networkType = SDP_NETWORK_TYPE;
   const char* addressType = SDP_IP4_ADDRESS_TYPE;
   addAddressData(networkType, addressType, ipAddress);
}

void SdpBody::addAddressData(const char* networkType, const char* addressType, const char* ipAddress)
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
   if(UTL_NOT_FOUND == (unsigned int)fieldIndex)
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
   
   while((nv = dynamic_cast<NameValuePair*>(iterator())))
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
   while((nv = dynamic_cast<NameValuePair*>(iterator())))
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
   UtlString aFieldMatch("a");

   while((nv = (NameValuePair*) iterator.findNext(&aFieldMatch)) != NULL)
   {
       if ( strcmp(nv->getValue(), pvalue) == 0 )
          return TRUE;
   }

   return FALSE;
}


/* ============================ FUNCTIONS ================================= */
