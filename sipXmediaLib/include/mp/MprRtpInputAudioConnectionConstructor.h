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

#ifndef _MprRtpInputAudioConnectionConstructor_h_
#define _MprRtpInputAudioConnectionConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MpRtpInputAudioConnection.h>
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
    MprRtpInputAudioConnectionConstructor(int samplesPerFrame = 80, 
                           int samplesPerSecond = 8000) :
      MpAudioResourceConstructor(DEFAULT_RTP_INPUT_RESOURCE_TYPE,
                                 0, //minInputs,
                                 0, //maxInputs,
                                 0, //minOutputs,
                                 1, //maxOutputs,
                                 samplesPerFrame,
                                 samplesPerSecond)
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
        assert(mSamplesPerFrame > 0);
        assert(mSamplesPerSecond > 0);
        assert(maxResourcesToCreate >= 1);
        numResourcesCreated = 1;
        resourceArray[0] = new MpRtpInputAudioConnection(resourceName,
                                                             999, 
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
    MprRtpInputAudioConnectionConstructor(const MprRtpInputAudioConnectionConstructor& rMprRtpInputAudioConnectionConstructor);


    /** Disable assignment operator
     */
    MprRtpInputAudioConnectionConstructor& operator=(const MprRtpInputAudioConnectionConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprRtpInputAudioConnectionConstructor_h_
