//  
// Copyright (C) 2006-2015 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#ifndef _MprToneDetectConstructor_h_
#define _MprToneDetectConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MprToneDetect.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief MprToneDetectConstructor is used to construct a ToneDetect resource
*
*/
class MprToneDetectConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /** Constructor
     */
    MprToneDetectConstructor()
    : MpAudioResourceConstructor(DEFAULT_TONE_DETECT_RESOURCE_TYPE,
                                 0, 1, //minInputs, maxInputs,
                                 1, 1) //minOutputs, maxOutputs
    {
    };

    /** Destructor
     */
    virtual ~MprToneDetectConstructor(){};

/* ============================ MANIPULATORS ============================== */

    /// Create a new resource
    virtual OsStatus newResource(const UtlString& resourceName,
                                 int maxResourcesToCreate,
                                 int& numResourcesCreated,
                                 MpResource* resourceArray[])
    {
        assert(maxResourcesToCreate >= 1);
        numResourcesCreated = 1;
        resourceArray[0] = new MprToneDetect(resourceName);
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
    MprToneDetectConstructor(const MprToneDetectConstructor& rMprToneDetectConstructor);


    /** Disable assignment operator
     */
    MprToneDetectConstructor& operator=(const MprToneDetectConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprToneDetectConstructor_h_
