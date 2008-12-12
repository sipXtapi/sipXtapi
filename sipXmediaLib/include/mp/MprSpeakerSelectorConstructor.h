//  
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifndef _MprSpeakerSelectorConstructor_h_
#define _MprSpeakerSelectorConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MprSpeakerSelector.h>

// DEFINES
/// Number of Speaker Selector inputs and outputs.
/// Must be less or equal then number of inputs in the Bridge minus number
/// of resources initially connected to the Bridge.
#define DEFAULT_SPEAKER_SELECTOR_MAX_IN_OUTPUTS  18
/// Maximum number of speakers we want to mix.
#define DEFAULT_SPEAKER_SELECTOR_MAX_SPEAKERS    3

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief MprSpeakerSelectorConstructor is used to construct Speaker Selector resource
*/
class MprSpeakerSelectorConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

      /// Constructor
   MprSpeakerSelectorConstructor(int maxInOutputs = DEFAULT_SPEAKER_SELECTOR_MAX_IN_OUTPUTS,
                                 int maxActiveSpeakers = DEFAULT_SPEAKER_SELECTOR_MAX_SPEAKERS)
   : MpAudioResourceConstructor(DEFAULT_SPEAKER_SELECTOR_RESOURCE_TYPE,
                                1, maxInOutputs, //minInputs, maxInputs,
                                1, maxInOutputs) //minOutputs, maxOutputs
   , mMaxActiveSpeakers(maxActiveSpeakers)
   {
   }

     /// Destructor
   virtual ~MprSpeakerSelectorConstructor() {}

/* ============================ MANIPULATORS ============================== */

     /// Create a new resource
   virtual OsStatus newResource(const UtlString& resourceName,
                                int maxResourcesToCreate,
                                int& numResourcesCreated,
                                MpResource* resourceArray[])
   {
      MprSpeakerSelector *pResource = new MprSpeakerSelector(resourceName,
                                                             mMaxInputs,
                                                             mMaxActiveSpeakers);

      // Return created resource
      assert(maxResourcesToCreate >= 1);
      numResourcesCreated = 1;
      resourceArray[0] = pResource;
      resourceArray[0]->enable();

      return OS_SUCCESS;
   }

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   int    mMaxActiveSpeakers; ///< Number of speakers we want to mix.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Disabled copy constructor
   MprSpeakerSelectorConstructor(const MprSpeakerSelectorConstructor& rMprSpeakerSelectorConstructor);

     /// Disabled assignment operator
   MprSpeakerSelectorConstructor& operator=(const MprSpeakerSelectorConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprSpeakerSelectorConstructor_h_
