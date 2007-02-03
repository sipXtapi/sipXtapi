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

#ifndef SETACTIVEGREETING_H
#define SETACTIVEGREETING_H

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
 * SetActiveGreeting Class
 *
 * @author John P. Coffey
 * @version 1.0
 */
class SetActiveGreeting : public VXMLCGICommand
{
public:
    /**
     * Ctor
     */
    SetActiveGreeting();

    /**
     * Virtual Destructor
     */
    virtual ~SetActiveGreeting();

    /** This does the work */
    virtual OsStatus execute ();

protected:

private:
};

#endif //SETACTIVEGREETING_H

