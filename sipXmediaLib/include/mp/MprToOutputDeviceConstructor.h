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

#ifndef _MprToOutputDeviceConstructor_h_
#define _MprToOutputDeviceConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MprToOutputDevice.h>
//#include <mp/MprToSpkr.h>

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
*  @brief MprToOutputDeviceConstructor is used to construct a ToOutputDevice
*         resource.
*
*/
class MprToOutputDeviceConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

      /// Constructor
    MprToOutputDeviceConstructor(MpOutputDeviceManager *defaultManager,
                                 MpOutputDeviceHandle   defaultDriver = 1,
                                 int samplesPerFrame = 80, 
                                 int samplesPerSecond = 8000)
    : MpAudioResourceConstructor(DEFAULT_TO_OUTPUT_DEVICE_RESOURCE_TYPE,
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
    virtual ~MprToOutputDeviceConstructor(){};

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
        resourceArray[0] = new MprToOutputDevice(resourceName,
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

   MpOutputDeviceManager *mpDefaultManager; ///< Manager that will be passed to
                                            ///< newly created resources.
   MpOutputDeviceHandle   mDefaultDriver;   ///< Device that will be used for
                                            ///< newly created resources.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

      /// Disable copy constructor
    MprToOutputDeviceConstructor(const MprToOutputDeviceConstructor& rMprToOutputDeviceConstructor);


      /// Disable assignment operator
    MprToOutputDeviceConstructor& operator=(const MprToOutputDeviceConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprToOutputDeviceConstructor_h_
