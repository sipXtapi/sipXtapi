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
#include <os/OsTimer.h>
#include <registry/RedirectPlugin.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipMessage;

class SipRedirectorTest : public RedirectPlugin
{
public:

   explicit SipRedirectorTest(const UtlString& instanceName);

   ~SipRedirectorTest();

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

   // URI parameter to look for.
   char mParameterName[50];

private:

};

class SipRedirectorTestNotification : public OsNotification
{
  public:

   SipRedirectorTestNotification(
      RedirectPlugin::RequestSeqNo requestSeqNo,
      int redirectorNo);

   OsStatus signal(const int eventData);

  private:

   RedirectPlugin::RequestSeqNo mRequestSeqNo;
   int mRedirectorNo;
};

class SipRedirectorPrivateStorageTest : public SipRedirectorPrivateStorage
{
  public:
   
   SipRedirectorPrivateStorageTest(const char *string,
                                   RedirectPlugin::RequestSeqNo requestSeqNo,
                                   int redirectorNo);

   virtual ~SipRedirectorPrivateStorageTest();

   RedirectPlugin::LookUpStatus actOnString();

   virtual UtlContainableType getContainableType() const;

   static const UtlContainableType TYPE ;    /**< Class type used for runtime checking */ 

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
