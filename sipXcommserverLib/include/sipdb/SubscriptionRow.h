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

#ifndef SUBSCRIPTIONROW_H
#define SUBSCRIPTIONROW_H
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
class SubscriptionRow
{
public:
    const char* uri;
    const char* callid;
    const char* contact;
    int4 notifycseq;
    int4 subscribecseq;
    int4 expires; // Absolute expiratoin time secs since 1/1/1970
    const char* eventtype;
    const char* to;
    const char* from;
    const char* file;
    const char* key;
    const char* recordroute;

    TYPE_DESCRIPTOR (
      ( KEY(to, HASHED),
        KEY(from, HASHED),
        KEY(callid, HASHED),
        KEY(eventtype, HASHED),
        KEY(key, HASHED),
        FIELD(subscribecseq),
        FIELD(uri),
        FIELD(contact),
        FIELD(expires),
        FIELD(file),
        FIELD(recordroute),
        FIELD(notifycseq) )
    );
};

#endif // SUBSCRIPTIONROW_H

