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
//#include <...>

// APPLICATION INCLUDES
#include "net/SipServerBase.h"
#include "os/OsBSem.h"
#include "os/OsDefs.h"
#include "os/OsRWMutex.h"
#include "SipImpliedSubscriptions.h"
#include "SipRegisterHooks.h"

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
class SipRedirectServer;
class SipRegistrarServer;
//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipRegistrar : public OsServerTask //public SipServerBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    // Singleton globals
    // Note: this class does not need to be a singleton.  The only method that
    // assumes singleton is getSipRegistrar
    static SipRegistrar* spInstance;
    static OsBSem sLock;

/* ============================ CREATORS ================================== */


   //:Default constructor

   virtual
   ~SipRegistrar();
     //:Destructor

/* ============================ MANIPULATORS ============================== */
   static SipRegistrar* SipRegistrar::startRegistrar(const UtlString workingDir,
      const char* configFileName);


    static SipRegistrar* getInstance();
    // Singleton constructor/accessor
    // Note: this class does not need to be a singleton.  The only method that
    // assumes singleton is getSipRegistrar

         void printMessageLog();

    virtual UtlBoolean handleMessage(OsMsg& eventMessage);


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   SipUserAgent* mSipUserAgent;
   UtlString mDefaultDomain;
   UtlString mDomainAliases;
   
   SipRedirectServer* mRedirectServer;
   OsMsgQ* mRedirectMsgQ;
   UtlBoolean mRedirectThreadInitialized;

   SipRegistrarServer* mRegistrarServer;
   OsMsgQ* mRegistrarMsgQ;
   UtlBoolean mRegistrarThreadInitialized;

   SipImpliedSubscriptions* mSipImpliedSubscriptions;
   SipRegisterHooks* mSipRegisterHooks;

   UtlString mMinExpiresTime;
   UtlString mRegistryCacheFileName;
   int mDefaultRegistryPeriod;
   int mDefaultQvalue;
   UtlBoolean mIsCredentialDB;
   UtlString mAuthAlgorithm;
   UtlString mAuthQop;
   UtlString mRealm;
   UtlString mConfigDirectory;
   UtlString mMediaServer;
   UtlString mVoicemailServer;
   UtlString mlocalDomainHost;


   SipRegistrar( SipUserAgent* sipUserAgent,
                 SipImpliedSubscriptions* sipImpliedSubscriptions,
                 SipRegisterHooks* sipRegisterHooks,
                 int maxExpiresTime,
                 const UtlString& defaultDomain,
                 const UtlString& domainAliases,
                 const UtlString& defaultMinExpiresTime,
                 const UtlBoolean& useCredentialDB,
                 const UtlString& defaultAuthAlgorithm,
                 const UtlString& defaultAuthQop,
                 const UtlString& defaultRealm,
                 const UtlString& configDir,
                 const UtlString& mediaServer,
                 const UtlString& voicemailServer);

   /* ============================ REGISTRAR =================================== */
   void startRegistrarServer();
   void sendToRegistrarServer(OsMsg& eventMessage);

   /* ============================ REDIRECT ==================================== */
    void startRedirectServer();
    void sendToRedirectServer(OsMsg& eventMessage);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipRegistrar_h_
