// 
//
// Copyright (c) PingTel Corp. (work in progress)
//
// This is an unpublished work containing PingTel Corporation's confidential
// and proprietary information.  Disclosure, use or reproduction without
// written authorization of PingTel Corp. is prohibited.
//
//! lib=mailbox
//////////////////////////////////////////////////////////////////////////////
#ifndef RETRIEVECGI_H
#define RETRIEVECGI_H

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

class RetrieveCGI : public CGICommand
{
public:
    /**
     * Ctor
     */
    RetrieveCGI (	const UtlBoolean& requestIsFromWebUI,
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

