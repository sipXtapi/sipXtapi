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

#ifndef _MprRtpInputAudioConnectionConstructor_h_
#define _MprRtpInputAudioConnectionConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MpRtpInputAudioConnection.h>
#include <mp/MprToSpkr.h>
#include <mp/MpPlcSilence.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief MprRtpInputAudioConnectionConstructor is used to construct a ToOutputDevice resource
*
*/
class MprRtpInputAudioConnectionConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /** Constructor
     */
    MprRtpInputAudioConnectionConstructor()
    : MpAudioResourceConstructor(DEFAULT_RTP_INPUT_RESOURCE_TYPE,
                                 0, 0, //minInputs, maxInputs,
                                 0, 1) //minOutputs, maxOutputs
    {
    };

    /** Destructor
     */
    virtual ~MprRtpInputAudioConnectionConstructor(){};

/* ============================ MANIPULATORS ============================== */

    /// Create a new resource
    virtual OsStatus newResource(const UtlString& resourceName,
                                 int maxResourcesToCreate,
                                 int& numResourcesCreated,
                                 MpResource* resourceArray[])
    {
        assert(maxResourcesToCreate >= 1);
        numResourcesCreated = 1;
        resourceArray[0] = new MpRtpInputAudioConnection(resourceName, 999);
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
    MprRtpInputAudioConnectionConstructor(const MprRtpInputAudioConnectionConstructor& rMprRtpInputAudioConnectionConstructor);


    /** Disable assignment operator
     */
    MprRtpInputAudioConnectionConstructor& operator=(const MprRtpInputAudioConnectionConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprRtpInputAudioConnectionConstructor_h_
