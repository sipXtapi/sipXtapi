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
