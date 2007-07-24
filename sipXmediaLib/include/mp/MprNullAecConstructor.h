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

#ifndef _MprNullAecConstructor_h_
#define _MprNullAecConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MprNullAec.h>
#include <mp/MprAudioFrameBuffer.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief MprNullAecConstructor is used to construct a Null AEC resource 
*
*/
class MprNullAecConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /** Constructor
     */
    MprNullAecConstructor(int samplesPerFrame = 80, 
                          int samplesPerSecond = 8000) :
      MpAudioResourceConstructor(DEFAULT_NULL_AEC_RESOURCE_TYPE,
                                 0,
                                 1,
                                 0,
                                 1,
                                 samplesPerFrame,
                                 samplesPerSecond)
    {
    };

    /** Destructor
     */
    virtual ~MprNullAecConstructor(){};

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
        numResourcesCreated = 2;

        UtlString bufferResourceName(resourceName);
        bufferResourceName.append(AEC_OUTPUT_BUFFER_RESOURCE_NAME_SUFFIX);
        MprAudioFrameBuffer* outBufferResource = new MprAudioFrameBuffer(bufferResourceName,
                                                   mSamplesPerFrame,
                                                   mSamplesPerSecond,
                                                   3);
        resourceArray[1] = outBufferResource;

        resourceArray[0] = new MprNullAec(resourceName,
                                          mSamplesPerFrame,
                                          mSamplesPerSecond,
                                          *outBufferResource);
        (resourceArray[0])->disable();


        (resourceArray[1])->enable();


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
    MprNullAecConstructor(const MprNullAecConstructor& rMprNullAecConstructor);


    /** Disable assignment operator
     */
    MprNullAecConstructor& operator=(const MprNullAecConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprNullAecConstructor_h_
