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

#ifndef ALIASROW_H
#define ALIASROW_H

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
 * The Alias Table Schema
 */
class AliasRow
{
public:
    const char* identity;    // this is the alias uri identity
    const char* contact;     // This is the link to the CredentialsDB (full URI)

    TYPE_DESCRIPTOR(
       (KEY(identity, INDEXED),        
        FIELD(contact)));
};

#endif //ALIASROW_H

