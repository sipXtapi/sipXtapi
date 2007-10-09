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

#ifndef _Sdp_h_
#define _Sdp_h_

// SYSTEM INCLUDES
//#include <...>
#include <iostream>

// APPLICATION INCLUDES
#include <utl/UtlInt.h>
#include <utl/UtlString.h>
#include <utl/UtlVoidPtr.h>
#include <utl/UtlCopyableSList.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlHashMap.h>

#include "SdpCandidate.h"

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

class SdpMediaLine;

class Sdp
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   typedef enum 
   {
      NET_TYPE_NONE,
      NET_TYPE_IN             // "IN" - Internet - RFC4566
   } SdpNetType;
   static const char* SdpNetTypeString[];

   typedef enum 
   {
      ADDRESS_TYPE_NONE,
      ADDRESS_TYPE_IP4,       // "IP4" - RFC4566
      ADDRESS_TYPE_IP6        // "IP6" - RFC4566
   } SdpAddressType;
   static const char* SdpAddressTypeString[];

   typedef enum 
   {
      BANDWIDTH_TYPE_NONE,
      BANDWIDTH_TYPE_CT,      // "CT" - Conference Total - RFC4566
      BANDWIDTH_TYPE_AS,      // "AS" - Application Specific - RFC4566
      BANDWIDTH_TYPE_TIAS,    // "TIAS" - Transport Independent Application Specific - RFC3890,
      BANDWIDTH_TYPE_RS,      // "RS" - RTCP bandwidth on active senders - RFC3556
      BANDWIDTH_TYPE_RR       // "RR" - RTCP bandwidth allocated to other participants - RFC3556
   } SdpBandwidthType;
   static const char* SdpBandwidthTypeString[];

   class SdpBandwidth : public UtlCopyableContainable
   {
   public:
      SdpBandwidth(SdpBandwidthType type, unsigned int bandwidth) : mType(type), mBandwidth(bandwidth) {}
      SdpBandwidth(const SdpBandwidth& rhs) : mType(rhs.mType), mBandwidth(rhs.mBandwidth) {}

      // Containable requirements
      UtlContainableType getContainableType() const { static char type[] = "SdpBandwidth"; return type;}
      unsigned hash() const { return directHash(); }
      int compareTo(UtlContainable const *) const { return 0; } // not implemented
      UtlCopyableContainable* clone() const { return new SdpBandwidth(*this); }

      // Accessors
      void setType(SdpBandwidthType type) { mType = type; }
      SdpBandwidthType getType() const { return mType; }
      static SdpBandwidthType getTypeFromString(const char * type);

      void setBandwidth(unsigned int bandwidth) { mBandwidth = bandwidth; }
      unsigned int getBandwidth() const { return mBandwidth; }

   private:
      SdpBandwidthType  mType;
      unsigned int      mBandwidth;
   };

   class SdpTime : public UtlCopyableContainable
   {
   public:
      class SdpTimeRepeat : public UtlCopyableContainable
      {
      public:
         SdpTimeRepeat(unsigned int repeatInterval, unsigned int activeDuration) :
            mRepeatInterval(repeatInterval), mActiveDuration(activeDuration) {}
          SdpTimeRepeat(const SdpTimeRepeat& rhs) :
            mRepeatInterval(rhs.mRepeatInterval), mActiveDuration(rhs.mActiveDuration), mOffsetsFromStartTime(rhs.mOffsetsFromStartTime) {}

         // Containable requirements
         UtlContainableType getContainableType() const { static char type[] = "SdpTimeRepeat"; return type;}
         unsigned hash() const { return directHash(); }
         int compareTo(UtlContainable const *) const { return 0; } // not implemented
         UtlCopyableContainable* clone() const { return new SdpTimeRepeat(*this); }

         // Accessors
         void setRepeatInterval(unsigned int repeatInterval) { mRepeatInterval = repeatInterval; }
         unsigned int getRepeatInterval() const { return mRepeatInterval; }

         void setActiveDuration(unsigned int activeDuration) { mActiveDuration = activeDuration; }
         unsigned int getActiveDuration() const { return mActiveDuration; }

         void addOffsetFromStartTime(unsigned int offset) { mOffsetsFromStartTime.insert(new UtlInt(offset)); }
         void clearOffsetsFromStartTime() { mOffsetsFromStartTime.destroyAll(); }
         const UtlCopyableSList& getOffsetsFromStartTime() const { return mOffsetsFromStartTime; }

      private:
         unsigned int      mRepeatInterval;
         unsigned int      mActiveDuration;
         UtlCopyableSList  mOffsetsFromStartTime;
      };

      SdpTime(uint64_t startTime, uint64_t stopTime) : mStartTime(startTime), mStopTime(stopTime) {}
      SdpTime(const SdpTime& rhs) : mStartTime(rhs.mStartTime), mStopTime(rhs.mStopTime), mRepeats(rhs.mRepeats) {}

      // Containable requirements
      UtlContainableType getContainableType() const { static char type[] = "SdpTime"; return type;}
      unsigned hash() const { return directHash(); }
      int compareTo(UtlContainable const *) const { return 0; } // not implemented
      UtlCopyableContainable* clone() const { return new SdpTime(*this); }

      // Accessors
      void setStartTime(uint64_t startTime) { mStartTime = startTime; }
      uint64_t getStartTime() const { return mStartTime; }

      void setStopTime(uint64_t stopTime) { mStopTime = stopTime; }
      uint64_t getStopTime() const { return mStopTime; }

      void addRepeat(SdpTimeRepeat* sdpTimeRepeat) { mRepeats.insert(sdpTimeRepeat); }
      void clearRepeats() { mRepeats.destroyAll(); }
      const UtlCopyableSList& getRepeats() const { return mRepeats; }

   private:
      uint64_t     mStartTime;
      uint64_t     mStopTime;
      UtlCopyableSList   mRepeats;       
   };

   class SdpTimeZone : public UtlCopyableContainable
   {
   public:
      SdpTimeZone(int adjustmentTime, int offset) : mAdjustmentTime(adjustmentTime), mOffset(offset) {}
      SdpTimeZone(const SdpTimeZone& rhs) : mAdjustmentTime(rhs.mAdjustmentTime), mOffset(rhs.mOffset) {}

      // Containable requirements
      UtlContainableType getContainableType() const { static char type[] = "SdpTimeZone"; return type;}
      unsigned hash() const { return directHash(); }
      int compareTo(UtlContainable const *) const { return 0; } // not implemented
      UtlCopyableContainable* clone() const { return new SdpTimeZone(*this); }

      // Accessors
      void setAdjustmentTime(int adjustmentTime) { mAdjustmentTime = adjustmentTime; }
      uint64_t getAdjustmentTime() const { return mAdjustmentTime; }

      void setOffset(int offset) { mOffset = offset; }
      uint64_t getOffset() const { return mOffset; }

   private:
      int         mAdjustmentTime;
      int         mOffset;
   };

   typedef enum 
   {
      CONFERENCE_TYPE_NONE,
      CONFERENCE_TYPE_BROADCAST, // "broadcast" - RFC4566
      CONFERENCE_TYPE_MODERATED, // "moderated" - RFC4566
      CONFERENCE_TYPE_TEST,      // "test" - RFC4566
      CONFERENCE_TYPE_H332       // "H332" - RFC4566
   } SdpConferenceType;
   static const char* SdpConferenceTypeString[];

   typedef enum 
   {
      GROUP_SEMANTICS_NONE,
      GROUP_SEMANTICS_LS,        // "LS" - Lip Sync - RFC3388
      GROUP_SEMANTICS_FID,       // "FID" - Flow Identifier - RFC3388
      GROUP_SEMANTICS_SRF,       // "SRF" - Single Reservation Flow - RFC3524
      GROUP_SEMANTICS_ANAT       // "ANAT" - Alternative Network Address Types - RFC4091
   } SdpGroupSemantics;
   static const char* SdpGroupSemanticsString[];

   class SdpGroup : public UtlCopyableContainable
   {
   public:
      SdpGroup(SdpGroupSemantics semantics) : mSemantics(semantics) {}
      SdpGroup(const SdpGroup& rhs) : mSemantics(rhs.mSemantics), mIdentificationTags(rhs.mIdentificationTags) {}

      // Containable requirements
      UtlContainableType getContainableType() const { static char type[] = "SdpGroup"; return type;}
      unsigned hash() const { return directHash(); }
      int compareTo(UtlContainable const *) const { return 0; } // not implemented
      UtlCopyableContainable* clone() const { return new SdpGroup(*this); }

      // Accessors
      void setSemantics(SdpGroupSemantics semantics) { mSemantics = semantics; }
      SdpGroupSemantics getSemantics() const { return mSemantics; }
      static SdpGroupSemantics getSemanticsFromString(const char * type);

      void addIdentificationTag(const char * identificationTag) { mIdentificationTags.insert(new UtlString(identificationTag)); }
      void clearIdentificationTags() { mIdentificationTags.destroyAll(); }
      const UtlCopyableSList& getIdentificationTags() const { return mIdentificationTags; }
      
   private:
      SdpGroupSemantics mSemantics;
      UtlCopyableSList          mIdentificationTags;
   };

   class SdpFoundation : public UtlCopyableContainable
   {
   public:
      SdpFoundation(SdpCandidate::SdpCandidateType candidateType, const char * baseAddress, const char * stunAddress) : 
         mCandidateType(candidateType), mBaseAddress(baseAddress), mStunAddress(stunAddress) {}
      SdpFoundation(const SdpFoundation& rhs) :
         mCandidateType(rhs.mCandidateType), mBaseAddress(rhs.mBaseAddress), mStunAddress(rhs.mStunAddress) {}

      // Containable requirements
      UtlContainableType getContainableType() const { static char type[] = "SdpFoundationId"; return type;}
      unsigned hash() const { return mBaseAddress.hash() ^ mStunAddress.hash() ^ mCandidateType; }
      int compareTo(UtlContainable const *) const;
      UtlCopyableContainable* clone() const { return new SdpFoundation(*this); }

   private:
      SdpCandidate::SdpCandidateType mCandidateType;
      UtlString                      mBaseAddress;
      UtlString                      mStunAddress;
   };

