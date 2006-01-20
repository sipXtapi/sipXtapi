// 
// Copyright (C) 2006 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlSListIterator.h"
#include "SipRegistrar.h"
#include "SipRegistrarServer.h"
#include "RegistrarInitialSync.h"
#include "SyncRpc.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Create the startup phase thread.
RegistrarInitialSync::RegistrarInitialSync(SipRegistrar* registrar)
   : OsTask("RegistrarInitSync-%d"),
     mRegistrar(registrar),
     mFinished(OsBSem::Q_PRIORITY, OsBSem::EMPTY)
{
};

int RegistrarInitialSync::run(void* pArg)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "RegistrarInitialSync started");

   UtlSListIterator*   peers              = mRegistrar->getPeers();
   RegistrationDB*     registrationDb     = mRegistrar->getRegistrationDB();
   SipRegistrarServer& sipRegistrarServer = mRegistrar->getRegistrarServer();
   
   // get the received update numbers for each peer from the local database
   restorePeerUpdateNumbers(peers, registrationDb);

   // having done that, we can begin accepting pull requests from peers.
   SyncRpcPullUpdates::registerSelf(*mRegistrar);

   pullUpdatesFromPeers(peers, mRegistrar->primaryName(), sipRegistrarServer.getDbUpdateNumber());
   
   /// Get any updates for unreachable peers from reachable ones.
   recoverUnReachablePeers( peers );

   /// Reset the DbUpdateNumber so that the upper half is the epoch time.
   sipRegistrarServer.resetDbUpdateNumberEpoch();

   // SipRegistrar manages the transition to operational phase, so it will send the resets

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "RegistrarInitialSync complete");
   
   mFinished.release();
   return 0; // exit thread
}

/// Recover the latest received update number for each peer from the local db.
void RegistrarInitialSync::restorePeerUpdateNumbers( UtlSListIterator* peers
                                                    ,RegistrationDB*   registrationDb
                                                    )
{
   // :TODO: 
}

   
/// Get any updates from peers that we missed or lost while down
void RegistrarInitialSync::pullUpdatesFromPeers( UtlSListIterator* peers
                                                ,const char*       primaryName
                                                ,intll             dbUpdateNumber
                                                )
{
   // :TODO: 
}

   
/// Get any updates for unreachable peers from reachable ones.
void RegistrarInitialSync::recoverUnReachablePeers( UtlSListIterator* peers )
{
   // :TODO: xo
}

   

void RegistrarInitialSync::waitForCompletion()
{
   mFinished.acquire();
}

   
/// destructor
RegistrarInitialSync::~RegistrarInitialSync()
{
};
