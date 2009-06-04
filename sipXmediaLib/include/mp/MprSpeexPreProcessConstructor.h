//  
// Copyright (C) 2006-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifndef _MprSpeexPreProcessConstructor_h_
#define _MprSpeexPreProcessConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MprSpeexPreProcess.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief MprSpeexPreProcessConstructor is used to construct a SpeexPreProcess
*         resource.
*
*/
class MprSpeexPreProcessConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

      /// Constructor
    MprSpeexPreProcessConstructor(UtlBoolean agc=FALSE, UtlBoolean noiseReduction=FALSE)
    : MpAudioResourceConstructor(DEFAULT_SPEEX_PREPROCESS_RESOURCE_TYPE,
                                 1, 1, //minInputs, maxInputs,
                                 1, 1) //minOutputs, maxOutputs
    , mAgc(agc)
    , mNoiseReduction(noiseReduction)
    {
    };

      /// Destructor
    virtual ~MprSpeexPreProcessConstructor(){};

/* ============================ MANIPULATORS ============================== */

      /// Create a new resource
    virtual OsStatus newResource(const UtlString& resourceName,
                                 int maxResourcesToCreate,
                                 int& numResourcesCreated,
                                 MpResource* resourceArray[])
    {
        assert(maxResourcesToCreate >= 1);
        numResourcesCreated = 1;
        MprSpeexPreprocess *pPreprocess = new MprSpeexPreprocess(resourceName);
        pPreprocess->setAGC(mAgc);
        pPreprocess->setNoiseReduction(mNoiseReduction);
        resourceArray[0] = pPreprocess;
        resourceArray[0]->enable();
        return(OS_SUCCESS);
    }

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   UtlBoolean mAgc;             ///< Is AGC enabled by default?
   UtlBoolean mNoiseReduction;  ///< Is noise reduction enabled by default?

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

      /// Disable copy constructor
    MprSpeexPreProcessConstructor(const MprSpeexPreProcessConstructor& rMprSpeexPreProcessConstructor);


      /// Disable assignment operator
    MprSpeexPreProcessConstructor& operator=(const MprSpeexPreProcessConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprSpeexPreProcessConstructor_h_
