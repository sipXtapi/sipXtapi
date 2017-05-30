//  
// Copyright (C) 2006-2017 SIPez LLC.  All rights reserved.
//  
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MprRecorderConstructor_h_
#define _MprRecorderConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MprRecorder.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief MprRecorderConstructor is used to construct a Buffer Recorder resource
*/
class MprRecorderConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /** Constructor
     */
    MprRecorderConstructor() 
    : MpAudioResourceConstructor(DEFAULT_RECORDER_RESOURCE_TYPE,
                                 1, MAXIMUM_RECORDER_CHANNELS, //minInputs, maxInputs,
                                 0, MAXIMUM_RECORDER_CHANNELS) //minOutputs, maxOutputs
    {
    };

    /** Destructor
     */
    virtual ~MprRecorderConstructor(){};

/* ============================ MANIPULATORS ============================== */

    /// Create a new resource
    virtual OsStatus newResource(const UtlString& resourceName,
                                 int maxResourcesToCreate,
                                 int& numResourcesCreated,
                                 MpResource* resourceArray[])
    {
        assert(maxResourcesToCreate >= 1);
        numResourcesCreated = 1;
        resourceArray[0] = new MprRecorder(resourceName);
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
    MprRecorderConstructor(
       const MprRecorderConstructor& rMprRecorderConstructor);


    /** Disable assignment operator
     */
    MprRecorderConstructor& operator=(const MprRecorderConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprRecorderConstructor_h_
