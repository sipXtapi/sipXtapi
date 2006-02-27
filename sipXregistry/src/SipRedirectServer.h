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

#ifndef SIPREDIRECTSERVER_H
#define SIPREDIRECTSERVER_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsServerTask.h"
#include "digitmaps/UrlMapping.h"
#include "os/OsConfigDb.h"
#include "utl/UtlHashMap.h"
#include "SipRedirector.h"
#include "RedirectSuspend.h"
#include "net/SipUserAgent.h"
#include "utl/UtlHashMapIterator.h"
#include "os/OsMutex.h"

// DEFINES
#define MREDIRECTORCOUNT        7
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipRegistrar;
class SipRedirectServerPrivateStorageIterator;

class SipRedirectServer : public OsServerTask  
{
   friend class SipRedirectServerPrivateStorageIterator;

  public:

   SipRedirectServer(OsConfigDb*   pOsConfigDb,  ///< Configuration parameters
                     SipUserAgent* pSipUserAgent ///< User Agent to use when sending responses
                     );

   virtual ~SipRedirectServer();

   static SipRedirectServer* getInstance();
   static SipRedirectServer* spInstance;

   /// Read in the configuration for the redirect server.
   UtlBoolean initialize( OsConfigDb& configDb    ///< Configuration parameters
                         );

   /**
    * Used by redirector asynchronous processing to request that
    * processing a request be resumed.
    *
    * May be called from any context.  Does not block.
    *
    * requestSeqNo - request sequence number
    *
    * redirectorNo - the number of this redirector
    */
   void resumeRequest(
      RequestSeqNo requestSeqNo,
      int redirectorNo);

   /**
    * Look up the private storage for a particular request.
    *
    * Caller must hold mMutex, and keep it until it discards the
    * return value.
    *
    * requestSeqNo - request sequence number.  The request must have
    * been previously suspended.
    *
    * redirectorNo - the number of this redirector
    *
    * @return Pointer to the private storage area for this request and
    * redirector, or NULL.
    */
   SipRedirectorPrivateStorage* getPrivateStorage(
      RequestSeqNo requestSeqNo,
      int redirectorNo);

   /**
    * Lock that is global for this SipRedirectServer to protect
    * mSuspendList and the private storage dependend from it.
    */
   OsMutex mRedirectorMutex;

  protected:

   UtlBoolean mIsStarted;
   SipUserAgent* mpSipUserAgent;
   SipRegistrar* mpRegistrar;
   
   UrlMapping mFallback;
   OsStatus mFallbackRulesLoaded;
   // A port number, which if found on an AOR to register,
   // will be removed, or PORT_NONE
   int mProxyNormalPort;

   // functions
   UtlBoolean handleMessage(OsMsg& eventMessage);

   // The list of redirect processors.
   SipRedirector* (mRedirectors[MREDIRECTORCOUNT]);

   // The sequence number for the next request.
   RequestSeqNo mNextSeqNo;

   // The list of all requests that have been suspended.
   UtlHashMap mSuspendList;

   // Service functions.
   void processRedirect(const SipMessage* message,
                        UtlString& method,
                        RequestSeqNo seqNo,
                        RedirectSuspend* suspendObject);

   void cancelRedirect(UtlInt& containableSeqNo,
                       RedirectSuspend* suspendObject);
};

/**
 * Iterator that returns the request numbers and private storage pointers
 * for all suspended requests.
 *
 * It has a limited set of operations to avoid unpleasant interactions
 * with the rest of the suspend/resume mechanism.
 *
 * Caller must hold mMutex, and keep it until it discards the
 * returned iterator and any pointers obtained from it.
 *
 * redirectorNo - the number of this redirector
 *
 * <p>
 * Example Code:
 * <pre>
 *    // Seize the global lock.
 *    OsLock lock(SipRedirectServer::getInstance()->mMutex);
 *
 *    // Create an iterator that walks through the suspended requests
 *    // and returns the private storage pointers for a chosen redirector.
 *    SipRedirectorPrivateStorageIterator itor(redirectorNo);
 *
 *    // Fetch a pointer to each element of myContentSource into pStorage.
 *    SipRedirectorPrivateStorage* pStorage;
 *    while (pStorage = itor())
 *    {
 *       // Do something to *pStorage.
 *       ...
 *       // Get the request sequence number for this request.
 *       requestSeqNo = itor.requestSeqNo();
 *    }
 * </pre>
 */
class SipRedirectServerPrivateStorageIterator : protected UtlHashMapIterator
{
  public:

/* ============================ CREATORS ================================== */

   /**
    * Constructor accepts the redirector number for which we are to
    * find the storage.  It finds the SipRedirectServer via
    * SipRedirectServer::getInstance().
    */
   SipRedirectServerPrivateStorageIterator(
      int redirectorNo);

/* ============================ MANIPULATORS ============================== */

   /**
    * Return the private storage pointer for the next request.
    *
    * @return The private storage pointer for the redirector specified in the
    * constructor.  Iterates through the list until it finds the next
    * suspended redirection request which has a non-NULL pointer for this
    * redirector.
    */
   virtual UtlContainable* operator()();

/* ============================ ACCESSORS ================================= */

   /**
    * Gets the request sequence number of the current request.
    *
    * @return The request sequence number of the suspended redirection
    * request that the iterator has just returned.
    */
   RequestSeqNo requestSeqNo() const;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   /**
    * The redirector number.
    */
   int mRedirectorNo;

};

#endif // SIPREDIRECTSERVER_H
