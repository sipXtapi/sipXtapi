//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
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
        void createPhoneButton(const wxString& bmpFile, const wxString& bmpDown, const int btnID, const int row, const int col, const enum TONE_ID toneId, const char charRepresentation);

};


#endif
