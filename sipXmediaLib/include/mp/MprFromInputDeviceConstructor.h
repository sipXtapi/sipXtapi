//  
// Copyright (C) 2006-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#ifndef _MprFromInputDeviceConstructor_h_
#define _MprFromInputDeviceConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MprFromInputDevice.h>
//#include <mp/MprFromMic.h>

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
*  @brief MprFromInputDeviceConstructor is used to construct a FromInputDevice
*         resource.
*
*/
class MprFromInputDeviceConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

      /// Constructor
    MprFromInputDeviceConstructor(MpInputDeviceManager *defaultManager,
                                  MpInputDeviceHandle   defaultDriver = 1,
                                  int samplesPerFrame = 80, 
                                  int samplesPerSecond = 8000)
    : MpAudioResourceConstructor(DEFAULT_FROM_INPUT_DEVICE_RESOURCE_TYPE,
                                 0, //minInputs,
                                 1, //maxInputs,
                                 1, //minOutputs,
                                 1, //maxOutputs,
                                 samplesPerFrame,
                                 samplesPerSecond)
    , mpDefaultManager(defaultManager)
    , mDefaultDriver(defaultDriver)
    {
    };

      /// Destructor
    virtual ~MprFromInputDeviceConstructor(){};

/* ============================ MANIPULATORS ============================== */

      /// Create a new resource
    virtual OsStatus newResource(const UtlString& resourceName,
                                 int maxResourcesToCreate,
                                 int& numResourcesCreated,
                                 MpResource* resourceArray[])
    {
        assert(mSamplesPerFrame > 0);
        assert(mSamplesPerSecond > 0);
        assert(maxResourcesToCreate >= 1);
        numResourcesCreated = 1;
        resourceArray[0] = new MprFromInputDevice(resourceName,
                                                 mSamplesPerFrame,
                                                 mSamplesPerSecond,
                                                 mpDefaultManager,
                                                 mDefaultDriver);
        (resourceArray[0])->disable();
        return(OS_SUCCESS);
    }

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   MpInputDeviceManager *mpDefaultManager; ///< Manager that will be passed to
                                           ///< newly created resources.
   MpInputDeviceHandle   mDefaultDriver;   ///< Device that will be used for
                                           ///< newly created resources.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

      /// Disable copy constructor
    MprFromInputDeviceConstructor(const MprFromInputDeviceConstructor& rMprFromInputDeviceConstructor);


      /// Disable assignment operator
    MprFromInputDeviceConstructor& operator=(const MprFromInputDeviceConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprFromInputDeviceConstructor_h_
