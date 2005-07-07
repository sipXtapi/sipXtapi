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
#ifndef AUTOATTENDANTCGI_H
#define AUTOATTENDANTCGI_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "net/Url.h"
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
 * Mailbox Class
 *
 * @author John P. Coffey
 * @version 1.0
 */
class AutoAttendantCGI : public CGICommand
{
public:
    /**
     * Ctor
     */
    AutoAttendantCGI ( const Url& from, const char* digits );

    /**
     * Virtual Dtor
     */
    virtual ~AutoAttendantCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL);

protected:

private:
    const Url m_from;
    const char* m_digits;
};

#endif //AUTOATTENDANTCGI_H

