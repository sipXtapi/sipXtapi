//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "ptapi/PtEventListener.h"
#include "ptapi/PtTerminalListener.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtTerminalListener::PtTerminalListener(const char * pTerminalName, PtEventMask* pMask)
        : PtEventListener(pMask)
{
   if (pTerminalName)
   {
           int len = strlen(pTerminalName);

           mpTerminalName = new char[len + 1];

           strcpy(mpTerminalName, pTerminalName);
   }
   else
                mpTerminalName = 0;
}

// Copy constructor
PtTerminalListener::PtTerminalListener(const PtTerminalListener& rPtTerminalListener)
{
        if (rPtTerminalListener.mpTerminalName)
        {
           int len = strlen(rPtTerminalListener.mpTerminalName);

           mpTerminalName = new char[len + 1];

           strcpy(mpTerminalName, rPtTerminalListener.mpTerminalName);
        }
        else
                mpTerminalName = 0;
}

// Destructor
PtTerminalListener::~PtTerminalListener()
{
        if (mpTerminalName)
        {
                delete[] mpTerminalName;
                mpTerminalName = 0;
        }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtTerminalListener&
PtTerminalListener::operator=(const PtTerminalListener& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

        if (rhs.mpTerminalName)
        {
           int len = strlen(rhs.mpTerminalName);

           mpTerminalName = new char[len + 1];

           strcpy(mpTerminalName, rhs.mpTerminalName);
        }
        else
                mpTerminalName = 0;

        return *this;
}

PtStatus PtTerminalListener::setTerminalName(const char* name)
{
        if (name)
        {
                if (mpTerminalName)
                        delete[] mpTerminalName;
                int len = strlen(name);

                mpTerminalName = new char[len + 1];
                if (mpTerminalName)
                {
                        strcpy (mpTerminalName, name);
                        return PT_SUCCESS;
                }
                else
                        return PT_RESOURCE_UNAVAILABLE;
        }

        return PT_INVALID_ARGUMENT;
}

void PtTerminalListener::terminalEventTransmissionEnded(const PtTerminalEvent& rEvent)
{
}

/* ============================ ACCESSORS ================================= */
PtStatus PtTerminalListener::getTerminalName(char* name, int len)
{
        if (name && len > 0)
        {
                if (mpTerminalName)
                {
                        int bytes = strlen(mpTerminalName);
                        bytes = (bytes > len) ? len : bytes;

                        memset(name, 0, len);
                        strncpy (name, mpTerminalName, bytes);
                        return PT_SUCCESS;
                }
                else
                        return PT_RESOURCE_UNAVAILABLE;
        }

        return PT_INVALID_ARGUMENT;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */
 PT_IMPLEMENT_CLASS_INFO(PtTerminalListener, PtEventListener)

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
