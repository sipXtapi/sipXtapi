//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _UtlDList_h_
#define _UtlDList_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlSList.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * UtlDList is a doubly linked list designed to contain any number of
 * UtlContainable derived object.  For more information on lists, please
 * look at UtlSList.
 * 
 * @see UtlSList
 */
class UtlDList : public UtlSList
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   
   /**
    * Constructor
    */
   UtlDList();


   /**
    * Destructor
    */
   virtual ~UtlDList() ;


/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


   /**
    * Get the ContainableType for the hash bag as a contained object.
    */
   virtual UtlContainableType getContainableType() const;

   
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   static UtlContainableType TYPE ;    /** < Class type used for runtime checking */ 

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


} ;

/* ============================ INLINE METHODS ============================ */\

#endif    // _UtlDList_h_

