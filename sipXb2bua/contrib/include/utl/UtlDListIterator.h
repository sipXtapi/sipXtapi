//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _UtlDListIterator_h_
#define _UtlDListIterator_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlSListIterator.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * UtlDListIterator allows developers to iterate (walks through) a UtlDList.
 * 
 * @see UtlIterator
 * @see UtlDList
 */
class UtlDList;
     
class UtlDListIterator : public UtlSListIterator
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /**
    * Constructor accepting a source UtlDList 
    */
   UtlDListIterator(const UtlDList& list) ;


   /**
     * Destructor
     */
    virtual ~UtlDListIterator();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    friend class UtlDList;
    


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    UtlDListIterator();
};

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlDListIterator_h_


