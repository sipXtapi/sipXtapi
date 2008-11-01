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

#ifndef _MprBridgeConstructor_h_
#define _MprBridgeConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MprBridge.h>

// DEFINES
#ifndef DEFAULT_BRIDGE_MAX_IN_OUTPUTS
#define DEFAULT_BRIDGE_MAX_IN_OUTPUTS 20
#endif
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief MprBridgeConstructor is used to construct a bridge mixer resource
*
*/
class MprBridgeConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

      /// Constructor
    MprBridgeConstructor(int minInOutputs = 1,
                         int maxInOutputs = DEFAULT_BRIDGE_MAX_IN_OUTPUTS,
                         UtlBoolean mixSilence=TRUE,
                         MprBridge::AlgType algorithm=MprBridge::ALG_LINEAR)
    : MpAudioResourceConstructor(DEFAULT_BRIDGE_RESOURCE_TYPE,
                                 minInOutputs, maxInOutputs, //minInputs, maxInputs,
                                 minInOutputs, maxInOutputs) //minOutputs, maxOutputs
    , mMixSilence(mixSilence)
    , mAlgorithm(algorithm)
    {
    };

      /// Destructor
    virtual ~MprBridgeConstructor(){};

/* ============================ MANIPULATORS ============================== */

      /// Create a new resource
    virtual OsStatus newResource(const UtlString& resourceName,
                                 int maxResourcesToCreate,
                                 int& numResourcesCreated,
                                 MpResource* resourceArray[])
    {
        assert(maxResourcesToCreate >= 1);
        numResourcesCreated = 1;
        resourceArray[0] = new MprBridge(resourceName, mMaxInputs,
                                         mMixSilence, mAlgorithm);
        resourceArray[0]->enable();
        return(OS_SUCCESS);
    }

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   UtlBoolean         mMixSilence;
   MprBridge::AlgType mAlgorithm;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

      /// Disabled copy constructor
    MprBridgeConstructor(const MprBridgeConstructor& rMprBridgeConstructor);


      /// Disable assignment operator
    MprBridgeConstructor& operator=(const MprBridgeConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprBridgeConstructor_h_
