//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
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
   void OnSpeakerSlider(wxEvent& event);

   /**
    * Event handler for the microphone gain slider control.
    */
   void OnMicrophoneSlider(wxEvent& event);

/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   DECLARE_EVENT_TABLE()

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        wxSlider* mpVolumeControl;
        wxSlider* mpMicGainControl;
};


#endif
