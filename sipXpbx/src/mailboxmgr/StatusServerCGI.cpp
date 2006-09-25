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

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include "mailboxmgr/VXMLDefs.h"
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/StatusServerCGI.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ CREATORS ================================== */

StatusServerCGI::StatusServerCGI(
    const UtlString& mailboxIdentity )
{
    m_mailboxIdentity = mailboxIdentity;
}

StatusServerCGI::~StatusServerCGI()
{}

OsStatus
StatusServerCGI::execute(UtlString* out)
{
    // forward the request to the mailbox manager
#ifdef WIN32
//  DebugBreak();
#endif
    UtlString notifyBodyText;
    OsStatus result =
       MailboxManager::getInstance()->getMWINotifyText ( m_mailboxIdentity, NULL, notifyBodyText );
    // send an OK back to the Status Server
    if (out)
    {
        // clear the response
        out->remove(0);

        // set the http headers
        char contentLengthHeader[40];
        sprintf( contentLengthHeader, "Content-Length: %d\r\n", notifyBodyText.length() );
        out->append( (UtlString)contentLengthHeader );
        out->append( "Content-Type: application/simple-message-summary\r\n" );


        // Separate the headers from the body content (very important otherwise no body sent!)
        out->append( "\r\n" );
        out->append( notifyBodyText.data() );
    }
    return result;
}
