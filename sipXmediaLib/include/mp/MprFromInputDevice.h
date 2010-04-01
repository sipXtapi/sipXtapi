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
#include <mp/MpResourceMsg.h>
#include <mp/MpBridgeAlgBase.h> // For MpBridgeGain type

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

     /// Send message to set gain to apply to the audio.
   static OsStatus setGain(const UtlString& namedResource, 
                           OsMsgQ& fgQ,
                           MpBridgeGain gain);
     /**<
     *  @param[in] gain - the gain value (multiplier) to apply to the incoming
     *             audio. Set to MP_BRIDGE_GAIN_PASSTHROUGH to pass audio
     *             unchanged.
     */

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

   typedef enum
   {
      MPRM_SET_GAIN = MpResourceMsg::MPRM_EXTERNAL_MESSAGE_START,
   } AddlMsgTypes;

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
   MpResamplerBase *mpResampler;
   MpBridgeGain mGain;
   MpBridgeAccum *mpGainBuffer;

     /// Handle resource messages for this resource.
   virtual UtlBoolean handleMessage(MpResourceMsg& rMsg);

     /// Actually set gain to apply to incoming audio.
   virtual UtlBoolean handleSetGain(MpBridgeGain gain);
     /**<
     *  @see setGain() for explanation of parameters.
     */

     /// Copy constructor (not implemented for this class)
   MprFromInputDevice(const MprFromInputDevice& rMprFromInputDevice);

     /// Assignment operator (not implemented for this class)
   MprFromInputDevice& operator=(const MprFromInputDevice& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprFromInputDevice_h_
