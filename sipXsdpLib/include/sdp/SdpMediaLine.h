// 
// Copyright (C) 2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
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

#ifndef _SdpMediaLine_h_
#define _SdpMediaLine_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <utl/UtlInt.h>
#include <utl/UtlString.h>
#include <utl/UtlCopyableSortedList.h>
#include <utl/UtlCopyableContainable.h>
#include <utl/UtlCopyableSList.h>

#include <sdp/Sdp.h>
#include <sdp/SdpCodec.h>
#include <sdp/SdpCandidate.h>
#include <sdp/SdpCandidatePair.h>


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: Container for SDP specification
// This class holds the information related to an SDP.
// Included in this information is:  TODO
//

class SdpMediaLine : public UtlCopyableContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   typedef enum 
   {
      MEDIA_TYPE_NONE,
      MEDIA_TYPE_AUDIO,          // "audio" - RFC4566
      MEDIA_TYPE_VIDEO,          // "video" - RFC4566
      MEDIA_TYPE_TEXT,           // "text" - RFC4566
      MEDIA_TYPE_APPLICATION,    // "application" - RFC4566
      MEDIA_TYPE_MESSAGE         // "message" - RFC4566
   } SdpMediaType;
   static const char* SdpMediaTypeString[];

   typedef enum 
   {
      PROTOCOL_TYPE_NONE,
      PROTOCOL_TYPE_UDP,         // "udp" - RFC4566
      PROTOCOL_TYPE_RTP_AVP,     // "RTP/AVP" - RFC4566
      PROTOCOL_TYPE_RTP_SAVP,    // "RTP/SAVP" - RFC4566
      PROTOCOL_TYPE_RTP_SAVPF,   // "RTP/SAVPF" - RFC3711
      PROTOCOL_TYPE_TCP,         // "TCP" - RFC4145
      PROTOCOL_TYPE_TCP_RTP_AVP, // "TCP/RTP/AVP" - RFC4571
      PROTOCOL_TYPE_TCP_TLS      // "TCP/TLS" - RFC4572
   } SdpTransportProtocolType;     
   static const char* SdpTransportProtocolTypeString[];

   class SdpConnection : public UtlCopyableContainable
   {
   public:
      SdpConnection(Sdp::SdpNetType netType = Sdp::NET_TYPE_NONE, 
                    Sdp::SdpAddressType addressType = Sdp::ADDRESS_TYPE_NONE, 
                    const char * address = 0, 
                    unsigned int port = 0,
                    unsigned int multicastIpV4Ttl=0) :
         mNetType(netType), mAddressType(addressType), mAddress(address), mPort(port), mMulticastIpV4Ttl(multicastIpV4Ttl) {}
      SdpConnection(const SdpConnection& rhs) :
         mNetType(rhs.mNetType), mAddressType(rhs.mAddressType), mAddress(rhs.mAddress), mPort(rhs.mPort), mMulticastIpV4Ttl(rhs.mMulticastIpV4Ttl) {}

      // Containable requirements
      UtlContainableType getContainableType() const { static char type[] = "SdpConnection"; return type;}
      unsigned hash() const { return directHash(); }
      int compareTo(UtlContainable const *) const { return 0; } // not implemented
      UtlCopyableContainable* clone() const { return new SdpConnection(*this); }

      // Accessors
      void setNetType(Sdp::SdpNetType netType) { mNetType = netType; }
      Sdp::SdpNetType getNetType() const { return mNetType; }

      void setAddressType(Sdp::SdpAddressType addressType) { mAddressType = addressType; }
      Sdp::SdpAddressType getAddressType() const { return mAddressType; }

      void setAddress(const char * address) { mAddress = address; }
      const UtlString& getAddress() const { return mAddress; }

      void setPort(unsigned int port) { mPort = port; }
      unsigned int getPort() const { return mPort; }

      void setMulticastIpV4Ttl(unsigned int multicastIpV4Ttl) { mMulticastIpV4Ttl = multicastIpV4Ttl; }
      unsigned int getMulticastIpV4Ttl() const { return mMulticastIpV4Ttl; }

   private:
      Sdp::SdpNetType      mNetType;
      Sdp::SdpAddressType  mAddressType;
      UtlString            mAddress;
      unsigned int         mPort;
      unsigned int         mMulticastIpV4Ttl;
   };

   typedef enum 
   {
      ENCRYPTION_METHOD_NONE,
      ENCRYPTION_METHOD_CLEAR,   // "clear" - RFC4566
      ENCRYPTION_METHOD_BASE64,  // "base64" - RFC4566
      ENCRYPTION_METHOD_URI,     // "uri" - RFC4566
      ENCRYPTION_METHOD_PROMPT   // "prompt" - RFC4566
   } SdpEncryptionMethod;
   static const char* SdpEncryptionMethodString[];

   typedef enum 
   {
      DIRECTION_TYPE_NONE,
      DIRECTION_TYPE_SENDRECV,   // "sendrecv" - RFC4566
      DIRECTION_TYPE_SENDONLY,   // "sendonly" - RFC4566
      DIRECTION_TYPE_RECVONLY,   // "recvonly" - RFC4566
      DIRECTION_TYPE_INACTIVE    // "inactive" - RFC4566
   } SdpDirectionType;
   static const char* SdpDirectionTypeString[];

   typedef enum 
   {
      ORIENTATION_TYPE_NONE,
      ORIENTATION_TYPE_PORTRAIT, // "portrait" - RFC 4566
      ORIENTATION_TYPE_LANDSCAPE,// "landscape" - RFC 4566
      ORIENTATION_TYPE_SEASCAPE  // "seascape" - RFC 4566
   } SdpOrientationType;
   static const char* SdpOrientationTypeString[];

   typedef enum 
   {
      TCP_SETUP_ATTRIBUTE_NONE,
      TCP_SETUP_ATTRIBUTE_ACTIVE,  // "active" - RFC4145
      TCP_SETUP_ATTRIBUTE_PASSIVE, // "passive" - RFC4145
      TCP_SETUP_ATTRIBUTE_ACTPASS, // "actpass" - RFC4145
      TCP_SETUP_ATTRIBUTE_HOLDCONN // "holdconn" - RFC4145
   } SdpTcpSetupAttribute;
   static const char* SdpTcpSetupAttributeString[];

   typedef enum 
   {
      TCP_CONNECTION_ATTRIBUTE_NONE,
      TCP_CONNECTION_ATTRIBUTE_NEW,      // "new" - RFC4145
      TCP_CONNECTION_ATTRIBUTE_EXISTING  // "existing" - RFC4145
   } SdpTcpConnectionAttribute;
   static const char* SdpTcpConnectionAttributeString[];

   typedef enum 
   {
      CRYPTO_SUITE_TYPE_NONE,
      CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80,   // "AES_CM_128_HMAC_SHA1_80" - RFC4568
      CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_32,   // "AES_CM_128_HMAC_SHA1_32" - RFC4568
      CRYPTO_SUITE_TYPE_F8_128_HMAC_SHA1_80        // "F8_128_HMAC_SHA1_80" - RFC4568
   } SdpCryptoSuiteType;
   static const char* SdpCryptoSuiteTypeString[];

   typedef enum 
   {
      CRYPTO_KEY_METHOD_NONE,
      CRYPTO_KEY_METHOD_INLINE  // "inline" - RFC4568
   } SdpCryptoKeyMethod;
   static const char* SdpCryptoKeyMethodString[];

   typedef enum
   {
      CRYPTO_SRTP_FEC_ORDER_NONE,
      CRYPTO_SRTP_FEC_ORDER_FEC_SRTP,              // "FEC_SRTP" - RFC 4568
      CRYPTO_SRTP_FEC_ORDER_SRTP_FEC               // "SRTP_FEC" - RFC 2568
   } SdpCryptoSrtpFecOrderType;
   static const char* SdpCryptoSrtpFecOrderTypeString[];

   class SdpCrypto : public UtlCopyableContainable
   {
   public:
      class SdpCryptoKeyParam : public UtlCopyableContainable
      {
      public:
         SdpCryptoKeyParam(SdpCryptoKeyMethod keyMethod=SdpMediaLine::CRYPTO_KEY_METHOD_NONE, const char * keyValue=0, unsigned int srtpLifetime=0, unsigned int srtpMkiValue=0, unsigned int srtpMkiLength=0) :
            mKeyMethod(keyMethod), mKeyValue(keyValue), mSrtpLifetime(srtpLifetime), mSrtpMkiValue(srtpMkiValue), mSrtpMkiLength(srtpMkiLength) {}
         SdpCryptoKeyParam(const SdpCryptoKeyParam& rhs) :
            mKeyMethod(rhs.mKeyMethod), mKeyValue(rhs.mKeyValue), mSrtpLifetime(rhs.mSrtpLifetime), mSrtpMkiValue(rhs.mSrtpMkiValue), mSrtpMkiLength(rhs.mSrtpMkiLength) {}

         // Containable requirements
         UtlContainableType getContainableType() const { static char type[] = "SdpCryptoKeyParam"; return type;}
         unsigned hash() const { return directHash(); }
         int compareTo(UtlContainable const *) const { return 0; } // not implemented
         UtlCopyableContainable* clone() const { return new SdpCryptoKeyParam(*this); }

         void setKeyMethod(SdpCryptoKeyMethod keyMethod) { mKeyMethod = keyMethod; }
         SdpCryptoKeyMethod getKeyMethod() const { return mKeyMethod; }

         void setKeyValue(const char * keyValue) { mKeyValue = keyValue; }
         const UtlString& getKeyValue() const { return mKeyValue; }

         void setSrtpLifetime(unsigned int srtpLifetime) { mSrtpLifetime = srtpLifetime; }
         unsigned int getSrtpLifetime() const { return mSrtpLifetime; }

         void setSrtpMkiValue(unsigned int srtpMkiValue) { mSrtpMkiValue = srtpMkiValue; }
         unsigned int getSrtpMkiValue() const { return mSrtpMkiValue; }

         void setSrtpMkiLength(unsigned int srtpMkiLength) { mSrtpMkiLength = srtpMkiLength; }
         unsigned int getSrtpMkiLength() const { return mSrtpMkiLength; }

      private:
         SdpCryptoKeyMethod mKeyMethod;
         UtlString          mKeyValue;  // srtp key-salt or generic key-info
         unsigned int       mSrtpLifetime;
         unsigned int       mSrtpMkiValue;
         unsigned int       mSrtpMkiLength;
      };

      SdpCrypto() : mTag(0), mSuite(SdpMediaLine::CRYPTO_SUITE_TYPE_NONE), mSrtpKdr(0), mEncryptedSrtp(1), mEncryptedSrtcp(1),
         mAuthenticatedSrtp(1), mSrtpFecOrder(SdpMediaLine::CRYPTO_SRTP_FEC_ORDER_FEC_SRTP), mSrtpWsh(0) {}
      SdpCrypto(const SdpCrypto& rSdpCandidatePair);
      ~SdpCrypto() { }

      // Containable requirements
      UtlContainableType getContainableType() const { static char type[] = "SdpCrypto"; return type;}
      unsigned hash() const { return directHash(); }
      int compareTo(UtlContainable const *) const { return 0; } // not implemented
      SdpCrypto& operator=(const SdpCrypto& rhs);
      UtlCopyableContainable* clone() const { return new SdpCrypto(*this); }

      // Accessors
      void setTag(unsigned int tag) { mTag = tag; }
      unsigned int getTag() const { return mTag; }

      void setSuite(SdpCryptoSuiteType suite) { mSuite = suite; }
      SdpCryptoSuiteType getSuite() const { return mSuite; }

      void addCryptoKeyParam(SdpCryptoKeyMethod keyMethod, const char * keyValue, unsigned int srtpLifetime=0, unsigned int srtpMkiValue=0, unsigned int srtpMkiLength=0)
      { addCryptoKeyParam(new SdpCryptoKeyParam(keyMethod, keyValue, srtpLifetime, srtpMkiValue, srtpMkiLength)); }
      void addCryptoKeyParam(SdpCryptoKeyParam* keyParam) { mCryptoKeyParams.insert(keyParam); }
      void clearCryptoKeyParams() { mCryptoKeyParams.destroyAll(); }
      const UtlCopyableSList& getCryptoKeyParams() const { return mCryptoKeyParams; }

      void setSrtpKdr(unsigned int srtpKdr) { mSrtpKdr = srtpKdr; }
      unsigned int getSrtpKdr() const { return mSrtpKdr; }

      void setEncryptedSrtp(bool encryptedSrtp) { mEncryptedSrtp = encryptedSrtp; }
      bool getEncryptedSrtp() const { return mEncryptedSrtp; }

      void setEncryptedSrtcp(bool encryptedSrtcp) { mEncryptedSrtcp = encryptedSrtcp; }
      bool getEncryptedSrtcp() const { return mEncryptedSrtcp; }

      void setAuthenticatedSrtp(bool authenticatedSrtp) { mAuthenticatedSrtp = authenticatedSrtp; }
      bool getAuthenticatedSrtp() const { return mAuthenticatedSrtp; }

      void setSrtpFecOrder(SdpCryptoSrtpFecOrderType srtpFecOrder) { mSrtpFecOrder = srtpFecOrder; }
      SdpCryptoSrtpFecOrderType getSrtpFecOrder() const { return mSrtpFecOrder; }
      static SdpCryptoSrtpFecOrderType getSrtpFecOrderFromString(const char * order);

      void setSrtpFecKey(SdpCryptoKeyMethod keyMethod, const char * keyValue, unsigned int srtpLifetime=0, unsigned int srtpMkiValue=0, unsigned int srtpMkiLength=0) 
      { mSrtpFecKey.setKeyMethod(keyMethod); mSrtpFecKey.setKeyValue(keyValue); mSrtpFecKey.setSrtpLifetime(srtpLifetime); 
        mSrtpFecKey.setSrtpMkiValue(srtpMkiValue); mSrtpFecKey.setSrtpMkiLength(srtpMkiLength); }
      const SdpCryptoKeyParam& getSrtpFecKey() const { return mSrtpFecKey; }

      void setSrtpWsh(unsigned int srtpWsh) { mSrtpWsh = srtpWsh; }
      unsigned int getSrtpWsh() const { return mSrtpWsh; }

      void addGenericSessionParam(const char * sessionParam) { mGenericSessionParams.insert(new UtlString(sessionParam)); }
      void clearGenericSessionParams() { mGenericSessionParams.destroyAll(); }
      const UtlCopyableSList& getGenericSessionParams() const { return mGenericSessionParams; }

   private:
      unsigned int       mTag;         
      SdpCryptoSuiteType mSuite;
      UtlCopyableSList   mCryptoKeyParams;
      unsigned int       mSrtpKdr;
      bool               mEncryptedSrtp;
      bool               mEncryptedSrtcp;
      bool               mAuthenticatedSrtp;
      SdpCryptoSrtpFecOrderType mSrtpFecOrder;
      SdpCryptoKeyParam  mSrtpFecKey;
      unsigned int       mSrtpWsh;
      UtlCopyableSList   mGenericSessionParams; 
   };

   typedef enum 
   {
      FINGERPRINT_HASH_FUNC_NONE,
      FINGERPRINT_HASH_FUNC_SHA_1,        // "sha-1" - RFC4572
      FINGERPRINT_HASH_FUNC_SHA_224,      // "sha-224" - RFC4572
      FINGERPRINT_HASH_FUNC_SHA_256,      // "sha-256" - RFC4572
      FINGERPRINT_HASH_FUNC_SHA_384,      // "sha-384" - RFC4572
      FINGERPRINT_HASH_FUNC_SHA_512,      // "sha-512" - RFC4572
      FINGERPRINT_HASH_FUNC_MD5,          // "md5" - RFC4572
      FINGERPRINT_HASH_FUNC_MD2           // "md2" - RFC4572
   } SdpFingerPrintHashFuncType;
   static const char* SdpFingerPrintHashFuncTypeString[];

   typedef enum 
   {
      KEYMANAGEMENT_PROTOCOL_NONE,
      KEYMANAGEMENT_PROTOCOL_MIKEY        // 'mikey' - RFC4567
   } SdpKeyManagementProtocolType;
   static const char* SdpKeyManagementProtocolTypeString[];

   typedef enum 
   {
      PRECONDITION_TYPE_NONE,
      PRECONDITION_TYPE_QOS               // "qos" - RFC3312
   } SdpPreConditionType;
   static const char* SdpPreConditionTypeString[];

   typedef enum 
   {
      PRECONDITION_STRENGTH_MANDATORY,    // "mandatory" - RFC3312
      PRECONDITION_STRENGTH_OPTIONAL,     // "optional" - RFC3312
      PRECONDITION_STRENGTH_NONE,         // "none" - RFC3312
      PRECONDITION_STRENGTH_FAILURE,      // "failure" - RFC3312
      PRECONDITION_STRENGTH_UNKNWOWN      // "unknown" - RFC3312
   } SdpPreConditionStrengthType;
   static const char* SdpPreConditionStrengthTypeString[];

   typedef enum 
   {
      PRECONDITION_STATUS_NONE,
      PRECONDITION_STATUS_E2E,            // "e2e" - RFC3312
      PRECONDITION_STATUS_LOCAL,          // "local" - RFC3312
      PRECONDITION_STATUS_REMOTE,         // "remote" - RFC3312
   } SdpPreConditionStatusType;
   static const char* SdpPreConditionStatusTypeString[];

   typedef enum 
   {
      PRECONDITION_DIRECTION_NONE,        // "none" - RFC3312
      PRECONDITION_DIRECTION_SEND,        // "send" - RFC3312
      PRECONDITION_DIRECTION_RECV,        // "recv" - RFC3312
      PRECONDITION_DIRECTION_SENDRECV,    // "sendrecv" - RFC3312
   } SdpPreConditionDirectionType;
   static const char* SdpPreConditionDirectionTypeString[];

   class SdpPreCondition : public UtlCopyableContainable
   {
   public:
      SdpPreCondition(SdpPreConditionType type, SdpPreConditionStatusType status, SdpPreConditionDirectionType direction) :
             mType(type), mStatus(status), mDirection(direction) {}
      SdpPreCondition(const SdpPreCondition& rhs) :
             mType(rhs.mType), mStatus(rhs.mStatus), mDirection(rhs.mDirection) {}

      // Containable requirements
      UtlContainableType getContainableType() const { static char type[] = "SdpPreCondition"; return type;}
      unsigned hash() const { return directHash(); }
      int compareTo(UtlContainable const *) const { return 0; } // not implemented
      UtlCopyableContainable* clone() const { return new SdpPreCondition(*this); }

      // Accessors
      void setType(SdpPreConditionType type) { mType = type; }
      SdpPreConditionType getType() const { return mType; }

      void setStatus(SdpPreConditionStatusType status) { mStatus = status; }
      SdpPreConditionStatusType getStatus() const { return mStatus; }

      void setDirection(SdpPreConditionDirectionType direction) { mDirection = direction; }
      SdpPreConditionDirectionType getDirection() const { return mDirection; }

   private:
      SdpPreConditionType          mType;
      SdpPreConditionStatusType    mStatus;
      SdpPreConditionDirectionType mDirection;
   };

   class SdpPreConditionDesiredStatus : public SdpPreCondition
   {
   public:
      SdpPreConditionDesiredStatus(SdpPreConditionType type, SdpPreConditionStrengthType strength, SdpPreConditionStatusType status, SdpPreConditionDirectionType direction) :
         SdpPreCondition(type, status, direction), mStrength(strength) {}
      SdpPreConditionDesiredStatus(const SdpPreConditionDesiredStatus& rhs) :
         SdpPreCondition(rhs), mStrength(rhs.mStrength) {}

      // Containable requirements
      UtlContainableType getContainableType() const { static char type[] = "SdpPreConditionDesiredStatus"; return type;}
      UtlCopyableContainable* clone() const { return new SdpPreConditionDesiredStatus(*this); }

      // Accessors
      void setStrength(SdpPreConditionStrengthType strength) { mStrength = strength; }
      SdpPreConditionStrengthType getStrength() const { return mStrength; }

   private:
      SdpPreConditionStrengthType  mStrength;
   };

   class SdpRemoteCandidate : public UtlCopyableContainable
   {
   public:
      SdpRemoteCandidate(unsigned int componentId, const char * connectionAddress, unsigned int port) :
         mComponentId(componentId), mConnectionAddress(connectionAddress), mPort(port) {}
      SdpRemoteCandidate(const SdpRemoteCandidate& rhs) :
         mComponentId(rhs.mComponentId), mConnectionAddress(rhs.mConnectionAddress), mPort(rhs.mPort) {}

      // Containable requirements
      UtlContainableType getContainableType() const { static char type[] = "SdpRemoteCandidate"; return type;}
      unsigned hash() const { return directHash(); }
      int compareTo(UtlContainable const *) const { return 0; } // not implemented
      UtlCopyableContainable* clone() const { return new SdpRemoteCandidate(*this); }

      // Accessors
      void setComponentId(unsigned int componentId) { mComponentId = componentId; }
      unsigned int getComponentId() const { return mComponentId; }

      void setConnectionAddress(const char * connectionAddress) { mConnectionAddress = connectionAddress; }
      const UtlString& getConnectionAddress() const { return mConnectionAddress; }

      void setPort(unsigned int port) { mPort = port; }
      unsigned int getPort() const { return mPort; }

   private:
      unsigned int   mComponentId;
      UtlString      mConnectionAddress;
      unsigned int   mPort;
   };


