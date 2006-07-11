//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "ptapi/PtComponent.h"
#include "ptapi/PtPhoneTextDisplay.h"
#include "ps/PsButtonTask.h"
#include "tao/TaoClientTask.h"
#include "tao/TaoEvent.h"
#include "tao/TaoEventDispatcher.h"
#include "tao/TaoString.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtPhoneTextDisplay::PtPhoneTextDisplay()
: mType(PtComponent::TEXT_DISPLAY)
{
}

// Copy constructor
PtPhoneTextDisplay::PtPhoneTextDisplay(const PtPhoneTextDisplay& rPtPhoneTextDisplay)
: mType(PtComponent::TEXT_DISPLAY)
{
}

// Destructor
PtPhoneTextDisplay::~PtPhoneTextDisplay()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtPhoneTextDisplay&
PtPhoneTextDisplay::operator=(const PtPhoneTextDisplay& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

PtStatus PtPhoneTextDisplay::setDisplay(char* displayInfo, int x, int y)
{
        return PT_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

PtStatus PtPhoneTextDisplay::getDisplay(char*& rpContents, int maxLen, int x, int y)
{
        return PT_SUCCESS;
}

PtStatus PtPhoneTextDisplay::getDisplayColumns(int& rNumColumns)
{
        return PT_SUCCESS;
}

PtStatus PtPhoneTextDisplay::getDisplayRows(int& rNumRows)
{
        return PT_SUCCESS;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
