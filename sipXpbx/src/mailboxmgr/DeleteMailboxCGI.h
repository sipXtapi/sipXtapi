// 
//
// Copyright (c) PingTel Corp. (work in progress)
//
// This is an unpublished work containing PingTel Corporation's confidential
// and proprietary information.  Disclosure, use or reproduction without
// written authorization of PingTel Corp. is prohibited.
//
//! lib=mailbox
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
class DeleteMailboxCGI : public CGICommand
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