/* ============================ CREATORS ================================== */

   SdpMediaLine();
     //:Default constructor

   SdpMediaLine(const SdpMediaLine& rSdpMediaLine);
     //:Copy constructor

   virtual
   ~SdpMediaLine();
     //:Destructor

   UtlCopyableContainable* clone() const { return new SdpMediaLine(*this); }

/* ============================ MANIPULATORS ============================== */

   SdpMediaLine& operator=(const SdpMediaLine& rhs);
     //:Assignment operator

   void setMediaType(SdpMediaType mediaType) { mMediaType = mediaType; }
   void setTransportProtocolType(SdpTransportProtocolType transportProtocolType) { mTransportProtocolType = transportProtocolType; }

   void addCodec(SdpCodec* codec) { mCodecs.insert(codec); }
   void clearCodecs() { mCodecs.destroyAll(); }

   void setTitle(const char * title) { mTitle = title; }

   void addConnection(Sdp::SdpNetType netType, Sdp::SdpAddressType addressType, const char * address, unsigned int port, unsigned int multicastIpV4Ttl=0) 
        { addConnection(new SdpConnection(netType, addressType, address, port, multicastIpV4Ttl)); }
   void addConnection(SdpConnection *connection) { mConnections.insert(connection); }
   void clearConnections() { mConnections.destroyAll(); }

   void addRtcpConnection(Sdp::SdpNetType netType, Sdp::SdpAddressType addressType, const char * address, unsigned int port, unsigned int multicastIpV4Ttl=0) 
        { addRtcpConnection(new SdpConnection(netType, addressType, address, port, multicastIpV4Ttl)); }
   void addRtcpConnection(SdpConnection *connection) { mRtcpConnections.insert(connection); }
   void clearRtcpConnections() { mRtcpConnections.destroyAll(); }

   void addBandwidth(Sdp::SdpBandwidthType type, unsigned int bandwidth) { addBandwidth(new Sdp::SdpBandwidth(type, bandwidth)); }
   void addBandwidth(Sdp::SdpBandwidth* sdpBandwidth) { mBandwidths.insert(sdpBandwidth); }
   void clearBandwidths() { mBandwidths.destroyAll(); }

   void setEncryptionKey(SdpEncryptionMethod method, const char * key) { mEncryptionMethod = method; mEncryptionKey = key; }
   void setDirection(SdpDirectionType direction) { mDirection = direction; }
   void setPacketTime(unsigned int packetTime) { mPacketTime = packetTime; }
   void setMaxPacketTime(unsigned int maxPacketTime) { mMaxPacketTime = maxPacketTime; }
   void setOrientation(SdpOrientationType orientation) { mOrientation = orientation; }
   void setDescriptionLanguage(const char * descriptionLanguage) { mDescriptionLanguage = descriptionLanguage; }
   void setLanguage(const char * language) { mLanguage = language; }
   void setFrameRate(unsigned int frameRate) { mFrameRate = frameRate; }
   void setQuality(unsigned int quality) { mQuality = quality; }

   void setTcpSetupAttribute(SdpTcpSetupAttribute tcpSetupAttribute) { mTcpSetupAttribute = tcpSetupAttribute; }
   void setTcpConnectionAttribute(SdpTcpConnectionAttribute tcpConnectionAttribute) { mTcpConnectionAttribute = tcpConnectionAttribute; }

