//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _Buttonpanel_h_
#define _Buttonpanel_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "stdwx.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * Displays a button bar.
 */
class ButtonPanel : public wxPanel
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   /**
    * ButtonPanel contructor.
    */
   ButtonPanel(wxWindow* parent, const wxPoint& pos, const wxSize& size, bool bLogo);

   /**
    * ButtonPanel destructor.
    */
   virtual ~ButtonPanel();

   /**
    * Handset Button event handler
    */
   void OnHandsetClick(wxCommandEvent& event);

   /**
    * Hold Button event handler
    */
   void OnHoldButton(wxCommandEvent& event);

   /**
    * Transfer Button event handler.
    */
   void OnTransferButton(wxCommandEvent& event);
   
   /**
    * Mute button event hander
    */
   void OnMuteButton(wxCommandEvent& event);

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

    /**
    * The handset button accepts the call, when the state is ringing.  If the
    * state is connected, the handset button disconnects the call.
    */
    wxBitmapButton* mpButtonHandset;

    /**
    * Puts the call on hold.
    */
    wxBitmapButton* mpButtonHold;

    /**
    * Transfers a call (blind transfer right now).
    */
    wxBitmapButton* mpButtonTransfer;
    
    wxBitmapButton* mpButtonMute;
};


#endif
