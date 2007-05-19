//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _UtlSortedListIterator_h_
#define _UtlSortedListIterator_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlListIterator.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlContainable;
class UtlSortedList;

/**
 * UtlSortedListIterator allows developers to iterator (walks through) an 
 * UtlSortedList.
 * 
 * @see UtlIterator
 * @see UtlSortedList
 */
class UtlSortedListIterator : public UtlListIterator
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /**
     * Constructor accepting a source UtlSortedList 
     */
    UtlSortedListIterator(const UtlSortedList& list);
   


/* ============================ MANIPULATORS ============================== */


    /**
     * Find the designated object, and reset the iterator so that it is the current position.
     * 
     * @return The  element or NULL if no more elements are available.
     */
    virtual UtlContainable* findNext(const UtlContainable* objectToFind);

    
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    friend class UtlSortedList;
    

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlSortedListIterator_h_


