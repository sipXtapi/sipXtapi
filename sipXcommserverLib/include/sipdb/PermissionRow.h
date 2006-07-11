// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef PERMISSIONROW_H
#define PERMISSIONROW_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "fastdb/fastdb.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * The Permission Schema
 */
class PermissionRow
{
public:
    const char* identity;
    const char* permission;
    TYPE_DESCRIPTOR((KEY(identity, INDEXED),
                     FIELD(permission)));
};

#endif //PERMISSIONROW_H

