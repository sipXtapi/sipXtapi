//
// Copyright (C) 2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#ifndef _MpOSSDeviceWrapperContainer_h_
#define _MpOSSDeviceWrapperContainer_h_


// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "utl/UtlHashMap.h"
#include "os/OsMutex.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpOSSDeviceWrapper;

class MpOSSDeviceWrapperContainer : protected UtlHashMap
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
///@name Creators
//@{

   MpOSSDeviceWrapperContainer();

   ~MpOSSDeviceWrapperContainer();
//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
     /// @brief Create new wrapper for name haven't founded in container. 
   MpOSSDeviceWrapper* getOSSDeviceWrapper(const UtlString& ossdev);

     /// @brief Excluding wrapper from container.
   void excludeFromContainer(MpOSSDeviceWrapper* pDev);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

     /// @brief Creating wrapper and return if it wasn't create yet.
   static
   MpOSSDeviceWrapperContainer* getContainer();

     /// @brief Releasing wrapper if it no longer needed (no references).
   static
   void releaseContainer(MpOSSDeviceWrapperContainer* pCont);

     /// @brief Excluding wrapper from container (static version).
   static
   UtlBoolean excludeWrapperFromContainer(MpOSSDeviceWrapper* pDev);

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   static MpOSSDeviceWrapperContainer* mpCont; ///< Pointer to allocated class
   static int refCount; ///< Number of all MpOSSDeviceWrapperContainer members

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsMutex mMutex;      ///< Internal syncronization mutex
};

/* ============================ INLINE METHODS ============================ */

#endif // _MpOSSDeviceWrapperContainer_h_
