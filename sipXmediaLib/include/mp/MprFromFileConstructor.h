//  
// Copyright (C) 2006-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#ifndef _MprFromFileConstructor_h_
#define _MprFromFileConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MprFromFile.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief MprFromFileConstructor is used to construct a FromFile resource.
*/
class MprFromFileConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

      /// Constructor
   MprFromFileConstructor()
   : MpAudioResourceConstructor(DEFAULT_FROM_FILE_RESOURCE_TYPE,
                                0, 1, //minInputs, maxInputs,
                                1, 1) //minOutputs, maxOutputs
   {
   }

     /// Destructor
   virtual ~MprFromFileConstructor() {}

/* ============================ MANIPULATORS ============================== */

     /// Create a new resource
   virtual OsStatus newResource(const UtlString& resourceName,
                                int maxResourcesToCreate,
                                int& numResourcesCreated,
                                MpResource* resourceArray[])
   {
      assert(maxResourcesToCreate >= 1);
      numResourcesCreated = 1;
      resourceArray[0] = new MprFromFile(resourceName);
      resourceArray[0]->enable();
      return(OS_SUCCESS);
   }

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Disabled copy constructor
   MprFromFileConstructor(const MprFromFileConstructor& rMprFromFileConstructor);

     /// Disabled assignment operator
   MprFromFileConstructor& operator=(const MprFromFileConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprFromFileConstructor_h_