/* ============================ CREATORS ================================== */

   Sdp();
     //:Default constructor

   Sdp(const Sdp& rSdp);
     //:Copy constructor

   virtual ~Sdp();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   Sdp& operator=(const Sdp& rhs);
     //:Assignment operator

   void setSdpVersion(unsigned int sdpVersion) { mSdpVersion = sdpVersion; }

   void setOriginatorInfo(const char* userName, uint64_t sessionId, uint64_t sessionVersion, SdpNetType netType, SdpAddressType addressType, const char* unicastAddress);
   void setOriginatorUserName(const char* originatorUserName) { mOriginatorUserName = originatorUserName; }
   void setOriginatorSessionId(uint64_t originatorSessionId) { mOriginatorSessionId = originatorSessionId; }
   void setOriginatorSessionVersion(uint64_t originatorSessionVersion) { mOriginatorSessionVersion = originatorSessionVersion; }
   void setOriginatorNetType(SdpNetType originatorNetType) { mOriginatorNetType = originatorNetType; }
   void setOriginatorAddressType(SdpAddressType originatorAddressType) { mOriginatorAddressType = originatorAddressType; }
   void setOriginatorUnicastAddress(const char* originatorUnicastAddress) { mOriginatorUnicastAddress = originatorUnicastAddress; }

   void setSessionName(const char * sessionName) { mSessionName = sessionName; }
   void setSessionInformation(const char * sessionInformation) { mSessionInformation = sessionInformation; }
   void setSessionUri(const char * sessionUri) { mSessionUri = sessionUri; }

   void addEmailAddress(const char * emailAddress) { mEmailAddresses.insert(new UtlString(emailAddress)); }
   void clearEmailAddresses() { mEmailAddresses.destroyAll(); }

   void addPhoneNumber(const char * phoneNumber) { mPhoneNumbers.insert(new UtlString(phoneNumber)); }
   void clearPhoneNumbers() { mPhoneNumbers.destroyAll(); }

   void addBandwidth(SdpBandwidthType type, unsigned int bandwidth) { addBandwidth(new SdpBandwidth(type, bandwidth)); }
   void addBandwidth(SdpBandwidth* sdpBandwidth) { mBandwidths.insert(sdpBandwidth); }
   void clearBandwidths() { mBandwidths.destroyAll(); }

   void addTime(uint64_t startTime, uint64_t stopTime) { addTime(new SdpTime(startTime, stopTime)); }
   void addTime(SdpTime* time) { mTimes.insert(time); }
   void clearTimes() { mTimes.destroyAll(); }

   void addTimeZone(int adjustmentTime, int offset) { addTimeZone(new SdpTimeZone(adjustmentTime, offset)); }
   void addTimeZone(SdpTimeZone* timeZone) { mTimeZones.insert(timeZone); }
   void clearTimeZones() { mTimeZones.destroyAll(); }

   void setCategory(const char * category) { mCategory = category; }
   void setKeywords(const char * keywords) { mKeywords = keywords; }
   void setToolNameAndVersion(const char * toolNameAndVersion) { mToolNameAndVersion = toolNameAndVersion; }
   void setConferenceType(SdpConferenceType conferenceType) { mConferenceType = conferenceType; }
   void setCharSet(const char * charSet) { mCharSet = charSet; }
   void setIcePassiveOnlyMode(bool icePassiveOnlyMode) { mIcePassiveOnlyMode = icePassiveOnlyMode; }

   void addGroup(SdpGroup* group) { mGroups.insert(group); }
   void clearGroups() { mGroups.destroyAll(); }

   void setSessionLanguage(const char * sessionLanguage) { mSessionLanguage = sessionLanguage; }
   void setDescriptionLanguage(const char * descriptionLanguage) { mDescriptionLanguage = descriptionLanguage; }
   void setMaximumPacketRate(double maximumPacketRate) { mMaximumPacketRate = maximumPacketRate; }

   void addMediaLine(SdpMediaLine* mediaLine);
   void clearMediaLines();
   
