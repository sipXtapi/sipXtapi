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

#ifndef DIALBYNAMEROW_H
#define DIALBYNAMEROW_H

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
 * The DialByName Table Schema
 */
class DialByNameRow
{
public:
    const char* np_identity;
    const char* np_contact;
    const char* np_digits;

    TYPE_DESCRIPTOR(
       (KEY(np_identity, INDEXED),      // non persistent AVL Tree
        KEY(np_digits, HASHED),    // non persistent Hashed
        FIELD(np_contact)));
};

#endif //DIALBYNAMEROW_H

