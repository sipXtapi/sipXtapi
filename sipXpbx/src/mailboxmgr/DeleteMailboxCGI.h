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

#ifndef DELETEMAILBOXCGI_H
#define DELETEMAILBOXCGI_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "mailboxmgr/DeleteMailboxCGI.h"

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
 * @author John P. Coffey
 * @version 1.0
 */
class DeleteMailboxCGI : public VXMLCGICommand
{
public:
    /**
     * Ctor
     */
    DeleteMailboxCGI ( const UtlString& mailboxIdentity );

    /**
     * Virtual Dtor
     */
    virtual ~DeleteMailboxCGI ();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL);

protected:

private:
    const UtlString m_mailboxIdentity;
};

#endif //DELETEMAILBOXCGI_H

