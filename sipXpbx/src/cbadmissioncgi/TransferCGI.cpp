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

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "net/Url.h"
#include "mailboxmgr/VXMLDefs.h"
#include "cbadmissioncgi/ConferenceManager.h"
#include "cbadmissioncgi/TransferCGI.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ CREATORS ================================== */

TransferCGI::TransferCGI( const UtlString& conferenceUrl ) :
    mConferenceUrl ( conferenceUrl )
{}

TransferCGI::~TransferCGI()
{}

OsStatus
TransferCGI::execute(UtlString* out)
{
    OsStatus result = OS_SUCCESS;

    UtlString dynamicVxml(VXML_BODY_BEGIN);
    if ( result == OS_SUCCESS )
    {
        // Contains the dynamically generated VXML script.
        dynamicVxml += "<form> \n" \
                           "<transfer dest=\"" + mConferenceUrl + "\" /> \n" \
                       "</form> \n";
    }
    else
    {
       dynamicVxml += VXML_INVALID_EXTN_SNIPPET;
    }
    
    dynamicVxml += VXML_END;

    if (out)
    {
        out->remove(0);
        UtlString responseHeaders;
        ConferenceManager::getResponseHeaders(dynamicVxml.length(), responseHeaders);

        out->append(responseHeaders.data());
        out->append(dynamicVxml.data());
    }

    return OS_SUCCESS;
}
