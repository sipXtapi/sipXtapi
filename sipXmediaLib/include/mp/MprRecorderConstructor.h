//  
// Copyright (C) 2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2009 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
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
                                 1, 1, //minInputs, maxInputs,
                                 0, 1) //minOutputs, maxOutputs
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
