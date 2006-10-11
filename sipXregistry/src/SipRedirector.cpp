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

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include <utl/UtlRegex.h>
#include "os/OsDateTime.h"
#include "os/OsFS.h"
#include "sipdb/ResultSet.h"
#include "SipRedirector.h"

// DEFINES
#define URL_MAPPING_RULES_FILENAME "mappingrules.xml"
#define URL_FALLBACK_RULES_FILENAME "fallbackrules.xml"
enum mappingSource
{
   UrlMappingRules = 1,
   UrlFallbackRules = 2
};

// DEFINES
#define CONFIG_ETC_DIR SIPX_CONFDIR

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// Destructor
SipRedirector::~SipRedirector()
{
}

// Null default cancel() implementation.
void SipRedirector::cancel(RequestSeqNo request)
{
}

void
SipRedirector::addContact(SipMessage& response,
                          const UtlString& requestString,
                          const Url& contact,
                          const char* label)
{
   // Get the number of contacts already present.
   int numContactsInHeader =
      response.getCountHeaderFields(SIP_CONTACT_FIELD);

   // Add the contact field to the response at the end.
   // Need to keep this UtlString allocated till the end of this function.
   // The semantics of the Contact: header have the additional restriction
   // that if the URI contains a '?', it must be enclosed in <...> (sec. 20).
   // But beware that the BNF in sec. 25.1 does not require this.
   // Scott has changed Url::toString to always add <...> if there are header
   // parameters in the URI.
   UtlString contactUtlString = contact.toString();
   const char* contactString = contactUtlString.data();
   response.setContactField(contactString, numContactsInHeader);

   OsSysLog::add(FAC_SIP, PRI_INFO,
                 "SipRedirector::addContact Redirector '%s' maps '%s' to '%s'",
                 label, requestString.data(), contactString);
}

void
SipRedirector::removeAllContacts(SipMessage& response)
{
   // Get the number of contacts already present.
   int numContactsInHeader =
      response.getCountHeaderFields(SIP_CONTACT_FIELD);

   OsSysLog::add(FAC_SIP, PRI_INFO,
                 "SipRedirector::removeAllContacts Removing %d contacts",
                 numContactsInHeader);

   for (int i = numContactsInHeader - 1; i >= 0; i--)
   {
      response.removeHeader(SIP_CONTACT_FIELD, i);
   }
}

SipRedirectorPrivateStorage::~SipRedirectorPrivateStorage()
{
}
