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

#ifndef _MprDecodeConstructor_h_
#define _MprDecodeConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MprDecode.h>
#include <mp/MprDejitter.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief MprDecodeConstructor is used to construct a Decode resource
*/
class MprDecodeConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

      /// Constructor
   MprDecodeConstructor()
   : MpAudioResourceConstructor(DEFAULT_DECODE_RESOURCE_TYPE,
                                1, 1, //minInputs, maxInputs,
                                1, 1) //minOutputs, maxOutputs
   {
   }

     /// Destructor
   virtual ~MprDecodeConstructor() {}

/* ============================ MANIPULATORS ============================== */

     /// Create a new resource
   virtual OsStatus newResource(const UtlString& resourceName,
                                int maxResourcesToCreate,
                                int& numResourcesCreated,
                                MpResource* resourceArray[])
   {
      MprDecode *pDecode = new MprDecode(resourceName);

      // Attach dejitter
      MprDejitter *pDejitter = new MprDejitter();
      pDecode->setMyDejitter(pDejitter, TRUE);

      // Return created resource
      assert(maxResourcesToCreate >= 1);
      numResourcesCreated = 1;
      resourceArray[0] = pDecode;
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
   MprDecodeConstructor(const MprDecodeConstructor& rMprDecodeConstructor);

     /// Disabled assignment operator
   MprDecodeConstructor& operator=(const MprDecodeConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprDecodeConstructor_h_
