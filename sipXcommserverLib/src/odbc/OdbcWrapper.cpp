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
#include "odbc/OdbcWrapper.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATICS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

bool odbcCheckExistence(const char* dbname)
{
   OdbcControlStruct *t = new OdbcControlStruct;
   
   SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &t->mEnvironmentHandle);
   return true;
}