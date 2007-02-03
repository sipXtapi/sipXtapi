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

#ifndef GetNavWebCGI_H
#define GetNavWebCGI_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "utl/UtlString.h"
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
 * Mailbox Class
 *
 * @author John P. Coffey
 * @version 1.0
 */
class GetNavWebCGI : public VXMLCGICommand
{
public:
    /**
     * Ctor
     */
    GetNavWebCGI (      const UtlString& mailboxIdentity );

    /**
     * Virtual Dtor
     */
    virtual ~GetNavWebCGI();

    /** This does the work */
    virtual OsStatus execute ( UtlString* out = NULL );


protected:

private:
        UtlString m_mailboxIdentity;
};

#endif //GetNavWebCGI_H
