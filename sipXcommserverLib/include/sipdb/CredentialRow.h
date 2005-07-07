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


#ifndef CREDENTIALROW_H
#define CREDENTIALROW_H

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
 * The Credential IMDB Schema
 */
class CredentialRow
{
public:
    const char* np_identity; // non persistent Primary Key derived from URI 
    const char* uri;         // this is the complete uri, not the PK
    const char* realm;       // the secondary key
    const char* userid;      // the user id from which the passtoken is generated
    const char* passtoken;   // the md5 pass token md5(uid:realm:password)
    const char* authtype;    // Authentication Type ("DIGEST (MD5 or Session & ??), "BASIC" "NONE")

    TYPE_DESCRIPTOR(
       (KEY(np_identity, INDEXED),
        KEY(realm, HASHED),
        FIELD(uri),
        FIELD(userid),
        FIELD(passtoken),
        FIELD(authtype)));
};

#endif //CREDENTIALROW_H

