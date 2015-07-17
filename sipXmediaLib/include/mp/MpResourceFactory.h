//  
// Copyright (C) 2006-2015 SIPfoundry Inc.  All rights reserved.
//
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#ifndef _MpResourceFactory_h_
#define _MpResourceFactory_h_

// SYSTEM INCLUDES
#include <os/OsStatus.h>
#include <utl/UtlString.h>
#include <utl/UtlHashMap.h>

// APPLICATION INCLUDES
// DEFINES
#define DEFAULT_BRIDGE_RESOURCE_TYPE "MprBridge"
#define DEFAULT_FROM_FILE_RESOURCE_TYPE "MprFromFile"
#define DEFAULT_FROM_INPUT_DEVICE_RESOURCE_TYPE "MprFromInputDevice"
#define DEFAULT_TONE_GEN_RESOURCE_TYPE "MprToneGen"
#define DEFAULT_TO_OUTPUT_DEVICE_RESOURCE_TYPE "MprToOutputDevice"
#define DEFAULT_TO_OUTPUT_DEVICE_WITH_AEC_RESOURCE_TYPE "MprToOutputDevice+Aec"
#define DEFAULT_SPEEX_PREPROCESS_RESOURCE_TYPE "MprSpeexPreprocess"
#define DEFAULT_NULL_RESOURCE_TYPE "MprNull"
#define DEFAULT_HOOK_RESOURCE_TYPE "MprHook"
#define DEFAULT_RTP_INPUT_RESOURCE_TYPE "MpRtpInputConnection"
#define DEFAULT_MCAST_RTP_INPUT_RESOURCE_TYPE "MpMcastRtpInputConnection"
#define DEFAULT_DECODE_RESOURCE_TYPE "MprDecode"
#define DEFAULT_RTP_OUTPUT_RESOURCE_TYPE "MpRtpOutputConnection"
#define DEFAULT_ENCODE_RESOURCE_TYPE "MprEncode"
#define DEFAULT_RECORDER_RESOURCE_TYPE "MprRecorder"
#define DEFAULT_SPLITTER_RESOURCE_TYPE "MprSplitter"
#define DEFAULT_NULL_AEC_RESOURCE_TYPE "MprNullAec"
#define DEFAULT_VAD_RESOURCE_TYPE "MprVad"
#define DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_TYPE "MprVoiceActivityNotifier"
#define DEFAULT_DELAY_RESOURCE_TYPE "MprDelay"
#define DEFAULT_SPEAKER_SELECTOR_RESOURCE_TYPE "SpeakerSelector"
#define DEFAULT_TONE_DETECT_RESOURCE_TYPE "MprToneDetect"
#define DEFAULT_NOTCH_FILTER_RESOURCE_TYPE "MprNotchFilter"
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpResourceConstructor;
class MpResource;

/**
*  @brief MpResourceFactory is used to construct resources by type name.
*
*  MpResourceConstructors are added to the MpResourceFactory and registered 
*  with a resource type name.  Resources are then constructed using the
*  MpResourceFactory.
*
*/
class MpResourceFactory
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /** Constructor
     */
    MpResourceFactory();

    /** Destructor
     */
    virtual ~MpResourceFactory();

/* ============================ MANIPULATORS ============================== */

    /// Add a constructor for a new resource type
    OsStatus addConstructor(MpResourceConstructor& resourceConstructor);

    /// Create a new resource of given type
    OsStatus newResource(const UtlString& resourceType, 
                         const UtlString& newResourceName,
                         int maxResourcesToCreate,
                         int& numResourcesCreated,
                         MpResource* resourcesCreated[]) const;
     /**<
     *  Creates one or more resources as defined by the implementation of
     *  the named resource constructor (typically of different resource types as
     *  opposed to duplicates).  Some resources are created in sets as 
     *  they work together in some way.  This interface allows the constructor
     *  to create multiple resources and assocate or initialize them together.
     *
     *  @param[in] newResourceName - name to give the new resource, must be unique
     *             to the target flowgraph that this resource is to be inserted.
     *  @param[in] maxResourcesToCreate - the size of the resourcesCreated passed in
     *  @param[out] numResourcesCreated - the actual number of resource created and
     *             contained in the resourcesCreated array
     *  @param[out] resourcesCreated - array containing the created resources.
     */
/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

    UtlBoolean constructorExists(const UtlString& resourceType);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


    /** Disabled copy constructor
     */
    MpResourceFactory(const MpResourceFactory& rMpResourceFactory);


    /** Disable assignment operator
     */
    MpResourceFactory& operator=(const MpResourceFactory& rhs);

    
    UtlHashMap mConstructors;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpResourceFactory_h_
