//  
// Copyright (C) 2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2009 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MprHookConstructor_h_
#define _MprHookConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MprHook.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief MprHookConstructor is used to construct a Hook resource.
*
*/
class MprHookConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /** Constructor
     */
    MprHookConstructor(int maxInOutputs = 5)
    : MpAudioResourceConstructor(DEFAULT_HOOK_RESOURCE_TYPE,
                                 0, 1, //minInputs, maxInputs,
                                 0, 1) //minOutputs, maxOutputs
    {
    };

    /** Destructor
     */
    virtual ~MprHookConstructor(){};

/* ============================ MANIPULATORS ============================== */

    /// Create a new resource
    virtual OsStatus newResource(const UtlString& resourceName,
                                 int maxResourcesToCreate,
                                 int& numResourcesCreated,
                                 MpResource* resourceArray[])
    {
        assert(maxResourcesToCreate >= 1);
        numResourcesCreated = 1;
        resourceArray[0] = new MprHook(resourceName);
        resourceArray[0]->enable();
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
    MprHookConstructor(const MprHookConstructor& rMprHookConstructor);


    /** Disable assignment operator
     */
    MprHookConstructor& operator=(const MprHookConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprHookConstructor_h_
