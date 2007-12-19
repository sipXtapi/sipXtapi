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
    MprBufferRecorderConstructor() 
    : MpAudioResourceConstructor(DEFAULT_BUFFER_RECORDER_RESOURCE_TYPE,
                                 0, 1, //minInputs, maxInputs,
                                 1, 1) //minOutputs, maxOutputs
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
        assert(maxResourcesToCreate >= 1);
        numResourcesCreated = 1;
        resourceArray[0] = new MprBufferRecorder(resourceName);
        resourceArray[0]->disable();
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
