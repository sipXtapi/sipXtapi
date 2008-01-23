//  
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#ifndef _MprFromInputDevice_h_
#define _MprFromInputDevice_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <mp/MpAudioResource.h>
#include <mp/MpResampler.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpInputDeviceManager;

/**
*  @brief Resource in which input media from source outside the flowgraph
*         is introduced.
*
*  The MprFromInputDevice get frames of media from its MpInputDeviceManager.
*  The MpInputDeviceManager provides some simple buffering of frames from 
*  each device for a small window of time.  It is the MprFromInputDevice
*  responsibility to decide how current to keep (e.g. delay to prevent
*  starvation) with the frames in the MpInputDeviceManager for the specific
*  device from which to recieve input.
*/
class MprFromInputDevice : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor
   MprFromInputDevice(const UtlString& rName, 
                      MpInputDeviceManager* deviceManager,
                      MpInputDeviceHandle deviceId);

     /// Destructor
   virtual
   ~MprFromInputDevice();

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
                                     int samplesPerFrame,
                                     int samplesPerSecond);

   MpInputDeviceManager* mpInputDeviceManager;
   UtlBoolean mFrameTimeInitialized;
   MpFrameTime mPreviousFrameTime;
   MpInputDeviceHandle mDeviceId;
   MpResampler mResampler;

     /// Copy constructor (not implemented for this class)
   MprFromInputDevice(const MprFromInputDevice& rMprFromInputDevice);

     /// Assignment operator (not implemented for this class)
   MprFromInputDevice& operator=(const MprFromInputDevice& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprFromInputDevice_h_
