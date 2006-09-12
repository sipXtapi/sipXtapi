// 
// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "registry/RedirectSuspend.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

RedirectSuspend::RedirectSuspend(int noRedirectors) :
   mSuspendCount(0),
   mNoRedirectors(noRedirectors),
   mRedirectors((struct redirector*)
                malloc(noRedirectors * sizeof (struct redirector)))
{
   for (int i = 0; i < noRedirectors; i++)
   {
      mRedirectors[i].suspended = FALSE;
      mRedirectors[i].needsCancel = FALSE;
      mRedirectors[i].privateStorage = NULL;
   }
}

RedirectSuspend::~RedirectSuspend()
{
   for (int i = 0; i < mNoRedirectors; i++)
   {
      if (mRedirectors[i].privateStorage)
      {
         delete mRedirectors[i].privateStorage;
      }
   }
   free(mRedirectors);
}

unsigned int RedirectSuspend::hash() const
{
  return 0;
}

static UtlContainableType TYPE = "UtlInt";

const char* const RedirectSuspend::getContainableType() const
{
  return TYPE;
}

int RedirectSuspend::compareTo(const UtlContainable*) const
{
  return TRUE;
}
