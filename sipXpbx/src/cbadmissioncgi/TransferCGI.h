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

#ifndef TRANSFERCGI_H
#define TRANSFERCGI_H

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
 *  TransferToExtnCGI Class
 *
 *  CGI for taking the extension dialed by the user and converting it 
 *  into a fully qualified SIP URL and then transfering to that extension.
 *
 *  @author Harippriya M Sivapatham
 *  @version 1.0
 */
class TransferCGI : public CGICommand
{
public:
    /**
     * Ctor
     */
    TransferCGI ( const UtlString& conferenceurl );

    /**
     * Virtual Dtor
     */
    virtual ~TransferCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL);

protected:

private:
    const UtlString mConferenceUrl;
};

#endif //TRANSFERCGI_H

