//  
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
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
#include <mp/MpResourceMsg.h>
#include <mp/MpResampler.h>
#include <os/OsMsgPool.h>

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
*  device to which is sent output.
*
*  The MprToOutputDevice also maintains so called "copy queue" - a queue to
*  which a copies of buffers are sent. Only copies of buffers accepted by
*  the device are sent to the copy queue. This queue is usually used by echo
*  canceler.
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
                     MpOutputDeviceManager* deviceManager,
                     MpOutputDeviceHandle deviceId);

     /// Destructor
   virtual
   ~MprToOutputDevice();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Send message to enable/disable copy queue.
   static OsStatus enableCopyQ(const UtlString& namedResource, 
                               OsMsgQ& fgQ,
                               UtlBoolean enable);
     /**<
     *  @param[in] namedResource - the name of the resource to send a message to.
     *  @param[in] fgQ - the queue of the flowgraph containing the resource which
     *             the message is to be received by.
     *  @param[in] enable - TRUE to enable copy queue, FALSE to disable it.
     *  @returns the result of attempting to queue the message to this resource.
     */

     /// Send message to enable/disable copy queue.
   OsStatus enableCopyQ(UtlBoolean enable);
     /**<
     *  @param[in] enable - TRUE to enable copy queue, FALSE to disable it.
     *  @returns the result of attempting to queue the message to this resource.
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Get pointer to the copy queue.
   inline OsMsgQ *getCopyQ();

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
      MPRM_ENABLE_COPY_QUEUE = MpResourceMsg::MPRM_EXTERNAL_MESSAGE_START,
      MPRM_DISABLE_COPY_QUEUE
   } AddlResMsgTypes;

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond);

   MpOutputDeviceManager* mpOutputDeviceManager;
   UtlBoolean mFrameTimeInitialized;
   MpFrameTime mFrameTime;
   MpFrameTime mMixerBufferPosition;
   MpOutputDeviceHandle mDeviceId;
   MpResamplerBase *mpResampler;

   OsMsgQ mCopyQ;    ///< Queue for the copy of data sent to output device.
                     ///< It is usually used by acoustic echo canceler.
   OsMsgPool mCopyQPool; ///< Pool for messages to be sent to mCopyQ.
   UtlBoolean mIsCopyQEnabled; ///< Should we copy data to mCopyQ or not?

     /// @copydoc MpResource::handleMessage()
   virtual UtlBoolean handleMessage(MpResourceMsg& rMsg);

     /// Copy constructor (not implemented for this class)
   MprToOutputDevice(const MprToOutputDevice& rMprToOutputDevice);

     /// Assignment operator (not implemented for this class)
   MprToOutputDevice& operator=(const MprToOutputDevice& rhs);

};

/* ============================ INLINE METHODS ============================ */

OsMsgQ *MprToOutputDevice::getCopyQ()
{
   return &mCopyQ;
}

#endif  // _MprToOutputDevice_h_
