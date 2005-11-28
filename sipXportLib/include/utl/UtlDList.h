//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


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

   static UtlContainableType TYPE ;    /** < Class type used for runtime checking */ 
   
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


} ;

/* ============================ INLINE METHODS ============================ */\

#endif    // _UtlDList_h_

