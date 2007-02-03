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

#ifndef CGICOMMAND_H
#define CGICOMMAND_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsStatus.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlString;

/**
 * Abstract Command Interface.  Classic command pattern
 *
 * @author John P. Coffey
 * @version 1.0
 */
class CGICommand
{

public:
    /** Ctor */
    CGICommand () {};
    /**
     * Virtual Destructor
     */
    virtual ~CGICommand() {};

    /** Declare business method */
    virtual OsStatus execute (UtlString* out = NULL) = 0;
};

#endif //CGICOMMAND_H

