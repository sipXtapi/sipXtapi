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

#ifndef AUTOATTENDANTCGI_H
#define AUTOATTENDANTCGI_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "net/Url.h"
#include "mailboxmgr/VXMLCGICommand.h"

// DEFINES
#define DEFAULT_AA_NAME    "operator"

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
class AutoAttendantCGI : public VXMLCGICommand
{
public:
    /**
     * Ctor
     */
    AutoAttendantCGI ( const Url& from, const UtlString& name, const char* digits );

    /**
     * Virtual Dtor
     */
    virtual ~AutoAttendantCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL);

protected:

private:
    const Url m_from;
    UtlString m_name;
    const char* m_digits;
};

#endif //AUTOATTENDANTCGI_H

