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

#ifndef SIPHUNTGROUPFILTER_H
#define SIPHUNTGROUPFILTER_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsMutex.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipMessage;

class SipHuntGroupFilter
{
public:
    /**
     * Singleton Accessor
     *
     * @return
     */
    static SipHuntGroupFilter* getInstance();

    /**
     * Virtual Destructor
     */
    virtual ~SipHuntGroupFilter();

    /** This does the work */
    OsStatus reorderContacts (
        const SipMessage& message, 
        SipMessage& finalResponse);

protected:
    // protected constructor (singleton pattern)
    SipHuntGroupFilter();

    // singleton creation mutex
    static OsMutex sLockMutex;

    // pointer to the singleton instance
    static SipHuntGroupFilter* spInstance;

    // static bool indicating whether we have any huntgroups
    static UtlBoolean sHuntGroupsDefined;

private:
   int getQValues(int numContacts, 
                      SipMessage& response, 
                      double** qv);

   OsStatus randomizeIndex(int count, int** index);

   OsStatus resetQValues(int numContacts, 
                        SipMessage& response, 
                        int* idx,
                        double* qv);



};
#endif // SIPHUNTGROUPFILTER_H

