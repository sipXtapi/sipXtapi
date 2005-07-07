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

#ifndef CHANGEPIN _H
#define CHANGEPIN_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
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
 * ForwardMessaeg Class
 *
 * @author John P. Coffey
 * @version 1.0
 */
class ChangePin : public CGICommand
{
public:
    /**
     * Ctor
     */
    ChangePin();

    /**
     * Virtual Destructor
     */
    virtual ~ChangePin();

    /** This does the work */
    virtual OsStatus execute (UtlString& result = NULL);

protected:

private:
};

#endif //CHANGEPIN_H

