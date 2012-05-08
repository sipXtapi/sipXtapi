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
#ifdef __pingtel_on_posix__ // [
#  include <limits.h>
#endif // __pingtel_on_posix__ ]

// APPLICATION INCLUDES
#include <utl/UtlSListIterator.h>
#include <sdp/SdpCandidate.h>
#include <os/OsSysLog.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType SdpCandidate::TYPE = "SdpCandidate";

const char* SdpCandidate::SdpCandidateTransportTypeString[] =
{
   "NONE",
   "UDP",
   "TCP-SO",
   "TCP-ACT",
   "TCP-PASS",
   "TLS-SO",
   "TLS-ACT",
   "TLS-PASS"
};

const char* SdpCandidate::SdpCandidateTypeString[] =
{
   "NONE",
   "HOST",
   "SRFLX",
   "PRFLX",
   "RELAY"
};

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SdpCandidate::SdpCandidate(const char * foundation,
                           unsigned int id,
                           SdpCandidateTransportType transport,
                           uint64_t priority,
                           const char * connectionAddress,
                           unsigned int port,
                           SdpCandidateType candidateType,
                           const char * relatedAddress,
                           unsigned int relatedPort, 
                           bool inUse) :
      mFoundation(foundation),
      mId(id),
      mTransport(transport),
      mPriority(priority),
      mConnectionAddress(connectionAddress),
      mPort(port),
      mCandidateType(candidateType),
      mRelatedAddress(relatedAddress),
      mRelatedPort(relatedPort),
      mInUse(inUse)
{
}

// Copy constructor
SdpCandidate::SdpCandidate(const SdpCandidate& rhs)
{
   operator=(rhs);  
}

// Destructor
SdpCandidate::~SdpCandidate()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SdpCandidate& SdpCandidate::operator=(const SdpCandidate& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   // Assign values
   mFoundation = rhs.mFoundation;
   mId = rhs.mId;
   mTransport = rhs.mTransport;
   mPriority = rhs.mPriority;
   mConnectionAddress = rhs.mConnectionAddress;
   mPort = rhs.mPort;
   mCandidateType = rhs.mCandidateType;
   mRelatedAddress = rhs.mRelatedAddress;
   mRelatedPort = rhs.mRelatedPort;
   mInUse = rhs.mInUse;
   mExtensionAttributes = rhs.mExtensionAttributes;

   return *this;
}

/* ============================ ACCESSORS ================================= */
UtlContainableType SdpCandidate::getContainableType() const 
{ 
   return SdpCandidate::TYPE;
}

unsigned SdpCandidate::hash() const 
{ 
   return unsigned(mPriority) ^ unsigned(mPriority << 32);  // Priority is to be unique per candidate - draft-ieft-mmusic-ice-12
}

int SdpCandidate::compareTo(UtlContainable const *rhs) const 
{ 
   int result ; 

   const SdpCandidate* pCandidate = static_cast<const SdpCandidate*>(rhs);
   if (rhs->isInstanceOf(SdpCandidate::TYPE) && pCandidate)
   {
      result = compareNumber(mPriority, pCandidate->mPriority, true /* reverse */); // We want to order a list of these from highest priority to lowest
      if(0 == result)
      {
         // Priority should be unique, so we shouldn't get here, but implementation is included for completeness
         result = mFoundation.compareTo(pCandidate->mFoundation);
      }
      if(0 == result) 
      {
         result = compareNumber(mId, pCandidate->mId);
      }
      if(0 == result)
      {
         result = compareNumber(mTransport, pCandidate->mTransport);
      }
      if(0 == result)
      {
         result = mConnectionAddress.compareTo(pCandidate->mConnectionAddress);
      }
      if(0 == result)
      {
         result = compareNumber(mPort, pCandidate->mPort);
      }
      if(0 == result)
      {
         result = compareNumber(mCandidateType, pCandidate->mCandidateType);
      }
      if(0 == result)
      {
         result = mRelatedAddress.compareTo(pCandidate->mRelatedAddress);
      }
      if(0 == result)
      {
         result = compareNumber(mRelatedPort, pCandidate->mRelatedPort);
      }
   }
   else
   {
      OsSysLog::add(FAC_SDP, PRI_ERR,
          "SdpCandidate::compareTo argument instance of: %s not instance of: %s",
          rhs->getContainableType(), SdpCandidate::TYPE);

      result = INT_MAX ; 
   }

   return result ;
}

