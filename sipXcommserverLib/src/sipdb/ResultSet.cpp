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

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include "sipdb/ResultSet.h"
#include "utl/UtlHashMapIterator.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

ResultSet::ResultSet() 
{}
 
ResultSet::~ResultSet() 
{
    // make sure we destroy the records 
    // we allocated in the addValue method
    destroyAll();
}

int
ResultSet::getSize() const
{
    return mCachedRows.entries();
}

void
ResultSet::destroyAll()
{
    // the pRecord is actually a UtlHashMap
    UtlHashMap* pRecord;
    while ( mCachedRows.entries() > 0 )
    {
        pRecord = (UtlHashMap*)mCachedRows.first();
        pRecord->destroyAll();
        mCachedRows.destroy( pRecord );
    }
}

void
ResultSet::clear()
{
    // finally empty the list
    mCachedRows.removeAll();
}

OsStatus 
ResultSet::getIndex(
    const int& index, 
    UtlHashMap& rRecord) const
{
    // empty out the record before populating it
    // they are not mine to destroy so don't destry them
    rRecord.removeAll();

    OsStatus result = OS_FAILED;
    if ((index >= 0) && (index < ((int)(mCachedRows.entries()))))
    {
        UtlHashMap *m = (UtlHashMap*)mCachedRows.at(index);
        m->copyInto(rRecord);
        result = OS_SUCCESS;
    }
    return result;
}

void
ResultSet::addValue( const UtlHashMap& record )
{
    UtlHashMap*     pNewRecord = new UtlHashMap() ;
    UtlContainable* pObj ;

    // Proceed with shallow copy
    UtlHashMapIterator itor(const_cast<UtlHashMap&>(record)) ;
    while ((pObj = (UtlContainable*) itor()) != NULL)
    {
        pNewRecord->insertKeyAndValue(itor.key(), itor.value()) ;
    }
    mCachedRows.append(pNewRecord) ;
}
