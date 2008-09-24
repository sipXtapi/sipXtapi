//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _UtlCopyableSList_h_
#define _UtlCopyableSList_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlSList.h"
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
 * UtlCopyableSList is a linked list that provides a copy contructor and an 
 * equals operator.  A deep copy of the array elements is performed.
 * 
 * @see UtlSList
 */
class UtlCopyableSList : public UtlSList
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   
   /**
    * Constructor
    */
   UtlCopyableSList();

   /**
    * Copy Constructor
    */
   UtlCopyableSList(const UtlCopyableSList& rhs);

   /**
    * Destructor
    */
   virtual ~UtlCopyableSList() ;


/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

   /**
    * Assignment operator
    */
   UtlCopyableSList& operator=(const UtlCopyableSList& rhs);

    /**
     * Append the designated containable object to the end of this list.
     * 
     * @return the object if successful, otherwise null
     */
    virtual UtlContainable* append(UtlCopyableContainable* obj) ;

    /// Insert the designated containable object at the designated position.
    virtual UtlContainable* insertAt(size_t N,           ///< zero-based position obj should be
                                     UtlCopyableContainable* obj ///< object to insert at N
                                     );
    /**<
     * It is an error to specify N > entries()
     *
     * @return obj if successful, NULL if N > entries
     */

    /**
     * Inserts the designated containable object at the end postion (tailer).
     * 
     * @return the object if successful, otherwise null
     */
    virtual UtlContainable* insert(UtlCopyableContainable* obj) ;

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

#endif    // _UtlCopyableSList_h_

