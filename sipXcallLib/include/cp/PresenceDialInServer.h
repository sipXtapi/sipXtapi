//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _PresenceDialInServer_h_
#define _PresenceDialInServer_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsBSem.h>
#include <net/StateChangeNotifier.h>
#include <tao/TaoAdaptor.h>
#include <utl/UtlHashMap.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class CallManager;
class TaoString;

/**
 * A PresenceDialInServer is an object that allows an extension to sign in and
 * sign off to a ACD queue by simply using a feature code.
 * 
 */

class PresenceDialInServer: public TaoAdaptor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */

   /// Constructor
   PresenceDialInServer(CallManager* callMgr, UtlString& configFile);
   
   /// Destructor
   virtual ~PresenceDialInServer();

/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean handleMessage(OsMsg& eventMessage);

/* ============================ ACCESSORS ================================= */

   /// Registered a StateChangeNotifier
   void addStateChangeNotifier(const char* fileUrl, StateChangeNotifier* notifier);

   /// Unregistered a StateChangeNotifier
   void removeStateChangeNotifier(const char* fileUrl);

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   void dumpTaoMessageArgs(unsigned char eventId, TaoString& args);
 
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   void parseConfig(UtlString& configFile);

   bool notifyStateChange(UtlString& contact, bool signIn);
  
   CallManager* mpCallManager;
   OsBSem mLock;
   
   UtlString mSignInFC;       /// Sign in feature code
   UtlString mSignOutFC;      /// Sign out feature code
   
   UtlString mSignInConfirmationAudio;
   UtlString mSignOutConfirmationAudio;
   UtlString mErrorAudio;
   
   OsMsgQ* mpIncomingQ;

   UtlHashMap mCalls;
   UtlHashMap mStateChangeNotifiers;   
};

/* ============================ INLINE METHODS ============================ */

#endif  // _PresenceDialInServer_h_
