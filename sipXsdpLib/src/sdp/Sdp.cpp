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
#include <os/OsIntTypes.h>
#include <sdp/Sdp.h>
#include <sdp/SdpMediaLine.h>
#include <utl/UtlHashMapIterator.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const char* Sdp::SdpNetTypeString[] = 
{
    // WARNING: this array MUST stay in synch with the SdpNetType enum
   "NONE",
   "IN"
};

const char* Sdp::SdpAddressTypeString[] =
{
    // WARNING: this array MUST stay in sync. with the SdpAddressType enum
   "NONE",
   "IP4",
   "IP6"
};

const char* Sdp::SdpBandwidthTypeString[] =
{
   "NONE",
   "CT",
   "AS",
   "TIAS",
   "RS",
   "RR"
};

const char* Sdp::SdpConferenceTypeString[] = 
{
   "NONE",
   "BROADCAST",
   "MODERATED",
   "TEST",
   "H332"
};

const char* Sdp::SdpGroupSemanticsString[] =
{
   "NONE",
   "LS",
   "FID",
   "SRF",
   "ANAT"
};


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
Sdp::Sdp()
{
   mSdpVersion = 1;
   mOriginatorSessionId = 0;
   mOriginatorSessionVersion = 0;
   mOriginatorNetType = NET_TYPE_NONE;
   mOriginatorAddressType = ADDRESS_TYPE_NONE;
   mConferenceType = CONFERENCE_TYPE_NONE;
   mIcePassiveOnlyMode = false;
   mMaximumPacketRate = 0;
}

// Copy constructor
Sdp::Sdp(const Sdp& rhs)
{
   operator=(rhs); 
}

// Destructor
Sdp::~Sdp()
{
   mFoundationIds.destroyAll();
}

// Assignment operator
Sdp& Sdp::operator=(const Sdp& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   // Assign values
   mSdpVersion = rhs.mSdpVersion;
   mOriginatorUserName = rhs.mOriginatorUserName;
   mOriginatorSessionId = rhs.mOriginatorSessionId;
   mOriginatorSessionVersion = rhs.mOriginatorSessionVersion;
   mOriginatorNetType = rhs.mOriginatorNetType;
   mOriginatorAddressType = rhs.mOriginatorAddressType;
   mOriginatorUnicastAddress = rhs.mOriginatorUnicastAddress;
   mSessionName = rhs.mSessionName;
   mSessionInformation = rhs.mSessionInformation;
   mSessionUri = rhs.mSessionUri;
   mEmailAddresses = rhs.mEmailAddresses;
   mPhoneNumbers = rhs.mPhoneNumbers;
   mBandwidths = rhs.mBandwidths;
   mTimes = rhs.mTimes;
   mTimeZones = rhs.mTimeZones;
   mCategory = rhs.mCategory;
   mKeywords = rhs.mKeywords;
   mToolNameAndVersion = rhs.mToolNameAndVersion;
   mConferenceType = rhs.mConferenceType;
   mCharSet = rhs.mCharSet;
   mIcePassiveOnlyMode = rhs.mIcePassiveOnlyMode;
   mGroups = rhs.mGroups;
   mSessionLanguage = rhs.mSessionLanguage;
   mDescriptionLanguage = rhs.mDescriptionLanguage;
   mMaximumPacketRate = rhs.mMaximumPacketRate;
   mMediaLines = rhs.mMediaLines;

   // Copy FoundationIds over
   mFoundationIds.destroyAll();
   UtlHashMapIterator it(rhs.mFoundationIds);
   SdpFoundation* item;
   UtlString* value;
   while((item = static_cast<SdpFoundation*>(it())) && (value = static_cast<UtlString*>(it.value())))
   {
      mFoundationIds.insertKeyAndValue(item->clone(), value->clone() );
   }

   return *this;
}

