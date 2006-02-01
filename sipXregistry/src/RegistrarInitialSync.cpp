// 
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <memory>
using std::auto_ptr;

// APPLICATION INCLUDES
#include "sipdb/RegistrationBinding.h"
#include "os/OsDateTime.h"
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

   // get the received update numbers for each peer from the local database
   restorePeerUpdateNumbers();

   // having done that, we can begin accepting pull requests from peers
   SyncRpcPullUpdates::registerSelf(*mRegistrar);

   // Get from peers any of our own updates that we have lost
   pullLocalUpdatesFromPeers();
   
   // Get from peers any peer updates that we missed or lost while down
   pullPeerUpdatesFromPeers();

   // Get any updates for unreachable peers from reachable ones.
   recoverUnReachablePeers();

   // Reset the DbUpdateNumber so that the upper half is the epoch time.
   getRegistrarServer().resetDbUpdateNumberEpoch();

   // SipRegistrar manages the transition to operational phase, so it will send resets to peers

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "RegistrarInitialSync complete");
   
   mFinished.release();
   return 0; // exit thread
}

/// Recover the latest received update number for each peer from the local db.
void RegistrarInitialSync::restorePeerUpdateNumbers()
{
   auto_ptr<UtlSListIterator> peers(mRegistrar->getPeers());
   RegistrarPeer* peer;
   while ((peer = static_cast<RegistrarPeer*>((*peers)())))
   {
      const char* name = peer->name();
      assert(name);
      
      // Set the last received update number for the peer to the max update number
      // for the peer that we see in the registration DB
      intll maxUpdateNumber = getRegistrarServer().getMaxUpdateNumberForRegistrar(name);
      peer->setReceivedFrom(maxUpdateNumber);
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "RegistrarInitialSync::restorePeerUpdateNumbers "
                    "for peer '%s' last received update # = %lld",
                    name, maxUpdateNumber);

      // We don't know the last sent update number for the peer yet, so zero it out
      peer->setSentTo(0);
   }
}

   
/// Get from peers any of our own updates that we have lost
void RegistrarInitialSync::pullLocalUpdatesFromPeers()
{
   auto_ptr<UtlSListIterator> peers(mRegistrar->getPeers());
   RegistrarPeer* peer;
   while ((peer = static_cast<RegistrarPeer*>((*peers)())))
   {
      // Call pullUpdates, passing the local registrar host name and DbUpdateNumber.
      // The purpose of this call is to recover any registrations for which the local
      // host was the primary but which for some reason were not saved in the local
      // persistent store (the canonical case is that the local file was lost or
      // corrupted - when this is the case, the local DbUpdateNumber will usually be zero).
      // If we can't reach the peer, then the invoke method marks it UnReachable.

      // Pulling updates changes maxUpdateNumber, so compute it on each iteration
      const char* primaryName = getPrimaryName();
      intll maxUpdateNumber = getRegistrarServer().getMaxUpdateNumberForRegistrar(primaryName);

      UtlSList bindings;
      RegistrarPeer::SynchronizationState state =
         SyncRpcPullUpdates::invoke(
            peer,            // the peer we're contacting
            primaryName,     // name of the calling registrar (this one)
            primaryName,     // name of the registrar whose updates we're pulling (this one)
            maxUpdateNumber, // pull all updates more recent than this
            &bindings);      // return bindings in this list

      // Apply the resulting updates to the DB
      if (state == RegistrarPeer::Reachable)
      {
         applyUpdatesToDirectory(bindings);
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "RegistrarInitialSync::pullLocalUpdatesFromPeers "
                       "received %d local updates from peer '%s'",
                       bindings.entries(), peer->name());
      }
   }
}


/// Get from peers any peer updates that we missed or lost while down
void RegistrarInitialSync::pullPeerUpdatesFromPeers()
{
   auto_ptr<UtlSListIterator> peers(mRegistrar->getPeers());
   RegistrarPeer* peer;
   const char* primaryName = getPrimaryName();
   while ((peer = static_cast<RegistrarPeer*>((*peers)())))
   {
      const char* peerName = peer->name();
      assert(peerName);
      UtlSList bindings;
      RegistrarPeer::SynchronizationState state =
         SyncRpcPullUpdates::invoke(
            peer, primaryName, peerName, peer->receivedFrom(), &bindings);

      // Apply the resulting updates to the DB
      if (state == RegistrarPeer::Reachable)
      {
         applyUpdatesToDirectory(bindings);
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "RegistrarInitialSync::pullLocalUpdatesFromPeers "
                       "received %d peer updates from peer '%s'",
                       bindings.entries(), peer->name());
      }
   }
}


/// Get any updates for unreachable peers from reachable ones.
void RegistrarInitialSync::recoverUnReachablePeers()
{
   // Defer implementation until after HA 1.0: see XRR-92
}


void RegistrarInitialSync::waitForCompletion()
{
   mFinished.acquire();
}


const char* RegistrarInitialSync::getPrimaryName()
{
   const char* primaryName = mRegistrar->primaryName();   
   assert(primaryName);
   return primaryName;
}


SipRegistrarServer& RegistrarInitialSync::getRegistrarServer()
{
   return mRegistrar->getRegistrarServer();
}


void RegistrarInitialSync::applyUpdatesToDirectory(UtlSList& bindings)
{
   int timeNow = OsDateTime::getSecsSinceEpoch();
   getRegistrarServer().applyUpdatesToDirectory(timeNow, bindings);
}
 
  
/// destructor
RegistrarInitialSync::~RegistrarInitialSync()
{
};
