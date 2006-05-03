//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _volumecontrolpanel_h_
#define _volumecontrolpanel_h_

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
 * The Panel that contains the volume / mic gain controls.
 */
class VolumeControlPanel : public wxPanel
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   /**
    * VolumeControlPanel contructor.
    */
   VolumeControlPanel(wxWindow* parent, const wxPoint& pos, const wxSize& size, const int volumeLevel, const int gainLevel);

   /**
    * VolumeControlPanel destructor.
    */
   virtual ~VolumeControlPanel();

/* ============================ MANIPULATORS ============================== */

   /**
    * Event handler for the speaker slider control.
    */
   void OnSpeakerSlider(wxScrollEvent& event);

   /**
    * Event handler for the microphone gain slider control.
    */
   void OnMicrophoneSlider(wxScrollEvent& event);

   void UpdateBackground(wxColour color);

/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   DECLARE_EVENT_TABLE()

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        wxSlider* mpVolumeControl;
        wxSlider* mpMicGainControl;
        wxStaticBitmap* mpMicBitmap;
        wxStaticBitmap* mpSpeakerBitmap;
};


#endif