Sdp::SdpAddressType Sdp::getAddressTypeFromString(const char * type)
{
   UtlString stringType(type);

   if(stringType.compareTo("IP4", UtlString::ignoreCase) == 0)
   {
      return ADDRESS_TYPE_IP4;
   }
   else if(stringType.compareTo("IP6", UtlString::ignoreCase) == 0)
   {
      return ADDRESS_TYPE_IP6;
   }
   else
   {
      return ADDRESS_TYPE_NONE;
   }
}

Sdp::SdpBandwidthType Sdp::SdpBandwidth::getTypeFromString(const char * type)
{
   UtlString stringType(type);

   if(stringType.compareTo("CT", UtlString::ignoreCase) == 0)
   {
      return BANDWIDTH_TYPE_CT;
   }
   else if(stringType.compareTo("AS", UtlString::ignoreCase) == 0)
   {
      return BANDWIDTH_TYPE_AS;
   }
   else if(stringType.compareTo("TIAS", UtlString::ignoreCase) == 0)
   {
      return BANDWIDTH_TYPE_TIAS;
   }
   else if(stringType.compareTo("RS", UtlString::ignoreCase) == 0)
   {
      return BANDWIDTH_TYPE_RS;
   }
   else if(stringType.compareTo("RR", UtlString::ignoreCase) == 0)
   {
      return BANDWIDTH_TYPE_RR;
   }
   else
   {
      return BANDWIDTH_TYPE_NONE;
   }
}

Sdp::SdpConferenceType Sdp::getConferenceTypeFromString(const char * type)
{
   UtlString stringType(type);

   if(stringType.compareTo("broadcast", UtlString::ignoreCase) == 0)
   {
      return CONFERENCE_TYPE_BROADCAST;
   }
   else if(stringType.compareTo("moderated", UtlString::ignoreCase) == 0)
   {
      return CONFERENCE_TYPE_MODERATED;
   }
   else if(stringType.compareTo("test", UtlString::ignoreCase) == 0)
   {
      return CONFERENCE_TYPE_TEST;
   }
   else if(stringType.compareTo("H332", UtlString::ignoreCase) == 0)
   {
      return CONFERENCE_TYPE_H332;
   }
   else
   {
      return CONFERENCE_TYPE_NONE;
   }
}

Sdp::SdpGroupSemantics Sdp::SdpGroup::getSemanticsFromString(const char * type)
{
   UtlString stringType(type);

   if(stringType.compareTo("LS", UtlString::ignoreCase) == 0)
   {
      return GROUP_SEMANTICS_LS;
   }
   else if(stringType.compareTo("FID", UtlString::ignoreCase) == 0)
   {
      return GROUP_SEMANTICS_FID;
   }
   else if(stringType.compareTo("SRF", UtlString::ignoreCase) == 0)
   {
      return GROUP_SEMANTICS_SRF;
   }
   else if(stringType.compareTo("ANAT", UtlString::ignoreCase) == 0)
   {
      return GROUP_SEMANTICS_ANAT;
   }
   else
   {
      return GROUP_SEMANTICS_NONE;
   }
}

int Sdp::SdpFoundation::compareTo(UtlContainable const *rhs) const
{
   int result = 0;

   const SdpFoundation* pFoundation = (const SdpFoundation*)(rhs);
   if (pFoundation)
   {
      if(mCandidateType == pFoundation->mCandidateType)
      {
         result = mBaseAddress.compareTo(pFoundation->mBaseAddress);
         if(0 == result)
         {
            result = mStunAddress.compareTo(pFoundation->mStunAddress);
         }         
      }
      else if(mCandidateType < pFoundation->mCandidateType)
      {
         result = -1;
      }
      else
      {
         result = 1;
      }
   }

   return result;
}


/* ============================ MANIPULATORS ============================== */

