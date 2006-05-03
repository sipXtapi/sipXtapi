//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _DialPadButton_h_
#define _DialPadButton_h_

// SYSTEM INCLUDES
#include <wx/listctrl.h>
// APPLICATION INCLUDES
#include "tapi/sipXtapi.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * Buttons for the dial pad.
 */
class DialPadButton : public wxBitmapButton
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   /**
    * DialPadButton contructor.
    */
   DialPadButton(wxWindow* parent, int id, const wxBitmap& bitmap, const wxPoint& pos, const wxSize& size, const enum TONE_ID toneId, const char charVal);

   /**
    * DialPadButton destructor.
    */
   virtual ~DialPadButton();
   
  void OnMouseDown(wxMouseEvent& event);
  void OnMouseUp(wxMouseEvent& event);

/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    DECLARE_EVENT_TABLE()
    const enum TONE_ID mToneId;
    const char mCharVal;

};


#endif
