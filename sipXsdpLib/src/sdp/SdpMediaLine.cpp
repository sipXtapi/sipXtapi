//
// Copyright (C) 2007-2012 SIPez LLC.  All rights reserved.
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

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlSortedListIterator.h>
#include <sdp/SdpMediaLine.h>


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const char* SdpMediaLine::SdpMediaTypeString[] =
{
    // WARNING: this array must stay in synch. with the enum SdpMediaType
   "none",
   "audio",
   "video",
   "text",
   "application",
   "message"
};

const char* SdpMediaLine::SdpTransportProtocolTypeString[] =
{
    // WARNING: this array must stay in synch. with the enum SdpTransportProtocolType
   "NONE",
   "UDP",
   "RTP/AVP",
   "RTP/SAVP",
   "RTP/SAVPF",
   "TCP",
   "TCP/RTP/AVP",
   "TCP/TLS",
   "UDP/TLS",
   "DCCP/TLS",
   "DCCP/TLS/RTP/SAVP",
   "UDP/TLS/RTP/SAVP",
   "TCP/TLS/RTP/SAVP"
};

const char* SdpMediaLine::SdpEncryptionMethodString[] =
{
   "NONE",
   "CLEAR",
   "BASE64",
   "URI",
   "PROMPT"
};

const char* SdpMediaLine::SdpDirectionTypeString[] =
{
   "NONE",
   "SENDRECV",
   "SENDONLY",
   "RECVONLY",
   "INACTIVE"
};

const char* SdpMediaLine::SdpOrientationTypeString[] =
{
   "NONE",
   "PORTRAIT",
   "LANDSCAPE",
   "SEASCAPE"
};

const char* SdpMediaLine::SdpTcpSetupAttributeString[] =
{
   "NONE",
   "ACTIVE",
   "PASSIVE",
   "ACTPASS",
   "HOLDCONN"
};

const char* SdpMediaLine::SdpTcpConnectionAttributeString[] =
{
   "NONE",
   "NEW",
   "EXISTING"
};

const char* SdpMediaLine::SdpCryptoSuiteTypeString[] =
{
   "NONE",
   "AES_CM_128_HMAC_SHA1_80",
   "AES_CM_128_HMAC_SHA1_32",
   "F8_128_HMAC_SHA1_80"
};

const char* SdpMediaLine::SdpCryptoKeyMethodString[] =
{
   "NONE",
   "INLINE"
};

const char* SdpMediaLine::SdpCryptoSrtpFecOrderTypeString[] = 
{
   "NONE",
   "FEC_SRTP",
   "SRTP_FEC"
};

const char* SdpMediaLine::SdpFingerPrintHashFuncTypeString[] =
{
   "NONE",
   "SHA-1",
   "SHA-224",
   "SHA-256",
   "SHA-384",
   "SHA-512",
   "MD5",
   "MD2"
};

const char* SdpMediaLine::SdpKeyManagementProtocolTypeString[] =
{
   "NONE",
   "MIKEY"
};

const char* SdpMediaLine::SdpPreConditionTypeString[] =
{
   "NONE",
   "QOS",
};

const char* SdpMediaLine::SdpPreConditionStrengthTypeString[] =
{
   "MANDATORY",
   "OPTIONAL",
   "NONE",
   "FAILURE",
   "UNKNOWN"
};

const char* SdpMediaLine::SdpPreConditionStatusTypeString[] =
{
   "NONE",
   "E2E",
   "LOCAL",
   "REMOTE"
};

const char* SdpMediaLine::SdpPreConditionDirectionTypeString[] =
{
   "NONE",
   "SEND",
   "RECV",
   "SENDRECV"
};


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Copy constructor
SdpMediaLine::SdpCrypto::SdpCrypto(const SdpMediaLine::SdpCrypto& rhs)
{
   operator=(rhs); 
}

