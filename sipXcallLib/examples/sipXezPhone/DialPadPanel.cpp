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
#include "DialPadPanel.h"
#include "sipXezPhoneSettings.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS

BEGIN_EVENT_TABLE(DialPadPanel, wxPanel)
END_EVENT_TABLE()

// Constructor
DialPadPanel::DialPadPanel(wxWindow* parent, const wxPoint& pos, const wxSize& size) :
   wxPanel(parent, IDR_DIALPADPANEL, pos, size, wxTAB_TRAVERSAL, "DialPadPanel")
{

    wxColor* pPanelColor = & (sipXezPhoneSettings::getInstance().getBackgroundColor());
    SetBackgroundColour(*pPanelColor);

    for (int index=0; index<12; index++)
    {
        mpButton[index] = NULL;
    }


    createPhoneButton(0, "res/1.bmp", "res/1_down.bmp", IDR_DIAL_BUTTON_1, 0, 0, ID_DTMF_1, '1');
    createPhoneButton(1, "res/2.bmp", "res/2_down.bmp", IDR_DIAL_BUTTON_2, 0, 1, ID_DTMF_2, '2');
    createPhoneButton(2, "res/3.bmp", "res/3_down.bmp", IDR_DIAL_BUTTON_3, 0, 2, ID_DTMF_3, '3');
    createPhoneButton(3, "res/4.bmp", "res/4_down.bmp", IDR_DIAL_BUTTON_4, 1, 0, ID_DTMF_4, '4');
    createPhoneButton(4, "res/5.bmp", "res/5_down.bmp", IDR_DIAL_BUTTON_5, 1, 1, ID_DTMF_5, '5');
    createPhoneButton(5, "res/6.bmp", "res/6_down.bmp", IDR_DIAL_BUTTON_6, 1, 2, ID_DTMF_6, '6');
    createPhoneButton(6, "res/7.bmp", "res/7_down.bmp", IDR_DIAL_BUTTON_7, 2, 0, ID_DTMF_7, '7');
    createPhoneButton(7, "res/8.bmp", "res/8_down.bmp", IDR_DIAL_BUTTON_8, 2, 1, ID_DTMF_8, '8');
    createPhoneButton(8, "res/9.bmp", "res/9_down.bmp", IDR_DIAL_BUTTON_9, 2, 2, ID_DTMF_9, '9');
    createPhoneButton(9, "res/star.bmp", "res/star_down.bmp", IDR_DIAL_BUTTON_STAR, 3, 0, ID_DTMF_STAR, '*');
    createPhoneButton(10, "res/0.bmp", "res/0_down.bmp", IDR_DIAL_BUTTON_0, 3, 1, ID_DTMF_0, '0');
    createPhoneButton(11, "res/gate.bmp", "res/gate_down.bmp", IDR_DIAL_BUTTON_GATE, 3, 2, ID_DTMF_POUND, '#');

}


void DialPadPanel::createPhoneButton(int index, const wxString& bmpFile, const wxString& bmpDown, const int btnID, const int row, const int col, const enum TONE_ID toneId, const char charRepresentation)
{
        static int lastRight = 0;
        static int lastBottom = 0;
        wxPoint pos;

        pos.x = lastRight;
        pos.y = lastBottom;

    wxBitmap bitmap(bmpFile,wxBITMAP_TYPE_BMP);
    bitmap.SetMask(new wxMask(bitmap, * (wxTheColourDatabase->FindColour("RED"))));

    mpButton[index] = new DialPadButton(this, btnID, bitmap, pos, wxSize(bitmap.GetWidth(),bitmap.GetHeight()),
                                        toneId, charRepresentation);

        if (bmpDown != "")
        {
                wxBitmap bitmap2(bmpDown,wxBITMAP_TYPE_BMP);
                bitmap2.SetMask(new wxMask(bitmap2, * (wxTheColourDatabase->FindColour("RED"))));
                mpButton[index]->SetBitmapSelected(bitmap2);
        }

    wxColor btnColor = (sipXezPhoneSettings::getInstance().getBackgroundColor());
    mpButton[index]->SetBackgroundColour(btnColor);


        lastRight = bitmap.GetWidth() + pos.x;
        if (col == 2)
        {
                lastBottom = bitmap.GetHeight() + pos.y;
                lastRight = 0;
        }
        return;
}

// Destructor
DialPadPanel::~DialPadPanel()
{
}

void DialPadPanel::UpdateBackground(wxColor color)
{
    SetBackgroundColour(color);
    for (int index=0; index<12; index++)
    {
        if (mpButton[index])
        {
            mpButton[index]->SetBackgroundColour(color);
        }
    }
}
