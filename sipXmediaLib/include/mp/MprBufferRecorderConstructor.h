//  
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>

#ifndef _MprBufferRecorderConstructor_h_
#define _MprBufferRecorderConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MprBufferRecorder.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief MprBufferRecorderConstructor is used to construct a Buffer Recorder resource
*/
class MprBufferRecorderConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /** Constructor
     */
    MprBufferRecorderConstructor(int samplesPerFrame = 80, 
                                 int samplesPerSecond = 8000) 
    : MpAudioResourceConstructor(DEFAULT_BUFFER_RECORDER_RESOURCE_TYPE,
                                 0, //minInputs,
                                 1, //maxInputs,
                                 1, //minOutputs,
                                 1, //maxOutputs,
                                 samplesPerFrame,
                                 samplesPerSecond)
    {
    };

    /** Destructor
     */
    virtual ~MprBufferRecorderConstructor(){};

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
        resourceArray[0] = 
           new MprBufferRecorder(resourceName,
                                 mSamplesPerFrame,
                                 mSamplesPerSecond);
        (resourceArray[0])->disable();
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
    MprBufferRecorderConstructor(
       const MprBufferRecorderConstructor& rMprBufferRecorderConstructor);


    /** Disable assignment operator
     */
    MprBufferRecorderConstructor& operator=(const MprBufferRecorderConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprBufferRecorderConstructor_h_
