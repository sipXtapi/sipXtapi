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

#ifndef _MpAudioResourceConstructor_h_
#define _MpAudioResourceConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpResourceConstructor.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief MpAudioResourceConstructor is used to construct a specific audio resource type
*
*  MpAudioResourceConstructors is an abstract class.  Derived types construct a
*  specific type of MpAudioResource.
*
*/
class MpAudioResourceConstructor : public MpResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /** Constructor
     */
    MpAudioResourceConstructor(const UtlString& resourceType,
                               int minInputs = 1,
                               int maxInputs = 1,
                               int minOutputs = 1,
                               int maxOutputs = 1,
                               int samplesPerFrame = 80, 
                               int samplesPerSecond = 8000) :
      MpResourceConstructor(resourceType,
                            minInputs,
                            maxInputs,
                            minOutputs,
                            maxOutputs),
      mSamplesPerFrame(samplesPerFrame),
      mSamplesPerSecond(samplesPerSecond)
    {
    };

    /** Destructor
     */
    virtual ~MpAudioResourceConstructor(){};

/* ============================ MANIPULATORS ============================== */

    /// Create new resource(s)
    virtual OsStatus newResource(const UtlString& resourceName, 
                                 int maxResourcesToCreate,
                                 int& numResourcesCreated,
                                 MpResource* resourcesCreated[]) = 0;
     /**<
     *  Creates one or more resources as defined by the implementation of
     *  the resource constructor (typically of different resource types as
     *  opposed to duplicates).  Some resources are created in sets as 
     *  they work together in some way.  This interface allows the constructor
     *  to create multiple resources and assocate or initialize them together.
     *
     *  @param[in] resourceName - name to give the new resource, must be unique
     *             to the target flowgraph that this resource is to be inserted.
     *  @param[in] maxResourcesToCreate - the size of the resourcesCreated passed in
     *  @param[out] numResourcesCreated - the actual number of resource created and
     *             contained in the resourcesCreated array
     *  @param[out] resourcesCreated - array containing the created resources.
     */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    int mSamplesPerFrame;
    int mSamplesPerSecond;

    /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    /// Disable default constructor
    MpAudioResourceConstructor();

    /** Disabled copy constructor
     */
    MpAudioResourceConstructor(const MpAudioResourceConstructor& rMpAudioResourceConstructor);


    /** Disable assignment operator
     */
    MpAudioResourceConstructor& operator=(const MpAudioResourceConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpAudioResourceConstructor_h_
