//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _videowindow_h_
#define _videowindow_h_

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
 * This panel contains the preview video and remote video windows
 * and the associated controls.
 */
class VideoWindow : public wxPanel
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   /**
    * VideoWindow contructor.
    */
   VideoWindow(wxWindow* parent, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);

   /**
    * VideoPanel destructor.
    */
   virtual ~VideoWindow();

   void OnPaint(wxPaintEvent& event);
/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   DECLARE_EVENT_TABLE()
   

};


#endif