// Assignment operator
SdpMediaLine::SdpCrypto&
SdpMediaLine::SdpCrypto::operator=(const SdpMediaLine::SdpCrypto& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   // Assign values
   mTag = rhs.mTag;
   mSuite = rhs.mSuite;
   mCryptoKeyParams = rhs.mCryptoKeyParams;
   mSrtpKdr = rhs.mSrtpKdr;
   mEncryptedSrtp = rhs.mEncryptedSrtp;
   mEncryptedSrtcp = rhs.mEncryptedSrtcp;
   mAuthenticatedSrtp = rhs.mAuthenticatedSrtp;
   mSrtpFecOrder = rhs.mSrtpFecOrder;
   mSrtpFecKey = rhs.mSrtpFecKey;
   mSrtpWsh = rhs.mSrtpWsh;
   mGenericSessionParams = rhs.mGenericSessionParams;

   return *this;
}


// Constructor
SdpMediaLine::SdpMediaLine() :
   mMediaType(SdpMediaLine::MEDIA_TYPE_NONE),
   mTransportProtocolType(SdpMediaLine::PROTOCOL_TYPE_NONE),
   mEncryptionMethod(SdpMediaLine::ENCRYPTION_METHOD_NONE),
   mDirection(SdpMediaLine::DIRECTION_TYPE_NONE),
   mPacketTime(0),
   mMaxPacketTime(0),
   mOrientation(SdpMediaLine::ORIENTATION_TYPE_NONE),
   mFrameRate(0),
   mQuality(0),
   mTcpSetupAttribute(SdpMediaLine::TCP_SETUP_ATTRIBUTE_NONE),
   mTcpConnectionAttribute(SdpMediaLine::TCP_CONNECTION_ATTRIBUTE_NONE),
   mFingerPrintHashFunction(SdpMediaLine::FINGERPRINT_HASH_FUNC_NONE),
   mKeyManagementProtocol(SdpMediaLine::KEYMANAGEMENT_PROTOCOL_NONE),
   mMaximumPacketRate(0),
   mRtpCandidatePresent(false),
   mRtcpCandidatePresent(false)
{
}

// Copy constructor
SdpMediaLine::SdpMediaLine(const SdpMediaLine& rhs)
{
   operator=(rhs); 
}

