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
class MpOss;

class MpOssContainer : protected UtlHashMap
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
///@name Creators
//@{

   MpOssContainer();

   ~MpOssContainer();
//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
     /// @brief Create new wrapper for name haven't founded in container. 
   MpOss* getOSSDeviceWrapper(const UtlString& ossdev);

     /// @brief Excluding wrapper from container.
   void excludeFromContainer(MpOss* pDev);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

     /// @brief Creating wrapper and return if it wasn't create yet.
   static
   MpOssContainer* getContainer();

     /// @brief Releasing wrapper if it no longer needed (no references).
   static
   void releaseContainer(MpOssContainer* pCont);

     /// @brief Excluding wrapper from container (static version).
   static
   UtlBoolean excludeWrapperFromContainer(MpOss* pDev);

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   static MpOssContainer* mpCont; ///< Pointer to allocated class
   static int refCount; ///< Number of all MpOSSDeviceWrapperContainer members

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsMutex mMutex;      ///< Internal synchronization mutex
};

/* ============================ INLINE METHODS ============================ */

#endif // _MpOSSDeviceWrapperContainer_h_
