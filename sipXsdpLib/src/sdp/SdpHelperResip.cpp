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

#ifdef BUILD_RESIP_SDP_HELPER

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <sdp/SdpHelperResip.h>
#include <sdp/Sdp.h>
#include <sdp/SdpMediaLine.h>
#include <sdp/SdpCodec.h>
#include <sdp/SdpCandidate.h>

using namespace resip;

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

Sdp::SdpAddressType SdpHelperResip::convertResipAddressType(resip::SdpContents::AddrType resipAddrType)
{
   return resipAddrType == SdpContents::IP4 ? Sdp::ADDRESS_TYPE_IP4 : Sdp::ADDRESS_TYPE_IP6;
}

SdpMediaLine::SdpEncryptionMethod SdpHelperResip::convertResipEncryptionMethod(resip::SdpContents::Session::Encryption::KeyType resipMethod)
{
   switch(resipMethod)
   {
   case SdpContents::Session::Encryption::Prompt:
      return SdpMediaLine::ENCRYPTION_METHOD_PROMPT;
   case SdpContents::Session::Encryption::Clear:
      return SdpMediaLine::ENCRYPTION_METHOD_CLEAR;
   case SdpContents::Session::Encryption::Base64:
      return SdpMediaLine::ENCRYPTION_METHOD_BASE64;
   case SdpContents::Session::Encryption::UriKey:
      return SdpMediaLine::ENCRYPTION_METHOD_URI;
   default:
      return SdpMediaLine::ENCRYPTION_METHOD_NONE;
   }
}

