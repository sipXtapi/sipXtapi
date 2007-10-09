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

#ifndef _SdpCandidate_h_
#define _SdpCandidate_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlInt.h"
#include "utl/UtlString.h"
#include "utl/UtlCopyableSList.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: Container for SdpCandidate specification
// This class holds the information related to an SdpCandidate.
// Included in this information is:  TODO
//

class SdpCandidate : public UtlCopyableContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   typedef enum
   {
      CANDIDATE_TRANSPORT_TYPE_NONE,
      CANDIDATE_TRANSPORT_TYPE_UDP,      // "udp"      - draft-ietf-mmusic-ice-12
      CANDIDATE_TRANSPORT_TYPE_TCP_SO,   // "tcp-so"   - TCP simultaneous-open - draft-ietf-mmusic-ice-tcp-02
      CANDIDATE_TRANSPORT_TYPE_TCP_ACT,  // "tcp-act"  - TCP active - draft-ietf-mmusic-ice-tcp-02
      CANDIDATE_TRANSPORT_TYPE_TCP_PASS, // "tcp-pass" - TCP passive - draft-ietf-mmusic-ice-tcp-02
      CANDIDATE_TRANSPORT_TYPE_TLS_SO,   // "tls-so"   - TCP simultaneous-open - draft-ietf-mmusic-ice-tcp-02
      CANDIDATE_TRANSPORT_TYPE_TLS_ACT,  // "tls-act"  - TCP active - draft-ietf-mmusic-ice-tcp-02
      CANDIDATE_TRANSPORT_TYPE_TLS_PASS  // "tls-pass" - TCP passive - draft-ietf-mmusic-ice-tcp-02
   } SdpCandidateTransportType;
   static const char* SdpCandidateTransportTypeString[];


   typedef enum
   {
      CANDIDATE_TYPE_NONE,
      CANDIDATE_TYPE_HOST,        // "host" - draft-ietf-mmusic-ice-12
      CANDIDATE_TYPE_SRFLX,       // "srflx" - server reflexive - draft-ietf-mmusic-ice-12
      CANDIDATE_TYPE_PRFLX,       // "prflx" - peer reflexive - draft-ietf-mmusic-ice-12
      CANDIDATE_TYPE_RELAY,       // "relay" - draft-ietf-mmusic-ice-12
   } SdpCandidateType;
   static const char* SdpCandidateTypeString[];

   class SdpCandidateExtensionAttribute : public UtlCopyableContainable
   {
   public:
      SdpCandidateExtensionAttribute(const char * name, const char * value) : mName(name), mValue(value) {}
      SdpCandidateExtensionAttribute(const SdpCandidateExtensionAttribute& rhs) : mName(rhs.mName), mValue(rhs.mValue) {}

      // Containable requirements
      UtlContainableType getContainableType() const { static char type[] = "SdpCrypto"; return type;}
      unsigned hash() const { return directHash(); }
      int compareTo(UtlContainable const *) const { return 0; } // not implemented
      UtlCopyableContainable* clone() const { return new SdpCandidateExtensionAttribute(*this); }

      // Accessors
      void setName(const char * name) { mName = name; }
      const UtlString& getName() const { return mName; }

      void setValue(const char * value) { mValue = value; }
      const UtlString& getValue() const { return mValue; }

   private:
      UtlString mName;
      UtlString mValue;
   };


/* ============================ CREATORS ================================== */


   SdpCandidate(const char * foundation = 0,
                unsigned int id = 0,
                SdpCandidateTransportType transport = CANDIDATE_TRANSPORT_TYPE_NONE,
                uint64_t priority = 0,
                const char * connectionAddress = 0,
                unsigned int port = 0,
                SdpCandidateType candidateType = CANDIDATE_TYPE_NONE,
                const char * relatedAddress = 0,
                unsigned int relatedPort = 0,
                bool inUse = false);
     //:Default constructor

   SdpCandidate(const SdpCandidate& rSdpCandidate);
     //:Copy constructor

   virtual ~SdpCandidate();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   SdpCandidate& operator=(const SdpCandidate& rhs);
     //:Assignment operator

   void setFoundation(const char * foundation) { mFoundation = foundation; }
   void setId(unsigned int id) { mId = id; }
   void setTransport(SdpCandidateTransportType transport) { mTransport = transport; }
   void setPriority(uint64_t priority) { mPriority = priority; }
   void setConnectionAddress(const char * connectionAddress) { mConnectionAddress = connectionAddress; }
   void setPort(unsigned int port) { mPort = port; }
   void setCandidateType(SdpCandidateType candidateType) { mCandidateType = candidateType; }
   void setRelatedAddress(const char * relatedAddress) { mRelatedAddress = relatedAddress; }
   void setRelatedPort(unsigned int relatedPort) { mRelatedPort = relatedPort; }

   void addExtensionAttribute(const char * name, const char * value) { addExtensionAttribute(new SdpCandidateExtensionAttribute(name, value)); }
   void addExtensionAttribute(SdpCandidateExtensionAttribute* sdpCandidateExtensionAttribute) { mExtensionAttributes.insert(sdpCandidateExtensionAttribute); }
   void clearExtensionAttributes() { mExtensionAttributes.destroyAll(); }

   void setInUse(bool inUse) { mInUse = inUse; }

/* ============================ ACCESSORS ================================= */

   // UtlContainable requirements
   virtual UtlContainableType getContainableType() const;
   virtual unsigned hash() const;
   virtual int compareTo(UtlContainable const *) const;
   static const UtlContainableType TYPE;    ///< Class type used for runtime checking 
   UtlCopyableContainable* clone() const { return new SdpCandidate(*this); }

   void toString(UtlString& sdpCandidateString) const;

/* ============================ INQUIRY =================================== */

   const UtlString& getFoundation() const { return mFoundation; }
   unsigned int getId() const { return mId; }
   SdpCandidateTransportType getTransport() const { return mTransport; }
   static SdpCandidateTransportType getCandidateTransportTypeFromString(const char * type);
   uint64_t getPriority() const { return mPriority; }
   const UtlString& getConnectionAddress() const { return mConnectionAddress; }
   unsigned int getPort() const { return mPort; }
   SdpCandidateType getCandidateType() const { return mCandidateType; }
   static SdpCandidateType getCandidateTypeFromString(const char * type);
   const UtlString& getRelatedAddress() const { return mRelatedAddress; }
   unsigned int getRelatedPort() const { return mRelatedPort; }
   const UtlCopyableSList& getExtensionAttributes() const { return mExtensionAttributes; }
   bool isInUse() const { return mInUse; }

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   int compareNumber(uint64_t first, uint64_t second, bool reverse = false) const;

   UtlString                  mFoundation;
   unsigned int               mId;
   SdpCandidateTransportType  mTransport;
   uint64_t                   mPriority;
   UtlString                  mConnectionAddress;
   unsigned int               mPort;
   SdpCandidateType           mCandidateType;
   UtlString                  mRelatedAddress;
   unsigned int               mRelatedPort;
   UtlCopyableSList           mExtensionAttributes;

   bool                       mInUse;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SdpCandidate_h_
