//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifndef _MprToOutputDevice_h_
#define _MprToOutputDevice_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <mp/MpAudioResource.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpOutputDeviceManager;

/**
*  @brief Resource in which input media from source outside the flowgraph
*         is introduced.
*
*  The MprToOutputDevice get frames of media from its MpOutputDeviceManager.
*  The MpOutputDeviceManager provides some simple buffering of frames from 
*  each device for a small window of time.  It is the MprToOutputDevice
*  responsibility to decide how current to keep (e.g. delay to prevent
*  starvation) with the frames in the MpOutputDeviceManager for the specific
*  device from which to recieve input.
*/
class MprToOutputDevice : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor
   MprToOutputDevice(const UtlString& rName, 
                     int samplesPerFrame, 
                     int samplesPerSec,
                     MpOutputDeviceManager* deviceManager,
                     MpOutputDeviceHandle deviceId);

     /// Destructor
   virtual
   ~MprToOutputDevice();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame=80,
                                     int samplesPerSecond=8000);

   MpOutputDeviceManager* mpOutputDeviceManager;
   UtlBoolean mFrameTimeInitialized;
   MpFrameTime mFrameTime;
   MpOutputDeviceHandle mDeviceId;

     /// Copy constructor (not implemented for this class)
   MprToOutputDevice(const MprToOutputDevice& rMprToOutputDevice);

     /// Assignment operator (not implemented for this class)
   MprToOutputDevice& operator=(const MprToOutputDevice& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprToOutputDevice_h_
