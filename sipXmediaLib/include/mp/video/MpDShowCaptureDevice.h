//  
// Copyright (C) 2006-2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 


#ifndef _MpDShowCaptureDevice_h_
#define _MpDShowCaptureDevice_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/video/MpCaptureDeviceBase.h"
#include "utl/UtlString.h"
#include "os/OsMsgQ.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

class VideoCapture;

/// Base class for video capture device classes.
class MpDShowCaptureDevice: public MpCaptureDeviceBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor.
   MpDShowCaptureDevice(const UtlString& deviceName);

   ~MpDShowCaptureDevice();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Initialize (open) device.
   virtual OsStatus initialize();
     /**<
     *  All parameters (such as fps, width, height) should be set before calling
     *  this function.
     */

     /// Uninitialize (close) device.
   virtual OsStatus unInitialize();

     /// Start producing video frames.
   virtual OsStatus startCapture();

     /// Stop producing video frames.
   virtual OsStatus stopCapture();

   virtual OsStatus setFrameSize(int width, int height);
   virtual OsStatus setFPS(float fps);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   virtual void getFrameSize(int& width, int& height) const;
   virtual float getFPS() const;

   /// Get device name as it is passed to constructor.
   virtual UtlString getDeviceName() const {return mDeviceName;}

   virtual OsMsgQ *getFramesQueue() const {return mpFrameQueue;}

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{


//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   MpDShowCaptureDevice(const MpDShowCaptureDevice&);
   MpDShowCaptureDevice& operator=(MpDShowCaptureDevice&);

   UtlString mDeviceName;
   VideoCapture* mpCapture;
   OsMsgQ* mpFrameQueue;

   mutable int mFrameWidth;
   mutable int mFrameHeight;
   mutable float mFPS;

   OsStatus applyFormat();

   struct FrameProxy;
   FrameProxy* mpFrameProxy;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpDShowCaptureDevice_h_
