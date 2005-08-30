//
// Copyright (C) 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
#ifndef _videopanel_h_
#define _videopanel_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "VideoWindow.h"
#include "PreviewWindow.h"
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
class VideoPanel : public wxPanel
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   /**
    * VideoPanel contructor.
    */
   VideoPanel(wxWindow* parent, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);

   /**
    * VideoPanel destructor.
    */
   virtual ~VideoPanel();

/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   DECLARE_EVENT_TABLE()
   
   wxPanel* mpVideoWindow;
   wxPanel* mpPreviewWindow;

};


#endif
