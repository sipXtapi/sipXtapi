//
// Copyright (C) 2005-2006 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004, 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "stdwx.h"
#include "sipXmgr.h"
#include "DialPadButton.h"
#include <os/OsCallback.h>
#include <os/OsTimer.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
BEGIN_EVENT_TABLE(DialPadButton, wxBitmapButton)
   EVT_LEFT_DOWN(DialPadButton::OnMouseDown)
#ifndef __MACH__
   /* EVT_LEFT_UP is broken on OS X. Disable it and use a timer instead. */
   EVT_LEFT_UP(DialPadButton::OnMouseUp)
#endif
END_EVENT_TABLE()

// Constructor
DialPadButton::DialPadButton(wxWindow* parent, int id, const wxBitmap& bitmap, const wxPoint& pos, const wxSize& size, const enum TONE_ID toneId, const char charVal) :
   wxBitmapButton(parent, id, bitmap, pos, size, 0),
   mToneId(toneId),
   mCharVal(charVal)
{
}

// Destructor
DialPadButton::~DialPadButton()
{

}

#ifdef __MACH__
static void buttonUpCallback(const int userData, const int eventData)
{
    if (sipXmgr::getInstance().getCurrentCall() > 0)
    {
        sipxCallStopTone(sipXmgr::getInstance().getCurrentCall());
    }
}

static OsCallback callback(0, buttonUpCallback);
static OsTime callbackDelay(350);
static OsTimer callbackTimer(callback);
#endif

void DialPadButton::OnMouseDown(wxMouseEvent& event)
{
    if (sipXmgr::getInstance().getCurrentCall() > 0)
    {
        sipxCallStartTone(sipXmgr::getInstance().getCurrentCall(), mToneId, true, true);
    }
#ifdef __MACH__
    callbackTimer.oneshotAfter(callbackDelay);
#endif
    event.Skip();
}

void DialPadButton::OnMouseUp(wxMouseEvent& event)
{
    if (sipXmgr::getInstance().getCurrentCall() > 0)
    {
        sipxCallStopTone(sipXmgr::getInstance().getCurrentCall());
    }
    event.Skip();
}
