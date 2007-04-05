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
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief MprBridgeConstructor is used to contruct a specific audio resource type
*
*  MprBridgeConstructors is an abstract class.  Derived types construct a
*  specific type of MpAudioResource.
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
                         int maxInOutputs = 1,
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
    virtual MpResource* newResource(const UtlString& resourceName)
    {
        assert(mSamplesPerFrame > 0);
        assert(mSamplesPerSecond > 0);
        return(new MprBridge(resourceName,
                             mSamplesPerFrame,
                             mSamplesPerSecond));
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