// Destructor
SdpMediaLine::~SdpMediaLine()
{
   clearCandidates();
   clearCandidatePairs();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SdpMediaLine&
SdpMediaLine::operator=(const SdpMediaLine& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   // Assign values
   mMediaType = rhs.mMediaType;
   mTransportProtocolType = rhs.mTransportProtocolType;
   mCodecs = rhs.mCodecs;
   mTitle = rhs.mTitle;
   mConnections = rhs.mConnections;
   mRtcpConnections = rhs.mRtcpConnections;
   mBandwidths = rhs.mBandwidths;
   mEncryptionMethod = rhs.mEncryptionMethod;
   mEncryptionKey = rhs.mEncryptionKey;
   mDirection = rhs.mDirection;
   mPacketTime = rhs.mPacketTime;
   mMaxPacketTime = rhs.mMaxPacketTime;
   mOrientation = rhs.mOrientation;
   mDescriptionLanguage = rhs.mDescriptionLanguage;
   mLanguage = rhs.mLanguage;
   mFrameRate = rhs.mFrameRate;
   mQuality = rhs.mQuality;
   mTcpSetupAttribute = rhs.mTcpSetupAttribute;
   mTcpConnectionAttribute = rhs.mTcpConnectionAttribute;
   mCryptos = rhs.mCryptos;
   mFingerPrintHashFunction = rhs.mFingerPrintHashFunction;
   mFingerPrint = rhs.mFingerPrint;
   mKeyManagementProtocol = rhs.mKeyManagementProtocol;
   mKeyManagementData = rhs.mKeyManagementData;
   mPreConditionCurrentStatus = rhs.mPreConditionCurrentStatus;
   mPreConditionConfirmStatus = rhs.mPreConditionConfirmStatus;
   mPreConditionDesiredStatus = rhs.mPreConditionDesiredStatus;
   mMaximumPacketRate = rhs.mMaximumPacketRate;
   mLabel = rhs.mLabel;
   mIdentificationTag = rhs.mIdentificationTag;
   mIceUserFrag = rhs.mIceUserFrag;
   mIcePassword = rhs.mIcePassword;
   mRemoteCandidates = rhs.mRemoteCandidates;
   mCandidates = rhs.mCandidates;
   mRtpCandidatePresent = rhs.mRtpCandidatePresent;
   mRtcpCandidatePresent = rhs.mRtcpCandidatePresent;
   mCandidatePairs = rhs.mCandidatePairs;

   return *this;
}

void SdpMediaLine::addCandidate(SdpCandidate* candidate) 
{ 
   mCandidates.insert(candidate);

   // Check if Candidate is in use (appears on m/c line or rtcp attributes)
   // First check m/c line(s)
   UtlSListIterator it(mConnections);
   SdpConnection* sdpConnection;
   while((sdpConnection = (SdpConnection*) it()))
   {
      if(candidate->getPort() == sdpConnection->getPort() &&
         candidate->getConnectionAddress() == sdpConnection->getAddress())  
      {
         mRtpCandidatePresent = true;
         candidate->setInUse(true);
         break;
      }
   }

   // Next check Rtcp Info
   if(isRtcpEnabled())        
   {
      UtlSListIterator it(mRtcpConnections);
      SdpConnection* sdpConnection;
      while((sdpConnection = (SdpConnection*) it()))
      {
         if(candidate->getPort() == sdpConnection->getPort() &&
            candidate->getConnectionAddress() == sdpConnection->getAddress())  
         {
            mRtcpCandidatePresent = true;
            candidate->setInUse(true);
            break;
         }
      }
   }
}

/* ============================ ACCESSORS ================================= */

void SdpMediaLine::toString(UtlString& sdpMediaLineString) const
{
   char stringBuffer[8192];

   UtlString codecsString;
   UtlString connectionsString;
   UtlString rtcpConnectionsString;
   UtlString bandwidthsString;
   UtlString cryptosString;
   UtlString preConditionCurrentStatusString;
   UtlString preConditionConfirmStatusString;
   UtlString preConditionDesiredStatusString;
   UtlString candidatesString;
   UtlString remoteCandidatesString;
   UtlString candidatePairsString;


   // Build Codecs String
   mCodecs.toString(codecsString);

   // Build Connections String
   {
      UtlString tempString;
      UtlSListIterator it(mConnections);
      SdpConnection* sdpConnection;
      while((sdpConnection = (SdpConnection*) it()))
      {
         sprintf(stringBuffer, "Connection: netType=%s, addrType=%s, addr=%s, port=%d, ttl=%d\n", 
            Sdp::SdpNetTypeString[sdpConnection->getNetType()], 
            Sdp::SdpAddressTypeString[sdpConnection->getAddressType()], 
            sdpConnection->getAddress().data(), 
            sdpConnection->getPort(),
            sdpConnection->getMulticastIpV4Ttl());
         connectionsString += stringBuffer;
      }
   }

   // Build Rtcp Connections String
   {
      UtlString tempString;
      UtlSListIterator it(mRtcpConnections);
      SdpConnection* sdpConnection;
      while((sdpConnection = (SdpConnection*) it()))
      {
         sprintf(stringBuffer, "RTCP Connection: netType=%s, addrType=%s, addr=%s, port=%d, ttl=%d\n", 
            Sdp::SdpNetTypeString[sdpConnection->getNetType()], 
            Sdp::SdpAddressTypeString[sdpConnection->getAddressType()], 
            sdpConnection->getAddress().data(), 
            sdpConnection->getPort(),
            sdpConnection->getMulticastIpV4Ttl());
         rtcpConnectionsString += stringBuffer;
      }
   }

   // Build Bandwidths String
   {
      UtlSListIterator it(mBandwidths);
      Sdp::SdpBandwidth* sdpBandwidth;
      while((sdpBandwidth = (Sdp::SdpBandwidth*) it()))
      {
         sprintf(stringBuffer, "Bandwidth: type=%s, bandwidth=%d\n", Sdp::SdpBandwidthTypeString[sdpBandwidth->getType()], sdpBandwidth->getBandwidth());
         bandwidthsString += stringBuffer;
      }
   }

   // Build Cryptos String
   {
      UtlSListIterator it(mCryptos);
      SdpCrypto* sdpCrypto;
      while((sdpCrypto = (SdpCrypto*) it()))
      {
         sprintf(stringBuffer, "Crypto: tag=%d, suite=%s", 
            sdpCrypto->getTag(), 
            SdpCryptoSuiteTypeString[sdpCrypto->getSuite()]);
         cryptosString += stringBuffer;

         const UtlSList& keyParams = sdpCrypto->getCryptoKeyParams();
         UtlSListIterator it2(keyParams);
         SdpCrypto::SdpCryptoKeyParam* keyParam;
         while((keyParam = (SdpCrypto::SdpCryptoKeyParam*)it2()))
         {
            sprintf(stringBuffer, "\n        Key Param: method=%s, key=%s, srtpLifetime=%d, srtpMkiValue=%d, srtpMkiLength=%d",
               SdpCryptoKeyMethodString[keyParam->getKeyMethod()], 
               keyParam->getKeyValue().data(),
               keyParam->getSrtpLifetime(),
               keyParam->getSrtpMkiValue(),
               keyParam->getSrtpMkiLength());
            cryptosString += stringBuffer;
         }

         sprintf(stringBuffer, "\n        kdr=%d, encryptSrtp=%d, encryptSrtcp=%d, authSrtp=%d, fecOrder=%s, wsh=%d",
            sdpCrypto->getSrtpKdr(), 
            sdpCrypto->getEncryptedSrtp(),
            sdpCrypto->getEncryptedSrtcp(),
            sdpCrypto->getAuthenticatedSrtp(), 
            SdpCryptoSrtpFecOrderTypeString[sdpCrypto->getSrtpFecOrder()],
            sdpCrypto->getSrtpWsh());
         cryptosString += stringBuffer;
         
         if(sdpCrypto->getSrtpFecKey().getKeyMethod() != SdpMediaLine::CRYPTO_KEY_METHOD_NONE)
         {
            sprintf(stringBuffer, "\n        fecKeyMethod=%s, fecKey=%s, fecLifetime=%d, fecMkiValue=%d, fecMkiLength=%d",
               SdpCryptoKeyMethodString[sdpCrypto->getSrtpFecKey().getKeyMethod()],
               sdpCrypto->getSrtpFecKey().getKeyValue().data(),
               sdpCrypto->getSrtpFecKey().getSrtpLifetime(),
               sdpCrypto->getSrtpFecKey().getSrtpMkiValue(),
               sdpCrypto->getSrtpFecKey().getSrtpMkiLength());
            cryptosString += stringBuffer;
         }

         const UtlSList& params = sdpCrypto->getGenericSessionParams();
         UtlSListIterator it3(params);
         UtlString* tempString;
         while((tempString = (UtlString*) it3()))
         {
            cryptosString += UtlString("\n        sessParam=") + tempString->data();
         }
         cryptosString += UtlString("\n");
      }
   }

   // Build PreConditionCurrentStatus String
   {
      UtlSListIterator it(mPreConditionCurrentStatus);
      SdpPreCondition* sdpPreCondition;
      while((sdpPreCondition = (SdpPreCondition*) it()))
      {
         sprintf(stringBuffer, "PreConditionCurrentStatus: type=%s, status=%s, direction=%s\n", 
            SdpPreConditionTypeString[sdpPreCondition->getType()], 
            SdpPreConditionStatusTypeString[sdpPreCondition->getStatus()], 
            SdpPreConditionDirectionTypeString[sdpPreCondition->getDirection()]);
         preConditionCurrentStatusString += stringBuffer;
      }
   }

   // Build PreConditionConfirmStatus String
   {
      UtlSListIterator it(mPreConditionConfirmStatus);
      SdpPreCondition* sdpPreCondition;
      while((sdpPreCondition = (SdpPreCondition*) it()))
      {
         sprintf(stringBuffer, "PreConditionConfirmStatus: type=%s, status=%s, direction=%s\n", 
            SdpPreConditionTypeString[sdpPreCondition->getType()], 
            SdpPreConditionStatusTypeString[sdpPreCondition->getStatus()], 
            SdpPreConditionDirectionTypeString[sdpPreCondition->getDirection()]);
         preConditionConfirmStatusString += stringBuffer;
      }
   }

   // Build PreConditionDesiredStatus String
   {
      UtlSListIterator it(mPreConditionDesiredStatus);
      SdpPreConditionDesiredStatus* sdpPreConditionDesiredStatus;
      while((sdpPreConditionDesiredStatus = (SdpPreConditionDesiredStatus*) it()))
      {
         sprintf(stringBuffer, "PreConditionDesiredStatus: type=%s, strength=%s, status=%s, direction=%s\n", 
            SdpPreConditionTypeString[sdpPreConditionDesiredStatus->getType()], 
            SdpPreConditionStrengthTypeString[sdpPreConditionDesiredStatus->getStrength()], 
            SdpPreConditionStatusTypeString[sdpPreConditionDesiredStatus->getStatus()], 
            SdpPreConditionDirectionTypeString[sdpPreConditionDesiredStatus->getDirection()]);
         preConditionDesiredStatusString += stringBuffer;
      }
   }

   // Build Candidates String
   {
      UtlString tempString;
      UtlSortedListIterator it(mCandidates);
      SdpCandidate* sdpCandidate;
      while((sdpCandidate = (SdpCandidate*) it()))
      {
         sdpCandidate->toString(tempString);
         candidatesString += tempString;
      }
   }

   // Build RemoteCandidates String
   {
      UtlSListIterator it(mRemoteCandidates);
      SdpRemoteCandidate* sdpRemoteCandidate;
      while((sdpRemoteCandidate = (SdpRemoteCandidate*) it()))
      {
         sprintf(stringBuffer, "Remote Candidate: componentId=%d, addr=%s, port=%d\n", 
            sdpRemoteCandidate->getComponentId(), 
            sdpRemoteCandidate->getConnectionAddress().data(), 
            sdpRemoteCandidate->getPort());
         remoteCandidatesString += stringBuffer;
      }
   }

   // Build Candidate Pairs String
   {
      UtlString tempString;
      UtlSortedListIterator it(mCandidatePairs);
      SdpCandidatePair* sdpCandidatePair;
      while((sdpCandidatePair = (SdpCandidatePair*) it()))
      {
         sdpCandidatePair->toString(tempString);
         candidatePairsString += tempString;
      }
   }

   sprintf(stringBuffer, 
      "MediaLine:\n"
      "Type: %s\n"
      "TransportProtocol: %s\n"
      "%s"
      "Title: \'%s'\n"
      "%s"
      "%s"
      "%s"
      "Encryption Key: method=%s, data=\'%s\'\n"
      "Direction: %s\n"
      "PacketTime: %d\n"
      "MaxPacketTime: %d\n"
      "Orientation: %s\n"
      "DescriptionLanguage: \'%s'\n"
      "Language: \'%s'\n"
      "FrameRate: %d\n"
      "Quality: %d\n"
      "TcpSetupAttrib: %s\n"
      "TcpConnectionAttrib: %s\n"
      "%s"
      "FingerPrint: type=%s, \'%s'\n"
      "KeyManagement: type=%s, \'%s'\n"
      "%s"
      "%s"
      "%s"
      "MaximumPacketRate: %lf\n"
      "Label: \'%s'\n"
      "IdentificationTag: \'%s'\n"
      "IceUserFrag: \'%s'\n"
      "IcePassword: \'%s'\n"
      "%s"
      "IceSupported: %d\n"
      "%s"
      "%s",
   SdpMediaTypeString[mMediaType],
   SdpTransportProtocolTypeString[mTransportProtocolType],
   codecsString.data(),
   mTitle.data(),
   connectionsString.data(),
   rtcpConnectionsString.data(),
   bandwidthsString.data(),
   SdpEncryptionMethodString[mEncryptionMethod],
   mEncryptionKey.data(),
   SdpDirectionTypeString[mDirection],
   mPacketTime,
   mMaxPacketTime,
   SdpOrientationTypeString[mOrientation],
   mDescriptionLanguage.data(),
   mLanguage.data(),
   mFrameRate,
   mQuality,
   SdpTcpSetupAttributeString[mTcpSetupAttribute],
   SdpTcpConnectionAttributeString[mTcpConnectionAttribute],
   cryptosString.data(),
   SdpFingerPrintHashFuncTypeString[mFingerPrintHashFunction],
   mFingerPrint.data(),
   SdpKeyManagementProtocolTypeString[mKeyManagementProtocol],
   mKeyManagementData.data(),
   preConditionCurrentStatusString.data(),
   preConditionConfirmStatusString.data(),
   preConditionDesiredStatusString.data(),
   mMaximumPacketRate,
   mLabel.data(),
   mIdentificationTag.data(),
   mIceUserFrag.data(),
   mIcePassword.data(),
   remoteCandidatesString.data(),
   isIceSupported(),
   candidatesString.data(),
   candidatePairsString.data());

   sdpMediaLineString = stringBuffer;
}

/* ============================ INQUIRY =================================== */

SdpMediaLine::SdpMediaType SdpMediaLine::getMediaTypeFromString(const UtlString& typeString)
{
    SdpMediaType type = MEDIA_TYPE_NONE;

    for(int typeIndex = MEDIA_TYPE_NONE + 1; 
        typeIndex < (int)(sizeof(SdpMediaTypeString) / sizeof(char*)); 
        typeIndex++)
    {
        if(typeString.compareTo(SdpMediaTypeString[typeIndex], UtlString::ignoreCase) == 0)
        {
            type = (SdpMediaType)typeIndex;
        }
    }

    return(type);
}

const char* SdpMediaLine::getStringForMediaType(SdpMediaType type)
{
    return(SdpMediaTypeString[type]);
}

SdpMediaLine::SdpTransportProtocolType 
SdpMediaLine::getTransportProtocolTypeFromString(const UtlString& stringType)
{
   if(stringType.compareTo("udp", UtlString::ignoreCase) == 0)
   {
      return PROTOCOL_TYPE_UDP;
   }
   else if(stringType.compareTo("RTP/AVP", UtlString::ignoreCase) == 0)
   {
      return PROTOCOL_TYPE_RTP_AVP;
   }
   else if(stringType.compareTo("RTP/SAVP", UtlString::ignoreCase) == 0)
   {
      return PROTOCOL_TYPE_RTP_SAVP;
   }
   else if(stringType.compareTo("RTP/SAVPF", UtlString::ignoreCase) == 0)
   {
      return PROTOCOL_TYPE_RTP_SAVPF;
   }
   else if(stringType.compareTo("TCP", UtlString::ignoreCase) == 0)
   {
      return PROTOCOL_TYPE_TCP;
   }
   else if(stringType.compareTo("TCP/RTP/AVP", UtlString::ignoreCase) == 0)
   {
      return PROTOCOL_TYPE_TCP_RTP_AVP;
   }
   else if(stringType.compareTo("TCP/TLS", UtlString::ignoreCase) == 0)
   {
      return PROTOCOL_TYPE_TCP_TLS;
   }
   else if(stringType.compareTo("UDP/TLS", UtlString::ignoreCase) == 0)
   {
      return PROTOCOL_TYPE_UDP_TLS;
   }
   else if(stringType.compareTo("DCCP/TLS", UtlString::ignoreCase) == 0)
   {
      return PROTOCOL_TYPE_DCCP_TLS;
   }
   else if(stringType.compareTo("DCCP/TLS/RTP/SAVP", UtlString::ignoreCase) == 0)
   {
      return PROTOCOL_TYPE_DCCP_TLS_RTP_SAVP;
   }
   else if(stringType.compareTo("UDP/TLS/RTP/SAVP", UtlString::ignoreCase) == 0)
   {
      return PROTOCOL_TYPE_UDP_TLS_RTP_SAVP;
   }
   else if(stringType.compareTo("TCP/TLS/RTP/SAVP", UtlString::ignoreCase) == 0)
   {
      return PROTOCOL_TYPE_TCP_TLS_RTP_SAVP;
   }
   else
   {
      return PROTOCOL_TYPE_NONE;
   }
}

const char* SdpMediaLine::getStringForTransportProtocolType(SdpTransportProtocolType type)
{
    return(SdpTransportProtocolTypeString[type]);
}

SdpMediaLine::SdpOrientationType 
SdpMediaLine::getOrientationTypeFromString(const char * type)
{
   UtlString stringType(type);

   if(stringType.compareTo("portrait", UtlString::ignoreCase) == 0)
   {
      return ORIENTATION_TYPE_PORTRAIT;
   }
   else if(stringType.compareTo("landscape", UtlString::ignoreCase) == 0)
   {
      return ORIENTATION_TYPE_LANDSCAPE;
   }
   else if(stringType.compareTo("seascape", UtlString::ignoreCase) == 0)
   {
      return ORIENTATION_TYPE_SEASCAPE;
   }
   else
   {
      return ORIENTATION_TYPE_NONE;
   }
}

SdpMediaLine::SdpTcpSetupAttribute 
SdpMediaLine::getTcpSetupAttributeFromString(const char * attrib)
{
   UtlString stringType(attrib);

   if(stringType.compareTo("active", UtlString::ignoreCase) == 0)
   {
      return TCP_SETUP_ATTRIBUTE_ACTIVE;
   }
   else if(stringType.compareTo("passive", UtlString::ignoreCase) == 0)
   {
      return TCP_SETUP_ATTRIBUTE_PASSIVE;
   }
   else if(stringType.compareTo("actpass", UtlString::ignoreCase) == 0)
   {
      return TCP_SETUP_ATTRIBUTE_ACTPASS;
   }
   else if(stringType.compareTo("holdconn", UtlString::ignoreCase) == 0)
   {
      return TCP_SETUP_ATTRIBUTE_HOLDCONN;
   }
   else
   {
      return TCP_SETUP_ATTRIBUTE_NONE;
   }
}

SdpMediaLine::SdpTcpConnectionAttribute 
SdpMediaLine::getTcpConnectionAttributeFromString(const char * attrib)
{
   UtlString stringType(attrib);

   if(stringType.compareTo("new", UtlString::ignoreCase) == 0)
   {
      return TCP_CONNECTION_ATTRIBUTE_NEW;
   }
   else if(stringType.compareTo("existing", UtlString::ignoreCase) == 0)
   {
      return TCP_CONNECTION_ATTRIBUTE_EXISTING;
   }
   else
   {
      return TCP_CONNECTION_ATTRIBUTE_NONE;
   }
}

SdpMediaLine::SdpCryptoSuiteType 
SdpMediaLine::getCryptoSuiteTypeFromString(const char * type)
{
   UtlString stringType(type);

   if(stringType.compareTo("AES_CM_128_HMAC_SHA1_80", UtlString::ignoreCase) == 0)
   {
      return CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80;
   }
   else if(stringType.compareTo("AES_CM_128_HMAC_SHA1_32", UtlString::ignoreCase) == 0)
   {
      return CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_32;
   }
   else if(stringType.compareTo("F8_128_HMAC_SHA1_80", UtlString::ignoreCase) == 0)
   {
      return CRYPTO_SUITE_TYPE_F8_128_HMAC_SHA1_80;
   }
   else
   {
      return CRYPTO_SUITE_TYPE_NONE;
   }
}

SdpMediaLine::SdpCryptoKeyMethod 
SdpMediaLine::getCryptoKeyMethodFromString(const char * type)
{
   UtlString stringType(type);

   if(stringType.compareTo("inline", UtlString::ignoreCase) == 0)
   {
      return CRYPTO_KEY_METHOD_INLINE;
   }
   else
   {
      return CRYPTO_KEY_METHOD_NONE;
   }
}

SdpMediaLine::SdpCryptoSrtpFecOrderType SdpMediaLine::SdpCrypto::getSrtpFecOrderFromString(const char * order)
{
   UtlString stringType(order);

   if(stringType.compareTo("FEC_SRTP", UtlString::ignoreCase) == 0)
   {
      return CRYPTO_SRTP_FEC_ORDER_FEC_SRTP;
   }
   else if(stringType.compareTo("SRTP_FEC", UtlString::ignoreCase) == 0)
   {
      return CRYPTO_SRTP_FEC_ORDER_SRTP_FEC;
   }
   else
   {
      return CRYPTO_SRTP_FEC_ORDER_NONE;
   }
}

SdpMediaLine::SdpFingerPrintHashFuncType 
SdpMediaLine::getFingerPrintHashFuncTypeFromString(const char * type)
{
   UtlString stringType(type);

   if(stringType.compareTo("sha-1", UtlString::ignoreCase) == 0)
   {
      return FINGERPRINT_HASH_FUNC_SHA_1;
   }
   else if(stringType.compareTo("sha-224", UtlString::ignoreCase) == 0)
   {
      return FINGERPRINT_HASH_FUNC_SHA_224;
   }
   else if(stringType.compareTo("sha-256", UtlString::ignoreCase) == 0)
   {
      return FINGERPRINT_HASH_FUNC_SHA_256;
   }
   else if(stringType.compareTo("sha-384", UtlString::ignoreCase) == 0)
   {
      return FINGERPRINT_HASH_FUNC_SHA_384;
   }
   else if(stringType.compareTo("sha-512", UtlString::ignoreCase) == 0)
   {
      return FINGERPRINT_HASH_FUNC_SHA_512;
   }
   else if(stringType.compareTo("md5", UtlString::ignoreCase) == 0)
   {
      return FINGERPRINT_HASH_FUNC_MD5;
   }
   else if(stringType.compareTo("md2", UtlString::ignoreCase) == 0)
   {
      return FINGERPRINT_HASH_FUNC_MD2;
   }
   else
   {
      return FINGERPRINT_HASH_FUNC_NONE;
   }
}

SdpMediaLine::SdpKeyManagementProtocolType 
SdpMediaLine::getKeyManagementProtocolTypeFromString(const char * type)
{
   UtlString stringType(type);

   if(stringType.compareTo("mikey", UtlString::ignoreCase) == 0)
   {
      return KEYMANAGEMENT_PROTOCOL_MIKEY;
   }
   else
   {
      return KEYMANAGEMENT_PROTOCOL_NONE;
   }
}

SdpMediaLine::SdpPreConditionType 
SdpMediaLine::getPreConditionTypeFromString(const char * type)
{
   UtlString stringType(type);

   if(stringType.compareTo("qos", UtlString::ignoreCase) == 0)
   {
      return PRECONDITION_TYPE_QOS;
   }
   else
   {
      return PRECONDITION_TYPE_NONE;
   }
}

SdpMediaLine::SdpPreConditionStatusType 
SdpMediaLine::getPreConditionStatusTypeFromString(const char * type)
{
   UtlString stringType(type);

   if(stringType.compareTo("e2e", UtlString::ignoreCase) == 0)
   {
      return PRECONDITION_STATUS_E2E;
   }
   else if(stringType.compareTo("local", UtlString::ignoreCase) == 0)
   {
      return PRECONDITION_STATUS_LOCAL;
   }
   else if(stringType.compareTo("remote", UtlString::ignoreCase) == 0)
   {
      return PRECONDITION_STATUS_REMOTE;
   }
   else
   {
      return PRECONDITION_STATUS_NONE;
   }
}

SdpMediaLine::SdpPreConditionDirectionType 
SdpMediaLine::getPreConditionDirectionTypeFromString(const char * type)
{
   UtlString stringType(type);

   if(stringType.compareTo("send", UtlString::ignoreCase) == 0)
   {
      return PRECONDITION_DIRECTION_SEND;
   }
   else if(stringType.compareTo("recv", UtlString::ignoreCase) == 0)
   {
      return PRECONDITION_DIRECTION_RECV;
   }
   else if(stringType.compareTo("sendrecv", UtlString::ignoreCase) == 0)
   {
      return PRECONDITION_DIRECTION_SENDRECV;
   }
   else
   {
      return PRECONDITION_DIRECTION_NONE;
   }
}

SdpMediaLine::SdpPreConditionStrengthType 
SdpMediaLine::getPreConditionStrengthTypeFromString(const char * type)
{
   UtlString stringType(type);

   if(stringType.compareTo("mandatory", UtlString::ignoreCase) == 0)
   {
      return PRECONDITION_STRENGTH_MANDATORY;
   }
   else if(stringType.compareTo("optional", UtlString::ignoreCase) == 0)
   {
      return PRECONDITION_STRENGTH_OPTIONAL;
   }
   else if(stringType.compareTo("none", UtlString::ignoreCase) == 0)
   {
      return PRECONDITION_STRENGTH_NONE;
   }
   else if(stringType.compareTo("failure", UtlString::ignoreCase) == 0)
   {
      return PRECONDITION_STRENGTH_FAILURE;
   }
   else if(stringType.compareTo("unknown", UtlString::ignoreCase) == 0)
   {
      return PRECONDITION_STRENGTH_UNKNWOWN;
   }
   else
   {
      return PRECONDITION_STRENGTH_NONE;
   }
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
