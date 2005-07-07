// 
//
// Copyright (c) PingTel Corp. (work in progress)
//
// This is an unpublished work containing PingTel Corporation's confidential
// and proprietary information.  Disclosure, use or reproduction without
// written authorization of PingTel Corp. is prohibited.
//
//! author="John P. Coffey"
//! lib=mailboxmgr
//////////////////////////////////////////////////////////////////////////////
#ifndef STATUSSERVERCGI_H
#define STATUSSERVERCGI_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
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
 * StatusServerCGI Class
 *
 * CGI to initiate an HTTP Post message to the Status Server
 *
 * @author John P. Coffey
 * @version 1.0
 */
class StatusServerCGI : public CGICommand
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
