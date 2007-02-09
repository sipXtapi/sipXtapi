//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
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
*  @brief Resource in which input media from source outside the flowgraph is introduced.
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
                       int samplesPerFrame, 
                       int samplesPerSec,
                       MpInputDeviceManager& deviceManager,
                       int deviceId);

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
                                      int samplesPerFrame=80,
                                      int samplesPerSecond=8000)
    {
       // Inline for review purposes.  Missing logic to react to frequent
       // starvation.
       UtlBoolean bufferOutput = FALSE;
       assert(mpInputDeviceManager);

       // Milliseconds per frame:
       int frameTimeInterval = samplesPerFrame * 1000 / samplesPerSecond;

       MpBufPtr buffer;
       // Inline for discussion and review purposes only
       if(!mFrameTimeInitialized)
       {

           // Start with a frame behind.  Possible need smarter
           // decision for starting.
           mPreviousFrameTime = mpInputDeviceManager->getCurrentFrameTime();
           mPreviousFrameTime -= (2 * frameTimeInterval);
       }


       mPreviousFrameTime += frameTimeInterval;
       int numFramesNotPlayed;
       int numFramedBufferedBehind;
       OsStatus getResult =
           mpInputDeviceManager->getFrame(mDeviceId,
                                          buffer,
                                          mPreviousFrameTime,
                                          numFramesNotPlayed,
                                          numFramedBufferedBehind);

           
       if(!mFrameTimeInitialized)
       {
           if(getResult == OS_SUCCESS)
           {
               mFrameTimeInitialized = TRUE;
           }

           if(numFramesNotPlayed > 1)
           {
               // TODO: now is a good time to adjust and get a newer
               // frame
               // could increment mPreviousFrameTime and getFrame again
           }
       }

       if(buffer.isValid())
       {
           outBufs[0] = buffer;
           bufferOutput = TRUE;
       }

       return(bufferOutput);
    };

    MpInputDeviceManager* mpInputDeviceManager;
    UtlBoolean mFrameTimeInitialized;
    int mPreviousFrameTime;
    int mDeviceId;

    /// Copy constructor (not implemented for this class)
    MprFromInputDevice(const MprFromInputDevice& rMprFromInputDevice);

    /// Assignment operator (not implemented for this class)
    MprFromInputDevice& operator=(const MprFromInputDevice& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprFromInputDevice_h_
