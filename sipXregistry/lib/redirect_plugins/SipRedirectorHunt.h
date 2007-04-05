// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPREDIRECTORHUNT_H
#define SIPREDIRECTORHUNT_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "registry/RedirectPlugin.h"
#include "os/OsMutex.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipMessage;

class SipRedirectorHunt : public RedirectPlugin
{
public:

   explicit SipRedirectorHunt(const UtlString& instanceName);

   ~SipRedirectorHunt();

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

   // String to use in place of class name in log messages:
   // "[instance] class".
   UtlString mLogName;
   
   // static bool indicating whether we have any huntgroups
   UtlBoolean mHuntGroupsDefined;

private:

};

#endif // SIPREDIRECTORHUNT_H
