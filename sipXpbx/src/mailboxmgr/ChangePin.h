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

