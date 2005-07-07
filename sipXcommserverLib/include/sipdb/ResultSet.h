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
#ifndef RESULTSET_H
#define RESULTSET_H

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include "os/OsStatus.h"
#include "utl/UtlDList.h"
#include "utl/UtlHashMap.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class ResultSet
{
public:
    ResultSet ();

    virtual ~ResultSet();

    int getSize() const;

    OsStatus getIndex(
        const int& index, 
        UtlHashMap& rRecord) const;

    void addValue(const UtlHashMap& record);

    void clear();

    void destroyAll();

private:
    UtlDList mCachedRows;
};

#endif //RESULTSET_H
