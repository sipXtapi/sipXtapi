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
#include "os/OsSysLog.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/ResultSet.h"
#include "sipdb/AliasDB.h"
#include "SipRedirectorAliasDB.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// Constructor
SipRedirectorAliasDB::SipRedirectorAliasDB()
{
}

// Destructor
SipRedirectorAliasDB::~SipRedirectorAliasDB()
{
}

// Initializer
OsStatus
SipRedirectorAliasDB::initialize(const UtlHashMap& configParameters,
                                 OsConfigDb& configDb,
                                 SipUserAgent* pSipUserAgent,
                                 int redirectorNo)
{
   return OS_SUCCESS;
}

// Finalizer
void
SipRedirectorAliasDB::finalize()
{
}

SipRedirector::LookUpStatus
SipRedirectorAliasDB::lookUp(
   const SipMessage& message,
   const UtlString& requestString,
   const Url& requestUri,
   const UtlString& method,
   SipMessage& response,
   RequestSeqNo requestSeqNo,
   int redirectorNo,
   SipRedirectorPrivateStorage*& privateStorage)
{
   UtlString requestIdentity;
   requestUri.getIdentity(requestIdentity);

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRedirectorAliasDB::lookUp "
                 "identity '%s'", requestIdentity.data());

   ResultSet aliases;
   AliasDB::getInstance()->getContacts(requestUri, aliases);
   int numAliasContacts = aliases.getSize();
   if (numAliasContacts > 0)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRedirectorAliasDB::lookUp "
                     "got %d AliasDB contacts", numAliasContacts);

      for (int i = 0; i < numAliasContacts; i++)
      {
         static UtlString contactKey("contact");

         UtlHashMap record;
         if (aliases.getIndex(i, record))
         {
            UtlString contact = *((UtlString*)record.findValue(&contactKey));
            Url contactUri(contact);

            // Add the contact.
            addContact(response, requestString, contactUri, "alias");
         }
      }
   }

   return SipRedirector::LOOKUP_SUCCESS;
}
