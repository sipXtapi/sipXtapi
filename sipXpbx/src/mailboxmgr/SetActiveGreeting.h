// 
//
// Copyright (c) PingTel Corp. (work in progress)
//
// This is an unpublished work containing PingTel Corporation's confidential
// and proprietary information.  Disclosure, use or reproduction without
// written authorization of PingTel Corp. is prohibited.
//
//! lib=mailboxmgr
//////////////////////////////////////////////////////////////////////////////
#ifndef SETACTIVEGREETING_H
#define SETACTIVEGREETING_H

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
 * SetActiveGreeting Class
 *
 * @author John P. Coffey
 * @version 1.0
 */
class SetActiveGreeting : public CGICommand
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

