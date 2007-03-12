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

#ifndef SIPREDIRECTORALIASDB_H
#define SIPREDIRECTORALIASDB_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "registry/RedirectPlugin.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * SipRedirectorAliasDB is singleton class whose object adds contacts that are
 * listed in the registration database.
 */

class SipRedirectorAliasDB : public RedirectPlugin
{
  public:

   explicit SipRedirectorAliasDB(const UtlString& instanceName);

   ~SipRedirectorAliasDB();

   virtual void readConfig(OsConfigDb& configDb);

   virtual OsStatus initialize(OsConfigDb& configDb,
                               SipUserAgent* pSipUserAgent,
                               int redirectorNo,
                               const UtlString& localDomainHost);

   virtual void finalize();

   virtual RedirectPlugin::LookUpStatus lookUp(
      const SipMessage& message,
      const UtlString& requestString,
      const Url& requestUri,
      const UtlString& method,
      SipMessage& response,
      RequestSeqNo requestSeqNo,
      int redirectorNo,
      SipRedirectorPrivateStorage*& privateStorage);

  private:

   // String to use in place of class name in log messages:
   // "[instance] class".
   UtlString mLogName;

  protected:
};

#endif // SIPREDIRECTORALIASDB_H
