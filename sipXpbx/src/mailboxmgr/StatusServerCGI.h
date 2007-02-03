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

#ifndef STATUSSERVERCGI_H
#define STATUSSERVERCGI_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
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
 * StatusServerCGI Class
 *
 * CGI to initiate an HTTP Post message to the Status Server
 *
 * @author John P. Coffey
 * @version 1.0
 */
class StatusServerCGI : public VXMLCGICommand
{
public:
    StatusServerCGI( const UtlString& mailboxIdentity );
    virtual ~StatusServerCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL);

private:
    UtlString m_mailboxIdentity;
    UtlString m_statusServerUrl;
};

#endif // STATUSSERVERCGI_H
