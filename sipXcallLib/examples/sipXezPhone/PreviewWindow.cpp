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
#include "PreviewWindow.h"
#include "sipXezPhoneSettings.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
BEGIN_EVENT_TABLE(PreviewWindow, wxPanel)
    EVT_PAINT(PreviewWindow::OnPaint) 
END_EVENT_TABLE()

// Constructor
PreviewWindow::PreviewWindow(wxWindow* parent, const wxPoint& pos, const wxSize& size) :
   wxPanel(parent, IDR_PREVIEW_WINDOW, pos, size, wxTAB_TRAVERSAL, "PreviewWindow")
{
    wxColor* wxBlack = wxTheColourDatabase->FindColour("BLACK");
    SetBackgroundColour(*wxBlack);
#ifdef VIDEO
    sipXmgr::getInstance().setPreviewWindow((void*)GetHWND());
#endif
    
}

void PreviewWindow::OnPaint(wxPaintEvent& event)
{
    {
      wxPaintDC dc(this);
    }
#ifdef VIDEO
    if (sipXmgr::getInstance().getCurrentCall())
    {
#ifdef _WIN32
        sipxConfigUpdatePreviewWindow(sipXmgr::getInstance().getSipxInstance(), (SIPX_WINDOW_HANDLE)GetHWND());
#endif
    }
#endif
}

// Destructor
PreviewWindow::~PreviewWindow()
{
}