void Sdp::setOriginatorInfo(const char* userName, 
                            uint64_t sessionId, 
                            uint64_t sessionVersion, 
                            SdpNetType netType, 
                            SdpAddressType addressType, 
                            const char* unicastAddress)
{
   mOriginatorUserName = userName;
   mOriginatorSessionId = sessionId;
   mOriginatorSessionVersion = sessionVersion;
   mOriginatorNetType = netType;
   mOriginatorAddressType = addressType;
   mOriginatorUnicastAddress = unicastAddress;
}

void Sdp::addMediaLine(SdpMediaLine* mediaLine) 
{ 
   mMediaLines.insert(mediaLine); 
}

void Sdp::clearMediaLines() 
{ 
   mMediaLines.destroyAll(); 
}

/* ============================ ACCESSORS ================================= */

void Sdp::toString(UtlString& sdpString) const
{
   char stringBuffer[2048];
   UtlString emailAddressesString;
   UtlString phoneNumbersString;
   UtlString bandwidthsString;
   UtlString timesString;
   UtlString timeZonesString;
   UtlString groupsString;
   UtlString mediaLinesString;

   // Build Email Address String
   {
      UtlSListIterator it(mEmailAddresses);
      UtlString* tempString;
      while((tempString = (UtlString*) it()))
      {
         emailAddressesString += UtlString("EmailAddress: '") + tempString->data() + UtlString("'\n");
      }
   }

   // Build Phone Number String
   {
      UtlSListIterator it(mPhoneNumbers);
      UtlString* tempString;
      while((tempString = (UtlString*) it()))
      {
         phoneNumbersString += UtlString("PhoneNumber: '") + tempString->data() + UtlString("'\n");
      }
   }

   // Build Bandwidths String
   {
      UtlSListIterator it(mBandwidths);
      SdpBandwidth* sdpBandwidth;
      while((sdpBandwidth = (SdpBandwidth*) it()))
      {
         sprintf(stringBuffer, "Bandwidth: type=%s, bandwidth=%d\n", SdpBandwidthTypeString[sdpBandwidth->getType()], sdpBandwidth->getBandwidth());
         bandwidthsString += stringBuffer;
      }
   }

   // Build Times String
   {
      UtlSListIterator it(mTimes);
      SdpTime* sdpTime;
      while((sdpTime = (SdpTime*) it()))
      {
         sprintf(stringBuffer, "Time: start=%" FORMAT_INTLL "u, stop=%" FORMAT_INTLL "u\n", sdpTime->getStartTime(), sdpTime->getStopTime());
         timesString += stringBuffer;

         // Add repeats
         const UtlSList& repeats = sdpTime->getRepeats();
         UtlSListIterator it2(repeats);
         SdpTime::SdpTimeRepeat *timeRepeat;
         while((timeRepeat = (SdpTime::SdpTimeRepeat*) it2()))
         {
            sprintf(stringBuffer, "TimeRepeat: interval=%d, duration=%d", timeRepeat->getRepeatInterval(), timeRepeat->getActiveDuration());
            timesString += stringBuffer;
            const UtlSList& offsetsFromStartTime = timeRepeat->getOffsetsFromStartTime();
            UtlSListIterator it3(offsetsFromStartTime);
            UtlInt *offset;
            while((offset = (UtlInt*)it3()))
            {
               sprintf(stringBuffer, ", offset=%d", offset->getValue());
               timesString += stringBuffer;
            }
            timesString += UtlString("\n");
         }
      }
   }

   // Build TimeZones String
   {
      UtlSListIterator it(mTimeZones);
      SdpTimeZone* sdpTimeZone;
      while((sdpTimeZone = (SdpTimeZone*) it()))
      {
         sprintf(stringBuffer, "TimeZone: adjustment time=%" FORMAT_INTLL "u, offset=%" FORMAT_INTLL "u\n", sdpTimeZone->getAdjustmentTime(), sdpTimeZone->getOffset());
         timeZonesString += stringBuffer;
      }
   }

   // Build Groups String
   {
      UtlSListIterator it(mGroups);
      SdpGroup* sdpGroup;
      while((sdpGroup = (SdpGroup*) it()))
      {
         sprintf(stringBuffer, "Group: semantics=%s", SdpGroupSemanticsString[sdpGroup->getSemantics()]);
         groupsString += stringBuffer;

         // Get id tags
         const UtlSList& idTags = sdpGroup->getIdentificationTags();
         UtlSListIterator it2(idTags);
         UtlString* tempString;
         while((tempString = (UtlString*) it2()))
         {
            groupsString += UtlString(", idTag=") + tempString->data();
         }
         groupsString += UtlString("\n");
      }
   }

   sprintf(stringBuffer,"Sdp:\n"
      "SdpVersion: %d\n"
      "OrigUserName: \'%s'\n"
      "OrigSessionId: %" FORMAT_INTLL "u\n"
      "OrigSessionVersion: %" FORMAT_INTLL "u\n"
      "OrigNetType: %s\n"
      "OrigAddressType: %s\n"
      "OrigUnicastAddr: \'%s\'\n"
      "SessionName: \'%s\'\n"
      "SessionInformation: \'%s\'\n"
      "SessionUri: \'%s\'\n"
      "%s"
      "%s"
      "%s"
      "%s"
      "%s"
      "Category: \'%s\'\n"
      "Keywords: \'%s\'\n"
      "ToolNameAndVersion: \'%s\'\n"
      "ConferenceType: %s\n"
      "CharSet: \'%s\'\n"
      "IcePassiveOnlyMode: %d\n"
      "%s"
      "SessionLanguage: \'%s\'\n"
      "DescriptionLanguage: \'%s\'\n"
      "MaximumPacketRate: %lf\n",
    mSdpVersion, 
    mOriginatorUserName.data(), 
    mOriginatorSessionId, 
    mOriginatorSessionVersion,
    SdpNetTypeString[mOriginatorNetType],
    SdpAddressTypeString[mOriginatorAddressType],
    mOriginatorUnicastAddress.data(),
    mSessionName.data(),
    mSessionInformation.data(),
    mSessionUri.data(),
    emailAddressesString.data(),
    phoneNumbersString.data(),
    bandwidthsString.data(),
    timesString.data(),
    timeZonesString.data(),
    mCategory.data(),
    mKeywords.data(),
    mToolNameAndVersion.data(),
    SdpConferenceTypeString[mConferenceType],
    mCharSet.data(),
    mIcePassiveOnlyMode ? 1 : 0,
    groupsString.data(),
    mSessionLanguage.data(),
    mDescriptionLanguage.data(),
    mMaximumPacketRate);

   sdpString = stringBuffer;

   // Add MediaLines
   {
      UtlSListIterator it(mMediaLines);
      SdpMediaLine* sdpMediaLine;
      while((sdpMediaLine = (SdpMediaLine*) it()))
      {
         sdpMediaLine->toString(mediaLinesString);
         sdpString += UtlString("\n") + mediaLinesString;
      }
   }
}

/* ============================ INQUIRY =================================== */

const UtlCopyableSList& Sdp::getMediaLines() const 
{ 
   return mMediaLines; 
}

UtlString Sdp::getLocalFoundationId(SdpCandidate::SdpCandidateType candidateType, 
                                    const char * baseAddress, 
                                    const char * stunAddress)
{
   SdpFoundation* sdpFoundation = new SdpFoundation(candidateType, baseAddress, stunAddress);
   UtlContainable* containable = mFoundationIds.findValue(sdpFoundation);
   if(containable)
   {
      delete sdpFoundation;
      return *((UtlString*)containable);
   }
   else
   {
      char foundationId[15];
      sprintf(foundationId, "%d", (int)mFoundationIds.entries() + 1);
      UtlString* ret = new UtlString(foundationId);
      mFoundationIds.insertKeyAndValue(sdpFoundation, ret);
      return *ret;
   }
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
