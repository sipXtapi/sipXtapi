//  
// Copyright (C) 2006-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifndef _MprToOutputDeviceWithAecConstructor_h_
#define _MprToOutputDeviceWithAecConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MprToOutputDevice.h>
#include <mp/MprSpeexEchoCancel.h>

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
*  @brief MprToOutputDeviceWithAecConstructor is used to construct
*         a ToOutputDevice resource with AEC attached to it.
*
*  Currently only Speex AEC is supported here.
*/
class MprToOutputDeviceWithAecConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

      /// Constructor
    MprToOutputDeviceWithAecConstructor(MpOutputDeviceManager *defaultManager,
                                        MpOutputDeviceHandle   defaultDriver = 1)
    : MpAudioResourceConstructor(DEFAULT_TO_OUTPUT_DEVICE_WITH_AEC_RESOURCE_TYPE,
                                 1, 1, //minInputs, maxInputs,
                                 0, 1) //minOutputs, maxOutputs
    , mpDefaultManager(defaultManager)
    , mDefaultDriver(defaultDriver)
    {
        OsSysLog::add(FAC_MP, PRI_DEBUG,
           "MprToOutputDeviceWithAecConstructor::MprToOutputDeviceWithAecConstructor");
    };

      /// Destructor
    virtual ~MprToOutputDeviceWithAecConstructor(){};

/* ============================ MANIPULATORS ============================== */

      /// Create a new resource
    virtual OsStatus newResource(const UtlString& resourceName,
                                 int maxResourcesToCreate,
                                 int& numResourcesCreated,
                                 MpResource* resourceArray[])
    {
        OsSysLog::add(FAC_MP, PRI_DEBUG,
           "MprToOutputDeviceWithAecConstructor::newResource");

        assert(maxResourcesToCreate >= 2);
        numResourcesCreated = 2;
        MprToOutputDevice *pToOutput = new MprToOutputDevice(resourceName,
                                                             mpDefaultManager,
                                                             mDefaultDriver);
        pToOutput->enableCopyQ(TRUE);
        resourceArray[0] = pToOutput;
        resourceArray[0]->disable();

        resourceArray[1] = new MprSpeexEchoCancel(resourceName+AEC_NAME_SUFFIX,
                                                  pToOutput->getCopyQ());
        resourceArray[1]->enable();
        return OS_SUCCESS;
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
    MprToOutputDeviceWithAecConstructor(const MprToOutputDeviceWithAecConstructor& rMprToOutputDeviceWithAecConstructor);


      /// Disable assignment operator
    MprToOutputDeviceWithAecConstructor& operator=(const MprToOutputDeviceWithAecConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprToOutputDeviceWithAecConstructor_h_
