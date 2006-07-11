//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "stdwx.h"
#include "CallList.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Constructor
CallList::CallList(wxWindow* parent, int id, const wxPoint& pos, const wxSize& size) :
   wxListCtrl(parent, id, pos, size, wxTAB_TRAVERSAL)
{

}

// Destructor
CallList::~CallList()
{
}
