// 
// Copyright (C) 2005 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _REGISTERINITIALSYNC_H_
#define _REGISTERINITIALSYNC_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsTask.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipRegistrar;

/**
 * RegistrarInitialSync is an OsTask that implements the startup
 * phase of operation: recover the local registration
 * database (if possible), and resynchronize with all peer
 * registrars.  Any updates are pulled from peer registrars so that the
 * local registrar can tell when its database is up to date and that no
 * more updates are available.  During the startup phase the database is
 * not yet known to be up to date, so the Registry/Redirect service does
 * not accept either any SIP request or any request to push updates from
 * any peer registrar.
 */
class RegistrarInitialSync : public OsTask
{
  public:

   /// Create the startup phase thread.
   RegistrarInitialSync(SipRegistrar* registrar);

   virtual int run(void* pArg);

   /// Wait until the startup phase has completed
   void waitForCompletion();
   
   /// destructor
   virtual ~RegistrarInitialSync();

  protected:
   friend class SipRegistrar;

   /// Recover the latest received update number for each peer from the local db.
   void restorePeerUpdateNumbers(UtlSListIterator* peers,
                                 RegistrationDB*   registrationDb
                                 );
   
   /// Get any updates from peers that we missed or lost while down
   void pullUpdatesFromPeers( UtlSListIterator* peers
                             ,const char*       primaryName
                             ,intll             dbUpdateNumber
                             );
   
   /// Get any updates for unreachable peers from reachable ones.
   void recoverUnReachablePeers( UtlSListIterator* peers );
   
  private:

   SipRegistrar* mRegistrar;
   OsBSem        mFinished;
   
   /// There is no copy constructor.
   RegistrarInitialSync(const RegistrarInitialSync&);

   /// There is no assignment operator.
   RegistrarInitialSync& operator=(const RegistrarInitialSync&);
    
};

#endif // _REGISTERINITIALSYNC_H_
