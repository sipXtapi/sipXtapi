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

// SYSTEM INCLUDES
#ifdef __pingtel_on_posix__ // [
#  include <limits.h>
#endif // __pingtel_on_posix__ ]

// APPLICATION INCLUDES
#include <sdp/SdpCandidatePair.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType SdpCandidatePair::TYPE = "SdpCandidatePair";

const char* SdpCandidatePair::SdpCandidatePairCheckStateString[] =
{
   "FROZEN",
   "WAITING",
   "INPROGRESS",
   "SUCCEEDED",
   "FAILED"
};

const char* SdpCandidatePair::SdpCandidatePairOffererTypeString[] =
{
   "LOCAL", 
   "REMOTE"
};

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SdpCandidatePair::SdpCandidatePair(const SdpCandidate& localCandidate,
                                   const SdpCandidate& remoteCandidate,
                                   SdpCandidatePairOffererType offerer)
: mLocalCandidate(localCandidate)
, mRemoteCandidate(remoteCandidate)
, mOfferer(offerer)
{
   resetPriority();
   mCheckState = CHECK_STATE_FROZEN;
}

// Copy constructor
SdpCandidatePair::SdpCandidatePair(const SdpCandidatePair& rhs)
{
   operator=(rhs); 
}

// Destructor
SdpCandidatePair::~SdpCandidatePair()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SdpCandidatePair&
SdpCandidatePair::operator=(const SdpCandidatePair& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   // Assign values
   mLocalCandidate = rhs.mLocalCandidate;
   mRemoteCandidate = rhs.mRemoteCandidate;
   mOfferer = rhs.mOfferer;
   mPriority = rhs.mPriority;
   mCheckState = rhs.mCheckState;

   return *this;
}

bool SdpCandidatePair::setCheckState(SdpCandidatePairCheckState checkState)
{
   bool stateChangeSuccess=false;
   switch(mCheckState)
   {
   case CHECK_STATE_FROZEN:
      switch(checkState)
      {
      case CHECK_STATE_WAITING:
      case CHECK_STATE_INPROGRESS:
         mCheckState = checkState;
         stateChangeSuccess = true;
         break;
      default:
         assert(false);
      }
      break;
   case CHECK_STATE_WAITING:
      switch(checkState)
      {
      case CHECK_STATE_INPROGRESS:
         mCheckState = checkState;
         stateChangeSuccess = true;
         break;
      default:
         assert(false);
      }
      break;
   case CHECK_STATE_INPROGRESS:
      switch(checkState)
      {
      case CHECK_STATE_SUCCEEDED:
      case CHECK_STATE_FAILED:
         mCheckState = checkState;
         stateChangeSuccess = true;
         break;
      default:
         assert(false);
      }
      break;
   case CHECK_STATE_SUCCEEDED:
   case CHECK_STATE_FAILED:
   default:
      assert(false);
      break;
   }
   return stateChangeSuccess;
}

/* ============================ ACCESSORS ================================= */

UtlContainableType SdpCandidatePair::getContainableType() const 
{ 
   return SdpCandidatePair::TYPE;
}

unsigned SdpCandidatePair::hash() const 
{
   // Priority is to be unique per candidate - draft-ieft-mmusic-ice-12
   return unsigned(mPriority) ^ unsigned(mPriority >> 32);
}

int SdpCandidatePair::compareTo(UtlContainable const *rhs) const 
{ 
   int result ; 

   const SdpCandidatePair* pCandidatePair = static_cast<const SdpCandidatePair*>(rhs);
   if (rhs->isInstanceOf(SdpCandidatePair::TYPE) && pCandidatePair)
   {
      // We want to order a list of these from highest priority to lowest
      result = compareNumber(mPriority, pCandidatePair->mPriority, true /* reverse */);
      if(0 == result) 
      {
         result = compareNumber(mCheckState, pCandidatePair->mCheckState);
      }
      if(0 == result)
      {
         result = mLocalCandidate.compareTo(&pCandidatePair->mLocalCandidate);
      }
      if(0 == result)
      {
         result = mRemoteCandidate.compareTo(&pCandidatePair->mRemoteCandidate);
      }
   }
   else
   {
      result = INT_MAX ; 
   }

   return result ;
}

void SdpCandidatePair::toString(UtlString& sdpCandidatePairString) const
{
   char stringBuffer[2048];

   UtlString localCandidateString;
   UtlString remoteCandidateString;

   mLocalCandidate.toString(localCandidateString);
   mRemoteCandidate.toString(remoteCandidateString);

   sprintf(stringBuffer, "SdpCandidatePair:\n"
      "Priority: %" FORMAT_INTLL "d\n"
      "State: %s\n"
      "Offerer: %s\n"
      "%s"
      "%s",
      mPriority,
      SdpCandidatePairCheckStateString[mCheckState],
      SdpCandidatePairOffererTypeString[mOfferer],
      localCandidateString.data(),
      remoteCandidateString.data());

   sdpCandidatePairString = stringBuffer;

}

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
void SdpCandidatePair::resetPriority()
{
   uint64_t offererPriority = mOfferer == OFFERER_LOCAL ? mLocalCandidate.getPriority() : mRemoteCandidate.getPriority();
   uint64_t answererPriority = mOfferer == OFFERER_LOCAL ? mRemoteCandidate.getPriority() : mLocalCandidate.getPriority();
   mPriority = (sipx_min(offererPriority, answererPriority) << 32) |
               (sipx_max(offererPriority, answererPriority) << 1) |
               (offererPriority > answererPriority ? 1 : 0);
}

int SdpCandidatePair::compareNumber(uint64_t first, uint64_t second, bool reverse) const
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
