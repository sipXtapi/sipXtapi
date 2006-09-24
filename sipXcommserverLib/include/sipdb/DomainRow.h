//
//
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 Scott Zuk <szuk@telusplanet.net>
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef DOMAINROW_H
#define DOMAINROW_H
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

/**
 * The Domain Table Schema
 */
class DomainRow
{
public:
    const char* domainname;  /// domain name
    const char* realm;       /// authorization realm of the domain

    TYPE_DESCRIPTOR (
      ( KEY(domainname, INDEXED),
        FIELD(realm)
      )
    );
};

#endif //DOMAINROW_H

