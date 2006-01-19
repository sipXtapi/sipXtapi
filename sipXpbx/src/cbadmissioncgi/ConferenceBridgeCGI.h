// 
// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef CONFERENCEBRIDGE_H
#define CONFERENCEBRIDGE_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "net/Url.h"
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
class ConferenceBridgeCGI : public CGICommand
{
public:
    /**
     * Ctor
     */
    ConferenceBridgeCGI (const UtlString& name,
                         const UtlString& contact,
                         const UtlString& confIds,
                         const UtlString& accessCode);

    /**
     * Virtual Dtor
     */
    virtual ~ConferenceBridgeCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL);

protected:

private:
    UtlString mName;
    UtlString mContact;
    UtlString mConfId;
    UtlString mAccessCode;
};

#endif //CONFERENCEBRIDGE_H

