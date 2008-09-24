//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _UtlCopyableSortedList_h_
#define _UtlCopyableSortedList_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlSortedList.h"
#include "utl/UtlCopyableContainable.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * UtlCopyableSortedList is a linked list that provides a copy contructor and an 
 * equals operator.  A deep copy of the array elements is performed.
 * 
 * @see UtlSortedList
 */
class UtlCopyableSortedList : public UtlSortedList
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   
   /**
    * Constructor
    */
   UtlCopyableSortedList();

   /**
    * Copy Constructor
    */
   UtlCopyableSortedList(const UtlCopyableSortedList& rhs);

   /**
    * Destructor
    */
   virtual ~UtlCopyableSortedList() ;


/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

   /**
    * Assignment operator
    */
   UtlCopyableSortedList& operator=(const UtlCopyableSortedList& rhs);

    /**
     * Inserts the designated containable object into the list
     * 
     * @return the object if successful, otherwise null
     */
    virtual UtlContainable* insert(UtlCopyableContainable* obj);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


   /**
    * Get the ContainableType for the hash bag as a contained object.
    */
   virtual UtlContainableType getContainableType() const;

   static UtlContainableType TYPE ;    /** < Class type used for runtime checking */ 
   
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


} ;

/* ============================ INLINE METHODS ============================ */\

#endif    // _UtlCopyableSortedList_h_

