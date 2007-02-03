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
#include "mailboxmgr/VXMLCGICommand.h"

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
class LoginCGI : public VXMLCGICommand
{
public:
    /**
     * Ctor
     */
    LoginCGI (  const UtlBoolean& requestIsFromWebUI,
                                const UtlString& userid,
                                const UtlString& password );

    /**
     * Virtual Dtor
     */
    virtual ~LoginCGI();

    /** gets the mailbox identity - this is not an extension */
    void getMailboxIdentity ( UtlString& mailboxIdentity ) const;

    /** gets the mailbox extension */
    void getExtension ( UtlString& extension ) const;

    /** This does the work */
    virtual OsStatus execute ( UtlString* out = NULL );

        OsStatus handleWebRequest( UtlString* out ) ;

        OsStatus handleOpenVXIRequest( UtlString* out ) ;



protected:

private:
        const UtlBoolean m_fromWebUI;
        const UtlString m_userid;
    const UtlString m_password;
    UtlString m_mailboxIdentity;
    UtlString m_extension;
};

#endif //LOGINCGI_H
