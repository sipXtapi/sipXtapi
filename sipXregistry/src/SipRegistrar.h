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

#ifndef _SipRegistrar_h_
#define _SipRegistrar_h_

// SYSTEM INCLUDES
#include <sipdb/RegistrationDB.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsBSem.h"
#include "os/OsRWMutex.h"
#include "net/SipServerBase.h"
#include "utl/PluginHooks.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsConfigDb;
class SipUserAgent;
class SipMessage;
class HttpServer;
class XmlRpcDispatch;
class RegistrationDB;
class RegistrarPeer;
class RegistrarTest;
class RegistrarSync;
class RegistrarInitialSync;
class SipRedirectServer;
class SipRegistrarServer;

/// Top Level sipXregistry thread
/**
 * This is the top level thread in the service; it spawns
 * all other threads and controls which are started at which time.
 */
class SipRegistrar : public OsServerTask // should be SipServerBase ?
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */


   //:Default constructor

   virtual
   ~SipRegistrar();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

    static SipRegistrar* getInstance(OsConfigDb* configDb,
                                     const char* configFileName
                                     );

    /// top level task
    virtual int run(void* pArg);
    
    /// Receive SIP or other OS messages.
    virtual UtlBoolean handleMessage(OsMsg& eventMessage);
    /**< Messages are dispatched to either the SipRegistrarServer or SipRedirectServer thread */

    /// Load persistent soft state
    void reloadPersistentRegistrations();
    
    /// Server for XML-RPC requests
    void startRpcServer();
    /**<
     * Begins operation of the HTTP/RPC service
     * sets mHttpServer and mXmlRpcDispatcher
     */
    
    /// Launch all Startup Phase threads and wait until synchronization state is known
    void startupPhase();
    /**<
     * Begin the RegistrarInitialSync thread and wait for it to finish
     */
    
    /// Launch all Operational Phase threads.
    void operationalPhase();
    /**<
     * Begins operation of the SipRegistrarServer and SipRedirectServer.
     */

    /// Read configuration for replication..
    void configurePeers();
    /**<
     * Sets mReplicationConfigured=true if replication is configured.
     */
    
    /// If replication configured, name of this registrar as primary, else NULL
    const char* primaryName();

    /// Get an iterator over all peers.
    UtlSListIterator* getPeers();
    /**<
     * @returns
     * - NULL if replication is not configured
     * - an iterator if replication is configured.
     *   Caller must delete the iterator when finished with it.
     */

    /// Get peer state object by name.
    RegistrarPeer* getPeer(const UtlString& peerName);
    /**<
     * @returns NULL if no peer is configured with peerName
     */

    /// Get the RegistrarTest thread object
    RegistrarTest* getRegistrarTest();
    
    /// Get the RegistrarSync thread object
    RegistrarSync* getRegistrarSync();
    
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   // Singleton globals
   static SipRegistrar* spInstance;
   static OsBSem sLock;

   OsConfigDb* mConfigDb; ///< this is 'owned by the main routine - do not delete
   UtlString   mConfigFileName;
   
   RegistrationDB* mRegistrationDb;

   HttpServer* mHttpServer;
   XmlRpcDispatch* mXmlRpcDispatcher;

   bool mReplicationConfigured; /// master switch for replication 
   UtlString mPrimaryName; ///< full name of this host as primary 
   UtlSList mPeers; ///< list of RegisterPeer objects.
   
   RegistrarInitialSync* mInitialSyncThread;
   
   SipUserAgent* mSipUserAgent;
   
   SipRedirectServer* mRedirectServer;
   OsMsgQ* mRedirectMsgQ;

   SipRegistrarServer* mRegistrarServer;
   OsMsgQ* mRegistrarMsgQ;

   RegistrarSync* mRegistrarSync;
   RegistrarTest* mRegistrarTest;

   // :TODO: still needed? UtlString mDefaultDomain;

   SipRegistrar( OsConfigDb* configDb,
                 const char* configFileName
                );

   /* ============================ REGISTRAR =================================== */
   void startRegistrarServer();
   void sendToRegistrarServer(OsMsg& eventMessage);

   /* ============================ REDIRECT ==================================== */
    void startRedirectServer();
    void sendToRedirectServer(OsMsg& eventMessage);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipRegistrar_h_
