//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _DialPadpanel_h_
#define _DialPadpanel_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "DialPadButton.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * A panel containing a telphone dial pad..
 */
class DialPadPanel : public wxPanel
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   /**
    * DialPadPanel contructor.
    */
   DialPadPanel(wxWindow* parent, const wxPoint& pos, const wxSize& size);

   /**
    * DialPadPanel destructor.
    */
   virtual ~DialPadPanel();

   void UpdateBackground(wxColor color);

/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    DECLARE_EVENT_TABLE()
        wxGridSizer* mpGridSizer;
        DialPadButton* mpButton[12];
        void createPhoneButton(int index, const wxString& bmpFile, const wxString& bmpDown, const int btnID, const int row, const int col, const enum TONE_ID toneId, const char charRepresentation);

};


#endif
