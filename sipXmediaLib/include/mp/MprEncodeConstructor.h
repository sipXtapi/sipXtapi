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

#ifndef _MprEncodeConstructor_h_
#define _MprEncodeConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MprEncode.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief MprEncodeConstructor is used to construct a Decode resource
*/
class MprEncodeConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

      /// Constructor
   MprEncodeConstructor()
   : MpAudioResourceConstructor(DEFAULT_ENCODE_RESOURCE_TYPE,
                                1, 1, //minInputs, maxInputs,
                                1, 1) //minOutputs, maxOutputs
   {
   }

     /// Destructor
   virtual ~MprEncodeConstructor() {}

/* ============================ MANIPULATORS ============================== */

     /// Create a new resource
   virtual OsStatus newResource(const UtlString& resourceName,
                                int maxResourcesToCreate,
                                int& numResourcesCreated,
                                MpResource* resourceArray[])
   {
      MprEncode *pEncode = new MprEncode(resourceName);

      // Return created resource
      assert(maxResourcesToCreate >= 1);
      numResourcesCreated = 1;
      resourceArray[0] = pEncode;
      resourceArray[0]->disable();

      return OS_SUCCESS;
   }

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Disabled copy constructor
   MprEncodeConstructor(const MprEncodeConstructor& rMprEncodeConstructor);

     /// Disabled assignment operator
   MprEncodeConstructor& operator=(const MprEncodeConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprEncodeConstructor_h_
