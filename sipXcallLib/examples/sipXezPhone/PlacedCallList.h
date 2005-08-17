//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
#ifndef _PlacedCallList_h_
#define _PlacedCallList_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "CallList.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * List control for Placed calls.
 */
class PlacedCallList : public CallList
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   /**
    * PlacedCallList contructor.
    */
   PlacedCallList(wxWindow* parent, const wxPoint& pos, const wxSize& size);

   /**
    * PlacedCallList destructor.
    */
   virtual ~PlacedCallList();

/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};


#endif
