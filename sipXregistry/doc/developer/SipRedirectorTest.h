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

#ifndef SIPREDIRECTORTEST_H
#define SIPREDIRECTORTEST_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsTimer.h"
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

class SipRedirectorTest : public SipRedirector
{
public:

   SipRedirectorTest();

   ~SipRedirectorTest();

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

class SipRedirectorTestNotification : public OsNotification
{
  public:

   SipRedirectorTestNotification::SipRedirectorTestNotification(
      RequestSeqNo requestSeqNo,
      int redirectorNo);

   OsStatus signal(const int eventData);

  private:

   RequestSeqNo mRequestSeqNo;
   int mRedirectorNo;
};

class SipRedirectorPrivateStorageTest : public SipRedirectorPrivateStorage
{
  public:
   
   SipRedirectorPrivateStorageTest(const char *string,
                                   RequestSeqNo requestSeqNo,
                                   int redirectorNo);

   virtual ~SipRedirectorPrivateStorageTest();

   SipRedirector::LookUpStatus actOnString();

  private:

   // Saved parameter string.
   char* mString;
   // Pointer to the next element of the parameter string to be used.
   char* mPtr;

   // Notification.
   SipRedirectorTestNotification mNotification;
   // Timer.
   OsTimer mTimer;
};

#endif // SIPREDIRECTORTEST_H
