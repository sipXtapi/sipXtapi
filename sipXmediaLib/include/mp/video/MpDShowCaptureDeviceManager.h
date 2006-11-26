//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 


#ifndef _MpDShowCaptureDeviceManager_h_
#define _MpDShowCaptureDeviceManager_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/video/MpCaptureDeviceManagerBase.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

/// DirectShow capture device manager.
class MpDShowCaptureDeviceManager : public MpCaptureDeviceManagerBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor
   MpDShowCaptureDeviceManager();
     /**<
     *  This constructor should initialize all member variables, but should not
     *  initialize underlying video system. All initialization touching
     *  OS functions should be done in initialize() method.
     */

     /// Virtual destructor
   virtual
   ~MpDShowCaptureDeviceManager();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Initialize this manager.
   virtual OsStatus initialize();
     /**<
     *  This function should be called before any other functions of video
     *  capture device manager, i.e right after constructor. It should do all
     *  initialization necessary for normal manager usage. E.g. it could
     *  enumerate all devices.
     */

     /// Uninitialize this manager.
   virtual OsStatus unInitialize();

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Return new capture device by its name.
   virtual MpCaptureDeviceBase *getDeviceByName(const UtlString &deviceName);
     /**<
     *  @note Capture device object should be deleted manually!
     */

     /// Return new capture device by its number.
   virtual MpCaptureDeviceBase *getDeviceByNum(int deviceNum);
     /**<
     *  @note Capture device object should be deleted manually!
     */

     /// Return number of video capture devices we could get from this manager.
   virtual int getDeviceCount() const;

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

#endif  // _MpDShowCaptureDeviceManager_h_
