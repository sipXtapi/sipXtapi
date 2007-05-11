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
//#include <mp/MprFromInputDevice.h>
#include <mp/MprFromMic.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

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
    MprFromInputDeviceConstructor(int samplesPerFrame = 80, 
                                  int samplesPerSecond = 8000)
    : MpAudioResourceConstructor(DEFAULT_FROM_INPUT_DEVICE_RESOURCE_TYPE,
                                 0, //minInputs,
                                 1, //maxInputs,
                                 1, //minOutputs,
                                 1, //maxOutputs,
                                 samplesPerFrame,
                                 samplesPerSecond)
    {
    };

      /// Destructor
    virtual ~MprFromInputDeviceConstructor(){};

/* ============================ MANIPULATORS ============================== */

      /// Create a new resource
    virtual MpResource* newResource(const UtlString& resourceName)
    {
        assert(mSamplesPerFrame > 0);
        assert(mSamplesPerSecond > 0);

        // TODO: use MprFromInputDevice instead
        return(new MprFromMic(resourceName,
                              mSamplesPerFrame,
                              mSamplesPerSecond,
                              MpMisc.pMicQ));
    }

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

      /// Disable copy constructor
    MprFromInputDeviceConstructor(const MprFromInputDeviceConstructor& rMprFromInputDeviceConstructor);


      /// Disable assignment operator
    MprFromInputDeviceConstructor& operator=(const MprFromInputDeviceConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprFromInputDeviceConstructor_h_
