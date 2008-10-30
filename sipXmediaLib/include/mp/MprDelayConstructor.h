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

#ifndef _MprDelayConstructor_h_
#define _MprDelayConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MprDelay.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief MprDelayConstructor is used to construct VAD resource
*/
class MprDelayConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

      /// Constructor
   MprDelayConstructor()
   : MpAudioResourceConstructor(DEFAULT_DELAY_RESOURCE_TYPE,
                                1, 1, //minInputs, maxInputs,
                                1, 1) //minOutputs, maxOutputs
   {
   }

     /// Destructor
   virtual ~MprDelayConstructor() {}

/* ============================ MANIPULATORS ============================== */

     /// Create a new resource
   virtual OsStatus newResource(const UtlString& resourceName,
                                int maxResourcesToCreate,
                                int& numResourcesCreated,
                                MpResource* resourceArray[])
   {
      MprDelay *pVadResource = new MprDelay(resourceName);

      // Return created resource
      assert(maxResourcesToCreate >= 1);
      numResourcesCreated = 1;
      resourceArray[0] = pVadResource;
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
   MprDelayConstructor(const MprDelayConstructor& rMprDelayConstructor);

     /// Disabled assignment operator
   MprDelayConstructor& operator=(const MprDelayConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprDelayConstructor_h_
