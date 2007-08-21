//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 


#ifndef _MpCaptureDeviceBase_h_
#define _MpCaptureDeviceBase_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsStatus.h"
#include "utl/UtlString.h"
#include "os/OsMsgQ.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

/// Base class for video capture device classes.
class MpCaptureDeviceBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor.
   MpCaptureDeviceBase() {};

     /// Make destructor virtual for all child classes.
   virtual
   ~MpCaptureDeviceBase() {};

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Initialize (open) device.
   virtual OsStatus initialize() =0;
     /**<
     *  All parameters (such as fps, width, height) should be set before calling
     *  this function.
     */

     /// Uninitialize (close) device.
   virtual OsStatus unInitialize() =0;

     /// Start producing video frames.
   virtual OsStatus startCapture() =0;

     /// Stop producing video frames.
   virtual OsStatus stopCapture() =0;

   virtual OsStatus setFrameSize(int width, int height) =0;
   virtual OsStatus setFPS(float fps) =0;

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   virtual void getFrameSize(int& width, int& height) const =0;
   virtual float getFPS() const =0;

     /// Get device name as it is passed to constructor.
   virtual UtlString getDeviceName() const =0;

   virtual OsMsgQ *getFramesQueue() const =0;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{


//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpCaptureDeviceBase_h_
