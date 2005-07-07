// 
// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPREDIRECTORMAPPING_H
#define SIPREDIRECTORMAPPING_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "SipRedirector.h"
#include "digitmaps/UrlMapping.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * SipRedirectorMapping is a class whose object adds contacts that are
 * listed a mapping rules file.
 *
 * Currently, we instantiate two objects within the class, one for
 * mappingrules.xml and one for fallbackrules.xml.
 */

class SipRedirectorMapping : public SipRedirector
{
  public:

   SipRedirectorMapping();

   ~SipRedirectorMapping();

   /**
    * Requires the following parameters:
    *
    * configDir - the directory containing the mapping rules file.
    *
    * mediaServer - the URI of the Media Server.
    *
    * voicemailServer - the URI of the voicemail server.
    *
    * localDomainHost - the SIP domain name
    *
    * mappingRulesFilename - the base name of the mapping rules file
    * to load (e.g., "mappingrules.xml")
    */
   virtual OsStatus initialize(const UtlHashMap& configParameters,
                               OsConfigDb& configDb,
                               SipUserAgent* pSipUserAgent,
                               int redirectorNo);

   virtual void finalize();

   virtual SipRedirector::LookUpStatus lookUp(
      const SipMessage& message,
      const UtlString& requestString,
      const Url& requestUri,
      const UtlString& method,
      SipMessage& response,
      RequestSeqNo requestSeqNo,
      int redirectorNo,
      SipRedirectorPrivateStorage*& privateStorage);

  protected:

   /**
    * Set to OS_SUCCESS once the file of mapping rules is loaded into memory.
    */
   OsStatus mMappingRulesLoaded;

   /**
    * The mapping rules parsed from the file.
    */
   UrlMapping mMap;

   /**
    * Name to use when reporting on this mapping.
    */
   UtlString mName;

   /**
    * True if this mapping is "fallback", that is, no contacts should
    * be added if there are already contacts in the set.
    */
   UtlBoolean mFallback;
};

#endif // SIPREDIRECTORMAPPING_H
