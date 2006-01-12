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
#include "RegistrarTest.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// constructor
RegistrarTest::RegistrarTest() :
   mutex(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   // :TODO: 
};

/// Signal that a peer has become UnReachable
void RegistrarTest::check()
{
   // :TODO: 
}


/// Check each unreachable peer.
UtlBoolean RegistrarTest::handleMessage( OsMsg& eventMessage ///< Timer expiration msg
                                        )
{
   UtlBoolean handled = TRUE;
   
   /*
    * Do a single check of each unreachable peer.
    * If any are still unreachable after all are checked, then
    * the timer is scheduled to retry, using a standard limited
    * exponential backoff.
    */
   // :TODO: 

   return handled;
}

/// destructor
RegistrarTest::~RegistrarTest()
{
   // :TODO: 
}
