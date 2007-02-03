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

#ifndef RETRIEVECGI_H
#define RETRIEVECGI_H

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

class RetrieveCGI : public VXMLCGICommand
{
public:
    /**
     * Ctor
     */
    RetrieveCGI (       const UtlBoolean& requestIsFromWebUI,
                                        const UtlString& from );

    /**
     * Virtual Dtor
     */
    virtual ~RetrieveCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL);

        OsStatus handleWebRequest( UtlString* out ) ;

        OsStatus handleOpenVXIRequest( UtlString* out ) ;

protected:

private:
    const UtlString m_from;

        const UtlBoolean m_fromWebUI;
};

#endif //RETRIEVECGI_H