//   void addCryptoSettings(unsigned int tag, SdpCryptoSuiteType suite, SdpCryptoKeyMethod keyMethod, const char * keyValue) { addCryptoSettings(new SdpCrypto(tag, suite, keyMethod, keyValue)); }
   void addCryptoSettings(SdpCrypto* crypto) { mCryptos.insert(crypto); }
   void clearCryptoSettings() { mCryptos.destroyAll(); }

   void setFingerPrint(SdpFingerPrintHashFuncType fingerPrintHashFunction, const char * fingerPrint) { mFingerPrintHashFunction = fingerPrintHashFunction; mFingerPrint = fingerPrint; }
   void setKeyManagementProtocol(SdpKeyManagementProtocolType protocol, const char* data) { mKeyManagementProtocol = protocol; mKeyManagementData = data; }

   void addPreConditionCurrentStatus(SdpPreConditionType type, SdpPreConditionStatusType status, SdpPreConditionDirectionType direction) 
        { addPreConditionCurrentStatus(new SdpPreCondition(type, status, direction)); }
   void addPreConditionCurrentStatus(SdpPreCondition* preCondition) { mPreConditionCurrentStatus.insert(preCondition); }
   void clearPreConditionCurrentStatus() { mPreConditionCurrentStatus.destroyAll(); }

   void addPreConditionConfirmStatus(SdpPreConditionType type, SdpPreConditionStatusType status, SdpPreConditionDirectionType direction) 
        { addPreConditionConfirmStatus(new SdpPreCondition(type, status, direction)); }
   void addPreConditionConfirmStatus(SdpPreCondition* preCondition) { mPreConditionConfirmStatus.insert(preCondition); }
   void clearPreConditionConfirmStatus() { mPreConditionConfirmStatus.destroyAll(); }

   void addPreConditionDesiredStatus(SdpPreConditionType type, SdpPreConditionStrengthType strength, SdpPreConditionStatusType status, SdpPreConditionDirectionType direction) 
        { addPreConditionDesiredStatus(new SdpPreConditionDesiredStatus(type, strength, status, direction)); }
   void addPreConditionDesiredStatus(SdpPreConditionDesiredStatus* preConditionDesiredStatus) { mPreConditionDesiredStatus.insert(preConditionDesiredStatus); }
   void clearPreConditionDesiredStatus() { mPreConditionDesiredStatus.destroyAll(); }

   void setMaximumPacketRate(double maximumPacketRate) { mMaximumPacketRate = maximumPacketRate; }
   void setLabel(const char * label) { mLabel = label; }
   void setIdentificationTag(const char * identificationTag) { mIdentificationTag = identificationTag; }

   void setIceUserFrag(const char * iceUserFrag) { mIceUserFrag = iceUserFrag; }
   void setIcePassword(const char * icePassword) { mIcePassword = icePassword; }

   void addRemoteCandidate(unsigned int componentId, const char * connectionAddress, unsigned int port) { addRemoteCandidate(new SdpRemoteCandidate(componentId, connectionAddress, port)); }
   void addRemoteCandidate(SdpRemoteCandidate* remoteCandidate) { mRemoteCandidates.insert(remoteCandidate); }
   void clearRemoteCandidates() { mRemoteCandidates.destroyAll(); }

   // Note:  Candidates should be added after m/c line and rtcp information is set, so that the in-use candidate 
   //        can be properly tagged and CandidatePresents flag can be properly set
   void addCandidate(const char * foundation, unsigned int id, SdpCandidate::SdpCandidateTransportType transport, uint64_t priority, const char * connectionAddress, 
                     unsigned int port, SdpCandidate::SdpCandidateType candidateType, const char * relatedAddress = 0, unsigned int relatedPort = 0)
        { addCandidate(new SdpCandidate(foundation, id, transport, priority, connectionAddress, port, candidateType, relatedAddress, relatedPort)); }
   void addCandidate(SdpCandidate* candidate);
   void clearCandidates() { mCandidates.destroyAll(); mRtpCandidatePresent = false; mRtcpCandidatePresent = false; }

   void addCandidatePair(const SdpCandidate& localCandidate, const SdpCandidate& remoteCandidate, SdpCandidatePair::SdpCandidatePairOffererType offerer)
        { addCandidatePair( new SdpCandidatePair(localCandidate, remoteCandidate, offerer)); }
   void addCandidatePair(SdpCandidatePair* sdpCandidatePair) { mCandidatePairs.insert(sdpCandidatePair); }
   void clearCandidatePairs() { mCandidatePairs.destroyAll(); }


