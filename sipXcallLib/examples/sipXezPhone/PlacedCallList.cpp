// $Id$
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
#include "stdwx.h"
#include "PlacedCallList.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Constructor
PlacedCallList::PlacedCallList(wxWindow* parent, const wxPoint& pos, const wxSize& size) :
   CallList(parent, IDR_PLACED_CALL_LIST, pos, size)
{

}

// Destructor
PlacedCallList::~PlacedCallList()
{

}