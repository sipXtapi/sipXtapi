//  
// Copyright (C) 2006-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#ifndef _MprRtpOutputConnectionConstructor_h_
#define _MprRtpOutputConnectionConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MpRtpOutputConnection.h>
#include <mp/MprToSpkr.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief MprRtpOutputConnectionConstructor is used to construct a ToOutputDevice resource
*
*/
class MprRtpOutputConnectionConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /** Constructor
     */
    MprRtpOutputConnectionConstructor()
    : MpAudioResourceConstructor(DEFAULT_RTP_OUTPUT_RESOURCE_TYPE,
                                 0, 1, //minInputs, maxInputs,
                                 0, 0) //minOutputs, maxOutputs
    {
    };

    /** Destructor
     */
    virtual ~MprRtpOutputConnectionConstructor(){};

/* ============================ MANIPULATORS ============================== */

    /// Create a new resource
    virtual OsStatus newResource(const UtlString& resourceName,
                                 int maxResourcesToCreate,
                                 int& numResourcesCreated,
                                 MpResource* resourceArray[])
    {
        assert(maxResourcesToCreate >= 1);
        numResourcesCreated = 1;
        resourceArray[0] = new MpRtpOutputConnection(resourceName, -1, NULL);
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
    MprRtpOutputConnectionConstructor(const MprRtpOutputConnectionConstructor& rMprRtpOutputConnectionConstructor);


    /** Disable assignment operator
     */
    MprRtpOutputConnectionConstructor& operator=(const MprRtpOutputConnectionConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprRtpOutputConnectionConstructor_h_
