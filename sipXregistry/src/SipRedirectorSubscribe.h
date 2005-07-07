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

#ifndef SIPREDIRECTORSUBSCRIBE_H
#define SIPREDIRECTORSUBSCRIBE_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "SipRedirector.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipMessage;

class SipRedirectorSubscribe : public SipRedirector
{
public:

   SipRedirectorSubscribe();

   ~SipRedirectorSubscribe();

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

private:

};

#endif // SIPREDIRECTORSUBSCRIBE_H
