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
        MailboxManager::getInstance()->getMWINotifyText ( m_mailboxIdentity, notifyBodyText );
    // send an OK back to the Status Server
    if (out)
    {
        // clear the response
        out->remove(0);

        // set the http headers, note that apache screws up the line separators
        // by automatically insserting \r's before each \n even in the body
        // so make sure we handle this correctly by removing the \r's
        char contentLengthHeader[40];
        sprintf( contentLengthHeader, "Content-Length: %d\n", notifyBodyText.length() );
        out->append( (UtlString)contentLengthHeader );
        out->append( "Content-Type: application/simple-message-summary\n" );


        // Separate the headers from the body content (very important otherwise no body sent!)
        out->append( "\n" );
        out->append( notifyBodyText.data() );
    }
    return result;
}
