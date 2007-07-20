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

#ifndef _MprNullConstructor_h_
#define _MprNullConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MprNull.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief MprNullConstructor is used to construct a Null resource (e.g. /dev/null)
*
*/
class MprNullConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /** Constructor
     */
    MprNullConstructor(int maxInOutputs = 5,
                       int samplesPerFrame = 80, 
                       int samplesPerSecond = 8000) :
      MpAudioResourceConstructor(DEFAULT_NULL_RESOURCE_TYPE,
                                 0,
                                 maxInOutputs,
                                 0,
                                 maxInOutputs,
                                 samplesPerFrame,
                                 samplesPerSecond)
    {
    };

    /** Destructor
     */
    virtual ~MprNullConstructor(){};

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
        resourceArray[0] = new MprNull(resourceName,
                                           mMaxInputs,
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
    MprNullConstructor(const MprNullConstructor& rMprNullConstructor);


    /** Disable assignment operator
     */
    MprNullConstructor& operator=(const MprNullConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprNullConstructor_h_
