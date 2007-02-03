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

#ifndef GET_ALL_GREETINGS_H
#define GET_ALL_GREETINGS_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
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
 * GetAllGreetingsCGI Class
 *
 * Returns the VXML snippet for playing all the greetings of the mailbox owner.
 *
 * @author Harippriya M Sivapatham
 * @version 1.0
 */
class GetAllGreetingsCGI : public VXMLCGICommand
{
public:
    /**
     * Ctor
     */
    GetAllGreetingsCGI( const UtlBoolean& requestIsFromWebUI,
                        const UtlString& mailbox,
                        const UtlString& status);

    /**
     * Virtual Destructor
     */
    virtual ~GetAllGreetingsCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out);

    OsStatus handleWebRequest( UtlString* out ) ;

        OsStatus handleOpenVXIRequest( UtlString* out ) ;

    OsStatus
    generateManageGreetingsHtml( const UtlString& greetingType,
                                                     const UtlString& defaultGreetingFilename,
                                                     const UtlString& activeGreetingType ,
                                                     const UtlString& displayName,
                                                     const UtlString& baseUrl,
                                                     UtlString& rHtmlContent,
                                                     UtlString& rSetActiveGreetingContent) const ;




protected:

private:

        UtlString m_mailboxIdentity;
    const UtlBoolean m_fromWeb ;
    const UtlString m_status ;
};

#endif //GET_ALL_GREETINGS_H
