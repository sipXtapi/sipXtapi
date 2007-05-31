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

    /// Create a new resource
    virtual MpResource* newResource(const UtlString& resourceName) = 0;

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
