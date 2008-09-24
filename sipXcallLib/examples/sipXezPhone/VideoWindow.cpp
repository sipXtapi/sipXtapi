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

// APPLICATION INCLUDES
#include "stdwx.h"
#include "sipXmgr.h"
#include "VideoWindow.h"
#include "sipXezPhoneSettings.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
BEGIN_EVENT_TABLE(VideoWindow, wxPanel)
    //EVT_PAINT(VideoWindow::OnPaint) 
END_EVENT_TABLE()

// Constructor
VideoWindow::VideoWindow(wxWindow* parent, const wxPoint& pos, const wxSize& size) :
   wxPanel(parent, IDR_VIDEO_WINDOW, pos, size, wxTAB_TRAVERSAL, "VideoWindow")
{
    wxColor* wxBlack = wxTheColourDatabase->FindColour("BLACK");
    SetBackgroundColour(*wxBlack);
#ifdef VIDEO    
    sipXmgr::getInstance().setVideoWindow((void*)GetHWND());
#endif
}

void VideoWindow::OnPaint(wxPaintEvent& event)
{
    {
      wxPaintDC dc(this);
    }

#ifdef VIDEO
    if (sipXmgr::getInstance().getCurrentCall())
    {
        sipxCallUpdateVideoWindow(sipXmgr::getInstance().getCurrentCall(), (SIPX_WINDOW_HANDLE)GetHWND());
    }
#endif
}

// Destructor
VideoWindow::~VideoWindow()
{
}