/* ============================ ACCESSORS ================================= */

   virtual UtlContainableType getContainableType() const { static char type[] = "SdpMediaLine"; return type;}
   virtual unsigned hash() const { return directHash(); }
   virtual int compareTo(UtlContainable const *) const { return 0; }  // not implemented

   void toString(UtlString& sdpMediaLineString) const;

/* ============================ INQUIRY =================================== */
   
   SdpMediaType getMediaType() const { return mMediaType; }
   static SdpMediaType getMediaTypeFromString(const char * type);
   SdpTransportProtocolType getTransportProtocolType() const { return mTransportProtocolType; }
   static SdpTransportProtocolType getTransportProtocolTypeFromString(const char * type);
   const UtlCopyableSList& getCodecs() const { return mCodecs; }
   const UtlString& getTitle() const  { return mTitle; }
   const UtlCopyableSList& getConnections() const { return mConnections; }
   const UtlCopyableSList& getRtcpConnections() const { return mRtcpConnections; }
   const UtlCopyableSList& getBandwidths() const { return mBandwidths; }
   SdpEncryptionMethod getEncryptionMethod() const { return mEncryptionMethod; }
   const UtlString& getEncryptionKey() const { return mEncryptionKey; }   
   SdpDirectionType getDirection() const { return mDirection; }
   unsigned int getPacketTime() const { return mPacketTime; }
   unsigned int getMaxPacketTime() const { return mMaxPacketTime; }
   SdpOrientationType getOrientation() const { return mOrientation; }
   static SdpOrientationType getOrientationTypeFromString(const char * type);
   const UtlString& getDescriptionLanguage() const { return mDescriptionLanguage; }
   const UtlString& getLanguage() const { return mLanguage; }
   unsigned int getFrameRate() const { return mFrameRate; }
   unsigned int getQuality() const { return mQuality; }
   SdpTcpSetupAttribute getTcpSetupAttribute() const { return mTcpSetupAttribute; }
   static SdpTcpSetupAttribute getTcpSetupAttributeFromString(const char * attrib);
   SdpTcpConnectionAttribute getTcpConnectionAttribute() const { return mTcpConnectionAttribute; }
   static SdpTcpConnectionAttribute getTcpConnectionAttributeFromString(const char * attrib);
   const UtlCopyableSList& getCryptos() const { return mCryptos; }
   static SdpCryptoSuiteType getCryptoSuiteTypeFromString(const char * type);
   static SdpCryptoKeyMethod getCryptoKeyMethodFromString(const char * type);
   SdpFingerPrintHashFuncType getFingerPrintHashFunction() const { return mFingerPrintHashFunction; }
   static SdpFingerPrintHashFuncType getFingerPrintHashFuncTypeFromString(const char * type);
   const UtlString& getFingerPrint() const { return mFingerPrint; }
   SdpKeyManagementProtocolType getKeyManagementProtocol() const { return mKeyManagementProtocol; }
   static SdpKeyManagementProtocolType getKeyManagementProtocolTypeFromString(const char * type);
   const UtlString& getKeyManagementData() const { return mKeyManagementData; }
   const UtlCopyableSList& getPreConditionCurrentStatus() const { return mPreConditionCurrentStatus; }
   const UtlCopyableSList& getPreConditionConfirmStatus() const { return mPreConditionConfirmStatus; }
   const UtlCopyableSList& getPreConditionDesiredStatus() const { return mPreConditionDesiredStatus; }
   static SdpPreConditionType getPreConditionTypeFromString(const char * type);
   static SdpPreConditionStatusType getPreConditionStatusTypeFromString(const char * type);
   static SdpPreConditionDirectionType getPreConditionDirectionTypeFromString(const char * type);
   static SdpPreConditionStrengthType getPreConditionStrengthTypeFromString(const char * type);
   double getMaximumPacketRate() const { return mMaximumPacketRate; }
   const UtlString& getLabel() const { return mLabel; }
   const UtlString& getIdentificationTag() const { return mIdentificationTag; }
   const UtlString& getIceUserFrag() const { return mIceUserFrag; }
   const UtlString& getIcePassword() const { return mIcePassword; }
   const UtlCopyableSList& getRemoteCandidates() const { return mRemoteCandidates; }
   const UtlCopyableSortedList& getCandidates() const { return mCandidates; }

   const bool isRtcpEnabled() const { return mRtcpConnections.entries() > 0; }
   const bool isRtpCandidatePresent() const { return mRtpCandidatePresent; }
   const bool isRtcpCandidatePresent() const { return mRtcpCandidatePresent; }
   const bool isIceSupported() const { return  mRtpCandidatePresent && (!isRtcpEnabled() || mRtcpCandidatePresent); }

   const UtlCopyableSortedList& getCandidatePairs() const { return mCandidatePairs; }
   UtlCopyableSortedList& getCandidatePairs() { return mCandidatePairs; }  // non-const version for manipulation

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   // m=  Note:  port is stored in each connection
   SdpMediaType   mMediaType;
   SdpTransportProtocolType mTransportProtocolType;
   UtlCopyableSList mCodecs;

   // i=
   UtlString      mTitle;

   // c=
   UtlCopyableSList mConnections;           // includes port from m- line
   UtlCopyableSList mRtcpConnections;       // a=rtcp <port> [<nettype> <addrtype> <connection addr>] - RFC3605

   // b=
   UtlCopyableSList mBandwidths;

   // k=
   SdpEncryptionMethod  mEncryptionMethod;
   UtlString            mEncryptionKey;

   // a= media level attributes (including defaults copied from session level attributes)
   SdpDirectionType mDirection;           // a=sendrecv, a=sendonly, a=recvonly, a=inactive - RFC4566
   unsigned int   mPacketTime;            // a=ptime:<packet time> in ms - RFC4566
   unsigned int   mMaxPacketTime;         // a=maxptime:<maximum packet time> in ms - RFC4566
   SdpOrientationType mOrientation;       // a=orient:<orientation> - RFC4566
   UtlString      mDescriptionLanguage;   // a=sdplang:<language tag> - RFC4566
   UtlString      mLanguage;              // a=lang:<language tag> - RFC4566
   unsigned int   mFrameRate;             // a=framerate:<frame rate> in video frames/sec - RFC4566
   unsigned int   mQuality;               // a=quality:<quality> 0-10 for vidoe (0 is worst, 10 is best) - RFC4566

   SdpTcpSetupAttribute mTcpSetupAttribute;// a=setup:<setup attribute> - RFC4145
   SdpTcpConnectionAttribute mTcpConnectionAttribute; // a=connection:<conn attribute> - RFC4145

   UtlCopyableSList mCryptos;               // a=crypto:<tag> <crypto-suite> <key method>:<keyvalud> [<session-params>] - RFC4568

   SdpFingerPrintHashFuncType mFingerPrintHashFunction; // a=fingerprint:<hash func> <fingerprint> - RFC4572
   UtlString      mFingerPrint;

   SdpKeyManagementProtocolType mKeyManagementProtocol; // a=key-mgmt:<protocol id> <key mgmt data> - RFC4567   
   UtlString      mKeyManagementData;

   UtlCopyableSList mPreConditionCurrentStatus; // a=curr:<pre cond type> <status type> <direction tag> - RFC3312
   UtlCopyableSList mPreConditionConfirmStatus; // a=conf:<pre cond type> <status type> <direction tag> - RFC3312 - are multiple allowed?
   UtlCopyableSList mPreConditionDesiredStatus; // a=des:<pre cond type> <strength tag> <status type> <direction tag> - RFC3312

   double         mMaximumPacketRate;     // a=maxprate:<packetrate> in packets/s - RFC3890
   UtlString      mLabel;                 // a=label:<label> - RFC4574
   UtlString      mIdentificationTag;     // a=mid:<id tag> - RFC3388

   // Ice settings
   UtlString      mIceUserFrag;           // a=ice-ufrag:<ufrag> (min 4 characters) - draft-ietf-mmusic-ice-12
   UtlString      mIcePassword;           // a=ice-pwd:<password> (min 22 characters) - draft-ietf-mmusic-ice-12
   UtlCopyableSList mRemoteCandidates;    // a=remote-candidates:<component id> <connection address> <port> ... - draft-ietf-mmusic-ice-12
   UtlCopyableSortedList mCandidates;     // a=candidate:<foundation> <component id> <transport> <qvalue> <connection address> 
                                          //             <port> [<candidate type>] [<relay addr>] [<relay port>] 
                                          //             [<ext attrib name> <ext attrib value>] - draft-ietf-mmusic-ice-12
   bool           mRtpCandidatePresent;  
   bool           mRtcpCandidatePresent;
   UtlCopyableSortedList mCandidatePairs;       
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SdpMediaLine_h_
