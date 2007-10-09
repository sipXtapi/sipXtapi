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

#ifndef _SdpCandidatePair_h_
#define _SdpCandidatePair_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <utl/UtlInt.h>
#include <utl/UtlString.h>
#include <sdp/SdpCandidate.h>


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: Container for SdpCandidatePair specification
// This class holds the information related to an SdpCandidatePair.
// Included in this information is:  TODO
//

class SdpCandidatePair : public UtlCopyableContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   typedef enum
   {
      CHECK_STATE_FROZEN,
      CHECK_STATE_WAITING,
      CHECK_STATE_INPROGRESS,
      CHECK_STATE_SUCCEEDED,
      CHECK_STATE_FAILED
   } SdpCandidatePairCheckState;
   static const char* SdpCandidatePairCheckStateString[];

   typedef enum
   {
      OFFERER_LOCAL,
      OFFERER_REMOTE
   } SdpCandidatePairOffererType;
   static const char* SdpCandidatePairOffererTypeString[];

/* ============================ CREATORS ================================== */

   SdpCandidatePair(const SdpCandidate& localCandidate,
                    const SdpCandidate& remoteCandidate,
                    SdpCandidatePairOffererType offerer);
     //:Default constructor

   SdpCandidatePair(const SdpCandidatePair& rSdpCandidatePair);
     //:Copy constructor

   virtual
   ~SdpCandidatePair();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   SdpCandidatePair& operator=(const SdpCandidatePair& rhs);
     //:Assignment operator

   void setLocalCandidate(const SdpCandidate& localCandidate) { mLocalCandidate = localCandidate; resetPriority(); }
   void setRemoteCandidate(const SdpCandidate& remoteCandidate) { mRemoteCandidate = remoteCandidate; resetPriority(); }
   void setOfferer(SdpCandidatePairOffererType offerer) { mOfferer = offerer; resetPriority(); }
   bool setCheckState(SdpCandidatePairCheckState checkState);

/* ============================ ACCESSORS ================================= */

   // UtlContainable requirements
   virtual UtlContainableType getContainableType() const;
   virtual unsigned hash() const;
   virtual int compareTo(UtlContainable const *) const;
   static const UtlContainableType TYPE;    ///< Class type used for runtime checking 
   UtlCopyableContainable* clone() const { return new SdpCandidatePair(*this); }

   void toString(UtlString& sdpCandidateString) const;

/* ============================ INQUIRY =================================== */

   const SdpCandidate& getLocalCandidate() const { return mLocalCandidate; }
   const SdpCandidate& getRemoteCandidate() const { return mRemoteCandidate; }
   SdpCandidatePairOffererType getOfferer() const { return mOfferer; }
   uint64_t getPriority() const { return mPriority; }
   SdpCandidatePairCheckState getCheckState() const { return mCheckState; }

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   int compareNumber(uint64_t first, uint64_t second, bool reverse = false) const;
   void resetPriority();

   SdpCandidate mLocalCandidate;
   SdpCandidate mRemoteCandidate;
   SdpCandidatePairOffererType mOfferer;
   uint64_t     mPriority;
   SdpCandidatePairCheckState mCheckState;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SdpCandidatePair_h_
