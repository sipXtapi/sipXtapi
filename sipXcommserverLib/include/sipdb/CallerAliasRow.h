// 
// 
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef CALLERALIASROW_H
#define CALLERALIASROW_H
// SYSTEM INCLUDES

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

/// The Caller Alias Base Schema
/**
 * Provides a mapping from f(caller-identity, target-domain) to caller-alias
 */
class CallerAliasRow
{
public:
    const char* identity;  /// uri of caller 
    const char* domain;    /// domain & port components from target uri
    const char* alias;     /// From header field value
    TYPE_DESCRIPTOR (
      ( KEY(identity, INDEXED),
        KEY(domain, INDEXED),
        FIELD(alias)
      )
    );
};

#endif //CALLERALIASROW_H