void SdpCandidate::toString(UtlString& sdpCandidateString) const
{
   char stringBuffer[512];
   UtlString extensionAttributesString;

   // build extension attributes string
   UtlSListIterator it(mExtensionAttributes);
   SdpCandidateExtensionAttribute* attribute;
   while((attribute = (SdpCandidateExtensionAttribute*) it()))
   {
      extensionAttributesString += attribute->getName() + UtlString("=") + attribute->getValue() + UtlString(", ");
   }
   
   sprintf(stringBuffer, "SdpCandidate: foundation=\'%s\', id=%d, transport=%s, priority=%" FORMAT_INTLL "d, addr=\'%s\', port=%d, type=%s\n"
                         "              relatedAddr=%s, relatedPort=%d, %sinUse=%d\n",
      mFoundation.data(),
      mId,
      SdpCandidateTransportTypeString[mTransport],
      mPriority,
      mConnectionAddress.data(),
      mPort,
      SdpCandidateTypeString[mCandidateType],
      mRelatedAddress.data(),
      mRelatedPort,
      extensionAttributesString.data(),
      mInUse);

   sdpCandidateString = stringBuffer;

}

/* ============================ INQUIRY =================================== */

UtlBoolean SdpCandidate::isInstanceOf(const UtlContainableType type) const
{
    // Check if it is my type and then defer parent type comparisons to parent
    return(areSameTypes(type, SdpCandidate::TYPE) ||
           UtlCopyableContainable::isInstanceOf(type));
}

SdpCandidate::SdpCandidateTransportType 
SdpCandidate::getCandidateTransportTypeFromString(const char * type)
{
   UtlString stringType(type);

   if(stringType.compareTo("udp", UtlString::ignoreCase) == 0)
   {
      return CANDIDATE_TRANSPORT_TYPE_UDP;
   }
   else if(stringType.compareTo("tcp-so", UtlString::ignoreCase) == 0)
   {
      return CANDIDATE_TRANSPORT_TYPE_TCP_SO;
   }
   else if(stringType.compareTo("tcp-act", UtlString::ignoreCase) == 0)
   {
      return CANDIDATE_TRANSPORT_TYPE_TCP_ACT;
   }
   else if(stringType.compareTo("tcp-pass", UtlString::ignoreCase) == 0)
   {
      return CANDIDATE_TRANSPORT_TYPE_TCP_PASS;
   }
   else if(stringType.compareTo("tls-so", UtlString::ignoreCase) == 0)
   {
      return CANDIDATE_TRANSPORT_TYPE_TLS_SO;
   }
   else if(stringType.compareTo("tls-act", UtlString::ignoreCase) == 0)
   {
      return CANDIDATE_TRANSPORT_TYPE_TLS_ACT;
   }
   else if(stringType.compareTo("tls-pass", UtlString::ignoreCase) == 0)
   {
      return CANDIDATE_TRANSPORT_TYPE_TLS_PASS;
   }
   else
   {
      return CANDIDATE_TRANSPORT_TYPE_NONE;
   }
}

SdpCandidate::SdpCandidateType 
SdpCandidate::getCandidateTypeFromString(const char * type)
{
   UtlString stringType(type);

   if(stringType.compareTo("host", UtlString::ignoreCase) == 0)
   {
      return CANDIDATE_TYPE_HOST;
   }
   else if(stringType.compareTo("srflx", UtlString::ignoreCase) == 0)
   {
      return CANDIDATE_TYPE_SRFLX;
   }
   else if(stringType.compareTo("prflx", UtlString::ignoreCase) == 0)
   {
      return CANDIDATE_TYPE_PRFLX;
   }
   else if(stringType.compareTo("relay", UtlString::ignoreCase) == 0)
   {
      return CANDIDATE_TYPE_RELAY;
   }
   else
   {
      return CANDIDATE_TYPE_NONE;
   }
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
int SdpCandidate::compareNumber(uint64_t first, uint64_t second, bool reverse) const
{
    int ret;
    if (first == second)
    {
        ret = 0;
    }
    else if (first < second)
    {
       if(!reverse)
       {
          ret = -1;
       }
       else
       { 
          ret = 1;
       }
    }
    else
    {
       if(!reverse)
       {
          ret = 1;
       }
       else
       {
          ret = -1;
       }
    }
    return ret;
}

/* ============================ FUNCTIONS ================================= */
