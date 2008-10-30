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

#ifndef _MprVoiceActivityNotifierConstructor_h_
#define _MprVoiceActivityNotifierConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MprVoiceActivityNotifier.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief MprVoiceActivityNotifierConstructor is used to construct a Decode resource
*/
class MprVoiceActivityNotifierConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

      /// Constructor
   MprVoiceActivityNotifierConstructor()
   : MpAudioResourceConstructor(DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_TYPE,
                                1, 1, //minInputs, maxInputs,
                                0, 1) //minOutputs, maxOutputs
   {
   }

     /// Destructor
   virtual ~MprVoiceActivityNotifierConstructor() {}

/* ============================ MANIPULATORS ============================== */

     /// Create a new resource
   virtual OsStatus newResource(const UtlString& resourceName,
                                int maxResourcesToCreate,
                                int& numResourcesCreated,
                                MpResource* resourceArray[])
   {
      MprVoiceActivityNotifier *pDecode = new MprVoiceActivityNotifier(resourceName);

      // Return created resource
      assert(maxResourcesToCreate >= 1);
      numResourcesCreated = 1;
      resourceArray[0] = pDecode;
      resourceArray[0]->enable();

      return OS_SUCCESS;
   }

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Disabled copy constructor
   MprVoiceActivityNotifierConstructor(const MprVoiceActivityNotifierConstructor& rMprVoiceActivityNotifierConstructor);

     /// Disabled assignment operator
   MprVoiceActivityNotifierConstructor& operator=(const MprVoiceActivityNotifierConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprVoiceActivityNotifierConstructor_h_
