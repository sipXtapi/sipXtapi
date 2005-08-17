//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
#ifndef _Buttonpanel_h_
#define _Buttonpanel_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
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
   ButtonPanel(wxWindow* parent, const wxPoint& pos, const wxSize& size);

   /**
    * ButtonPanel destructor.
    */
   virtual ~ButtonPanel();

   /**
    * Handset Button event handler
    */
   void OnHandsetClick(wxEvent& event);

   /**
    * Hold Button event handler
    */
   void OnHoldButton(wxEvent& event);

   /**
    * Transfer Button event handler.
    */
   void OnTransferButton(wxEvent& event);
   
   /**
    * Mute button event hander
    */
   void OnMuteButton(wxEvent& event);

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
