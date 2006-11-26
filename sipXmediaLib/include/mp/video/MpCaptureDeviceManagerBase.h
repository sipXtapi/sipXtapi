//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 


#ifndef _MpCaptureDeviceManagerBase_h_
#define _MpCaptureDeviceManagerBase_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "os/OsStatus.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpCaptureDeviceBase;


/// @brief Base class for all video capture device managers. It define common
/// interface for all capture device managers.
class MpCaptureDeviceManagerBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor
     /**
     *  This constructor should initialize all member variables, but should not
     *  initialize underlying video system. All initialization touching
     *  OS functions should be done in initialize() method.
     */
   MpCaptureDeviceManagerBase()
   {};

     /// Make destructor virtual for all successor classes.
   virtual
   ~MpCaptureDeviceManagerBase()
   {};

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Initialize this manager.
   virtual OsStatus initialize() =0;
     /**<
     *  This function should be called before any other functions of video
     *  capture device manager, i.e right after constructor. It should do all
     *  initialization necessary for normal manager usage. E.g. it could
     *  enumerate all devices.
     */

     /// Uninitialize this manager.
   virtual OsStatus unInitialize() =0;

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Return new capture device by its name.
   virtual MpCaptureDeviceBase *getDeviceByName(const UtlString &deviceName) =0;
     /**<
     *  @note Capture device object should be deleted manually!
     */

     /// Return new capture device by its number.
   virtual MpCaptureDeviceBase *getDeviceByNum(int deviceNum) =0;
     /**<
     *  @note Capture device object should be deleted manually!
     */

     /// Return number of video capture devices we could get from this manager.
   virtual int getDeviceCount() const =0;

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

#endif  // _MpCaptureDeviceManagerBase_h_
