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

// APPLICATION INCLUDES
#include "sipdb/RegistrationBinding.h"
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
     mRegistrationDb(NULL),
     mFinished(OsBSem::Q_PRIORITY, OsBSem::EMPTY)
{
};

int RegistrarInitialSync::run(void* pArg)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "RegistrarInitialSync started");

   UtlSListIterator*   peers              = mRegistrar->getPeers();
   SipRegistrarServer& sipRegistrarServer = mRegistrar->getRegistrarServer();
   
   // recover the largest update number from the database for this primary
   intll dbUpdateNumber = sipRegistrarServer.restoreLocalUpdateNumber();

   // get the received update numbers for each peer from the local database
   restorePeerUpdateNumbers(*peers);

   // having done that, we can begin accepting pull requests from peers
   SyncRpcPullUpdates::registerSelf(*mRegistrar);

   // Get from peers any of our own updates that we have lost
   const char* primaryName = mRegistrar->primaryName();
   pullLocalUpdatesFromPeers(*peers,
                             primaryName,
                             dbUpdateNumber);
   
   // Get from peers any peer updates that we missed or lost while down
   pullPeerUpdatesFromPeers(*peers, primaryName);

   // Get any updates for unreachable peers from reachable ones.
   recoverUnReachablePeers(*peers);

   // We're done with the peers iterator
   delete peers;

   // Reset the DbUpdateNumber so that the upper half is the epoch time.
   sipRegistrarServer.resetDbUpdateNumberEpoch();

   // SipRegistrar manages the transition to operational phase, so it will send the resets

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "RegistrarInitialSync complete");
   
   mFinished.release();
   return 0; // exit thread
}

/// Recover the latest received update number for each peer from the local db.
void RegistrarInitialSync::restorePeerUpdateNumbers(UtlSListIterator& peers)
{
   RegistrarPeer* peer;
   while ((peer = static_cast<RegistrarPeer*>(peers())))
   {
      const char* name = peer->name();
      assert(name);
      
      // Set the last received update number for the peer to the max update number
      // for the peer that we see in the registration DB
      intll maxUpdateNumber = getRegistrationDb()->getMaxUpdateNumberForRegistrar(name);
      peer->setReceivedFrom(maxUpdateNumber);

      // We don't know the last sent update number for the peer yet, so zero it out
      peer->setSentTo(0);
   }
}

   
/// Get from peers any of our own updates that we have lost
void RegistrarInitialSync::pullLocalUpdatesFromPeers(
   UtlSListIterator& peers,            // list of peers
   const char*       primaryName,      // name of this registrar
   intll             DbUpdateNumber)   // largest local DB update number
{
   assert(primaryName);
   RegistrarPeer* peer;
   while ((peer = static_cast<RegistrarPeer*>(peers())))
   {
      // Call pullUpdates, passing the local registrar host name and DbUpdateNumber.
      // The purpose of this call is to recover any registrations for which the local
      // host was the primary but which for some reason were not saved in the local
      // persistent store (the canonical case is that the local file was lost or
      // corrupted - when this is the case, the local DbUpdateNumber will usually be zero).
      // If we can't reach the peer, then the invoke method marks it UnReachable.
      UtlSList bindings;
      RegistrarPeer::SynchronizationState state =
         SyncRpcPullUpdates::invoke(peer, primaryName, primaryName, DbUpdateNumber, &bindings);

      // Apply the resulting updates to the DB
      if (state == RegistrarPeer::Reachable)
      {
         applyUpdates(bindings);
      }
   }

   // restore the iterator's state
   peers.reset();
}


/// Get from peers any peer updates that we missed or lost while down
void RegistrarInitialSync::pullPeerUpdatesFromPeers(
   UtlSListIterator& peers,            // list of peers
   const char*       primaryName)      // name of this registrar
{
   RegistrarPeer* peer;
   while ((peer = static_cast<RegistrarPeer*>(peers())))
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
         applyUpdates(bindings);
      }
      
   }

   // restore the iterator's state
   peers.reset();
}

   
/// Apply updates to the registration DB
void RegistrarInitialSync::applyUpdates(UtlSList& bindings)
{
   UtlSListIterator bindingsIter(bindings);
   RegistrationBinding* binding;
   RegistrationDB* regDb = getRegistrationDb();
   while ((binding = static_cast<RegistrationBinding*>(bindingsIter())))
   {
      regDb->updateBinding(*binding);
   }
}


/// Get any updates for unreachable peers from reachable ones.
void RegistrarInitialSync::recoverUnReachablePeers( UtlSListIterator& peers )
{
   // Defer implementation until after HA 1.0: see XRR-92

   // restore the iterator's state
   peers.reset();
}


void RegistrarInitialSync::waitForCompletion()
{
   mFinished.acquire();
}


RegistrationDB* RegistrarInitialSync::getRegistrationDb()
{
   if (mRegistrationDb == NULL)
   {
      mRegistrationDb = mRegistrar->getRegistrationDB();
   }
   return mRegistrationDb;
}
 
  
/// destructor
RegistrarInitialSync::~RegistrarInitialSync()
{
};
