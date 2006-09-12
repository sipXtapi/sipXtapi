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
#include "SipRedirector.h"
#include "sipdb/CallerAliasDB.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class SipRedirectorCallerAlias : public SipRedirector
{
public:

   SipRedirectorCallerAlias();

   ~SipRedirectorCallerAlias();

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

   CallerAliasDB* mpCallerAliasDB;
   
private:

};

#endif // SIPREDIRECTORCALLERALIAS_H
