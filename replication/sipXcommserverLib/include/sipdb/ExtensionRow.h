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

#ifndef EXTENSIONROW_H
#define EXTENSIONROW_H

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
 * The Extension Table Schema
 */
class ExtensionRow
{
public:
    const char* np_identity;
    const char* uri;
    const char* extension;

    TYPE_DESCRIPTOR(
       (KEY(np_identity, INDEXED),
        KEY(uri, HASHED),
        FIELD(extension)));
};

#endif //EXTENSIONROW_H

