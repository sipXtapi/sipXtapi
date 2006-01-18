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

   UtlSList unreachablePeers;

   UtlSListIterator* peers = mRegistrar->getPeers();
   RegistrationDB*   registrationDb = mRegistrar->getRegistrationDB();
   
   //:TODO: restorePeerUpdateNumbers(peers, registrationDb);
   
   SyncRpcPullUpdates::registerSelf(*mRegistrar);
      
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "RegistrarInitialSync complete");
   
   mFinished.release();
   return 0;
}

void RegistrarInitialSync::waitForCompletion()
{
   mFinished.acquire();
}

   
/// destructor
RegistrarInitialSync::~RegistrarInitialSync()
{
};
