// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdio.h>
#include <stdlib.h>

// APPLICATION INCLUDES
#include <utl/UtlSListIterator.h>
#include <net/SdpBody.h>
#include <net/NameValuePair.h>
#include <net/NameValueTokenizer.h>
#include <net/SdpCodecFactory.h>

#define MAXIMUM_LONG_INT_CHARS 20
#define MAXIMUM_MEDIA_TYPES 20

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define SDP_NAME_VALUE_DELIMITOR '='
#define SDP_SUBFIELD_SEPARATOR ' '
#define SDP_SUBFIELD_SEPARATORS "\t "

#define SDP_AUDIO_MEDIA_TYPE "audio"
#define SDP_VIDEO_MEDIA_TYPE "video"
#define SDP_APPLICATION_MEDIA_TYPE "application"

#define SDP_RTP_MEDIA_TRANSPORT_TYPE "RTP/AVP"
#define SDP_MLAW_PAYLOAD 0
#define SDP_ALAW_PAYLOAD 8

#define SDP_NETWORK_TYPE "IN"
#define SDP_IP4_ADDRESS_TYPE "IP4"
#define NTP_TO_EPOCH_DELTA 2208988800UL

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SdpBody::SdpBody(const char* bodyBytes, int byteCount)
{
   mBodyContentType.remove(0);
   mBodyContentType.append(SDP_CONTENT_TYPE);

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
   setOriginator("sipX", 5, 5,
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

void SdpBody::getBestAudioCodecs(//int numRtpCodecs,
   SdpCodecFactory& localRtpCodecs,
                                 int& numCodecsInCommon,
                                 SdpCodec**& codecsInCommonArray,
                                 UtlString& rtpAddress, int& rtpPort) const
{

   int mediaIndex = 0;
   int numTypes;
   int payloadTypes[MAXIMUM_MEDIA_TYPES];
   numCodecsInCommon = 0;
   codecsInCommonArray = new SdpCodec*[localRtpCodecs.getCodecCount()];

   rtpAddress.remove(0);
   rtpPort = 0;
   while(mediaIndex >= 0)
   {
      mediaIndex = findMediaType(SDP_AUDIO_MEDIA_TYPE, mediaIndex);

      if(mediaIndex >= 0)
      {
         // This is kind of a bad assumption if there is more
         // than one media field, each might have a different
         // port and address
         getMediaPort(mediaIndex, &rtpPort);
         getMediaAddress(mediaIndex, &rtpAddress);

         if(rtpPort >= 0)
         {
            getMediaPayloadType(mediaIndex, MAXIMUM_MEDIA_TYPES,
                                &numTypes, payloadTypes);

            getCodecsInCommon(numTypes,
                              payloadTypes,
                              localRtpCodecs,
                              numCodecsInCommon,
                              codecsInCommonArray);
            if (numCodecsInCommon >0)
               break;
         }
         mediaIndex++;
      }
   }
}


void SdpBody::getCodecsInCommon(int payloadIdCount,
                                int payloadTypes[],
                                SdpCodecFactory& localRtpCodecs,
                                int& numCodecsInCommon,
                                SdpCodec* codecsInCommonArray[]) const
{
   UtlString mimeSubtype;
   int sampleRate;
   int numChannels;
   const SdpCodec* matchingCodec = NULL;

   numCodecsInCommon = 0;

   for(int typeIndex = 0; typeIndex < payloadIdCount; typeIndex++)
   {
      // Until the real SdpCodec is needed we assume all of
      // the rtpCodecs are send AND receive.
      // We are also going to cheat and assume that all of
      // the media records are send AND receive

      // Get the rtpmap for the payload type
      if(getPayloadRtpMap(payloadTypes[typeIndex],
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
            // Create a copy of the SDP codec and set
            // the payload type for it
            codecsInCommonArray[numCodecsInCommon] =
               new SdpCodec(*matchingCodec);
            codecsInCommonArray[numCodecsInCommon]->setCodecPayloadFormat(payloadTypes[typeIndex]);
            numCodecsInCommon++;

         }
      }

      // If no payload type set and this is a static payload
      // type assume the payload type is the same as our internal
      // codec id
      else if(payloadTypes[typeIndex] <=
              SdpCodec::SDP_CODEC_MAXIMUM_STATIC_CODEC)
      {
         if((matchingCodec = localRtpCodecs.getCodecByType(payloadTypes[typeIndex])))
         {
            // Create a copy of the SDP codec and set
            // the payload type for it
            codecsInCommonArray[numCodecsInCommon] =
               new SdpCodec(*matchingCodec);
            codecsInCommonArray[numCodecsInCommon]->setCodecPayloadFormat(payloadTypes[typeIndex]);

            numCodecsInCommon++;
         }
      }
   }
}

void SdpBody::addAudioCodecs(const char* rtpAddress, int rtpPort, 
                             int rtcpPort, int numRtpCodecs, 
                             SdpCodec* rtpCodecs[])
{
   int codecArray[MAXIMUM_MEDIA_TYPES];
   int codecIndex;
   int preExistingMedia = getMediaSetCount();

   // If there are not media fields we only need one global one
   // for the SDP body
   if(!preExistingMedia)
   {
      addAddressData(rtpAddress);
      char timeString[100];
      sprintf(timeString, "%d %d", 0, //OsDateTime::getSecsSinceEpoch(),
              0);
      addValue("t", timeString);
   }

   // Stuff the SDP codec codes in an integer array
   for(codecIndex = 0;
       codecIndex < MAXIMUM_MEDIA_TYPES && codecIndex < numRtpCodecs;
       codecIndex++)
   {
      codecArray[codecIndex] =
         (rtpCodecs[codecIndex])->getCodecPayloadFormat();
   }


   // Add the media record
   addMediaData(SDP_AUDIO_MEDIA_TYPE, rtpPort, 1,
         SDP_RTP_MEDIA_TRANSPORT_TYPE, numRtpCodecs,
         codecArray);

   // It is assumed that rtcp is the odd port immediately after the rtp port.
   // If that is not true, we must add a parameter to specify the rtcp port.
   if ((rtcpPort > 0) && ((rtcpPort != rtpPort + 1) || (rtcpPort % 2) == 0))
   {
       char cRtcpBuf[32] ;
       sprintf(cRtcpBuf, "rtcp:%d", rtcpPort) ;

       addValue("a", cRtcpBuf) ;
   }

   // add attribute records defining the extended types
   addCodecParameters(numRtpCodecs, rtpCodecs);

   // If this is not the only media record we need a local
   // address record for this media record
   if(preExistingMedia)
   {
      addAddressData(rtpAddress);
   }
}

void SdpBody::addCodecParameters(int numRtpCodecs,
                                 SdpCodec* rtpCodecs[])
{
   const SdpCodec* codec = NULL;
   UtlString mimeSubtype;
   int payloadType;
   int sampleRate;
   int numChannels;
   UtlString formatParameters;
   for(int codecIndex = 0;
       codecIndex < MAXIMUM_MEDIA_TYPES && codecIndex < numRtpCodecs;
       codecIndex++)
   {
      codec = rtpCodecs[codecIndex];
      if(codec)
      {
         codec->getEncodingName(mimeSubtype);
         sampleRate = codec->getSampleRate();
         numChannels = codec->getNumChannels();
         codec->getSdpFmtpField(formatParameters);
         payloadType = codec->getCodecPayloadFormat();

         // Build an rtpmap
         addRtpmap(payloadType, mimeSubtype.data(),
                   sampleRate, numChannels);

         // Add the format specific parameters if present
         if(!formatParameters.isNull())
         {
            addFormatParameters(payloadType,
                                formatParameters.data());
         }
      }
   }
}

void SdpBody::addAudioCodecs(const char* rtpAddress, int rtpPort,
                             int rtcpPort, int numRtpCodecs, 
                             SdpCodec* rtpCodecs[],const SdpBody* sdpRequest)
{
   int preExistingMedia = getMediaSetCount();
   int mediaIndex = 0;
   UtlBoolean fieldFound = TRUE;
   UtlString mediaType;
   int mediaPort;
   int mediaPortPairs;
   UtlString mediaTransportType;
   int numPayloadTypes;
   int payloadTypes[MAXIMUM_MEDIA_TYPES];
   int supportedPayloadTypes[MAXIMUM_MEDIA_TYPES];
   SdpCodec* codecsInCommon[MAXIMUM_MEDIA_TYPES];
   int supportedPayloadCount;

   // if there are no media fields already, add a global
   // address field
   if(!preExistingMedia)
   {
      addAddressData(rtpAddress);
      char timeString[100];
      sprintf(timeString, "%d %d", 0, //OsDateTime::getSecsSinceEpoch(),
              0);
      addValue("t", timeString);
   }

   // Loop through the fields in the sdpRequest
   while(fieldFound)
   {
      fieldFound = sdpRequest->getMediaData(mediaIndex, &mediaType,
                                            &mediaPort, &mediaPortPairs,
                                            &mediaTransportType,
                                            MAXIMUM_MEDIA_TYPES, &numPayloadTypes,
                                            payloadTypes);

      if(fieldFound)
      {
         // Check for unsupported stuff
         // i.e. non audio media, non RTP transported media, etc
         if(strcmp(mediaType.data(), SDP_AUDIO_MEDIA_TYPE) != 0 ||
            mediaPort <= 0 || mediaPortPairs <= 0 ||
            strcmp(mediaTransportType.data(), SDP_RTP_MEDIA_TRANSPORT_TYPE) != 0)
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
            SdpCodecFactory codecFactory(numRtpCodecs,
                                         rtpCodecs);

            supportedPayloadCount = 0;
            sdpRequest->getCodecsInCommon(numPayloadTypes, payloadTypes,
                                          codecFactory, supportedPayloadCount,
                                          codecsInCommon);

            // Add the modified list of supported codecs
            if(supportedPayloadCount)
            {
               for(int payloadIndex = 0;
                   payloadIndex < supportedPayloadCount;
                   payloadIndex++)
               {
                  supportedPayloadTypes[payloadIndex] =
                     codecsInCommon[payloadIndex]->getCodecPayloadFormat();
               }

               mediaPort = rtpPort;
               mediaPortPairs = 1;
               addMediaData(mediaType.data(),
                            mediaPort, mediaPortPairs,
                            mediaTransportType.data(),
                            supportedPayloadCount,
                            supportedPayloadTypes);

               if (strcmp(mediaTransportType.data(), SDP_RTP_MEDIA_TRANSPORT_TYPE) == 0)
               {
                   // It is assumed that rtcp is the odd port immediately after 
                   // the rtp port.  If that is not true, we must add a parameter 
                   // to specify the rtcp port.
                   if ((rtcpPort > 0) && ((rtcpPort != rtpPort + 1) || (rtcpPort % 2) == 0))
                   {
                       char cRtcpBuf[32] ;
                       sprintf(cRtcpBuf, "rtcp:%d", rtcpPort) ;
                       addValue("a", cRtcpBuf) ;
                   }
               }

               addCodecParameters(supportedPayloadCount,
                                  codecsInCommon);
            }

            // Zero out the port to indicate none are supported
            else
            {
               mediaPort = 0;
               addMediaData(mediaType.data(),
                            mediaPort, mediaPortPairs,
                            mediaTransportType.data(),
                            numPayloadTypes,
                            payloadTypes);
            }

            // Free up the codec copies
            if (codecsInCommon != NULL) {
               for(int codecIndex = 0; codecIndex < supportedPayloadCount; codecIndex++)
               {
                  delete codecsInCommon[codecIndex];
                  codecsInCommon[codecIndex] = NULL;
               }
            }
         }

         if(preExistingMedia)
         {
            addAddressData(rtpAddress);
         }

         // Copy all atribute fields verbatum
         // someday
      }

      mediaIndex++;
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

   addValue("c", value.data());
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


/* ============================ FUNCTIONS ================================= */

