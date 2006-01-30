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

   For each Reachable peer, if the local DbUpdateNumber is greater than the
   PeerSentDbUpdateNumber, the pushUpdates XML-RPC method is used to push a single update.
   A successful return in turn updates the PeerSentDbUpdateNumber.
   If any fault is returned by pushUpdate, the peer is marked UnReachable, which triggers
   the RegistrarTest (RegistrarTest) thread to begin attempting to reestablish contact.

   After completing one pass over the Reachable peers, if DbUpdateNumber is less than the
   lowest PeerSentDbUpdateNumber for all Reachable peers (indicating that there remains at
   least one update to be propagated), the RegistrarSync thread calls sendUpdates itself.
*/

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "RegistrarSync.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// constructor
RegistrarSync::RegistrarSync() :
   mutex(OsBSem::Q_PRIORITY, OsBSem::FULL)
   // :TODO: initialize counting semaphore
{
};

/// Signal that there may be updates ready to send.
void RegistrarSync::sendUpdates()
{
   // :TODO: 
}


/// Task main loop.
int RegistrarSync::run(void* pArg)
{
   // :TODO: 

   return 0;
}

/// destructor
RegistrarSync::~RegistrarSync()
{
};
