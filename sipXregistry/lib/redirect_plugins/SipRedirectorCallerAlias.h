// 
// 
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPREDIRECTORCALLERALIAS_H
#define SIPREDIRECTORCALLERALIAS_H

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "registry/RedirectPlugin.h"
#include "sipdb/CallerAliasDB.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class SipRedirectorCallerAlias : public RedirectPlugin
{
public:

   explicit SipRedirectorCallerAlias(const UtlString& instanceName);

   ~SipRedirectorCallerAlias();

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

protected:

   CallerAliasDB* mpCallerAliasDB;
   
   // String to use in place of class name in log messages:
   // "[instance] class".
   UtlString mLogName;

private:

};

#endif // SIPREDIRECTORCALLERALIAS_H
