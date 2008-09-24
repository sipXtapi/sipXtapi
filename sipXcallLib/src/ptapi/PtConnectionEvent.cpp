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
#include <string.h>

// APPLICATION INCLUDES
#include "ptapi/PtConnectionEvent.h"
#include "ptapi/PtConnection.h"
#include "tao/TaoClientTask.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtConnectionEvent::PtConnectionEvent(PtEvent::PtEventId eventId) :
PtCallEvent(eventId)
{
        setEventAddress(0);
        mpClient = NULL;
}

PtConnectionEvent::PtConnectionEvent(PtEvent::PtEventId eventId,
                                                        int metaCode,
                                                        const char* addressName,
                                                        const char* callId,
                                                        int sipResponseCode,
                                                        const char* sipResponseText,
                                                        const char* newCallId,
                                                        const char** oldCallIds,
                                                        int numOldCalls,
                                                        PtEvent::PtEventCause cause,
                                                        TaoClientTask *pClient) :
PtCallEvent(eventId, metaCode, callId, pClient, sipResponseCode, sipResponseText, newCallId, oldCallIds, numOldCalls)
{
        mEventCause = cause;
        setEventAddress(addressName);
}

PtConnectionEvent::PtConnectionEvent(TaoClientTask *pClient) :
PtCallEvent(pClient)
{
        setEventAddress(0);
}

// Copy constructor
PtConnectionEvent
::PtConnectionEvent(const PtConnectionEvent& rPtConnectionEvent)
:
PtCallEvent(rPtConnectionEvent)
{
        mEventCause = rPtConnectionEvent.mEventCause;
        setEventAddress(rPtConnectionEvent.mAddressName);
}

// Destructor
PtConnectionEvent::~PtConnectionEvent()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtConnectionEvent&
PtConnectionEvent::operator=(const PtConnectionEvent& rhs)
{
        if (this == &rhs)            // handle the assignment to self case
          return *this;

        PtCallEvent::operator=(rhs);
        setEventAddress(rhs.mAddressName);
        mEventCause = rhs.mEventCause;

   return *this;
}

void PtConnectionEvent::setEventAddress(const char* address)
{
        if (address)
        {
                int len = strlen(address);
                if (len > 127)
                        len = 127;
                strncpy(mAddressName, address, len);
                mAddressName[len] = 0;
        }
        else
                memset(mAddressName, 0, 128);
}

/* ============================ ACCESSORS ================================= */
// Return the terminal connection object associated with this event
PtStatus
PtConnectionEvent::getConnection(PtConnection& rConnection) const
{
        PtConnection conn(mpClient, mAddressName, mCallId);
        rConnection = conn;
        return PT_SUCCESS;
}
/* ============================ INQUIRY =================================== */
// PT_IMPLEMENT_CLASS_INFO(PtConnectionEvent, PtCallEvent)

/* //////////////////////////// PROTECTED ///////////////////////////////// */
// Protected constructor.

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
