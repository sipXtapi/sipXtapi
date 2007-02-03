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

#ifndef SPECIALAAMENU_H
#define SPECIALAAMENU_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
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

class SpecialAAMenuCGI : public VXMLCGICommand
{
public:
    /**
     * Ctor
     */
    SpecialAAMenuCGI (const UtlString& option);

    /**
     * Virtual Destructor
     */
    virtual ~SpecialAAMenuCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL);

protected:

private:
    const UtlString mOption;
};

#endif //SPECIALAAMENU_H

