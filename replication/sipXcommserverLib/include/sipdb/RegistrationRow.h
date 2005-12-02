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

#ifndef REGISTRATIONROW_H
#define REGISTRATIONROW_H
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
 * The Registration Base Schema
 */
class RegistrationRow
{
public:
    const char* np_identity;
    const char* uri;
    const char* callid;
    const char* contact;
    const char* qvalue;       
    int4 cseq;
    int4 expires;           // Absolute expiration time, secs since 1/1/1970
    TYPE_DESCRIPTOR (
      ( KEY(np_identity, INDEXED),
        KEY(callid, HASHED),
        KEY(cseq, HASHED),
        FIELD(uri),
        FIELD(contact),
        FIELD(qvalue),
        FIELD(expires) )
    );
};

#endif //REGISTRATIONROW_H

