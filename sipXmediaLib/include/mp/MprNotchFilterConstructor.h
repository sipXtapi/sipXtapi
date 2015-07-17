//  
// Copyright (C) 2008-2015 SIPfoundry Inc.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#ifndef _MprNotchFilterConstructor_h_
#define _MprNotchFilterConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MprNotchFilter.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief MprNotchFilterConstructor is used to construct VAD resource
*/
class MprNotchFilterConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

      /// Constructor
   MprNotchFilterConstructor()
   : MpAudioResourceConstructor(DEFAULT_NOTCH_FILTER_RESOURCE_TYPE,
                                1, 1, //minInputs, maxInputs,
                                1, 1) //minOutputs, maxOutputs
   {
   }

     /// Destructor
   virtual ~MprNotchFilterConstructor() {}

/* ============================ MANIPULATORS ============================== */

     /// Create a new resource
   virtual OsStatus newResource(const UtlString& resourceName,
                                int maxResourcesToCreate,
                                int& numResourcesCreated,
                                MpResource* resourceArray[])
   {
      MprNotchFilter *pResource = new MprNotchFilter(resourceName);

      // Return created resource
      assert(maxResourcesToCreate >= 1);
      numResourcesCreated = 1;
      resourceArray[0] = pResource;
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
   MprNotchFilterConstructor(const MprNotchFilterConstructor& rMprNotchFilterConstructor);

     /// Disabled assignment operator
   MprNotchFilterConstructor& operator=(const MprNotchFilterConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprNotchFilterConstructor_h_
