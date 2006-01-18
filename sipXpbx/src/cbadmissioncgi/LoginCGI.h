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

#ifndef LOGINCGI_H
#define LOGINCGI_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlString.h"
#include "mailboxmgr/CGICommand.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * Mailbox Class
 *
 */
class LoginCGI : public CGICommand
{
public:
    /**
     * Ctor
     */
    LoginCGI ( const UtlString& contact, const UtlString& confid, const UtlString& accessCode );

    /**
     * Virtual Dtor
     */
    virtual ~LoginCGI();


    /** This does the work */
    virtual OsStatus execute ( UtlString* out = NULL );

protected:

private:
    const UtlString mContact;
    const UtlString mConfId;
    const UtlString mAccessCode;
};

#endif //LOGINCGI_H
