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

#ifndef _MprSplitterConstructor_h_
#define _MprSplitterConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MprSplitter.h>

// DEFINES
#ifndef DEFAULT_SPLITTER_MAX_OUTPUTS
#define DEFAULT_SPLITTER_MAX_OUTPUTS 4
#endif
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief MprSplitterConstructor is used to construct a bridge mixer resource
*
*/
class MprSplitterConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /** Constructor
     */
    MprSplitterConstructor(int minInOutputs = 1,
                         int maxOutputs = DEFAULT_SPLITTER_MAX_OUTPUTS,
                         int samplesPerFrame = 80, 
                         int samplesPerSecond = 8000) :
      MpAudioResourceConstructor(DEFAULT_SPLITTER_RESOURCE_TYPE,
                                 0,
                                 1,
                                 0,
                                 maxOutputs,
                                 samplesPerFrame,
                                 samplesPerSecond)
    {
    };

    /** Destructor
     */
    virtual ~MprSplitterConstructor(){};

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
        resourceArray[0] = new MprSplitter(resourceName,
                                           mMaxOutputs,
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
    MprSplitterConstructor(const MprSplitterConstructor& rMprSplitterConstructor);


    /** Disable assignment operator
     */
    MprSplitterConstructor& operator=(const MprSplitterConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprSplitterConstructor_h_