Sdp* SdpHelperResip::createSdpFromResipSdp(const resip::SdpContents& resipSdp)
{
   bool rtcpEnabled = true;
   Sdp* sdp = new Sdp();
   const SdpContents::Session* resipSession = &resipSdp.session();

   
   sdp->setSdpVersion(resipSession->version()); // v=
   
   sdp->setOriginatorInfo(resipSession->origin().user().c_str(),   // o=
                          resipSession->origin().getSessionId(), 
                          resipSession->origin().getVersion(), 
                          Sdp::NET_TYPE_IN, 
                          convertResipAddressType(resipSession->origin().getAddressType()), 
                          resipSession->origin().getAddress().c_str());

   sdp->setSessionName(resipSession->name().c_str());  // s=
   sdp->setSessionInformation(resipSession->information().c_str());  // i=
   if(!resipSession->uri().host().empty())
   {
      sdp->setSessionUri(Data::from(resipSession->uri()).c_str()); // u=
   }

   // Populate email addresses, e=
   {
      const std::list<SdpContents::Session::Email>& emails = resipSession->getEmails();
      std::list<SdpContents::Session::Email>::const_iterator it;
      for(it = emails.begin(); it != emails.end(); it++)
      {
         if(it->getFreeText().empty())
         {
            sdp->addEmailAddress(it->getAddress().c_str());
         }
         else
         {
            sdp->addEmailAddress(Data(it->getAddress() + Data(" (") + it->getFreeText() + Data(")")).c_str());
         }
      }
   }

   // Populate phone numbers, p=
   {
      const std::list<SdpContents::Session::Phone>& phones = resipSession->getPhones();
      std::list<SdpContents::Session::Phone>::const_iterator it;
      for(it = phones.begin(); it != phones.end(); it++)
      {
         if(it->getFreeText().empty())
         {
            sdp->addPhoneNumber(it->getNumber().c_str());
         }
         else
         {
            sdp->addPhoneNumber(Data(it->getNumber() + Data(" (") + it->getFreeText() + Data(")")).c_str());
         }
      }
   }

   // Populate bandwidths, b=
   {
      const std::list<SdpContents::Session::Bandwidth>& bandwidths = resipSession->bandwidths();
      std::list<SdpContents::Session::Bandwidth>::const_iterator it;
      bool RRSetToZero=false;
      bool RSSetToZero=false;
      for(it = bandwidths.begin(); it != bandwidths.end(); it++)
      {
         Sdp::SdpBandwidthType type = Sdp::SdpBandwidth::getTypeFromString(it->modifier().c_str());
         if(type != Sdp::BANDWIDTH_TYPE_NONE)
         {
            sdp->addBandwidth(type, it->kbPerSecond());
            if(type == Sdp::BANDWIDTH_TYPE_RR && it->kbPerSecond() == 0) 
            {
               RRSetToZero = true;
            }
            else if(type == Sdp::BANDWIDTH_TYPE_RS && it->kbPerSecond() == 0)
            {
               RSSetToZero = true;
            }
         }
      }
      // Rtcp if considered disabled if RR and RS are both set to 0
      if(RRSetToZero && RSSetToZero) 
      {
         rtcpEnabled = false; 
      }
   }

   // Populate Times, t=, r=
   {
      const std::list<SdpContents::Session::Time>& times = resipSession->getTimes();
      std::list<SdpContents::Session::Time>::const_iterator it;
      for(it = times.begin(); it != times.end(); it++)
      {
         Sdp::SdpTime *sdpTime = new Sdp::SdpTime(it->getStart(), it->getStop());
         
         // Add repeats
         const std::list<SdpContents::Session::Time::Repeat>& repeats = it->getRepeats();
         std::list<SdpContents::Session::Time::Repeat>::const_iterator it2;
         for(it2 = repeats.begin(); it2 != repeats.end(); it2++)
         {  
            Sdp::SdpTime::SdpTimeRepeat* sdpTimeRepeat = new Sdp::SdpTime::SdpTimeRepeat(it2->getInterval(), it2->getDuration());

            // Add offsets
            const std::list<int>& offsets = it2->getOffsets();
            std::list<int>::const_iterator it3;
            for(it3 = offsets.begin(); it3 != offsets.end(); it3++)
            {
               sdpTimeRepeat->addOffsetFromStartTime(*it3);
            }
            sdpTime->addRepeat(sdpTimeRepeat);
         }
         sdp->addTime(sdpTime);
      }
   }

   // Populate Timezones, z=
   {
      const std::list<SdpContents::Session::Timezones::Adjustment> &adjustments = resipSession->getTimezones().getAdjustments();
      std::list<SdpContents::Session::Timezones::Adjustment>::const_iterator it;
      for(it = adjustments.begin(); it != adjustments.end(); it++)
      {
         sdp->addTimeZone(it->time, it->offset);
      } 
   }

   // Look for any session level attributes
   if(resipSession->exists("cat"))
   {
      sdp->setCategory(resipSession->getValues("cat").front().c_str());
   }
   if(resipSession->exists("keywds"))
   {
      sdp->setKeywords(resipSession->getValues("keywds").front().c_str());
   }
   if(resipSession->exists("tool"))
   {
      sdp->setToolNameAndVersion(resipSession->getValues("tool").front().c_str());
   }
   if(resipSession->exists("type"))
   {
      sdp->setConferenceType(Sdp::getConferenceTypeFromString(resipSession->getValues("type").front().c_str()));
   }
   if(resipSession->exists("charset"))
   {
      sdp->setCharSet(resipSession->getValues("charset").front().c_str());
   }
   if(resipSession->exists("ice-passive"))
   {
      sdp->setIcePassiveOnlyMode(true);
   }
   if(resipSession->exists("group"))
   {
      const std::list<Data>& values = resipSession->getValues("group");
      std::list<Data>::const_iterator it;
      for(it = values.begin(); it != values.end(); it++)
      {
         Data token;
         ParseBuffer pb(*it);
         const char* anchor = pb.position();
         pb.skipToChar(Symbols::SPACE[0]);
         if(!pb.eof())
         {
            pb.data(token, anchor);

            Sdp::SdpGroupSemantics semantics = Sdp::SdpGroup::getSemanticsFromString(token.c_str());
            if(semantics != Sdp::GROUP_SEMANTICS_NONE)
            {
               Sdp::SdpGroup* sdpGroup = new Sdp::SdpGroup(semantics);
               while(!pb.eof())
               {
                  anchor = pb.skipWhitespace();
                  pb.skipToChar(Symbols::SPACE[0]);
                  pb.data(token, anchor);
                  sdpGroup->addIdentificationTag(token.c_str());
               }
               sdp->addGroup(sdpGroup);
            }
         }
      }
   }
   if(resipSession->exists("lang"))
   {
      sdp->setSessionLanguage(resipSession->getValues("lang").front().c_str());
   }
   if(resipSession->exists("sdplang"))
   {
      sdp->setDescriptionLanguage(resipSession->getValues("sdplang").front().c_str());
   }
   if(resipSession->exists("maxprate"))
   {
      sdp->setMaximumPacketRate(resipSession->getValues("maxprate").front().convertDouble());
   }

   // Iterate through the m= lines
   const std::list<SdpContents::Session::Medium>& medias = resipSession->media();
   std::list<SdpContents::Session::Medium>::const_iterator it;
   for(it = medias.begin(); it != medias.end(); it++)
   {
      bool rtcpEnabledForMedia = rtcpEnabled;  // Default to Session setting
      const SdpContents::Session::Medium& resipMedia = *it;
      SdpMediaLine* mediaLine = new SdpMediaLine();
  
      mediaLine->setMediaType(SdpMediaLine::getMediaTypeFromString(resipMedia.name().c_str()));
      mediaLine->setTransportProtocolType(SdpMediaLine::getTransportProtocolTypeFromString(resipMedia.protocol().c_str()));

      // Get PTime for media line to assign to codecs
      unsigned int ptime=20; // default  !slg! this default should be dependant on codec type - ie. G723 should be 30, etc.
      if(resipMedia.exists("ptime"))
      {
         ptime = resipMedia.getValues("ptime").front().convertInt();
         mediaLine->setPacketTime(ptime);
      }

      // Iterate Through Codecs
      {
         const std::list<SdpContents::Session::Codec>& codecs = resipMedia.codecs();
         std::list<SdpContents::Session::Codec>::const_iterator it2;
         for(it2 = codecs.begin(); it2 != codecs.end(); it2++)
         {  
            const SdpContents::Session::Codec& resipCodec = *it2;
   
            SdpCodec* codec = new SdpCodec(resipCodec.payloadType(), 
                                           resipMedia.name().c_str(), 
                                           resipCodec.getName().c_str(), 
                                           resipCodec.getRate(), 
                                           ptime * 1000, 
                                           resipCodec.encodingParameters().empty() ? 1 : resipCodec.encodingParameters().convertInt() /* Num Channels */, 
                                           resipCodec.parameters().c_str());
            mediaLine->addCodec(codec);
         }
      }

      mediaLine->setTitle(resipMedia.information().c_str());  // i=

      // Iterrate through bandwidths, b=     
      // Note:  this is done before connections so that we can see if RTCP is disabled or not
      {
         const std::list<SdpContents::Session::Bandwidth>& bandwidths = resipMedia.bandwidths();
         std::list<SdpContents::Session::Bandwidth>::const_iterator it;
         bool RRSetToZero=false;
         bool RSSetToZero=false;
         for(it = bandwidths.begin(); it != bandwidths.end(); it++)
         {
            Sdp::SdpBandwidthType type = Sdp::SdpBandwidth::getTypeFromString(it->modifier().c_str());
            if(type != Sdp::BANDWIDTH_TYPE_NONE)
            {
               mediaLine->addBandwidth(type, it->kbPerSecond());
               if(type == Sdp::BANDWIDTH_TYPE_RR)
               {
                  if(it->kbPerSecond() == 0) 
                  {
                     RRSetToZero = true;
                  }
                  else
                  {
                     // Explicit settings in the media override session bandwidth settings that may have disabled rtcp
                     rtcpEnabledForMedia = true;
                  }
               }
               else if(type == Sdp::BANDWIDTH_TYPE_RS)
               {
                  if(it->kbPerSecond() == 0)
                  {
                     RSSetToZero = true;
                  }
                  else
                  {
                     // Explicit settings in the media override session bandwidth settings that may have disabled rtcp
                     rtcpEnabledForMedia = true;
                  }
               }
            }
         }
         // Rtcp if considered disabled if RR and RS are both set to 0
         if(RRSetToZero && RSSetToZero) 
         {
            rtcpEnabledForMedia = false; 
         }
      }
   
      // Iterate Through Connections, c=
      {
         unsigned int numPorts = resipMedia.multicast();
         unsigned int i=0;

         // If the number of ports specified on the m= line matches the number of multicast addresses, then pair them up 1-1
         // if there are multiple addresses and multiple ports but the numbers don't match create num addresses * num ports connections
         // If their is only one port specified, the use it with each connection
         // If there is only one address and multiple ports then add addtional connections for each port
         const std::list<SdpContents::Session::Connection>& connections = resipMedia.getConnections();
         std::list<SdpContents::Session::Connection>::const_iterator it2;
         if(connections.size() == numPorts) // if there is a 1-1 mapping between numAddresses and numPorts, then pair them up
         {
            for(it2 = connections.begin(); it2 != connections.end(); it2++, i++)
            {
               const SdpContents::Session::Connection& resipConnection = *it2;

               mediaLine->addConnection(Sdp::NET_TYPE_IN, 
                                        convertResipAddressType(resipConnection.getAddressType()), 
                                        resipConnection.getAddress().c_str(), 
                                        resipMedia.port() + (i*2), 
                                        resipConnection.ttl());
            }
         }
         else  
         {
            for(it2 = connections.begin(); it2 != connections.end(); it2++)
            {
               const SdpContents::Session::Connection& resipConnection = *it2;

               for(i = 0; i < numPorts; i++)
               {
                  mediaLine->addConnection(Sdp::NET_TYPE_IN, 
                                           convertResipAddressType(resipConnection.getAddressType()), 
                                           resipConnection.getAddress().c_str(), 
                                           resipMedia.port() + (i*2), 
                                           resipConnection.ttl());
               }
            }
         }
      }

      // Add Rtcp connections (if enabled) - if attributes are present use them, otherwise use default rule
      {
         if(rtcpEnabledForMedia)  // Set when iterrating through bandwidth settings
         {
            // Note: RFC3605 rtcp-attribute does not support multicast streams - see section 3.2 of RFC
            if(mediaLine->getConnections().entries() == 1 && resipMedia.exists("rtcp"))
            {
               unsigned int ipv4ttl = 0;
               Sdp::SdpAddressType addrType = Sdp::ADDRESS_TYPE_NONE;

               ParseBuffer pb(resipMedia.getValues("rtcp").front());
               Data address;
               unsigned int port = pb.uInt32();
               pb.skipToChar(Symbols::SPACE[0]);
               if(!pb.eof())
               {
                  const char* anchor = pb.skipWhitespace();
                  if(!pb.eof())
                  {
                     pb.skipChar('I');
                     pb.skipChar('N');

                     anchor = pb.skipChar(Symbols::SPACE[0]);
                     pb.skipToChar(Symbols::SPACE[0]);
                     pb.data(address, anchor);  // use address as temp storage
                     addrType = Sdp::getAddressTypeFromString(address.c_str());
                     anchor = pb.skipChar(Symbols::SPACE[0]);
                     pb.skipToOneOf(Symbols::SPACE, Symbols::SLASH);
                     pb.data(address, anchor);
                     if(addrType == Sdp::ADDRESS_TYPE_IP4 && !pb.eof() && *pb.position() == Symbols::SLASH[0])
                     {
                        pb.skipChar();
                        ipv4ttl = pb.integer();
                     }
                  }
               }
               if(address.empty())  // If connection address was not specified here - use the c= line
               {
                  SdpMediaLine::SdpConnection *connection = (SdpMediaLine::SdpConnection*)mediaLine->getConnections().first();
                  mediaLine->addRtcpConnection(connection->getNetType(), 
                                               connection->getAddressType(), 
                                               connection->getAddress().data(), 
                                               port, 
                                               connection->getMulticastIpV4Ttl());
               }
               else
               {
                  mediaLine->addRtcpConnection(Sdp::NET_TYPE_IN, addrType, address.c_str(), port, ipv4ttl);
               }
            }
            else  // default rule is that rtcp is sent on all connections, by adding 1 to the port
            {
               // Iterrate through each connection and add a corresponding RTCP connection
               const UtlSList& connections = mediaLine->getConnections();
               UtlSListIterator it2(connections);
               SdpMediaLine::SdpConnection* connection;
               while(connection = (SdpMediaLine::SdpConnection*)it2())
               {
                  mediaLine->addRtcpConnection(connection->getNetType(), 
                                               connection->getAddressType(), 
                                               connection->getAddress().data(), 
                                               connection->getPort() + 1, 
                                               connection->getMulticastIpV4Ttl());
               }
            }
         }
      }

      // k=
      if(resipMedia.getEncryption().getMethod() != SdpContents::Session::Encryption::NoEncryption)
      {
         mediaLine->setEncryptionKey(convertResipEncryptionMethod(resipMedia.getEncryption().getMethod()), resipMedia.getEncryption().getKey().c_str());
      }
      else if(resipSession->getEncryption().getMethod() != SdpContents::Session::Encryption::NoEncryption) // check if set at session level
      {
         mediaLine->setEncryptionKey(convertResipEncryptionMethod(resipSession->getEncryption().getMethod()), resipSession->getEncryption().getKey().c_str());
      }

      // Set direction, a=sendrecv, a=sendonly, a=recvonly, a=inactive
      SdpMediaLine::SdpDirectionType direction = SdpMediaLine::DIRECTION_TYPE_SENDRECV; // default
      if(resipMedia.exists("sendrecv"))
      {
         direction = SdpMediaLine::DIRECTION_TYPE_SENDRECV;
      }
      else if(resipMedia.exists("sendonly"))
      {
         direction = SdpMediaLine::DIRECTION_TYPE_SENDONLY;
      }
      else if(resipMedia.exists("recvonly"))
      {
         direction = SdpMediaLine::DIRECTION_TYPE_RECVONLY;
      }
      else if(resipMedia.exists("inactive"))
      {
         direction = SdpMediaLine::DIRECTION_TYPE_INACTIVE;
      }
      // Now check if direction is set at session level
      else if(resipSession->exists("sendrecv"))
      {
         direction = SdpMediaLine::DIRECTION_TYPE_SENDRECV;
      }
      else if(resipSession->exists("sendonly"))
      {
         direction = SdpMediaLine::DIRECTION_TYPE_SENDONLY;
      }
      else if(resipSession->exists("recvonly"))
      {
         direction = SdpMediaLine::DIRECTION_TYPE_RECVONLY;
      }
      else if(resipSession->exists("inactive"))
      {
         direction = SdpMediaLine::DIRECTION_TYPE_INACTIVE;
      }
      mediaLine->setDirection(direction);

      // Note ptime is set above
      if(resipMedia.exists("maxptime"))
      {
         mediaLine->setMaxPacketTime(resipMedia.getValues("maxptime").front().convertUnsignedLong());
      }
      
      if(resipMedia.exists("orient"))
      {
         mediaLine->setOrientation(SdpMediaLine::getOrientationTypeFromString(resipMedia.getValues("orient").front().c_str()));
      }

      if(resipMedia.exists("sdplang"))
      {
         mediaLine->setDescriptionLanguage(resipMedia.getValues("sdplang").front().c_str());
      } 
      else if(resipSession->exists("sdplang"))
      {
         mediaLine->setDescriptionLanguage(resipSession->getValues("sdplang").front().c_str());
      }

      if(resipMedia.exists("lang"))
      {
         mediaLine->setLanguage(resipMedia.getValues("lang").front().c_str());
      }
      else if(resipSession->exists("lang"))
      {
         mediaLine->setLanguage(resipSession->getValues("lang").front().c_str());
      }

      if(resipMedia.exists("framerate"))
      {
         mediaLine->setFrameRate(resipMedia.getValues("framerate").front().convertUnsignedLong());
      }

      if(resipMedia.exists("quality"))
      {
         mediaLine->setQuality(resipMedia.getValues("quality").front().convertUnsignedLong());
      }

      if(resipMedia.exists("setup"))
      {
         mediaLine->setTcpSetupAttribute(SdpMediaLine::getTcpSetupAttributeFromString(resipMedia.getValues("setup").front().c_str()));
      }

      if(resipMedia.exists("connection"))
      {
         mediaLine->setTcpConnectionAttribute(SdpMediaLine::getTcpConnectionAttributeFromString(resipMedia.getValues("connection").front().c_str()));
      }

      if(resipMedia.exists("crypto"))
      {
         const std::list<Data>& cryptos = resipMedia.getValues("crypto");
         std::list<Data>::const_iterator it2;
         for(it2 = cryptos.begin(); it2 != cryptos.end(); it2++)
         {
            ParseBuffer pb(*it2);
            unsigned int tag = pb.uInt32();
            Data token;
            SdpMediaLine::SdpCryptoSuiteType suite;
            
            pb.skipToChar(Symbols::SPACE[0]);
            const char * anchor = pb.skipWhitespace();
            pb.skipToChar(Symbols::SPACE[0]);
            pb.data(token, anchor);
            suite = SdpMediaLine::getCryptoSuiteTypeFromString(token.c_str());
            if(suite != SdpMediaLine::CRYPTO_SUITE_TYPE_NONE)
            {
               SdpMediaLine::SdpCrypto *crypto = new SdpMediaLine::SdpCrypto;
               crypto->setTag(tag);
               crypto->setSuite(suite);
               anchor = pb.skipWhitespace();

               // Loop through KeyParams
               while(true)
               {
                  SdpMediaLine::SdpCryptoKeyMethod method;
                  Data keyValue;
                  unsigned int lifetime = 0;
                  unsigned int mkiValue = 0;
                  unsigned int mkiLength = 0;

                  parseCryptoParams(pb, method, keyValue, lifetime, mkiValue, mkiLength);
                  crypto->addCryptoKeyParam(method, keyValue.c_str(), lifetime, mkiValue, mkiLength);

                  if(pb.eof() || *pb.position() != ';') break;
                  anchor = pb.skipChar();
               }
               while(!pb.eof())
               {
                  anchor = pb.skipWhitespace();
                  pb.skipToOneOf(" =");
                  pb.data(token, anchor);
                  if(token == "KDR")
                  {
                     pb.skipChar();
                     crypto->setSrtpKdr(pb.uInt32());
                  }
                  else if(token == "UNENCRYPTED_SRTP")
                  {
                     crypto->setEncryptedSrtp(false);
                  }
                  else if(token == "UNENCRYPTED_SRTCP")
                  {
                     crypto->setEncryptedSrtcp(false);
                  }
                  else if(token == "UNAUTHENTICATED_SRTP")
                  {
                     crypto->setAuthenticatedSrtp(false);
                  }
                  else if(token == "FEC_ORDER")
                  {
                     anchor = pb.skipChar();
                     pb.skipToChar(Symbols::SPACE[0]);
                     pb.data(token, anchor);
                     crypto->setSrtpFecOrder(SdpMediaLine::SdpCrypto::getSrtpFecOrderFromString(token.c_str()));
                  }
                  else if(token == "FEC_KEY")
                  {
                     anchor = pb.skipChar();
                     SdpMediaLine::SdpCryptoKeyMethod method;
                     Data keyValue;
                     unsigned int lifetime = 0;
                     unsigned int mkiValue = 0;
                     unsigned int mkiLength = 0;

                     parseCryptoParams(pb, method, keyValue, lifetime, mkiValue, mkiLength);
                     crypto->setSrtpFecKey(method, keyValue.c_str(), lifetime, mkiValue, mkiLength);
                  }
                  else if(token == "WSH")
                  {
                     pb.skipChar();
                     crypto->setSrtpWsh(pb.uInt32());
                  }
                  else
                  {
                     if(!pb.eof() && *pb.position() == '=')
                     {
                        pb.skipToChar(Symbols::SPACE[0]);
                        pb.data(token, anchor);
                     }
                     crypto->addGenericSessionParam(token.c_str());
                  }
               }
               mediaLine->addCryptoSettings(crypto);
            }
         }
      }

      Data fingerprint;
      if(resipMedia.exists("fingerprint"))
      {
         fingerprint = resipMedia.getValues("fingerprint").front();
      }
      else if(resipSession->exists("fingerprint"))
      {
         fingerprint = resipSession->getValues("fingerprint").front();
      }
      if(!fingerprint.empty())
      {
         ParseBuffer pb(fingerprint);
         Data token;
         SdpMediaLine::SdpFingerPrintHashFuncType hashFunc;
         const char * anchor = pb.position();
         pb.skipToChar(Symbols::SPACE[0]);
         pb.data(token, anchor);
         hashFunc = SdpMediaLine::getFingerPrintHashFuncTypeFromString(token.c_str());
         if(hashFunc != SdpMediaLine::FINGERPRINT_HASH_FUNC_NONE)
         {
            anchor = pb.skipWhitespace();
            pb.skipToChar(Symbols::SPACE[0]);
            pb.data(token, anchor);
            mediaLine->setFingerPrint(hashFunc, token.c_str());
         }
      }

      Data keymgmt;
      if(resipMedia.exists("key-mgmt"))
      {
         keymgmt = resipMedia.getValues("key-mgmt").front();
      }
      else if(resipSession->exists("key-mgmt"))
      {
         keymgmt = resipSession->getValues("key-mgmt").front();
      }
      if(!keymgmt.empty())
      {
         ParseBuffer pb(keymgmt);
         Data token;
         SdpMediaLine::SdpKeyManagementProtocolType keyMgmtType;
         const char * anchor = pb.position();
         pb.skipToChar(Symbols::SPACE[0]);
         pb.data(token, anchor);
         keyMgmtType = SdpMediaLine::getKeyManagementProtocolTypeFromString(token.c_str());
         if(keyMgmtType != SdpMediaLine::KEYMANAGEMENT_PROTOCOL_NONE)
         {
            anchor = pb.skipWhitespace();
            pb.skipToChar(Symbols::SPACE[0]);
            pb.data(token, anchor);
            mediaLine->setKeyManagementProtocol(keyMgmtType, token.c_str());
         }
      }

      if(resipMedia.exists("curr"))
      {
         const std::list<Data>& preconds = resipMedia.getValues("curr");
         std::list<Data>::const_iterator it2;
         for(it2 = preconds.begin(); it2 != preconds.end(); it2++)
         {
            ParseBuffer pb(*it2);
            Data token;
            SdpMediaLine::SdpPreConditionType type;
            SdpMediaLine::SdpPreConditionStatusType status;
            SdpMediaLine::SdpPreConditionDirectionType direction;
            const char * anchor = pb.position();
            pb.skipToChar(Symbols::SPACE[0]);
            pb.data(token, anchor);
            type = SdpMediaLine::getPreConditionTypeFromString(token.c_str());
            if(type != SdpMediaLine::PRECONDITION_TYPE_NONE)
            {
               anchor = pb.skipWhitespace();
               pb.skipToChar(Symbols::SPACE[0]);
               pb.data(token, anchor);
               status = SdpMediaLine::getPreConditionStatusTypeFromString(token.c_str());
               if(status != SdpMediaLine::PRECONDITION_STATUS_NONE)
               {
                  anchor = pb.skipWhitespace();
                  pb.skipToChar(Symbols::SPACE[0]);
                  pb.data(token, anchor);
                  direction = SdpMediaLine::getPreConditionDirectionTypeFromString(token.c_str());
                  mediaLine->addPreConditionCurrentStatus(type, status, direction);
               }
            }
         }
      }

      if(resipMedia.exists("conf"))
      {
         const std::list<Data>& preconds = resipMedia.getValues("conf");
         std::list<Data>::const_iterator it2;
         for(it2 = preconds.begin(); it2 != preconds.end(); it2++)
         {
            ParseBuffer pb(*it2);
            Data token;
            SdpMediaLine::SdpPreConditionType type;
            SdpMediaLine::SdpPreConditionStatusType status;
            SdpMediaLine::SdpPreConditionDirectionType direction;
            const char * anchor = pb.position();
            pb.skipToChar(Symbols::SPACE[0]);
            pb.data(token, anchor);
            type = SdpMediaLine::getPreConditionTypeFromString(token.c_str());
            if(type != SdpMediaLine::PRECONDITION_TYPE_NONE)
            {
               anchor = pb.skipWhitespace();
               pb.skipToChar(Symbols::SPACE[0]);
               pb.data(token, anchor);
               status = SdpMediaLine::getPreConditionStatusTypeFromString(token.c_str());
               if(status != SdpMediaLine::PRECONDITION_STATUS_NONE)
               {
                  anchor = pb.skipWhitespace();
                  pb.skipToChar(Symbols::SPACE[0]);
                  pb.data(token, anchor);
                  direction = SdpMediaLine::getPreConditionDirectionTypeFromString(token.c_str());
                  mediaLine->addPreConditionConfirmStatus(type, status, direction);
               }
            }
         }
      }

      if(resipMedia.exists("des"))
      {
         const std::list<Data>& preconds = resipMedia.getValues("des");
         std::list<Data>::const_iterator it2;
         for(it2 = preconds.begin(); it2 != preconds.end(); it2++)
         {
            ParseBuffer pb(*it2);
            Data token;
            SdpMediaLine::SdpPreConditionType type;
            SdpMediaLine::SdpPreConditionStrengthType strength;
            SdpMediaLine::SdpPreConditionStatusType status;
            SdpMediaLine::SdpPreConditionDirectionType direction;
            const char * anchor = pb.position();
            pb.skipToChar(Symbols::SPACE[0]);
            pb.data(token, anchor);
            type = SdpMediaLine::getPreConditionTypeFromString(token.c_str());
            if(type != SdpMediaLine::PRECONDITION_TYPE_NONE)
            {
               anchor = pb.skipWhitespace();
               pb.skipToChar(Symbols::SPACE[0]);
               pb.data(token, anchor);
               strength = SdpMediaLine::getPreConditionStrengthTypeFromString(token.c_str());
               anchor = pb.skipWhitespace();
               pb.skipToChar(Symbols::SPACE[0]);
               pb.data(token, anchor);
               status = SdpMediaLine::getPreConditionStatusTypeFromString(token.c_str());
               if(status != SdpMediaLine::PRECONDITION_STATUS_NONE)
               {
                  anchor = pb.skipWhitespace();
                  pb.skipToChar(Symbols::SPACE[0]);
                  pb.data(token, anchor);
                  direction = SdpMediaLine::getPreConditionDirectionTypeFromString(token.c_str());
                  mediaLine->addPreConditionDesiredStatus(type, strength, status, direction);
               }
            }
         }
      }

      if(resipMedia.exists("maxprate"))
      {
         mediaLine->setMaximumPacketRate(resipMedia.getValues("maxprate").front().convertDouble());
      }
      else if(resipSession->exists("maxprate"))
      {
         mediaLine->setMaximumPacketRate(resipSession->getValues("maxprate").front().convertDouble());
      }

      if(resipMedia.exists("label"))
      {
         mediaLine->setLabel(resipMedia.getValues("label").front().c_str());
      }

      if(resipMedia.exists("mid"))
      {
         mediaLine->setIdentificationTag(resipMedia.getValues("mid").front().c_str());
      }

      if(resipMedia.exists("ice-ufrag"))
      {
         mediaLine->setIceUserFrag(resipMedia.getValues("ice-ufrag").front().c_str());
      }
      else if(resipSession->exists("ice-ufrag"))
      {
         mediaLine->setIceUserFrag(resipSession->getValues("ice-ufrag").front().c_str());
      }

      if(resipMedia.exists("ice-pwd"))
      {
         mediaLine->setIcePassword(resipMedia.getValues("ice-pwd").front().c_str());
      }
      else if(resipSession->exists("ice-pwd"))
      {
         mediaLine->setIcePassword(resipSession->getValues("ice-pwd").front().c_str());
      }

      if(resipMedia.exists("remote-candidates"))
      {
         // multiple remote candidates are allowed on one line - we will also accept multiple remote-candidate attributes
         const std::list<Data>& preconds = resipMedia.getValues("remote-candidates");
         std::list<Data>::const_iterator it2;
         for(it2 = preconds.begin(); it2 != preconds.end(); it2++)
         {
            ParseBuffer pb(*it2);
            Data token;
            unsigned int componentId;
            unsigned int port;
            const char * anchor;

            while(!pb.eof())
            {
               componentId = pb.uInt32();
               pb.skipToChar(Symbols::SPACE[0]);
               anchor = pb.skipWhitespace();
               pb.skipToChar(Symbols::SPACE[0]);
               pb.data(token, anchor);
               pb.skipWhitespace();
               port = pb.uInt32();  
               pb.skipToChar(Symbols::SPACE[0]);
               pb.skipWhitespace();
               mediaLine->addRemoteCandidate(componentId, token.c_str(), port);
            }
         }
      }

      if(resipMedia.exists("candidate"))
      {
         const std::list<Data>& preconds = resipMedia.getValues("candidate");
         std::list<Data>::const_iterator it2;
         for(it2 = preconds.begin(); it2 != preconds.end(); it2++)
         {
            ParseBuffer pb(*it2);
            Data foundation;
            unsigned int componentId;
            Data token;
            SdpCandidate::SdpCandidateTransportType transport;
            uint64_t priority;
            Data address;
            unsigned int port;
            SdpCandidate::SdpCandidateType candidateType = SdpCandidate::CANDIDATE_TYPE_NONE;
            Data relatedAddress;
            unsigned int relatedPort = 0;

            const char * anchor = pb.position();
            pb.skipToChar(Symbols::SPACE[0]);
            pb.data(foundation, anchor);
            pb.skipWhitespace();
            componentId = pb.uInt32();
            pb.skipToChar(Symbols::SPACE[0]);
            anchor = pb.skipWhitespace();
            pb.skipToChar(Symbols::SPACE[0]);
            pb.data(token, anchor);
            transport = SdpCandidate::getCandidateTransportTypeFromString(token.c_str());
            if(transport != SdpCandidate::CANDIDATE_TRANSPORT_TYPE_NONE)
            {
               pb.skipWhitespace();
               priority = pb.uInt64();
               pb.skipToChar(Symbols::SPACE[0]);
               anchor = pb.skipWhitespace();
               pb.skipToChar(Symbols::SPACE[0]);
               pb.data(address, anchor);
               pb.skipWhitespace();
               port = pb.uInt32();
               pb.skipToChar(Symbols::SPACE[0]);
               anchor = pb.skipWhitespace();
               if(!pb.eof())
               {
                  pb.skipToChar(Symbols::SPACE[0]);
                  pb.data(token, anchor);
                  if(token == "typ")
                  {
                     anchor = pb.skipWhitespace();
                     pb.skipToChar(Symbols::SPACE[0]);
                     pb.data(token, anchor);
                     candidateType = SdpCandidate::getCandidateTypeFromString(token.c_str());
                     anchor = pb.skipWhitespace();
                     if(!pb.eof())
                     {
                        pb.skipToChar(Symbols::SPACE[0]);
                        pb.data(token, anchor);
                     }
                  }
               
                  if(!pb.eof() && token == "raddr")
                  {
                     anchor = pb.skipWhitespace();
                     pb.skipToChar(Symbols::SPACE[0]);
                     pb.data(relatedAddress, anchor);
                     anchor = pb.skipWhitespace();
                     if(!pb.eof())
                     {
                        pb.skipToChar(Symbols::SPACE[0]);
                        pb.data(token, anchor);
                     }
                  }
                  
                  if(!pb.eof() && token == "rport")
                  {
                     pb.skipWhitespace();
                     relatedPort = pb.uInt32();
                     pb.skipToChar(Symbols::SPACE[0]);
                     anchor = pb.skipWhitespace();
                     if(!pb.eof())
                     {
                        pb.skipToChar(Symbols::SPACE[0]);
                        pb.data(token, anchor);
                     }
                  }
               
                  SdpCandidate* candidate = new SdpCandidate(foundation.c_str(), componentId, transport, priority, address.c_str(), port, candidateType, relatedAddress.c_str(), relatedPort);
                  while(!pb.eof())
                  {
                     Data extAttribValue;
                     anchor = pb.skipWhitespace();
                     pb.skipToChar(Symbols::SPACE[0]);
                     pb.data(extAttribValue, anchor);
                     anchor = pb.skipWhitespace();
                     candidate->addExtensionAttribute(token.c_str(), extAttribValue.c_str());
                     if(!pb.eof())
                     {
                        pb.skipToChar(Symbols::SPACE[0]);
                        pb.data(token, anchor);
                     }
                  }
                  mediaLine->addCandidate(candidate);
               }
            }
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

void SdpHelperResip::parseCryptoParams(ParseBuffer& pb, 
                                       SdpMediaLine::SdpCryptoKeyMethod& keyMethod, 
                                       Data& keyValue, 
                                       unsigned int& srtpLifetime, 
                                       unsigned int& srtpMkiValue, 
                                       unsigned int& srtpMkiLength)
{
   const char * anchor = pb.position();
   Data token;
   pb.skipToChar(Symbols::COLON[0]);
   pb.data(token, anchor);
   keyMethod = SdpMediaLine::getCryptoKeyMethodFromString(token.c_str());
   if(keyMethod == SdpMediaLine::CRYPTO_KEY_METHOD_INLINE)
   {
      anchor = pb.skipChar();
      pb.skipToOneOf("|;", Symbols::SPACE);
      pb.data(keyValue, anchor);
      if(!pb.eof() && *pb.position() == '|')
      {
         pb.skipChar();
         unsigned int base = pb.uInt32();
         pb.skipToOneOf("^|:;", Symbols::SPACE);
         if(*pb.position() == '^')
         {
            pb.skipChar();
            unsigned int exponent = pb.uInt32();  
            srtpLifetime = base;
            while(0 != --exponent) srtpLifetime *= base;
            pb.skipToOneOf("|;", Symbols::SPACE);
            if(!pb.eof() && *pb.position() == '|')
            {
               pb.skipChar();
               base = pb.uInt32();
               pb.skipToOneOf(":;", Symbols::SPACE);
            }
         }
         else if(*pb.position() != ':')
         {
            srtpLifetime = base;
            if(!pb.eof())
            {
               if(*pb.position() == '|')
               {
                  pb.skipChar();
                  base = pb.uInt32();
               }
               pb.skipToOneOf(":;", Symbols::SPACE);
            }
         }
         if(!pb.eof() && *pb.position() == ':')
         {
            srtpMkiValue = base;
            pb.skipChar();
            srtpMkiLength = pb.uInt32();
            pb.skipToOneOf(";", Symbols::SPACE);
         }
      }
   }
   else
   {
      anchor = pb.skipChar();
      pb.skipToChar(Symbols::SPACE[0]);
      pb.data(keyValue, anchor);
      if(!pb.eof())
      {
         pb.skipToOneOf(";", Symbols::SPACE);
      }
   }
}

/* ============================ FUNCTIONS ================================= */

#endif