// 
// Copyright (C) 2006 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

/*
The RegistrarSync thread is responsible for propagating updates to Reachable peer registrars.

The RegistrarSync thread operation is governed by a private static OsBSem (binary semaphore).
The thread main loop waits on that semaphore. The static C++ method RegistrarSync::sendUpdates;
when invoked, increments the semaphore value, indicating to the RegistrarSync thread that there
may be updates available to be propagated, or that connectivity to a previously UnReachable
peer has been restored. On each pass through the loop, the thread does:

   For each Reachable peer, if the localDbUpdateNumber is greater than the
   peerSentDbUpdateNumber, the pushUpdates XML-RPC method is used to push a single update.
   A successful return in turn updates the peerSentDbUpdateNumber.
   If any fault is returned by pushUpdate, the peer is marked UnReachable, which triggers
   the RegistrarTest (RegistrarTest) thread to begin attempting to reestablish contact.

   After completing one pass over the Reachable peers, if DbUpdateNumber is less than the
   lowest peerSentDbUpdateNumber for all Reachable peers (indicating that there remains at
   least one update to be propagated), the RegistrarSync thread calls sendUpdates itself.
*/

// SYSTEM INCLUDES
#include <memory>
using std::auto_ptr;

// APPLICATION INCLUDES
#include "utl/UtlSListIterator.h"
#include "RegistrarPeer.h"
#include "RegistrarSync.h"
#include "SipRegistrar.h"
#include "SipRegistrarServer.h"
#include "SyncRpc.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// constructor
RegistrarSync::RegistrarSync(SipRegistrar& registrar) :
   mRegistrar(registrar),
   mMutex(OsBSem::Q_PRIORITY, OsBSem::EMPTY)
{
};

/// Signal that there may be updates ready to send
void RegistrarSync::sendUpdates()
{
   mMutex.release();
}


/// Task main loop
int RegistrarSync::run(void* pArg)
{
   // :TODO: implement

   // :TODO: move updating of PeerSentDbUpdateNumber out of SyncRpcPushUpdates::invoke
   // and into RegistrarSync?
   // :TODO: shut down the thread when requested

   while(true)
   {
      // Wait until there is work to do
      mMutex.acquire();
      
      // For each Reachable peer, if the local DbUpdateNumber is greater than the
      // PeerSentDbUpdateNumber, then push a single update.
      auto_ptr<UtlSListIterator> peers(mRegistrar.getPeers());
      RegistrarPeer* peer;
      while ((peer = static_cast<RegistrarPeer*>((*peers)())))
      {
         if (peer->isReachable())
         {
            intll localDbUpdateNumber = getRegistrarServer().getDbUpdateNumber();
            intll peerSentDbUpdateNumber = peer->sentTo();
            if (localDbUpdateNumber > peerSentDbUpdateNumber)
            {
               UtlSList bindings;
               bool isUpdateToSend(
                  getRegistrarServer().getNextUpdateToSend(peer, bindings));
               assert(isUpdateToSend);
               if (isUpdateToSend)
               {
                  SyncRpcPushUpdates::invoke(peer, mRegistrar.primaryName(), &bindings);
               }
            }
         }
      }
   }

   return 0;
}


SipRegistrarServer& RegistrarSync::getRegistrarServer()
{
   return mRegistrar.getRegistrarServer();
}


/// destructor
RegistrarSync::~RegistrarSync()
{
};
