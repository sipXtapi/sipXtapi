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
#include <stdlib.h>

// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include <net/Url.h>
#include <net/SipMessage.h>
#include <sipdb/ResultSet.h>
#include <registry/SipRegistrar.h>
#include <SipRedirectorTest.h>
#include <registry/SipRedirectServer.h>

//*
// Special test redirector.
//
// Looks for a parameter named "t<redirector-number>".  If it is
// present, the redirector will execute a series of suspensions.  The
// string is a set of fields separated by slashes, which are processed
// one for each cycle of redirector processing in the request.  An
// empty field means to not request suspension (for this processing),
// a non-empty field means to request suspension and then request
// resume after that number of seconds.  A field of '*' means to return
// LOOKUP_ERROR_REQUEST on that cycle, and "!" means to return
// LOOKUP_ERROR_SERVER.
//
// E.g., "t1=10" means to suspend for 10 seconds.
// "t1=10;t2=5" means that redirector 1 will suspend for 10 seconds
// and redirector 2 will suspend for 5 seconds.  (And the reprocessing
// will begin at 10 seconds.)
// "t1=10/;t2=/10" means that redirector 1 will request a 10 second
// suspension, and upon reprocessing, redirector 2 will request a 10
// second suspension.  The request will finish on the third cycle.
// "t1=10/*" will suspend for 10 seconds on the first cycle and then return
// LOOKUP_ERROR_REQUEST on the second cycle.

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Static factory function.
extern "C" RedirectPlugin* getRedirectPlugin(const UtlString& instanceName)
{
   return new SipRedirectorTest(instanceName);
}

// Constructor
SipRedirectorTest::SipRedirectorTest(const UtlString& instanceName) :
   RedirectPlugin(instanceName)
{
}

// Destructor
SipRedirectorTest::~SipRedirectorTest()
{
}

// Read config information.
void SipRedirectorTest::readConfig(OsConfigDb& configDb)
{
}

// Initializer
OsStatus
SipRedirectorTest::initialize(OsConfigDb& configDb,
                              SipUserAgent* pSipUserAgent,
                              int redirectorNo,
                               const UtlString& localDomainHost)
{
   return OS_SUCCESS;
}

// Finalizer
void
SipRedirectorTest::finalize()
{
}

RedirectPlugin::LookUpStatus SipRedirectorPrivateStorageTest::actOnString()
{
   // Check if we should return an error.
   if (mPtr[0] == '*')
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRedirectorPrivateStorageTest::actOnString Returning "
                    "LOOKUP_ERROR_REQUEST, from string '%s', remaining '%s'",
                    mString, mPtr);
      // We don't have to update anything, since we will not be called again.
      return RedirectPlugin::LOOKUP_ERROR_REQUEST;
   }
   if (mPtr[0] == '!')
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRedirectorPrivateStorageTest::actOnString Returning "
                    "LOOKUP_ERROR_SERVER, from string '%s', remaining '%s'",
                    mString, mPtr);
      // We don't have to update anything, since we will not be called again.
      return RedirectPlugin::LOOKUP_ERROR_SERVER;
   }
   // strtol will return 0 if mPtr[0] == / or NUL.
   int wait = strtol(mPtr, &mPtr, 10);
   if (mPtr[0] == '/')
   {
      mPtr++;
   }
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipRedirectorPrivateStorageTest::actOnString Wait time %d, from string '%s', remaining '%s'",
                 wait, mString, mPtr);
   // 0 is not a valid wait time.
   if (wait == 0)
   {
      return RedirectPlugin::LOOKUP_SUCCESS;
   }
   else
   {
      mTimer.oneshotAfter(OsTime(wait, 0));
      return RedirectPlugin::LOOKUP_SUSPEND;
   }
}

RedirectPlugin::LookUpStatus
SipRedirectorTest::lookUp(
   const SipMessage& message,
   const UtlString& requestString,
   const Url& requestUri,
   const UtlString& method,
   SipMessage& response,
   RequestSeqNo requestSeqNo,
   int redirectorNo,
   SipRedirectorPrivateStorage*& privateStorage)
{
   UtlString parameter;
   char parameter_name[10];
   sprintf(parameter_name, "t%d", redirectorNo);

   //OsSysLog::add(FAC_SIP, PRI_DEBUG,
   //              "SipRedirectorTest::LookUp redirectorNo %d, parameter_name '%s'",
   //              redirectorNo, parameter_name);

   if (!requestUri.getFieldParameter(parameter_name, parameter))
   {
      return RedirectPlugin::LOOKUP_SUCCESS;
   }

   if (!privateStorage)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRedirectorTest::LookUp Creating storage for parameter '%s', requestSeqNo %d, redirectorNo %d",
                    parameter.data(), requestSeqNo, redirectorNo);
      SipRedirectorPrivateStorageTest *storage =
         new SipRedirectorPrivateStorageTest(parameter, requestSeqNo,
                                             redirectorNo);
      privateStorage = storage;
   }

   return ((SipRedirectorPrivateStorageTest*) privateStorage)->actOnString();
}

SipRedirectorPrivateStorageTest::SipRedirectorPrivateStorageTest(
   const char *string,
   RedirectPlugin::RequestSeqNo requestSeqNo,
   int redirectorNo) :
   mNotification(requestSeqNo, redirectorNo),
   mTimer(mNotification)
{
   // Copy the string.
   mString = new char[strlen(string) + 1];
   strcpy(mString, string);
   // Initialize the parsing pointer.
   mPtr = mString;
}

SipRedirectorPrivateStorageTest::~SipRedirectorPrivateStorageTest()
{
   free(mString);
}

SipRedirectorTestNotification::SipRedirectorTestNotification(
   RedirectPlugin::RequestSeqNo requestSeqNo,
   int redirectorNo) :
   mRequestSeqNo(requestSeqNo),
   mRedirectorNo(redirectorNo)
{
}

OsStatus SipRedirectorTestNotification::signal(const int eventData)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipRedirectorTestNotification::signal Fired mRequestSeqNo %d, mRedirectorNo %d",
                 mRequestSeqNo, mRedirectorNo);
   SipRedirectServer::getInstance()->
      resumeRequest(mRequestSeqNo, mRedirectorNo);
   return OS_SUCCESS;
}