/* ============================ ACCESSORS ================================= */

   void toString(UtlString& sdpString) const;

/* ============================ INQUIRY =================================== */

   static SdpAddressType getAddressTypeFromString(const char * type);

   unsigned int getSdpVersion() const { return mSdpVersion; }

   const UtlString& getOriginatorUserName() const { return mOriginatorUserName; }
   uint64_t getOriginatorSessionId() const { return mOriginatorSessionId; }
   uint64_t getOriginatorSessionVersion() const { return mOriginatorSessionVersion; }
   SdpNetType getOriginatorNetType() const { return mOriginatorNetType; }
   SdpAddressType getOriginatorAddressType() const { return mOriginatorAddressType; }
   const UtlString& getOriginatorUnicastAddress() const { return mOriginatorUnicastAddress; }

   const UtlString& getSessionName() const { return mSessionName; }
   const UtlString& getSessionInformation() const { return mSessionInformation; }
   const UtlString& getSessionUri() const { return mSessionUri; }

   const UtlCopyableSList& getEmailAddresses() const { return mEmailAddresses; }
   const UtlCopyableSList& getPhoneNumbers() const { return mPhoneNumbers; }
   const UtlCopyableSList& getBandwidths() const { return mBandwidths; }
   const UtlCopyableSList& getTimes() const { return mTimes; }
   const UtlCopyableSList& getTimeZones() const { return mTimeZones; }

   const UtlString& getCategory() const { return mCategory; }
   const UtlString& getKeywords() const { return mKeywords; }
   const UtlString& getToolNameAndVersion() const { return mToolNameAndVersion; }
   SdpConferenceType getConferenceType() const { return mConferenceType; }
   static SdpConferenceType getConferenceTypeFromString(const char * type);
   const UtlString& getCharSet() const { return mCharSet; }
   bool isIcePassiveOnlyMode() const { return mIcePassiveOnlyMode; }

   const UtlCopyableSList& getGroups() const { return mGroups; }

   const UtlString& getSessionLanguage() const { return mSessionLanguage; }
   const UtlString& getDescriptionLanguage() const { return mDescriptionLanguage; }
   double getMaximumPacketRate() const { return mMaximumPacketRate; }

   const UtlCopyableSList& getMediaLines() const;

   UtlString getLocalFoundationId(SdpCandidate::SdpCandidateType candidateType, const char * baseAddress, const char * stunAddress=0);


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   // v=
   unsigned int   mSdpVersion;

   // o=
   UtlString      mOriginatorUserName;
   uint64_t       mOriginatorSessionId;
   uint64_t       mOriginatorSessionVersion;
   SdpNetType     mOriginatorNetType;
   SdpAddressType mOriginatorAddressType;
   UtlString      mOriginatorUnicastAddress;

   // s=
   UtlString      mSessionName;

   // i=
   UtlString      mSessionInformation;

   // u=         
   UtlString      mSessionUri;

   // e=
   UtlCopyableSList mEmailAddresses;

   // p=
   UtlCopyableSList mPhoneNumbers;

   // c= is only stored in sdpMediaLine

   // b=
   UtlCopyableSList mBandwidths;

   // t=, r=
   UtlCopyableSList mTimes;

   // z=
   UtlCopyableSList mTimeZones;

   // k= is only stored in sdpMediaLine

   // a= session level only attributes 
   UtlString      mCategory;           // a=cat:<category> - RFC4566
   UtlString      mKeywords;           // a=keywds:<keywords> - RFC4566
   UtlString      mToolNameAndVersion; // a=tool:<name and version of tool> - RFC4566
   SdpConferenceType mConferenceType;  // a=type:<conference type> - RFC4566
   UtlString      mCharSet;            // a=charset:<character set> - RFC4566
   bool           mIcePassiveOnlyMode; // a=ice-passive - ietf-draft-mmusic-ice-12
   UtlCopyableSList mGroups;             // a=group:<semantics> <id-tag> ... - RFC3388

   // a= attributes that have meaning when not associated to a particular media line
   UtlString      mSessionLanguage;     // a=lang:<language tag> - RFC4566
   UtlString      mDescriptionLanguage; // a=sdplang:<language tag> - RFC4566
   double         mMaximumPacketRate;   // a=maxprate:<packetrate> in packets/s - RFC3890

   // Media Lines
   UtlCopyableSList       mMediaLines;

   // Foundation Id 
   UtlHashMap     mFoundationIds;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _Sdp_h_
