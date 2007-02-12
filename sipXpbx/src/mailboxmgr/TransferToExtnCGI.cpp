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
//#include <...>

// APPLICATION INCLUDES
#include "net/Url.h"
#include "mailboxmgr/VXMLDefs.h"
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/TransferToExtnCGI.h"
#include "mailboxmgr/ValidateMailboxCGIHelper.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ CREATORS ================================== */

TransferToExtnCGI::TransferToExtnCGI( const UtlString& extension ) :
    m_extension ( extension )
{}

TransferToExtnCGI::~TransferToExtnCGI()
{}

OsStatus
TransferToExtnCGI::execute(UtlString* out)
{
    OsStatus result = OS_SUCCESS;

        // Get fully qualified SIP URL for the extension
    Url extensionUrl;
        MailboxManager* pMailboxManager = MailboxManager::getInstance();
        pMailboxManager->getWellFormedURL( m_extension, extensionUrl );

        // Validate the mailbox id and extension.
        // If valid, lazy create the physical folders for the mailbox if necessary.
    UtlBoolean checkPermissions = FALSE ;
    ValidateMailboxCGIHelper validateMailboxHelper ( extensionUrl.toString() );
    result = validateMailboxHelper.validate( checkPermissions );
    UtlString dynamicVxml;
        if ( result == OS_SUCCESS )
    {
/*
        UtlString transferUrlString;
        validateMailboxHelper.getMailboxIdentity( transferUrlString );
        if (transferUrlString.index("sip:") == UTL_NOT_FOUND)
        {
            transferUrlString = "sip:" + transferUrlString;
        }
*/
        UtlString ivrPromptUrl;
        MailboxManager::getInstance()->getIvrPromptURL( ivrPromptUrl );

        // Contains the dynamically generated VXML script.
            dynamicVxml =   VXML_BODY_BEGIN \
                        "<form> \n" \
                            "<transfer dest=\"" + extensionUrl.toString() /* transferUrlString */ + "\" /> \n" \
                        "</form> \n" \
                        VXML_END;
            // Write out the dynamic VXML script to be processed by OpenVXI
    } else
    {
            dynamicVxml =   VXML_BODY_BEGIN \
                        VXML_INVALID_EXTN_SNIPPET\
                            VXML_END;
    }

        if (out)
        {
        out->remove(0);
        UtlString responseHeaders;
        MailboxManager::getResponseHeaders(dynamicVxml.length(), responseHeaders);

        out->append(responseHeaders.data());
        out->append(dynamicVxml.data());
        }

    return OS_SUCCESS;
}