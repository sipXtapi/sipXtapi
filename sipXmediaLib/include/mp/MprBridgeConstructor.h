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
#define DEFAULT_BRIDGE_MAX_IN_OUTPUTS 10
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

    /** Constructor
     */
    MprBridgeConstructor(int minInOutputs = 1,
                         int maxInOutputs = DEFAULT_BRIDGE_MAX_IN_OUTPUTS,
                         int samplesPerFrame = 80, 
                         int samplesPerSecond = 8000) :
      MpAudioResourceConstructor(DEFAULT_BRIDGE_RESOURCE_TYPE,
                                 minInOutputs,
                                 maxInOutputs,
                                 minInOutputs,
                                 maxInOutputs,
                                 samplesPerFrame,
                                 samplesPerSecond)
    {
    };

    /** Destructor
     */
    virtual ~MprBridgeConstructor(){};

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
        resourceArray[0] = new MprBridge(resourceName,
                                             mMaxInputs,
                                             mSamplesPerFrame,
                                             mSamplesPerSecond);
        (resourceArray[0])->enable();
        return(OS_SUCCESS);
    }

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    /** Disabled copy constructor
     */
    MprBridgeConstructor(const MprBridgeConstructor& rMprBridgeConstructor);


    /** Disable assignment operator
     */
    MprBridgeConstructor& operator=(const MprBridgeConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprBridgeConstructor_h_
